#ifndef ROOT_THttpConnection
#define ROOT_THttpConnection

#include "THashTable.h"
#include "THashList.h"
#include "TNamed.h"
#include "TUrl.h"

class TDatime;
class THttpDocPart;
class THttpReply;
class TSocket;

class THttpSocketPtr {
public:

   class THttpSocket {
   public:
      THttpSocket(const TUrl& url): fSocket(0), fReferences(0), fUrl(url)
      { ReOpen(); }
      ~THttpSocket();
      void ReOpen();
      operator const TSocket*() const { return fSocket; }
      TSocket* GetSocket() { return fSocket; }
      TUrl& GetUrl() { return fUrl; }

      void Inc() { ++fReferences; }
      void Dec();

   private:
      UInt_t   fReferences; //! references to this socket
      TSocket *fSocket; // the actual socket
      TUrl     fUrl; // socket's URL

      friend class THttpSocketPtr;
   };

public:
   THttpSocketPtr(THttpSocket* sock = 0): fSock(sock) {
      if (fSock) fSock->Inc();
   }
   THttpSocketPtr(const THttpSocketPtr& p): fSock(p.fSock){
      if (fSock) fSock->Inc();
   }
   THttpSocketPtr& operator=(const THttpSocketPtr& rhs) {
   // Assignment op.
   // Reduce ref count for original socket, increase for rhs.
      if (fSock) fSock->Dec();
      fSock = rhs.fSock;
      if (fSock) ++fSock->fReferences;
      return *this;
   }

   ~THttpSocketPtr() {
      if (fSock) fSock->Dec();
   }
   THttpSocket* operator->() const { return fSock; }
   THttpSocket& operator *() const { return *fSock; }
   operator bool() const { return (fSock != 0); }

   Bool_t Create(const TUrl&);

private:
   THttpSocket* fSock;
};


class THttpRequest: public TNamed {
public:
   class TContentRange {
   public:
      TContentRange(Long64_t from = -1, Long64_t to = -1):
         fFrom(from), fTo(to) {}
      TContentRange(const TContentRange& other):
         fFrom(other.fFrom), fTo(other.fTo) {}

      Long64_t From() const { return fFrom; }
      Long64_t To()   const { return fTo; }
      Long64_t Len()  const { return fTo - fFrom; }

      void SetFrom(Long64_t from) { fFrom = from; }
      void SetTo(Long64_t to)     { fTo = to; }

      void Reset() { fFrom = -1; }
      operator bool() const { return fFrom != -1; }
   private:
      Long64_t fFrom;
      Long64_t fTo;
   };

   THttpRequest(const char* uri = "", const THttpRequest* reuse = 0):
      TNamed(uri, "HTTP Request"), fUrl(uri), fHttpVersion(1.0)
   {
      if (reuse) fSocket = reuse->GetSocket();
      fUserAgent = Form("ROOT-THttpRequest{%d}", Class_Version());
   }
   THttpRequest(const TUrl& url, const THttpRequest* reuse = 0):
      fUrl(url), fHttpVersion(1.0)
   {
      SetNameTitle(fUrl.GetUrl(), "HTTP Request");
      if (reuse) fSocket = reuse->GetSocket();
      fUserAgent = Form("ROOT-THttpRequest{%d}", Class_Version());
   }
   virtual ~THttpRequest() {}

   void SetHttpVersion(Float_t version) { fHttpVersion = version; }
   Float_t GetHttpVersion() const { return fHttpVersion; }
   const TUrl& GetUrl() const { return fUrl; }

   void SetRange(const TContentRange& range) { fContentRange = range; }
   const TContentRange& GetRange() const { return fContentRange; }

   THttpReply* Get();
   THttpReply* Put();

   void SetUserAgent(const char* agent = 0) { fUserAgent = agent; }
   const char* GetUserAgent() const { return fUserAgent; }

protected:
   void BuildRequest(TString& req) const;
   THttpSocketPtr GetSocket() const { return fSocket; }
   Bool_t CheckSocket();

private:
   TUrl           fUrl;         // The URL used for the request
   Float_t        fHttpVersion; // 1.0 or 1.1 for now
   TContentRange  fContentRange;// content range to be requested
   TString        fUserAgent;   // user agent identifier
   THttpSocketPtr fSocket;      // socket to use

   friend class THttpReply;

   ClassDef(THttpRequest, 1); // Class representing a HTTP request
};


class THttpStatus: public TObject {
public:


   // HTTP 1.1 status codes, see http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
   enum ECode {
      k__UNINITIALIZED__,

      kContinue = 100,
      kSwitchingProtocols,
      k__LAST__100 = kSwitchingProtocols,

      kOK = 200,
      kCreated,
      kAccepted,
      kNonAuthoritativeInformation,
      kNoContent,
      kResetContent,
      kPartialContent,
      k__LAST__200 = kPartialContent,

      kMultipleChoices = 300,
      kMovedPermanently,
      kFound,
      kSeeOther,
      kNotModified,
      kUseProxy,
      k__UNUSED__,
      kTemporaryRedirect,
      k__LAST__300 = kTemporaryRedirect,
      
      kBadRequest = 400,
      kUnauthorized,
      kPaymentRequired,
      kForbidden,
      kNotFound,
      kMethodNotAllowed,
      kNotAcceptable,
      kProxyAuthenticationRequired,
      kRequestTimeout,
      kConflict,
      kGone,
      kLengthRequired,
      kPreconditionFailed,
      kRequestEntityTooLarge,
      kRequestURITooLong,
      kUnsupportedMediaType,
      kRequestedRangeNotSatisfiable,
      kExpectationFailed,
      k__LAST__400 = kExpectationFailed,

      kInternalServerError = 500,
      kNotImplemented,
      kBadGateway,
      kServiceUnavailable,
      kGatewayTimeout,
      kHTTPVersionNotSupported,
      k__LAST__500 = kHTTPVersionNotSupported
   };

   // HTTP 1.1 status code ranges, to be calculated as EStatusCode / 100
   enum ECodeClass {
      kInformational = 1,
      kSuccessful = 2,
      kRedirection = 3,
      kClientError = 4,
      kServerError = 5
   };

   THttpStatus(ECode code = k__UNINITIALIZED__): fCode(code) {}

   ECode       GetCode() const { return fCode; }
   ECodeClass  GetCodeClass() const { return (ECodeClass) (((int)fCode) / 100); }

   const char* GetName() const { return fgNames[Linearize(fCode)]; }
   const char* GetDescription() const { return fgDescriptions[Linearize(fCode)]; }

   const char* GetCodeClassName() const { return fgCodeClassNames[Linearize(GetCodeClass())]; }
   const char* GetCodeClassDescription() const { return fgCodeClassDescriptions[Linearize(GetCodeClass())]; }

protected:
   static Int_t Linearize(ECode code);
   static Int_t Linearize(ECodeClass cclass) { return ((int) cclass) - 1; }

private:
   ECode fCode;

   static const char* fgNames[41];
   static const char* fgDescriptions[41];
   static const char* fgCodeClassNames[5];
   static const char* fgCodeClassDescriptions[5];

   ClassDef(THttpStatus, 1); // HTTP status code
};


class THttpHeader: public TNamed {
public:
   THttpHeader(const char* name, const char* value): TNamed(name, value) {};
   virtual ~THttpHeader() {}

   const TString& GetString() const { return fTitle; }
   Bool_t GetDatime(TDatime& datime) const;
   Bool_t GetNumber(Double_t& value) const {
      value = fTitle.Atof(); return fTitle.IsFloat() || fTitle.IsHex(); }
   Bool_t GetNumber(Long64_t& value) const {
      value = fTitle.Atoll(); return fTitle.IsFloat() || fTitle.IsHex(); }

   ClassDef(THttpHeader, 1); // Represents a HTTP header with name and value
};

class THttpDocPart: public TObject {
public:
   THttpDocPart(const char* url = ""): fName(url)
   { fHeaders.SetOwner(); }

   const char* GetName() const { return fName; }

   const Char_t* GetContent() const { return fContent; }
   Bool_t GetLine(TString& s, Ssiz_t& pos) const;

   const THttpHeader* GetHttpHeader(const char* key) const {
      TString sKey(key);
      sKey.ToUpper();
      return (const THttpHeader*) fHeaders.FindObject(sKey);
   }
   const TCollection* GetHttpHeaders() const { return &fHeaders; }
   const TCollection* GetParts() const { return fMultiPartContent; }

   Long64_t GetSize() const;

protected:
   void SetName(const char* name) { fName = name; }
   Int_t ParsePart(THttpSocketPtr socket, THttpStatus* status = 0);
   Int_t ReadContent(THttpSocketPtr socket, 
      const char* boundary, Long64_t contentsize = -1);
   Int_t RecvLine(THttpSocketPtr socket, TString& line) const;

private:
   TString    fName; // URL or boundary of this document
   THashTable fHeaders; // A collection of HTTP headers, each header is a THttpHeader
   TString    fContent; // content of the reply
   THashList *fMultiPartContent; // content for muliparts (THttpDocParts)

   ClassDef(THttpDocPart, 1); // Represents a HTTP document (part)
};



class THttpReply: public THttpDocPart {
public:

   THttpReply(const THttpRequest* req = 0, const char* action = 0):
   fRequest(req), fAction(action)
   { 
      if (req) fUrl = req->GetUrl();
      SetName(fUrl.GetUrl());
      ReadReply();
   }

   const THttpRequest* GetRequest() const { return fRequest; }
   TUrl& GetUrl() { return fUrl; }

protected:
   Int_t ReadReply();

private:
   THttpStatus         fStatus;  // Status of the request / reply
   const THttpRequest *fRequest; // The HTTP request used to get this reply
   TString             fAction;  // Action used to trigger the reply
   TUrl                fUrl; // URL of this reply

   ClassDef(THttpReply, 1); // Class representing a HTTP reply to a THttpRequest
};

#endif // ROOT_THttpConnection
