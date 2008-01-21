#include "THttpConnection.h"

#include "TDatime.h"
#include "THashList.h"
#include "TSocket.h"

Bool_t THttpHeader::GetDatime(TDatime& datime) const
{
   TDatime old;
   old.Set();
   datime.Set();
   datime.Set(GetString());
   return old != datime;
}

THttpSocketPtr::THttpSocket::~THttpSocket()
{
   // destructor
   delete fSocket;
}

void THttpSocketPtr::THttpSocket::Dec()
{
   // Decrease the ref count, and if 0 delete the socket
   --fReferences;
   if (!fReferences) {
      delete fSocket;
      fSocket = 0;
   }
}

void THttpSocketPtr::THttpSocket::ReOpen()
{
   // Create a socket for url
   delete fSocket;
   fSocket = new TSocket(fUrl.GetHost(), fUrl.GetPort());
}

Bool_t THttpSocketPtr::Create(const TUrl& url)
{
   // Create the socket
   if (fSock) return kFALSE;
   fSock = new THttpSocket(url);
   ++fSock->fReferences;
   return kTRUE;
}


Bool_t THttpRequest::CheckSocket() {
   if (!fSocket || strcmp(fSocket->GetUrl().GetUrl(), fUrl.GetUrl())) {
      // no need to delete; copy c'tor will take care of it.
      fSocket.Create(fUrl);
      if (!fSocket->GetSocket()->IsValid()) {
         Error("CheckSocket", "Cannot connect to remote host %s:%d", fUrl.GetHost(),fUrl.GetPort());
         return kFALSE;
      }
   }

   return kTRUE;
}

void THttpRequest::BuildRequest(TString& req) const
{
   req += fUrl.GetProtocol();
   req += "://";
   req += fUrl.GetHost();
   req += ":";
   req += fUrl.GetPort();
   req += "/";
   req += fUrl.GetFile();
   if (fHttpVersion >= 1.1)
      req += " HTTP/1.1";
   else
      req += " HTTP/1.0";
   req += "\r\n";
   if (fHttpVersion >= 1.1) {
      req += "Host: ";
      req += fUrl.GetHost();
      req += "\r\n";
   }
   //req += GetUserAgent();
   req += "\r\n";
   if (fContentRange) {
      req += "Range: bytes=";
      req += fContentRange.From();
      req += "-";
      req += fContentRange.To();
      req += "\r\n";
   }
   req += "\r\n";
}

THttpReply* THttpRequest::Get()
{
   // Trigger a server reply using the HTTP GET method and return the server's reply.
   // It is the caller's responsibility to delete the returned object.

   if (!CheckSocket()) return kFALSE;
   TString msg("GET ");
   BuildRequest(msg);
   if (fSocket->GetSocket()->SendRaw(msg.Data(), msg.Length()) == -1) {
      Error("Get", "Cannot send command to remote host %s", fUrl.GetHost());
      return 0;
   }

   return new THttpReply(this, "GET");
}


THttpReply* THttpRequest::Put()
{
   // Trigger a server reply using the HTTP PUT method and return the server's reply.
   // It is the caller's responsibility to delete the returned object.
   if (!CheckSocket()) return kFALSE;
   TString msg("PUT ");
   BuildRequest(msg);
   if (fSocket->GetSocket()->SendRaw(msg.Data(), msg.Length()) == -1) {
      Error("Put", "Cannot send command to remote host %s", fUrl.GetHost());
      return 0;
   }

   return new THttpReply(this, "PUT");
}

Int_t THttpDocPart::ParsePart(THttpSocketPtr socket, THttpStatus* status /*= 0*/)
{
   // Read the server's reply headers and content

   TString line;
   TString boundary;

   Long64_t recvd_total = 0;
   Long64_t recvd = 0;
   Long64_t contentlength = -1;

   // empty line signals "end of header"
   while ((recvd = RecvLine(socket, line)) > 0) {

      recvd_total += recvd;

      if (gDebug > 0)
         Info("ReadHeaders", "header: %s", line.Data());

      if (status && line.BeginsWith("HTTP/1.")) {
         TString scode = line(9, 3);
         *status = (THttpStatus::ECode) scode.Atoi();
         if (status->GetCodeClass() != THttpStatus::kSuccessful) {
            TString mess = line(13, 1000);
            Error("ReadHeaders", "%s: %s (%d)\n%d", fName.Data(), mess.Data(), status->GetCode(), status->GetDescription());
            return -1;
         }
      } else {
         Ssiz_t posColumn = line.Index(':');
         if (posColumn != kNPOS) {
            TString hdr(line(0, posColumn));
            TString val(line(posColumn + 1, line.Length()));
            hdr.ToUpper();
            val = val.Strip(TString::kBoth);
            fHeaders.Add(new THttpHeader(hdr, val));

            if (hdr == "CONTENT-TYPE" && val.BeginsWith("multipart", TString::kIgnoreCase)) {
               boundary = "--" + val(val.Index("boundary=") + 9, line.Length());
            } 
         } else if (gDebug > 0)
            Info("ReadHeaders", "Ignoring apparent header %s", line.Data());
      }
   } // receive the headers

   if (recvd < 0) {
      Error("ReadHeaders", "Error receiving headers for %s", fName.Data());
      return -2;
   }

   if (boundary.Length()) {
      // we have a multipart document; skip eveything before boundary:
      while ((recvd = RecvLine(socket, line)) > 0 && !line.BeginsWith(boundary))
         recvd_total += recvd;

      if (recvd < 0) {
         Error("ReadHeaders", "Cannot find leading multi-part boundary for %s", fName.Data());
         return -3;
      }

      // now parse the content - which is a regular part
      if (!fMultiPartContent) fMultiPartContent = new THashList();

      THttpDocPart* mpContent = new THttpDocPart(fName + boundary);
      fMultiPartContent->AddLast(mpContent);
      if (mpContent->ParsePart(socket) < 0) {
         Error("ReadHeaders", "Cannot parse multi-part headers for %s", fName.Data());
         return -4;
      }
   }

   if (gDebug > 0)
      Info("ReadHeaders", "got all headers");

   Int_t ncont = ReadContent(socket, boundary, GetSize());
   if (ncont < 0) return ncont;
   return ncont + recvd_total > 0 ? 1 : 0;
}


Int_t THttpDocPart::ReadContent(THttpSocketPtr socket, const char* boundary, Long64_t contentsize /* = -1 */)
{
   // Read the server's reply content block.
   // If contentsize is >=0, exactly contentsize bytes will be read (or an error issued).
   // Otherwise we are assumed to be in a text/plain part, and read until either the end of
   // stream or until the boundary is found.
   // boundary is meant to be a multipart boundary, e.g. --NextBoundary.
   // A RFC1341 multipart document is e.g.
   //   MIME-Version: 1.0 
   //   Content-Type: multipart/alternative; boundary=boundary42 
   //   
   //   
   //   --boundary42 
   //   Content-Type: text/plain; charset=us-ascii 
   //   
   //   ...plain text version of message goes here.... 
   //   
   //   --boundary42 
   //   Content-Type: text/richtext 
   //   
   //   .... richtext version of same message goes here ... 
   //   --boundary42 
   //   Content-Type: text/x-whatever 
   //   
   //   .... fanciest formatted version of same  message  goes  here 
   //   ... 
   //   --boundary42-- 
   // We will return when boundary is found, which also means that the caller needs
   // to check for a possible trailing (i.e. non-read) "--" signaling the end of the
   // multipart document.

   Int_t recvd_total = 0;

   if (contentsize >= 0)
      fContent.Resize((Ssiz_t)contentsize + 1);
   Ssiz_t bufSize = fContent.Sizeof() - 1;
   Long64_t toread = contentsize;
   if (contentsize < 0) toread = 1; // read byte by byte

   Int_t matchingBoundaryPos = -1;
   Int_t boundaryLen = -1;
   if (boundary) boundaryLen = strlen(boundary);

   while (true) {
      if (bufSize < recvd_total + toread) {
         bufSize = recvd_total + 1;
         if (toread < 1024)
            bufSize += 1024;
         else bufSize += toread;
         fContent.Resize((Ssiz_t)(bufSize - 1));
      }
      Int_t recvd = socket->GetSocket()->RecvRaw((char*)(fContent.Data() + recvd_total), (Int_t)toread);
      recvd_total += recvd;

      if (recvd == -1) {
         Error("ReadReply", "Error receiving data from remote host %s", fName.Data());
         return -1;
      } else if (recvd != toread) {
         // we're done, even if multipart: no more to be read means done!
         return recvd_total;
      }

      if (contentsize >= 0)
         return 1;

      if (boundary && boundaryLen) {
         // we are in a content-type = text/plain part without content length,
         // and we are looking for a boundary. We will simply search for
         // newline + boundary:
         if (matchingBoundaryPos == -1 && fContent[recvd_total] == '\n') {
            matchingBoundaryPos = recvd_total;
         } else {
            if (fContent[recvd_total] != boundary[recvd_total - matchingBoundaryPos])
               // not matching
               matchingBoundaryPos = -1;
            else if (boundaryLen == recvd_total - matchingBoundaryPos)
               // the boundary matches
               return recvd_total;
         }
      }
   }
}

Int_t THttpReply::ReadReply()
{
   // Read the server's reply of a GET or PUT request.   
   THttpSocketPtr socket = fRequest->GetSocket();

   Int_t nrecvd = ParsePart(socket, &fStatus);
   if (nrecvd == -1 && fRequest->GetHttpVersion() >= 1.1) {
      if (gDebug > 0)
         Info("ReadReply", "HTTP/1.1 socket closed, now reopened. Request again, please!");
      socket->ReOpen();
      return -42;
   }
   return nrecvd;
}


//______________________________________________________________________________
Int_t THttpDocPart::RecvLine(THttpSocketPtr socket, TString& line) const
{
   // Read a line from the socket. Reads at most one less than the number of
   // characters specified by size. Reading stops when a newline character
   // is found, The newline (\n) and cr (\r), if any, are removed.
   // Returns -1 in case of error, or the number of characters read (>= 0)
   // otherwise.

   char c;
   Int_t err = 0;
   line = "";
   while ((err = socket->GetSocket()->RecvRaw(&c, 1)) >= 0) {
      if (c == '\n' || err == 0) {
         if (line[line.Length() - 1] == '\r')
            line.Remove(line.Length() - 1);
         break;
      }
      line += c;
   }
   if (err < 0) {
      //if (fRequest->GetHttpVersion() < 1.1 || gDebug > 0)
      //   Error("RecvLine", "error receiving data from remote host %s", fUrl.GetHost());
      return -1;
   }
   return line.Length();
}


const char* THttpStatus::fgNames[41] = {
   "Continue",
   "Switching Protocols",
   "OK",
   "Created",
   "Accepted",
   "Non-Authoritative Information",
   "No Content",
   "Reset Content",
   "Partial Content",
   "Multiple Choices",
   "Moved Permanently",
   "Found",
   "See Other",
   "Not Modified",
   "Use Proxy",
   "(Unused)",
   "Temporary Redirect",
   "Bad Request",
   "Unauthorized",
   "Payment Required",
   "Forbidden",
   "Not Found",
   "Method Not Allowed",
   "Not Acceptable",
   "Proxy Authentication Required",
   "Request Timeout",
   "Conflict",
   "Gone",
   "Length Required",
   "Precondition Failed",
   "Request Entity Too Large",
   "Request-URI Too Long",
   "Unsupported Media Type",
   "Requested Range Not Satisfiable",
   "Expectation Failed",
   "Internal Server Error",
   "Not Implemented",
   "Bad Gateway",
   "Service Unavailable",
   "Gateway Timeout",
   "HTTP Version Not Supported"
};

#ifndef __CINT__
const char* THttpStatus::fgDescriptions[41] = {
   /* 100*/
   "The client SHOULD continue with its request. This interim response is used to inform the client that the "
   "initial part of the request has been received and has not yet been rejected by the server. The client "
   "SHOULD continue by sending the remainder of the request or, if the request has already been completed, "
   "ignore this response. The server MUST send a final response after the request has been completed. See "
   "section 8.2.3 for detailed discussion of the use and handling of this status code.",
   /* 101 */
   "The server understands and is willing to comply with the client's request, via the Upgrade message header "
   "field (section 14.42), for a change in the application protocol being used on this connection. The server "
   "will switch protocols to those defined by the response's Upgrade header field immediately after the empty "
   "line which terminates the 101 response.\n"
   "The protocol SHOULD be switched only when it is advantageous to do so. For example, switching to a newer "
   "version of HTTP is advantageous over older versions, and switching to a real-time, synchronous protocol "
   "might be advantageous when delivering resources that use such features.",
   /* 200 */
   "The request has succeeded. The information returned with the response is dependent on the method used in "
   "the request, for example:\n"
   "GET an entity corresponding to the requested resource is sent in the response;\n"
   "HEAD the entity-header fields corresponding to the requested resource are sent in the response without any "
   "message-body;\n"
   "POST an entity describing or containing the result of the action;\n"
   "TRACE an entity containing the request message as received by the end server.",

   /* 201 */
   "The request has been fulfilled and resulted in a new resource being created. The newly created resource "
   "can be referenced by the URI(s) returned in the entity of the response, with the most specific URI for "
   "the resource given by a Location header field. The response SHOULD include an entity containing a list of "
   "resource characteristics and location(s) from which the user or user agent can choose the one most "
   "appropriate. The entity format is specified by the media type given in the Content-Type header field. The "
   "origin server MUST create the resource before returning the 201 status code. If the action cannot be "
   "carried out immediately, the server SHOULD respond with 202 (Accepted) response instead.\n"
   "A 201 response MAY contain an ETag response header field indicating the current value of the entity tag "
   "for the requested variant just created, see section 14.19.",

   /* 202 */
   "The request has been accepted for processing, but the processing has not been completed. The request "
   "might or might not eventually be acted upon, as it might be disallowed when processing actually takes "
   "place. There is no facility for re-sending a status code from an asynchronous operation such as this.\n"
   "The 202 response is intentionally non-committal. Its purpose is to allow a server to accept a request "
   "for some other process (perhaps a batch-oriented process that is only run once per day) without "
   "requiring that the user agent's connection to the server persist until the process is completed. The "
   "entity returned with this response SHOULD include an indication of the request's current status and "
   "either a pointer to a status monitor or some estimate of when the user can expect the request to be "
   "fulfilled.",
   /* 203 */
   "The returned metainformation in the entity-header is not the definitive set as available from the "
   "origin server, but is gathered from a local or a third-party copy. The set presented MAY be a subset "
   "or superset of the original version. For example, including local annotation information about the "
   "resource might result in a superset of the metainformation known by the origin server. Use of this "
   "response code is not required and is only appropriate when the response would otherwise be 200 (OK).",
   /* 204 */
   "The server has fulfilled the request but does not need to return an entity-body, and might want to "
   "return updated metainformation. The response MAY include new or updated metainformation in the form "
   "of entity-headers, which if present SHOULD be associated with the requested variant.\n"
   "If the client is a user agent, it SHOULD NOT change its document view from that which caused the "
   "request to be sent. This response is primarily intended to allow input for actions to take place "
   "without causing a change to the user agent's active document view, although any new or updated "
   "metainformation SHOULD be applied to the document currently in the user agent's active view.\n"
   "The 204 response MUST NOT include a message-body, and thus is always terminated by the first empty "
   "line after the header fields.",
   /* 205 */
   "The server has fulfilled the request and the user agent SHOULD reset the document view which caused "
   "the request to be sent. This response is primarily intended to allow input for actions to take place "
   "via user input, followed by a clearing of the form in which the input is given so that the user can "
   "easily initiate another input action. The response MUST NOT include an entity.",
   /* 206 */
   "The server has fulfilled the partial GET request for the resource. The request MUST have included a "
   "Range header field (section 14.35) indicating the desired range, and MAY have included an If-Range "
   "header field (section 14.27) to make the request conditional.\n"
   "The response MUST include the following header fields:\n"
   "      - Either a Content-Range header field (section 14.16) indicating the range included with this "
   "response, or a multipart/byteranges Content-Type including Content-Range fields for each part. If a "
   "Content-Length header field is present in the response, its value MUST match the actual number of "
   "OCTETs transmitted in the message-body.\n"
   "      - Date\n"
   "      - ETag and/or Content-Location, if the header would have been sent in a 200 response to the "
   "same request\n"
   "      - Expires, Cache-Control, and/or Vary, if the field-value might differ from that sent in any "
   "previous response for the same variant\n"
   "If the 206 response is the result of an If-Range request that used a strong cache validator (see "
   "section 13.3.3), the response SHOULD NOT include other entity-headers. If the response is the result "
   "of an If-Range request that used a weak validator, the response MUST NOT include other entity-headers; "
   "this prevents inconsistencies between cached entity-bodies and updated headers. Otherwise, the response "
   "MUST include all of the entity-headers that would have been returned with a 200 (OK) response to the same "
   "request.\n"
   "A cache MUST NOT combine a 206 response with other previously cached content if the ETag or Last-Modified "
   "headers do not match exactly, see 13.5.4.\n"
   "A cache that does not support the Range and Content-Range headers MUST NOT cache 206 (Partial) responses.",
   /* 300 */
   "The requested resource corresponds to any one of a set of representations, each with its own specific "
   "location, and agent- driven negotiation information (section 12) is being provided so that the user (or "
   "user agent) can select a preferred representation and redirect its request to that location.\n"
   "Unless it was a HEAD request, the response SHOULD include an entity containing a list of resource "
   "characteristics and location(s) from which the user or user agent can choose the one most appropriate. "
   "The entity format is specified by the media type given in the Content- Type header field. Depending upon "
   "the format and the capabilities of the user agent, selection of the most appropriate choice MAY be performed "
   "automatically. However, this specification does not define any standard for such automatic selection.\n"
   "If the server has a preferred choice of representation, it SHOULD include the specific URI for that "
   "representation in the Location field; user agents MAY use the Location field value for automatic redirection. "
   "This response is cacheable unless indicated otherwise.",
   /* 301 */
   "The requested resource has been assigned a new permanent URI and any future references to this resource "
   "SHOULD use one of the returned URIs. Clients with link editing capabilities ought to automatically re-link "
   "references to the Request-URI to one or more of the new references returned by the server, where possible. "
   "This response is cacheable unless indicated otherwise.\n"
   "The new permanent URI SHOULD be given by the Location field in the response. Unless the request method was "
   "HEAD, the entity of the response SHOULD contain a short hypertext note with a hyperlink to the new URI(s).\n"
   "If the 301 status code is received in response to a request other than GET or HEAD, the user agent MUST NOT "
   "automatically redirect the request unless it can be confirmed by the user, since this might change the conditions "
   "under which the request was issued.\n"
   "      Note: When automatically redirecting a POST request after receiving a 301 status code, some existing "
   "HTTP/1.0 user agents will erroneously change it into a GET request.",
   /* 302 */
   "The requested resource resides temporarily under a different URI. Since the redirection might be altered "
   "on occasion, the client SHOULD continue to use the Request-URI for future requests. This response is only "
   "cacheable if indicated by a Cache-Control or Expires header field.\n"
   "The temporary URI SHOULD be given by the Location field in the response. Unless the request method was HEAD, "
   "the entity of the response SHOULD contain a short hypertext note with a hyperlink to the new URI(s).\n"
   "If the 302 status code is received in response to a request other than GET or HEAD, the user agent MUST "
   "NOT automatically redirect the request unless it can be confirmed by the user, since this might change the "
   "conditions under which the request was issued.\n"
   "      Note: RFC 1945 and RFC 2068 specify that the client is not allowed to change the method on the redirected "
   "request.  However, most existing user agent implementations treat 302 as if it were a 303 response, performing a "
   "GET on the Location field-value regardless of the original request method. The status codes 303 and 307 have been "
   "added for servers that wish to make unambiguously clear which kind of reaction is expected of the client.",
   /* 303 */
   "The response to the request can be found under a different URI and SHOULD be retrieved using a GET method on "
   "that resource. This method exists primarily to allow the output of a POST-activated script to redirect the "
   "user agent to a selected resource. The new URI is not a substitute reference for the originally requested "
   "resource. The 303 response MUST NOT be cached, but the response to the second (redirected) request might be "
   "cacheable.\n"
   "The different URI SHOULD be given by the Location field in the response. Unless the request method was HEAD, "
   "the entity of the response SHOULD contain a short hypertext note with a hyperlink to the new URI(s).\n"
   "      Note: Many pre-HTTP/1.1 user agents do not understand the 303 status. When interoperability with such "
   "clients is a concern, the 302 status code may be used instead, since most user agents react to a 302 response "
   "as described here for 303.",
   /* 304 */
   "If the client has performed a conditional GET request and access is allowed, but the document has not been "
   "modified, the server SHOULD respond with this status code. The 304 response MUST NOT contain a message-body, "
   "and thus is always terminated by the first empty line after the header fields.\n"
   "The response MUST include the following header fields:\n"
   "      - Date, unless its omission is required by section 14.18.1\n"
   "If a clockless origin server obeys these rules, and proxies and clients add their own Date to any response "
   "received without one (as already specified by [RFC 2068], section 14.19), caches will operate correctly.\n"
   "      - ETag and/or Content-Location, if the header would have been sent in a 200 response to the same request\n"
   "      - Expires, Cache-Control, and/or Vary, if the field-value might differ from that sent in any previous "
   "response for the same variant\n"
   "If the conditional GET used a strong cache validator (see section 13.3.3), the response SHOULD NOT include "
   "other entity-headers. Otherwise (i.e., the conditional GET used a weak validator), the response MUST NOT "
   "include other entity-headers; this prevents inconsistencies between cached entity-bodies and updated headers.\n"
   "If a 304 response indicates an entity not currently cached, then the cache MUST disregard the response "
   "and repeat the request without the conditional.\n"
   "If a cache uses a received 304 response to update a cache entry, the cache MUST update the entry to reflect "
   "any new field values given in the response.",
   /* 305 */
   "The requested resource MUST be accessed through the proxy given by the Location field. The Location field "
   "gives the URI of the proxy. The recipient is expected to repeat this single request via the proxy. 305 "
   "responses MUST only be generated by origin servers.\n"
   "      Note: RFC 2068 was not clear that 305 was intended to redirect a single request, and to be generated "
   "by origin servers only.  Not observing these limitations has significant security consequences.",
   /* 306 */
   "The 306 status code was used in a previous version of the specification, is no longer used, and the code is "
   "reserved.",
   /* 307 */
   "The requested resource resides temporarily under a different URI. Since the redirection MAY be altered on "
   "occasion, the client SHOULD continue to use the Request-URI for future requests. This response is only "
   "cacheable if indicated by a Cache-Control or Expires header field.\n"
   "The temporary URI SHOULD be given by the Location field in the response. Unless the request method was HEAD, "
   "the entity of the response SHOULD contain a short hypertext note with a hyperlink to the new URI(s), since "
   "many pre-HTTP/1.1 user agents do not understand the 307 status. Therefore, the note SHOULD contain the "
   "information necessary for a user to repeat the original request on the new URI.\n"
   "If the 307 status code is received in response to a request other than GET or HEAD, the user agent MUST "
   "NOT automatically redirect the request unless it can be confirmed by the user, since this might change "
   "the conditions under which the request was issued.",

   /* 400 */
   "The request could not be understood by the server due to malformed syntax. The client SHOULD NOT repeat "
   "the request without modifications.",
   /* 401 */
   "The request requires user authentication. The response MUST include a WWW-Authenticate header field (section "
   "14.47) containing a challenge applicable to the requested resource. The client MAY repeat the request with a "
   "suitable Authorization header field (section 14.8). If the request already included Authorization credentials, "
   "then the 401 response indicates that authorization has been refused for those credentials. If the 401 response "
   "contains the same challenge as the prior response, and the user agent has already attempted authentication at "
   "least once, then the user SHOULD be presented the entity that was given in the response, since that entity might "
   "include relevant diagnostic information. HTTP access authentication is explained in \"HTTP Authentication: Basic "
   "and Digest Access Authentication\" [43].",
   /* 402 */
   "This code is reserved for future use.",
   /* 403 */
   "The server understood the request, but is refusing to fulfill it. Authorization will not help and the request "
   "SHOULD NOT be repeated. If the request method was not HEAD and the server wishes to make public why the request "
   "has not been fulfilled, it SHOULD describe the reason for the refusal in the entity. If the server does not wish "
   "to make this information available to the client, the status code 404 (Not Found) can be used instead.",
   /* 404 */
   "The server has not found anything matching the Request-URI. No indication is given of whether the condition is "
   "temporary or permanent. The 410 (Gone) status code SHOULD be used if the server knows, through some internally "
   "configurable mechanism, that an old resource is permanently unavailable and has no forwarding address. This status "
   "code is commonly used when the server does not wish to reveal exactly why the request has been refused, or when no "
   "other response is applicable.",
   /* 405 */
   "The method specified in the Request-Line is not allowed for the resource identified by the Request-URI. The "
   "response MUST include an Allow header containing a list of valid methods for the requested resource.",
   /* 406 */
   "The resource identified by the request is only capable of generating response entities which have content "
   "characteristics not acceptable according to the accept headers sent in the request.\n"
   "Unless it was a HEAD request, the response SHOULD include an entity containing a list of available entity "
   "characteristics and location(s) from which the user or user agent can choose the one most appropriate. The "
   "entity format is specified by the media type given in the Content-Type header field. Depending upon the "
   "format and the capabilities of the user agent, selection of the most appropriate choice MAY be performed "
   "automatically. However, this specification does not define any standard for such automatic selection.\n"
   "      Note: HTTP/1.1 servers are allowed to return responses which are not acceptable according to the "
   "accept headers sent in the request. In some cases, this may even be preferable to sending a 406 response. "
   "User agents are encouraged to inspect the headers of an incoming response to determine if it is acceptable.\n"
   "If the response could be unacceptable, a user agent SHOULD temporarily stop receipt of more data and query "
   "the user for a decision on further actions.",
   /* 407 */
   "This code is similar to 401 (Unauthorized), but indicates that the client must first authenticate itself "
   "with the proxy. The proxy MUST return a Proxy-Authenticate header field (section 14.33) containing a "
   "challenge applicable to the proxy for the requested resource. The client MAY repeat the request with a "
   "suitable Proxy-Authorization header field (section 14.34). HTTP access authentication is explained in "
   "\"HTTP Authentication: Basic and Digest Access Authentication\" [43].",
   /* 408 */
   "The client did not produce a request within the time that the server was prepared to wait. The client "
   "MAY repeat the request without modifications at any later time.",
   /* 409 */
   "The request could not be completed due to a conflict with the current state of the resource. This code "
   "is only allowed in situations where it is expected that the user might be able to resolve the conflict "
   "and resubmit the request. The response body SHOULD include enough information for the user to recognize "
   "the source of the conflict. Ideally, the response entity would include enough information for the user or "
   "user agent to fix the problem; however, that might not be possible and is not required.\n"
   "Conflicts are most likely to occur in response to a PUT request. For example, if versioning were being "
   "used and the entity being PUT included changes to a resource which conflict with those made by an earlier "
   "(third-party) request, the server might use the 409 response to indicate that it can't complete the "
   "request. In this case, the response entity would likely contain a list of the differences between the "
   "two versions in a format defined by the response Content-Type.",
   /* 410 */
   "The requested resource is no longer available at the server and no forwarding address is known. This "
   "condition is expected to be considered permanent. Clients with link editing capabilities SHOULD delete "
   "references to the Request-URI after user approval. If the server does not know, or has no facility to "
   "determine, whether or not the condition is permanent, the status code 404 (Not Found) SHOULD be used "
   "instead. This response is cacheable unless indicated otherwise.\n"
   "The 410 response is primarily intended to assist the task of web maintenance by notifying the recipient "
   "that the resource is intentionally unavailable and that the server owners desire that remote links to "
   "that resource be removed. Such an event is common for limited-time, promotional services and for "
   "resources belonging to individuals no longer working at the server's site. It is not necessary to mark "
   "all permanently unavailable resources as \"gone\" or to keep the mark for any length of time -- that is "
   "left to the discretion of the server owner.",
   /* 411 */
   "The server refuses to accept the request without a defined Content- Length. The client MAY repeat the "
   "request if it adds a valid Content-Length header field containing the length of the message-body in the "
   "request message.",
   /* 412 */
   "The precondition given in one or more of the request-header fields evaluated to false when it was "
   "tested on the server. This response code allows the client to place preconditions on the current "
   "resource metainformation (header field data) and thus prevent the requested method from being applied "
   "to a resource other than the one intended.",
   /* 413 */
   "The server is refusing to process a request because the request entity is larger than the server is "
   "willing or able to process. The server MAY close the connection to prevent the client from continuing "
   "the request.\n"
   "If the condition is temporary, the server SHOULD include a Retry- After header field to indicate that "
   "it is temporary and after what time the client MAY try again.",
   /* 414 */
   "The server is refusing to service the request because the Request-URI is longer than the server is "
   "willing to interpret. This rare condition is only likely to occur when a client has improperly "
   "converted a POST request to a GET request with long query information, when the client has descended "
   "into a URI \"black hole\" of redirection (e.g., a redirected URI prefix that points to a suffix of "
   "itself), or when the server is under attack by a client attempting to exploit security holes present "
   "in some servers using fixed-length buffers for reading or manipulating the Request-URI.",
   /* 415 */
   "The server is refusing to service the request because the entity of the request is in a format not "
   "supported by the requested resource for the requested method."
   /* 416 */
   "A server SHOULD return a response with this status code if a request included a Range request-header "
   "field (section 14.35), and none of the range-specifier values in this field overlap the current extent "
   "of the selected resource, and the request did not include an If-Range request-header field. (For "
   "byte-ranges, this means that the first- byte-pos of all of the byte-range-spec values were greater "
   "than the current length of the selected resource.)\n"
   "When this status code is returned for a byte-range request, the response SHOULD include a "
   "Content-Range entity-header field specifying the current length of the selected resource (see "
   "section 14.16). This response MUST NOT use the multipart/byteranges content- type.",
   /* 417 */
   "The expectation given in an Expect request-header field (see section 14.20) could not be met by "
   "this server, or, if the server is a proxy, the server has unambiguous evidence that the request "
   "could not be met by the next-hop server.",
   /* 500*/
   "The server encountered an unexpected condition which prevented it from fulfilling the request.",
   /* 501 */
   "The server does not support the functionality required to fulfill the request. This is the appropriate "
   "response when the server does not recognize the request method and is not capable of supporting it for "
   "any resource.",
   /* 502 */
   "The server, while acting as a gateway or proxy, received an invalid response from the upstream server "
   "it accessed in attempting to fulfill the request.",
   /* 503 */
   "The server is currently unable to handle the request due to a temporary overloading or maintenance of "
   "the server. The implication is that this is a temporary condition which will be alleviated after some "
   "delay. If known, the length of the delay MAY be indicated in a Retry-After header. If no Retry-After "
   "is given, the client SHOULD handle the response as it would for a 500 response.\n"
   "      Note: The existence of the 503 status code does not imply that a server must use it when becoming "
   "overloaded. Some servers may wish to simply refuse the connection.",
   /* 504 */
   "The server, while acting as a gateway or proxy, did not receive a timely response from the upstream "
   "server specified by the URI (e.g. HTTP, FTP, LDAP) or some other auxiliary server (e.g. DNS) it needed "
   "to access in attempting to complete the request.\n"
   "      Note: Note to implementors: some deployed proxies are known to return 400 or 500 when DNS "
   "lookups time out.",
   /* 505 */
   "The server does not support, or refuses to support, the HTTP protocol version that was used in the "
   "request message. The server is indicating that it is unable or unwilling to complete the request "
   "using the same major version as the client, as described in section 3.1, other than with this error "
   "message. The response SHOULD contain an entity describing why that version is not supported and what "
   "other protocols are supported by that server."
};
#endif

const char* THttpStatus::fgCodeClassNames[5] = {
   "Informational",
   "Successful",
   "Redirection",
   "Client Error",
   "Server Error"
};

#ifndef __CINT__
const char* THttpStatus::fgCodeClassDescriptions[5] = {
   "This class of status code indicates a provisional response, consisting only of the Status-Line and optional headers, and is terminated by an empty line. There are no required headers for this class of status code. Since HTTP/1.0 did not define any 1xx status codes, servers MUST NOT send a 1xx response to an HTTP/1.0 client except under experimental conditions.\n"
   "A client MUST be prepared to accept one or more 1xx status responses prior to a regular response, even if the client does not expect a 100 (Continue) status message. Unexpected 1xx status responses MAY be ignored by a user agent.\n"
   "Proxies MUST forward 1xx responses, unless the connection between the proxy and its client has been closed, or unless the proxy itself requested the generation of the 1xx response. (For example, if a proxy adds a \"Expect: 100-continue\" field when it forwards a request, then it need not forward the corresponding 100 (Continue) response(s).)",

   "This class of status code indicates that the client's request was successfully received, understood, and accepted.",

   "This class of status code indicates that further action needs to be taken by the user agent in order to fulfill the request. The action required MAY be carried out by the user agent without interaction with the user if and only if the method used in the second request is GET or HEAD. A client SHOULD detect infinite redirection loops, since such loops generate network traffic for each redirection.",

   "The 4xx class of status code is intended for cases in which the client seems to have erred. Except when responding to a HEAD request, the server SHOULD include an entity containing an explanation of the error situation, and whether it is a temporary or permanent condition. These status codes are applicable to any request method. User agents SHOULD display any included entity to the user.\n"
   "If the client is sending data, a server implementation using TCP SHOULD be careful to ensure that the client acknowledges receipt of the packet(s) containing the response, before the server closes the input connection. If the client continues sending data to the server after the close, the server's TCP stack will send a reset packet to the client, which may erase the client's unacknowledged input buffers before they can be read and interpreted by the HTTP application.",

   "Response status codes beginning with the digit \"5\" indicate cases in which the server is aware that it has erred or is incapable of performing the request. Except when responding to a HEAD request, the server SHOULD include an entity containing an explanation of the error situation, and whether it is a temporary or permanent condition. User agents SHOULD display any included entity to the user. These response codes are applicable to any request method."
};
#endif

Int_t THttpStatus::Linearize(ECode code)
{
   // return a linearized index corresponding to code.
   switch (((int)code)/100) {
      case 0: return -1;
      case 1: return code - 100;
      case 2: return code - 200 + (k__LAST__100 - 100);
      case 3: return code - 300 + (k__LAST__100 - 100) + (k__LAST__200 - 200);
      case 4: return code - 400 + (k__LAST__100 - 100) + (k__LAST__200 - 200) + (k__LAST__300 - 300);
      case 5: return code - 500 + (k__LAST__100 - 100) + (k__LAST__200 - 200) + (k__LAST__300 - 300) + (k__LAST__400 - 400);
   };
   return -2;
}

Long64_t THttpDocPart::GetSize() const
{
   // Return the size of the document
   Long64_t size = -1;

   const THttpHeader* hdr = GetHttpHeader("Content-Range");
   if (hdr) {
      Long64_t first, last;
#ifdef R__WIN32
      sscanf(hdr->GetString().Data(), "bytes %I64d-%I64d/%I64d", &first, &last, &size);
#else
      sscanf(hdr->GetString().Data(), "bytes %lld-%lld/%lld", &first, &last, &size);
#endif
   } else {
      hdr = GetHttpHeader("Content-Length");
      if (hdr)
         if (!hdr->GetNumber(size)) return -1;
   }

   return size;
}

Bool_t THttpDocPart::GetLine(TString& s, Ssiz_t& pos) const
{
   // Iterate through the content like so:
   //    TString line;
   //    Ssiz_t pos = 0;
   //    while (reply->GetLine(line, pos))
   //       cout << line << endl;

   Bool_t ret = fContent.Tokenize(s, pos, "\n");
   s.Strip(TString::kBoth, '\r');
   return ret;
}
