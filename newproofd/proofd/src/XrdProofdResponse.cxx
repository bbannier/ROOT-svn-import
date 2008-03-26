// @(#)root/proofd:$Id$
// Author: Gerardo Ganis  12/12/2005

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// XrdProofdResponse                                                    //
//                                                                      //
// Authors: G. Ganis, CERN, 2005                                        //
//                                                                      //
// Utility class to handle replies to clients.                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <string.h>

#include "XrdProofdAux.h"
#include "XrdProofdProtocol.h"
#include "XrdProofdResponse.h"

// Tracing utils
#include "XrdProofdTrace.h"

int XrdProofdResponse::fgMaxRetry = 5; // Max number of retries on send failures

//______________________________________________________________________________
int XrdProofdResponse::Send()
{
   // Auxilliary Send method
   XPDLOC(RSP, "Response::Send")

   if (!fLink || (fLink->FDnum() <= 0)) {
      TRACE(XERR, "send: link is undefined! ");
      return 0;
   }

   int rc = 0;
   XrdOucString tmsg;
   {  XrdSysMutexHelper mh(fMutex);

      fResp.status = static_cast<kXR_unt16>(htons(kXR_ok));
      fResp.dlen   = 0;
      tmsg.form("sending OK");

      // Send over
      rc = LinkSend((char *)&fResp, sizeof(fResp));
    }
   TRACER(this, DBG, tmsg.c_str());
   return rc;
}

//______________________________________________________________________________
int XrdProofdResponse::Send(XResponseType rcode)
{
   // Auxilliary Send method
   XPDLOC(RSP, "Response::Send")

   if (!fLink || (fLink->FDnum() <= 0)) {
      TRACE(XERR, "link is undefined! ");
      return 0;
   }

   int rc = 0;
   XrdOucString tmsg;
   {  XrdSysMutexHelper mh(fMutex);

      fResp.status        = static_cast<kXR_unt16>(htons(rcode));
      fResp.dlen          = 0;
      tmsg.form("sending OK: status = %d", rcode);

      // Send over
      rc = LinkSend((char *)&fResp, sizeof(fResp));
   }
   TRACER(this, DBG, tmsg.c_str());
   return rc;
}

//______________________________________________________________________________
int XrdProofdResponse::Send(const char *msg)
{
   // Auxilliary Send method
   XPDLOC(RSP, "Response::Send")

   if (!fLink || (fLink->FDnum() <= 0)) {
      TRACE(XERR, "link is undefined! ");
      return 0;
   }

   int rc = 0;
   XrdOucString tmsg;
   {  XrdSysMutexHelper mh(fMutex);

      fResp.status        = static_cast<kXR_unt16>(htons(kXR_ok));
      fRespIO[1].iov_base = (caddr_t)msg;
      fRespIO[1].iov_len  = strlen(msg)+1;
      fResp.dlen          = static_cast<kXR_int32>(htonl(fRespIO[1].iov_len));
      tmsg.form("sending OK: %s", msg);

      // Send over
      rc = LinkSend(fRespIO, 2, sizeof(fResp) + fRespIO[1].iov_len);
   }
   TRACER(this, DBG, tmsg.c_str());
   return rc;
}

//______________________________________________________________________________
int XrdProofdResponse::Send(XResponseType rcode, void *data, int dlen)
{
   // Auxilliary Send method
   XPDLOC(RSP, "Response::Send")

   if (!fLink || (fLink->FDnum() <= 0)) {
      TRACE(XERR, "link is undefined! ");
      return 0;
   }

   int rc = 0;
   XrdOucString tmsg;
   {  XrdSysMutexHelper mh(fMutex);

      fResp.status        = static_cast<kXR_unt16>(htons(rcode));
      fRespIO[1].iov_base = (caddr_t)data;
      fRespIO[1].iov_len  = dlen;
      fResp.dlen          = static_cast<kXR_int32>(htonl(dlen));
      tmsg.form("sending %d data bytes; status=%d", dlen, rcode);

      // Send over
      rc = LinkSend(fRespIO, 2, sizeof(fResp) + dlen);
   }
   TRACER(this, DBG, tmsg.c_str());
   return rc;
}

//______________________________________________________________________________
int XrdProofdResponse::Send(XResponseType rcode, int info, char *data)
{
   // Auxilliary Send method
   XPDLOC(RSP, "Response::Send")

   if (!fLink || (fLink->FDnum() <= 0)) {
      TRACE(XERR, "link is undefined! ");
      return 0;
   }

   int rc = 0;
   XrdOucString tmsg;
   {  XrdSysMutexHelper mh(fMutex);

      kXR_int32 xbuf = static_cast<kXR_int32>(htonl(info));
      int dlen = 0;
      int nn = 2;

      fResp.status        = static_cast<kXR_unt16>(htons(rcode));
      fRespIO[1].iov_base = (caddr_t)(&xbuf);
      fRespIO[1].iov_len  = sizeof(xbuf);
      if (data) {
         nn = 3;
         fRespIO[2].iov_base = (caddr_t)data;
         fRespIO[2].iov_len  = dlen = strlen(data);
         tmsg.form("sending %d data bytes; info=%d; status=%d",
                   dlen, info, rcode);
      } else {
         tmsg.form("sending info=%d; status=%d", info, rcode);
      }
      fResp.dlen          = static_cast<kXR_int32>(htonl((dlen+sizeof(xbuf))));

      // Send over
      rc = LinkSend(fRespIO, nn, sizeof(fResp) + dlen);
   }
   TRACER(this, DBG, tmsg.c_str());
   return rc;
}

//______________________________________________________________________________
int XrdProofdResponse::Send(XResponseType rcode, XProofActionCode acode,
                            void *data, int dlen )
{
   // Auxilliary Send method
   XPDLOC(RSP, "Response::Send")

   if (!fLink || (fLink->FDnum() <= 0)) {
      TRACE(XERR, "link is undefined! ");
      return 0;
   }

   int rc = 0;
   XrdOucString tmsg;
   {  XrdSysMutexHelper mh(fMutex);

      kXR_int32 xbuf = static_cast<kXR_int32>(htonl(acode));
      int nn = 2;

      fResp.status        = static_cast<kXR_unt16>(htons(rcode));
      fRespIO[1].iov_base = (caddr_t)(&xbuf);
      fRespIO[1].iov_len  = sizeof(xbuf);
      if (data) {
         nn = 3;
         fRespIO[2].iov_base = (caddr_t)data;
         fRespIO[2].iov_len  = dlen;
         tmsg.form("sending %d data bytes; status=%d; action=%d",
                   dlen, rcode, acode);
      } else {
         tmsg.form("sending status=%d; action=%d", rcode, acode);
      }
      fResp.dlen = static_cast<kXR_int32>(htonl((dlen+sizeof(xbuf))));

      // Send over
      rc = LinkSend(fRespIO, nn, sizeof(fResp) + dlen);
   }
   TRACER(this, DBG, tmsg.c_str());
   return rc;
}

//______________________________________________________________________________
int XrdProofdResponse::Send(XResponseType rcode, XProofActionCode acode,
                            kXR_int32 cid, void *data, int dlen )
{
   // Auxilliary Send method
   XPDLOC(RSP, "Response::Send")

   if (!fLink || (fLink->FDnum() <= 0)) {
      TRACE(XERR, "link is undefined! ");
      return 0;
   }

   int rc = 0;
   XrdOucString tmsg;
   {  XrdSysMutexHelper mh(fMutex);

      kXR_int32 xbuf = static_cast<kXR_int32>(htonl(acode));
      kXR_int32 xcid = static_cast<kXR_int32>(htonl(cid));
      int hlen = sizeof(xbuf) + sizeof(xcid);
      int nn = 3;

      fResp.status        = static_cast<kXR_unt16>(htons(rcode));
      fRespIO[1].iov_base = (caddr_t)(&xbuf);
      fRespIO[1].iov_len  = sizeof(xbuf);
      fRespIO[2].iov_base = (caddr_t)(&xcid);
      fRespIO[2].iov_len  = sizeof(xcid);
      if (data) {
         nn = 4;
         fRespIO[3].iov_base = (caddr_t)data;
         fRespIO[3].iov_len  = dlen;
         tmsg.form("sending %d data bytes; status=%d; action=%d; cid=%d",
                   dlen, rcode, acode, cid);
      } else {
         tmsg.form("sending status=%d; action=%d; cid=%d", rcode, acode, cid);
      }
      fResp.dlen = static_cast<kXR_int32>(htonl((dlen+hlen)));

      // Send over
      rc = LinkSend(fRespIO, nn, sizeof(fResp) + dlen);
   }
   TRACER(this, DBG, tmsg.c_str());
   return rc;
}

//______________________________________________________________________________
int XrdProofdResponse::Send(XResponseType rcode, XProofActionCode acode,
                            int info )
{
   // Auxilliary Send method
   XPDLOC(RSP, "Response::Send")

   if (!fLink || (fLink->FDnum() <= 0)) {
      TRACE(XERR, "link is undefined! ");
      return 0;
   }

   int rc = 0;
   XrdOucString tmsg;
   {  XrdSysMutexHelper mh(fMutex);

      kXR_int32 xbuf = static_cast<kXR_int32>(htonl(acode));
      kXR_int32 xinf = static_cast<kXR_int32>(htonl(info));
      int hlen = sizeof(xbuf) + sizeof(xinf);

      fResp.status        = static_cast<kXR_unt16>(htons(rcode));
      fRespIO[1].iov_base = (caddr_t)(&xbuf);
      fRespIO[1].iov_len  = sizeof(xbuf);
      fRespIO[2].iov_base = (caddr_t)(&xinf);
      fRespIO[2].iov_len  = sizeof(xinf);
      tmsg.form("sending info=%d; status=%d; action=%d", info, rcode, acode);
      fResp.dlen = static_cast<kXR_int32>(htonl((hlen)));

      // Send over
      rc = LinkSend(fRespIO, 3, sizeof(fResp));
   }
   TRACER(this, DBG, tmsg.c_str());
   return rc;
}

//______________________________________________________________________________
int XrdProofdResponse::SendI(kXR_int32 int1, kXR_int16 int2, kXR_int16 int3,
                            void *data, int dlen )
{
   // Auxilliary Send method
   XPDLOC(RSP, "Response::SendI")

   if (!fLink || (fLink->FDnum() <= 0)) {
      TRACE(XERR, "link is undefined! ");
      return 0;
   }

   int rc = 0;
   XrdOucString tmsg;
   {  XrdSysMutexHelper mh(fMutex);

      kXR_int32 i1 = static_cast<kXR_int32>(htonl(int1));
      kXR_int16 i2 = static_cast<kXR_int16>(htons(int2));
      kXR_int16 i3 = static_cast<kXR_int16>(htons(int3));
      int ilen = sizeof(i1) + sizeof(i2) + sizeof(i3);
      int nn = 4;

      fResp.status        = static_cast<kXR_unt16>(htons(kXR_ok));
      fRespIO[1].iov_base = (caddr_t)(&i1);
      fRespIO[1].iov_len  = sizeof(i1);
      fRespIO[2].iov_base = (caddr_t)(&i2);
      fRespIO[2].iov_len  = sizeof(i2);
      fRespIO[3].iov_base = (caddr_t)(&i3);
      fRespIO[3].iov_len  = sizeof(i3);
      if (data) {
         nn = 5;
         fRespIO[4].iov_base = (caddr_t)data;
         fRespIO[4].iov_len  = dlen;
         tmsg.form("sending %d data bytes; int1=%d; int2=%d; int3=%d",
                    dlen, int1, int2, int3);
      } else {
         tmsg.form("sending int1=%d; int2=%d; int3=%d", int1, int2, int3);
      }
      fResp.dlen = static_cast<kXR_int32>(htonl((dlen+ilen)));

      // Send over
      rc = LinkSend(fRespIO, nn, sizeof(fResp) + dlen);
   }
   TRACER(this, DBG, tmsg.c_str());
   return rc;
}

//______________________________________________________________________________
int XrdProofdResponse::SendI(kXR_int32 int1, kXR_int32 int2, void *data, int dlen )
{
   // Auxilliary Send method
   XPDLOC(RSP, "Response::SendI")

   if (!fLink || (fLink->FDnum() <= 0)) {
      TRACE(XERR, "link is undefined! ");
      return 0;
   }

   int rc = 0;
   XrdOucString tmsg;
   {  XrdSysMutexHelper mh(fMutex);

      kXR_int32 i1 = static_cast<kXR_int32>(htonl(int1));
      kXR_int32 i2 = static_cast<kXR_int32>(htonl(int2));
      int ilen = sizeof(i1) + sizeof(i2);
      int nn = 3;

      fResp.status        = static_cast<kXR_unt16>(htons(kXR_ok));
      fRespIO[1].iov_base = (caddr_t)(&i1);
      fRespIO[1].iov_len  = sizeof(i1);
      fRespIO[2].iov_base = (caddr_t)(&i2);
      fRespIO[2].iov_len  = sizeof(i2);
      if (data) {
         nn = 4;
         fRespIO[3].iov_base = (caddr_t)data;
         fRespIO[3].iov_len  = dlen;
         tmsg.form("sending %d data bytes; int1=%d; int2=%d",
                   dlen, int1, int2);
      } else {
         tmsg.form("sending int1=%d; int2=%d", int1, int2);
      }
      fResp.dlen = static_cast<kXR_int32>(htonl((dlen+ilen)));

      // Send over
      rc = LinkSend(fRespIO, nn, sizeof(fResp) + dlen);
   }
   TRACER(this, DBG, tmsg.c_str());
   return rc;
}

//______________________________________________________________________________
int XrdProofdResponse::SendI(kXR_int32 int1, void *data, int dlen )
{
   // Auxilliary Send method
   XPDLOC(RSP, "Response::SendI")

   if (!fLink || (fLink->FDnum() <= 0)) {
      TRACE(XERR, "link is undefined! ");
      return 0;
   }

   int rc = 0;
   XrdOucString tmsg;
   {  XrdSysMutexHelper mh(fMutex);

      kXR_int32 i1 = static_cast<kXR_int32>(htonl(int1));
      int ilen = sizeof(i1);
      int nn = 2;

      fResp.status        = static_cast<kXR_unt16>(htons(kXR_ok));
      fRespIO[1].iov_base = (caddr_t)(&i1);
      fRespIO[1].iov_len  = sizeof(i1);
      if (data) {
         nn = 3;
         fRespIO[2].iov_base = (caddr_t)data;
         fRespIO[2].iov_len  = dlen;
         tmsg.form("sending %d data bytes; int1=%d", dlen, int1);
      } else {
         tmsg.form("sending int1=%d", int1);
      }
      fResp.dlen          = static_cast<kXR_int32>(htonl((dlen+ilen)));

      // Send over
      rc = LinkSend(fRespIO, nn, sizeof(fResp) + dlen);
   }
   TRACER(this, DBG, tmsg.c_str());
   return rc;
}

//______________________________________________________________________________
int XrdProofdResponse::Send(void *data, int dlen)
{
   // Auxilliary Send method
   XPDLOC(RSP, "Response::Send")

   if (!fLink || (fLink->FDnum() <= 0)) {
      TRACE(XERR, "link is undefined! ");
      return 0;
   }

   int rc = 0;
   XrdOucString tmsg;
   {  XrdSysMutexHelper mh(fMutex);

      fResp.status        = static_cast<kXR_unt16>(htons(kXR_ok));
      fRespIO[1].iov_base = (caddr_t)data;
      fRespIO[1].iov_len  = dlen;
      fResp.dlen          = static_cast<kXR_int32>(htonl(dlen));
      tmsg.form("sending %d data bytes; status=0", dlen);

      // Send over
      rc = LinkSend(fRespIO, 2, sizeof(fResp) + dlen);
   }
   TRACER(this, DBG, tmsg.c_str());
   return rc;
}

//______________________________________________________________________________
int XrdProofdResponse::Send(struct iovec *IOResp, int iornum, int iolen)
{
   // Auxilliary Send method
   XPDLOC(RSP, "Response::Send")

   if (!fLink || (fLink->FDnum() <= 0)) {
      TRACE(XERR, "link is undefined! ");
      return 0;
   }

   int rc = 0;
   XrdOucString tmsg;
   {  XrdSysMutexHelper mh(fMutex);

      int i, dlen = 0;
      if (iolen < 0) {
         for (i = 1; i < iornum; i++)
            dlen += IOResp[i].iov_len;
      } else {
         dlen = iolen;
      }

      fResp.status        = static_cast<kXR_unt16>(htons(kXR_ok));
      IOResp[0].iov_base = fRespIO[0].iov_base;
      IOResp[0].iov_len  = fRespIO[0].iov_len;
      fResp.dlen          = static_cast<kXR_int32>(htonl(dlen));
      tmsg.form("sending %d data bytes; status=0", dlen);

      // Send over
      rc = LinkSend(fRespIO, iornum, sizeof(fResp) + dlen);
   }
   TRACER(this, DBG, tmsg.c_str());
   return rc;
}

//______________________________________________________________________________
int XrdProofdResponse::Send(XErrorCode ecode, const char *msg)
{
   // Auxilliary Send method
   XPDLOC(RSP, "Response::Send")

   if (!fLink || (fLink->FDnum() <= 0)) {
      TRACE(XERR, "link is undefined! ");
      return 0;
   }

   int rc = 0;
   XrdOucString tmsg;
   {  XrdSysMutexHelper mh(fMutex);

      int dlen;
      kXR_int32 erc = static_cast<kXR_int32>(htonl(ecode));

      fResp.status        = static_cast<kXR_unt16>(htons(kXR_error));
      fRespIO[1].iov_base = (char *)&erc;
      fRespIO[1].iov_len  = sizeof(erc);
      fRespIO[2].iov_base = (caddr_t)msg;
      fRespIO[2].iov_len  = strlen(msg)+1;
      dlen   = sizeof(erc) + fRespIO[2].iov_len;
      fResp.dlen          = static_cast<kXR_int32>(htonl(dlen));
      tmsg.form("sending err %d: %s", ecode, msg);

      // Send over
      rc = LinkSend(fRespIO, 3, sizeof(fResp) + dlen);
   }
   TRACER(this, DBG, tmsg.c_str());
   return rc;
}

//______________________________________________________________________________
int XrdProofdResponse::Send(XPErrorCode ecode, const char *msg)
{
   // Auxilliary Send method
   XPDLOC(RSP, "Response::Send")

   if (!fLink || (fLink->FDnum() <= 0)) {
      TRACE(XERR, "link is undefined! ");
      return 0;
   }

   int rc = 0;
   XrdOucString tmsg;
   {  XrdSysMutexHelper mh(fMutex);

      int dlen;
      kXR_int32 erc = static_cast<kXR_int32>(htonl(ecode));

      fResp.status        = static_cast<kXR_unt16>(htons(kXR_error));
      fRespIO[1].iov_base = (char *)&erc;
      fRespIO[1].iov_len  = sizeof(erc);
      fRespIO[2].iov_base = (caddr_t)msg;
      fRespIO[2].iov_len  = strlen(msg)+1;
      dlen   = sizeof(erc) + fRespIO[2].iov_len;
      fResp.dlen          = static_cast<kXR_int32>(htonl(dlen));
      tmsg.form("sending err %d: %s", ecode, msg);

      // Send over
      rc = LinkSend(fRespIO, 3, sizeof(fResp) + dlen);
   }
   TRACER(this, DBG, tmsg.c_str());
   return rc;
}

//______________________________________________________________________________
int XrdProofdResponse::LinkSend(const char *buff, int len)
{
   // Method actually sending the buffer(s) over the link.
   // Sending is automatically re-tried in case of cancellation.
   // Return 0 on success, -1 on failure.
   XPDLOC(RSP, "Response::LinkSend")

   // Number of attempts
   int ntry = fgMaxRetry;

   int rc = 0;
   while (ntry--) {
      if ((rc = fLink->Send(buff, len)) >= 0) {
         // Success
         break;
      } else {
         // Keep retrying ...
         TRACER(this, DBG, "problems sending "<<len<<" bytes; retrying")
      }
   }
   // Done
   return ((rc < 0) ? fLink->setEtext("send failure") : 0);
}

//______________________________________________________________________________
int XrdProofdResponse::LinkSend(const struct iovec *iov, int iocnt, int len)
{
   // Method actually sending the buffer(s) over the link.
   // Sending is automatically re-tried in case of cancellation.
   // Return 0 on success, -1 on failure.
   XPDLOC(RSP, "Response::LinkSend")

   // Number of attempts
   int ntry = fgMaxRetry;

   int rc = 0;
   while (ntry--) {
      if ((rc = fLink->Send(iov, iocnt, len)) >= 0) {
         // Success
         break;
      } else {
         // Keep retrying ...
         TRACER(this, DBG, "problems sending "<<len<<" bytes ("<<
                           strerror(errno)<<"); retrying")
      }
   }
   // Done
   return ((rc < 0) ? fLink->setEtext("send failure") : 0);
}

//______________________________________________________________________________
void XrdProofdResponse::Set(unsigned char *stream)
{
   // Auxilliary Set method

   XrdSysMutexHelper mh(fMutex);

   fResp.streamid[0] = stream[0];
   fResp.streamid[1] = stream[1];

   SetTrsid();
}

//______________________________________________________________________________
void XrdProofdResponse::Set(unsigned short sid)
{
   // Auxilliary Set method

   unsigned char stream[2];

   {  XrdSysMutexHelper mh(fMutex);

      memcpy((void *)&stream[0], (const void *)&sid, sizeof(sid));

      fResp.streamid[0] = stream[0];
      fResp.streamid[1] = stream[1];
   }
   SetTrsid();
}

//______________________________________________________________________________
void XrdProofdResponse::GetSID(unsigned short &sid)
{
   // Get stream ID (to be able to restore it later

   {  XrdSysMutexHelper mh(fMutex);
      memcpy((void *)&sid, (void *)&fResp.streamid[0], sizeof(sid));
   }
}

//______________________________________________________________________________
void XrdProofdResponse::Set(XrdLink *l)
{
   // Set the link to be used by this response

   {  XrdSysMutexHelper mh(fMutex);
      fLink = l;
   }
   GetSID(fSID);
}

//______________________________________________________________________________
void XrdProofdResponse::SetTraceID()
{
   // Auxilliary set method
   XPDLOC(RSP, "Response::SetTraceID")

   {  XrdSysMutexHelper mh(fMutex);
      if (fLink && fTag.length() > 0) {
         fTraceID.form("%s%s: %s: ", fTrsid, fLink->ID, fTag.c_str());
      } else if (fLink) {
         fTraceID.form("%s%s: ", fTrsid, fLink->ID);
      } else if (fTag.length() > 0) {
         fTraceID.form("%s%s: ", fTrsid, fTag.c_str());
      } else {
         fTraceID.form("%s: ", fTrsid);
      }
   }

   TRACE(DBG,"trace set to '"<<fTraceID<<"'")
}

//______________________________________________________________________________
void XrdProofdResponse::SetTrsid()
{
   // Auxilliary Set method

   static char hv[] = "0123456789abcdef";

   int i;
   char *outbuff = fTrsid;
   for (i = 0; i < (int)sizeof(fResp.streamid); i++) {
      *outbuff++ = hv[(fResp.streamid[i] >> 4) & 0x0f];
      *outbuff++ = hv[ fResp.streamid[i]       & 0x0f];
   }
   *outbuff++ = ' ';
   *outbuff = '\0';
}
