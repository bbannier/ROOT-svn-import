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
// XrdProofdProtocol                                                    //
//                                                                      //
// Authors: G. Ganis, CERN, 2005                                        //
//                                                                      //
// XrdProtocol implementation to coordinate 'proofserv' applications.   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "XrdProofdPlatform.h"

#ifdef OLDXRDOUC
#  include "XrdOuc/XrdOucError.hh"
#  include "XrdOuc/XrdOucLogger.hh"
#else
#  include "XrdSys/XrdSysError.hh"
#  include "XrdSys/XrdSysLogger.hh"
#endif

#include "XrdVersion.hh"
#include "Xrd/XrdBuffer.hh"
#include "XrdNet/XrdNetDNS.hh"
#include "XrdOuc/XrdOucReqID.hh"

#include "XrdProofdClient.h"
#include "XrdProofdClientMgr.h"
#include "XrdProofdManager.h"
#include "XrdProofdPriorityMgr.h"
#include "XrdProofdProofServMgr.h"
#include "XrdProofdProtocol.h"
#include "XrdProofdResponse.h"
#include "XrdProofServProxy.h"

// Tracing utils
#include "XrdProofdTrace.h"
XrdOucTrace          *XrdProofdTrace = 0;
static const char    *gTraceID = "";

// Static variables
static XrdOucReqID   *XrdProofdReqID = 0;

// Loggers: we need two to avoid deadlocks
static XrdSysLogger   gMainLogger;

//
// Static area: general protocol managing section
int                   XrdProofdProtocol::fgCount    = 0;
XrdObjectQ<XrdProofdProtocol>
                      XrdProofdProtocol::fgProtStack("ProtStack",
                                                     "xproofd protocol anchor");
XrdBuffManager       *XrdProofdProtocol::fgBPool    = 0;
int                   XrdProofdProtocol::fgMaxBuffsz= 0;
XrdSysError           XrdProofdProtocol::fgEDest(0, "xpd");
XrdSysLogger          XrdProofdProtocol::fgMainLogger;

//
// Static area: protocol configuration section
bool                  XrdProofdProtocol::fgConfigDone = 0;
//
int                   XrdProofdProtocol::fgReadWait = 0;
// Cluster manager
XrdProofdManager     *XrdProofdProtocol::fgMgr = 0;
// Communication to priority manager
int                   XrdProofdProtocol::fgPriorityMgrFd = -1;
// Communication to proofserv manager
int                   XrdProofdProtocol::fgProofServMgrFd = -1;

// Local definitions
#define MAX_ARGS 128
#define TRACEID gTraceID

// Macros used to set conditional options
#ifndef XPDCOND
#define XPDCOND(n,ns) ((n == -1 && ns == -1) || (n > 0 && n >= ns))
#endif
#ifndef XPDSETSTRING
#define XPDSETSTRING(n,ns,c,s) \
 { if (XPDCOND(n,ns)) { \
     SafeFree(c); c = strdup(s.c_str()); ns = n; }}
#endif

#ifndef XPDADOPTSTRING
#define XPDADOPTSTRING(n,ns,c,s) \
  { char *t = 0; \
    XPDSETSTRING(n, ns, t, s); \
    if (t && strlen(t)) { \
       SafeFree(c); c = t; \
  } else \
       SafeFree(t); }
#endif

#ifndef XPDSETINT
#define XPDSETINT(n,ns,i,s) \
 { if (XPDCOND(n,ns)) { \
     i = strtol(s.c_str(),0,10); ns = n; }}
#endif

typedef struct {
   kXR_int32 ptyp;  // must be always 0 !
   kXR_int32 rlen;
   kXR_int32 pval;
   kXR_int32 styp;
} hs_response_t;

extern "C" {
//_________________________________________________________________________________
XrdProtocol *XrdgetProtocol(const char *, char *parms, XrdProtocol_Config *pi)
{
   // This protocol is meant to live in a shared library. The interface below is
   // used by the server to obtain a copy of the protocol object that can be used
   // to decide whether or not a link is talking a particular protocol.

   // Return the protocol object to be used if static init succeeds
   if (XrdProofdProtocol::Configure(parms, pi)) {

      // Issue herald
      char msg[256];
      sprintf(msg,"xproofd: protocol V %s successfully loaded", XPROOFD_VERSION);
      pi->eDest->Say(0, msg);

      return (XrdProtocol *) new XrdProofdProtocol();
   }
   return (XrdProtocol *)0;
}

//_________________________________________________________________________________
int XrdgetProtocolPort(const char * /*pname*/, char * /*parms*/, XrdProtocol_Config *pi)
{
      // This function is called early on to determine the port we need to use. The
      // The default is ostensibly 1093 but can be overidden; which we allow.

      // Default XPD_DEF_PORT (1093)
      int port = (pi && pi->Port > 0) ? pi->Port : XPD_DEF_PORT;
      return port;
}}

//__________________________________________________________________________________
XrdProofdProtocol::XrdProofdProtocol()
   : XrdProtocol("xproofd protocol handler"), fProtLink(this)
{
   // Protocol constructor
   fLink = 0;
   fArgp = 0;
   fPClient = 0;
   fSecClient = 0;
   fAuthProt = 0;
   fResponses.reserve(10);

   // Instantiate a Proofd protocol object
   Reset();
}

//______________________________________________________________________________
XrdProofdResponse *XrdProofdProtocol::Response(kXR_unt16 sid)
{
   // Get response instance corresponding to stream ID 'sid'

   // Atomic
   XrdSysMutexHelper mh(fMutex);

   if (sid > 0)
      if (sid <= fResponses.size())
         if (fResponses[sid-1])
            return fResponses[sid-1];

   return (XrdProofdResponse *)0;
}

//______________________________________________________________________________
XrdProofdResponse *XrdProofdProtocol::GetNewResponse(kXR_unt16 sid)
{
   // Create new response instance for stream ID 'sid'

   // Atomic
   XrdSysMutexHelper mh(fMutex);

   TRACE(DBG,"GetNewResponse: capacity = "<<fResponses.capacity()<<", sid: "<<sid);

   if (sid > 0) {
      if (sid > fResponses.size()) {
         if (sid > fResponses.capacity()) {
            int newsz = (sid < 2 * fResponses.capacity()) ? 2 * fResponses.capacity() : sid+1 ;
            fResponses.reserve(newsz);
            TRACE(DBG,"GetNewResponse: new capacity = "<<fResponses.capacity());
         }
         int nnew = sid - fResponses.size();
         while (nnew--)
            fResponses.push_back(new XrdProofdResponse());
      }
   } else {
      TRACE(DBG,"GetNewResponse: wrong sid: "<<sid);
      return (XrdProofdResponse *)0;
   }

   // Done
   return fResponses[sid-1];
}

//______________________________________________________________________________
XrdProtocol *XrdProofdProtocol::Match(XrdLink *lp)
{
   // Check whether the request matches this protocol

   struct ClientInitHandShake hsdata;
   char  *hsbuff = (char *)&hsdata;

   static hs_response_t hsresp = {0, 0, htonl(XPROOFD_VERSBIN), 0};

   XrdProofdProtocol *xp;
   int dlen;

   // Peek at the first 20 bytes of data
   if ((dlen = lp->Peek(hsbuff,sizeof(hsdata),fgReadWait)) != sizeof(hsdata)) {
      if (dlen <= 0) lp->setEtext("Match: handshake not received");
      return (XrdProtocol *)0;
   }

   // Verify that this is our protocol
   hsdata.third  = ntohl(hsdata.third);
   if (dlen != sizeof(hsdata) ||  hsdata.first || hsdata.second
       || !(hsdata.third == 1) || hsdata.fourth || hsdata.fifth) return 0;

   // Respond to this request with the handshake response
   if (!lp->Send((char *)&hsresp, sizeof(hsresp))) {
      lp->setEtext("Match: handshake failed");
      return (XrdProtocol *)0;
   }

   // We can now read all 20 bytes and discard them (no need to wait for it)
   int len = sizeof(hsdata);
   if (lp->Recv(hsbuff, len) != len) {
      lp->setEtext("Match: reread failed");
      return (XrdProtocol *)0;
   }

   // Get a protocol object off the stack (if none, allocate a new one)
   if (!(xp = fgProtStack.Pop()))
      xp = new XrdProofdProtocol();

   // Bind the protocol to the link and return the protocol
   xp->fLink = lp;
   strcpy(xp->fSecEntity.prot, "host");
   xp->fSecEntity.host = strdup((char *)lp->Host());

   // Dummy data used by 'proofd'
   kXR_int32 dum[2];
   if (xp->GetData("dummy",(char *)&dum[0],sizeof(dum)) != 0) {
      xp->Recycle(0,0,0);
      return (XrdProtocol *)0;
   }

   // We are done
   return (XrdProtocol *)xp;
}

//_____________________________________________________________________________
int XrdProofdProtocol::Stats(char *buff, int blen, int)
{
   // Return statistics info about the protocol.
   // Not really implemented yet: this is a reduced XrdXrootd version.

   static char statfmt[] = "<stats id=\"xproofd\"><num>%ld</num></stats>";

   // If caller wants only size, give it to him
   if (!buff)
      return sizeof(statfmt)+16;

   // We have only one statistic -- number of successful matches
   return snprintf(buff, blen, statfmt, fgCount);
}

//______________________________________________________________________________
void XrdProofdProtocol::Reset()
{
   // Reset static and local vars

   // Init local vars
   fLink      = 0;
   fArgp      = 0;
   fStatus    = 0;
   fClntCapVer = 0;
   fConnType  = kXPD_ClientMaster;
   fSuperUser = 0;
   fPClient   = 0;
   fCID       = -1;
   fAdminPath = "";
   if (fAuthProt) {
      fAuthProt->Delete();
      fAuthProt = 0;
   }
   memset(&fSecEntity, 0, sizeof(fSecEntity));
   fResponses.clear();
   // Magic numbers cut & pasted from Xrootd
   fhcPrev    = 13;
   fhcMax     = 28657;
   fhcNext    = 21;
   fhcNow     = 13;
   fhalfBSize = 0;
}

//______________________________________________________________________________
int XrdProofdProtocol::Configure(char *, XrdProtocol_Config *pi)
{
   // Protocol configuration tool
   // Function: Establish configuration at load time.
   // Output: 1 upon success or 0 otherwise.

   XrdOucString mp;

   // Only once
   if (fgConfigDone)
      return 1;
   fgConfigDone = 1;

   // Copy out the special info we want to use at top level
   fgEDest.logger(&fgMainLogger);
   XrdProofdTrace = new XrdOucTrace(&fgEDest);
   fgBPool        = pi->BPool;
   fgReadWait     = pi->readWait;

   // Debug flag
   TRACESET(XERR, 1);
   if (pi->DebugON)
      XrdProofdTrace->What |= (TRACE_REQ | TRACE_LOGIN | TRACE_FORK | TRACE_DBG);

   // Process the config file for directives meaningful to us
   if (pi->ConfigFN) {
      // Create and Configure the manager
      fgMgr = new XrdProofdManager(pi, &fgEDest);
      if (fgMgr->Config(0))
         return 0;
   }
   // Communication pipe to the priority manager
   fgPriorityMgrFd = (fgMgr && fgMgr->PriorityMgr()) ? fgMgr->PriorityMgr()->WriteFd() : -1;
   mp = "Proofd : Configure: write-pipe to priority manager: ";
   mp += fgPriorityMgrFd;
   fgEDest.Say(0, mp.c_str());

   // Communication pipe to the proofserv manager
   fgProofServMgrFd = (fgMgr && fgMgr->SessionMgr()) ? fgMgr->SessionMgr()->WriteFd() : -1;
   mp = "Proofd : Configure: write-pipe to proofserv manager: ";
   mp += fgProofServMgrFd;
   fgEDest.Say(0, mp.c_str());

   char msgs[256];
   sprintf(msgs,"Proofd : Configure: mgr: %p", &fgMgr);
   fgEDest.Say(0, msgs);

   // Pre-initialize some i/o values
   fgMaxBuffsz = fgBPool->MaxSize();

   // Schedule protocol object cleanup; the maximum number of objects
   // and the max age are taken from XrdXrootdProtocol: this may need
   // some optimization in the future.
   fgProtStack.Set(pi->Sched, XrdProofdTrace, TRACE_MEM);
   fgProtStack.Set((pi->ConnMax/3 ? pi->ConnMax/3 : 30), 60*60);

   // Initialize the request ID generation object
   XrdProofdReqID = new XrdOucReqID((int)fgMgr->Port(), pi->myName,
                                    XrdNetDNS::IPAddr(pi->myAddr));

   // Indicate we configured successfully
   fgEDest.Say(0, "XProofd protocol version " XPROOFD_VERSION
               " build " XrdVERSION " successfully loaded.");

   // Return success
   return 1;
}

//______________________________________________________________________________
int XrdProofdProtocol::Process(XrdLink *)
{
   // Process the information received on the active link.
   // (We ignore the argument here)

   int rc = 0;
   TRACEI(this, REQ, "Process: enter: instance: " << this);

   // Read the next request header
   if ((rc = GetData("request", (char *)&fRequest, sizeof(fRequest))) != 0)
      return rc;
   TRACEI(this, DBG, "Process: after GetData: rc: " << rc);

   // Deserialize the data
   fRequest.header.requestid = ntohs(fRequest.header.requestid);
   fRequest.header.dlen      = ntohl(fRequest.header.dlen);

#if 0
   // We keep the Response instance until we are done
   XrdSysMutexHelper mh(fResponse.fMutex);

   // Set the stream ID for the reply
   fResponse.Set(fRequest.header.streamid);
   fResponse.Set(fLink);
#else
   kXR_unt16 rid;
   memcpy((void *)&rid, (const void *)&(fRequest.header.streamid[0]), 2);
   XrdProofdResponse *response = 0;
   if (!(response = Response(rid))) {
      if (!(response = GetNewResponse(rid))) {
         TRACEI(this, DBG, "Process: could not get Response instance for rid: "<< rid);
         return rc;
      }
      // Set the stream ID for the reply
      response->Set(fRequest.header.streamid);
      response->Set(fLink);
   }
#endif

   unsigned short sid;
   memcpy((void *)&sid, (const void *)&(fRequest.header.streamid[0]), 2);
   TRACEI(this, DBG, "Process: sid: " << sid <<
               ", req: " <<fRequest.header.requestid <<
               ", dlen: " <<fRequest.header.dlen);

   // Every request has an associated data length. It better be >= 0 or we won't
   // be able to know how much data to read.
   if (fRequest.header.dlen < 0) {
      response->Send(kXR_ArgInvalid, "Process: Invalid request data length");
      return fLink->setEtext("Process: protocol data length error");
   }

   // Read any argument data at this point, except when the request is to forward
   // a buffer: the argument may have to be segmented and we're not prepared to do
   // that here.
   if (fRequest.header.requestid != kXP_sendmsg && fRequest.header.dlen) {
      if (GetBuff(fRequest.header.dlen+1) != 1) {
         response->Send(kXR_ArgTooLong, "fRequest.argument is too long");
         return 0;
      }
      if ((rc = GetData("arg", fArgp->buff, fRequest.header.dlen)))
         return rc;
      fArgp->buff[fRequest.header.dlen] = '\0';
   }

   // Continue with request processing at the resume point
   return Process2();
}

//______________________________________________________________________________
int XrdProofdProtocol::Process2()
{
   // Local processing method: here the request is dispatched to the appropriate
   // method

   int rc = 1;
   XPD_SETRESP(this, "Process2");

   TRACEP(this, respid, REQ, "Process2: enter: req id: " << fRequest.header.requestid);


   // If the user is logged in check if the wanted action is to be done by us
   if (fStatus && (fStatus & XPD_LOGGEDIN)) {
      // We must have a client instance if here
      if (!fPClient) {
         TRACEP(this, respid, XERR, "Process2: client undefined!!! ");
         response->Send(kXR_InvalidRequest,"Process2: client undefined!!! ");
         return 1;
      }
      switch(fRequest.header.requestid) {
         case kXP_interrupt:
            return Interrupt();
         case kXP_ping:
            return Ping();
         case kXP_sendmsg:
            {  // This part is serialized at client level
               XrdSysMutexHelper mh(fPClient->Mutex());
               return SendMsg();
            }
         case kXP_urgent:
            return Urgent();
      }
   }

   // The request is for the manager
   return fgMgr->Process(this);
}

//______________________________________________________________________
void XrdProofdProtocol::Recycle(XrdLink *, int, const char *)
{
   // Recycle call. Release the instance and give it back to the stack.

   const char *srvtype[6] = {"ANY", "Worker", "Master",
                             "TopMaster", "Internal", "Admin"};

   // Document the disconnect
   TRACEI(this, REQ,"Recycle: enter: instance: " <<this<<", type: "<<srvtype[fConnType+1]);

   // If we have a buffer, release it
   if (fArgp) {
      fgBPool->Release(fArgp);
      fArgp = 0;
   }

   // Flag for internal connections: those deserve a different treatment
   bool proofsrv = (fConnType == kXPD_Internal) ? 1 : 0;

   // Locate the client instance
   XrdProofdClient *pmgr = fPClient;

   if (pmgr) {

      if (!proofsrv) {

         // Disconnect from attached proofservs
         pmgr->DisconnectFromProofServ(this);

         // Reset the corresponding client slot in the list of this client
         pmgr->ResetClientSlot(this);

         // Remove the client admin path
         XrdOucString cmd;
         XrdProofdAux::Form(cmd, "/bin/rm -fr %s", fAdminPath.c_str());
         TRACEI(this, REQ,"Recycle: executing: '" <<cmd<<"'");
         system(cmd.c_str());

         // Signal the session manager that a client has just gone
         PostClientDisconnection();

      } else {

         // Internal connection: we need to remove this instance from the list
         // of proxy servers and to notify the attached clients.
         XrdProofServProxy *psrv = pmgr->GetServer(this);

         if (psrv) {
            // Tell other attached clients, if any, that this session is gone
            XrdOucString msg("Recycle: session: ");
            msg += psrv->Tag();
            msg += "terminated by peer";
            psrv->Broadcast(msg.c_str());
            TRACEI(this,DBG, msg);

            // Tell the session manager that this session has gone
            PostSessionRemoval(psrv->SrvPID());

            // Reset instance
            psrv->Reset();
         }
      }
   }

   // Set fields to starting point (debugging mostly)
   Reset();

   // Push ourselves on the stack
   fgProtStack.Push(&fProtLink);
}

//______________________________________________________________________________
int XrdProofdProtocol::GetBuff(int quantum)
{
   // Allocate a buffer to handle quantum bytes

   TRACE(ACT, "GetBuff: len: "<<quantum);

   // The current buffer may be sufficient for the current needs
   if (!fArgp || quantum > fArgp->bsize)
      fhcNow = fhcPrev;
   else if (quantum >= fhalfBSize || fhcNow-- > 0)
      return 1;
   else if (fhcNext >= fhcMax)
      fhcNow = fhcMax;
   else {
      int tmp = fhcPrev;
      fhcNow = fhcNext;
      fhcPrev = fhcNext;
      fhcNext = tmp + fhcNext;
   }

   // We need a new buffer
   if (fArgp)
      fgBPool->Release(fArgp);
   if ((fArgp = fgBPool->Obtain(quantum)))
      fhalfBSize = fArgp->bsize >> 1;
   else {
      XrdProofdResponse *response = Response(fRequest.header.requestid);
      if (!response) {
         TRACEI(this, XERR, "GetBuff: could not get Response instance for requid:"
                           << fRequest.header.requestid);
         return 1;
      }
      return response->Send(kXR_NoMemory, "insufficient memory for requested buffer");
   }

   // Success
   return 1;
}

//______________________________________________________________________________
int XrdProofdProtocol::GetData(const char *dtype, char *buff, int blen)
{
   // Get data from the open link

   int rlen;

   // Read the data but reschedule the link if we have not received all of the
   // data within the timeout interval.
   TRACEI(this,ACT, "GetData: dtype: "<<(dtype ? dtype : " - ")<<", blen: "<<blen);

   rlen = fLink->Recv(buff, blen, fgReadWait);

   if (rlen  < 0)
      if (rlen != -ENOMSG) {
         XrdOucString emsg = "GetData: link read error: errno: ";
         emsg += -rlen;
         TRACEI(this,XERR, emsg.c_str());
         return fLink->setEtext(emsg.c_str());
      } else {
         TRACEI(this,DBG, "GetData: connection closed by peer (errno: "<<-rlen<<")");
         return -1;
      }
   if (rlen < blen) {
      TRACEI(this,XERR, "GetData: " << dtype <<
                  " timeout; read " <<rlen <<" of " <<blen <<" bytes");
      return 1;
   }
   TRACEI(this,DBG, "GetData: rlen: "<<rlen);

   return 0;
}

//______________________________________________________________________________
int XrdProofdProtocol::SendData(XrdProofServProxy *xps,
                                kXR_int32 sid, XrdSrvBuffer **buf)
{
   // Send data over the open link. Segmentation is done here, if required.

   int rc = 1;

   TRACEI(this,ACT, "SendData: enter: length: "<<fRequest.header.dlen<<" bytes ");

   // Buffer length
   int len = fRequest.header.dlen;

   // Quantum size
   int quantum = (len > fgMaxBuffsz ? fgMaxBuffsz : len);

   // Make sure we have a large enough buffer
   if (!fArgp || quantum < fhalfBSize || quantum > fArgp->bsize) {
      if ((rc = GetBuff(quantum)) <= 0)
         return rc;
   } else if (fhcNow < fhcNext)
      fhcNow++;

   // Now send over all of the data as unsolicited messages
   while (len > 0) {
      if ((rc = GetData("data", fArgp->buff, quantum)))
         return rc;
      if (buf && !(*buf))
         *buf = new XrdSrvBuffer(fArgp->buff, quantum, 1);
      // Send
      if (sid > -1) {
         if (xps->Response()->Send(kXR_attn, kXPD_msgsid, sid, fArgp->buff, quantum))
            return 1;
      } else {

         // Get ID of the client
         int cid = ntohl(fRequest.sendrcv.cid);
         TRACEI(this, DBG, "SendMsg: INT: client ID: "<<cid);
         if (xps->SendData(cid, fArgp->buff, quantum))
            return 1;
      }
      // Next segment
      len -= quantum;
      if (len < quantum)
         quantum = len;
   }

   // Done
   return 0;
}

//______________________________________________________________________________
int XrdProofdProtocol::SendDataN(XrdProofServProxy *xps,
                                 XrdSrvBuffer **buf)
{
   // Send data over the open client links of session 'xps'.
   // Used when all the connected clients are eligible to receive the message.
   // Segmentation is done here, if required.

   int rc = 1;

   TRACEI(this,ACT, "SendDataN: enter: length: "<<fRequest.header.dlen<<" bytes ");

   // Buffer length
   int len = fRequest.header.dlen;

   // Quantum size
   int quantum = (len > fgMaxBuffsz ? fgMaxBuffsz : len);

   // Make sure we have a large enough buffer
   if (!fArgp || quantum < fhalfBSize || quantum > fArgp->bsize) {
      if ((rc = GetBuff(quantum)) <= 0)
         return rc;
   } else if (fhcNow < fhcNext)
      fhcNow++;

   // Now send over all of the data as unsolicited messages
   while (len > 0) {
      if ((rc = GetData("data", fArgp->buff, quantum)))
         return rc;
      if (buf && !(*buf))
         *buf = new XrdSrvBuffer(fArgp->buff, quantum, 1);

      // Send to connected clients
      xps->SendDataN(fArgp->buff, quantum);

      // Next segment
      len -= quantum;
      if (len < quantum)
         quantum = len;
   }

   // Done
   return 0;
}

//_____________________________________________________________________________
int XrdProofdProtocol::SendMsg()
{
   // Handle a request to forward a message to another process

   static const char *crecv[4] = {"master proofserv", "top master",
                                  "client", "undefined"};
   int rc = 1;

   XPD_SETRESP(this, "SendMsg");

   // Unmarshall the data
   int psid = ntohl(fRequest.sendrcv.sid);
   int opt = ntohl(fRequest.sendrcv.opt);
   bool external = !(opt & kXPD_internal);

   // Find server session
   XrdProofServProxy *xps = 0;
   if (!fPClient || !(xps = fPClient->GetProofServ(psid))) {
      TRACEP(this, respid, XERR, "SendMsg: session ID not found: "<< psid);
      response->Send(kXR_InvalidRequest,"session ID not found");
      return rc;
   }
   XrdSysMutexHelper mh(xps->Mutex());

   // Forward message as unsolicited
   int len = fRequest.header.dlen;

   // Notify
   TRACEP(this, respid, DBG, "SendMsg: psid: "<<psid<<", xps: "<<xps<<", status: "<<xps->Status()<<
               ", cid: "<<fCID);

   if (external) {

      if (opt & kXPD_process) {
         TRACEP(this, respid, DBG, "SendMsg: EXT: setting proofserv in 'running' state");
         xps->SetStatus(kXPD_running);
         PostSession(1, fPClient->UI().fUser.c_str(),
                        fPClient->UI().fGroup.c_str(), xps->SrvPID());
      }

      // Send to proofsrv our client ID
      if (fCID == -1) {
         response->Send(kXR_ServerError,"EXT: getting clientSID");
         return rc;
      }
      if (SendData(xps, fCID)) {
         response->Send(kXR_ServerError,"EXT: sending message to proofserv");
         return rc;
      }
      // Notify to user
      response->Send();
      TRACEP(this, respid, DBG, "SendMsg: EXT: message sent to proofserv ("<<len<<" bytes)");

   } else {

      bool saveStartMsg = 0;
      XrdSrvBuffer *savedBuf = 0;
      // Additional info about the message
      if (opt & kXPD_setidle) {
         TRACEP(this, respid, DBG, "SendMsg: INT: setting proofserv in 'idle' state");
         xps->SetStatus(kXPD_idle);
         PostSession(-1, fPClient->UI().fUser.c_str(),
                         fPClient->UI().fGroup.c_str(), xps->SrvPID());

      } else if (opt & kXPD_querynum) {
         TRACEP(this, respid, DBG, "SendMsg: INT: got message with query number");
         // Save query num message for later clients
         savedBuf = xps->QueryNum();
      } else if (opt & kXPD_startprocess) {
         TRACEP(this, respid, DBG, "SendMsg: INT: setting proofserv in 'running' state");
         xps->SetStatus(kXPD_running);
         // Save start processing message for later clients
         xps->DeleteStartMsg();
         saveStartMsg = 1;
      } else if (opt & kXPD_logmsg) {
         // We broadcast log messages only not idle to catch the
         // result from processing
         if (xps->Status() == kXPD_running) {
            TRACEP(this, respid, DBG, "SendMsg: INT: broadcasting log message");
            opt |= kXPD_fb_prog;
         }
      }
      bool fbprog = (opt & kXPD_fb_prog);

      if (!fbprog) {
         //
         // The message is strictly for the client requiring it
         int rs = SendData(xps, -1, &savedBuf);
         if (rs != 0) {
            response->Send(kXR_ServerError,
                           "SendMsg: INT: sending message to client"
                           " or master proofserv");
            return rc;
         }
      } else {
         // Send to all connected clients
         if (SendDataN(xps, &savedBuf)) {
            response->Send(kXR_ServerError,
                           "SendMsg: INT: sending message to client"
                           " or master proofserv");
            return rc;
         }
      }
      // Save start processing messages, if required
      if (saveStartMsg)
         xps->SetStartMsg(savedBuf);

      TRACEP(this, respid, DBG, "SendMsg: INT: message sent to "<<crecv[xps->SrvType()]<<
                  " ("<<len<<" bytes)");
      // Notify to proofsrv
      response->Send();
   }

   // Over
   return rc;
}

//______________________________________________________________________________
int XrdProofdProtocol::Urgent()
{
   // Handle generic request of a urgent message to be forwarded to the server
   unsigned int rc = 1;

   XPD_SETRESP(this, "Urgent");

   // Unmarshall the data
   int psid = ntohl(fRequest.proof.sid);
   int type = ntohl(fRequest.proof.int1);
   int int1 = ntohl(fRequest.proof.int2);
   int int2 = ntohl(fRequest.proof.int3);

   TRACEP(this, respid, REQ, "Urgent: enter: psid: "<<psid<<", type: "<< type);

   // Find server session
   XrdProofServProxy *xps = 0;
   if (!fPClient || !(xps = fPClient->GetProofServ(psid))) {
      TRACEP(this, respid, XERR, "Urgent: session ID not found");
      response->Send(kXR_InvalidRequest,"Urgent: session ID not found");
      return rc;
   }

   TRACEP(this, respid, DBG, "Urgent: xps: "<<xps<<", status: "<<xps->Status());

   // Check ID matching
   if (!xps->Match(psid)) {
      response->Send(kXP_InvalidRequest,"Urgent: IDs do not match - do nothing");
      return rc;
   }

   // Prepare buffer
   int len = 3 *sizeof(kXR_int32);
   char *buf = new char[len];
   // Type
   kXR_int32 itmp = static_cast<kXR_int32>(htonl(type));
   memcpy(buf, &itmp, sizeof(kXR_int32));
   // First info container
   itmp = static_cast<kXR_int32>(htonl(int1));
   memcpy(buf + sizeof(kXR_int32), &itmp, sizeof(kXR_int32));
   // Second info container
   itmp = static_cast<kXR_int32>(htonl(int2));
   memcpy(buf + 2 * sizeof(kXR_int32), &itmp, sizeof(kXR_int32));
   // Send over
   if (xps->Response()->Send(kXR_attn, kXPD_urgent, buf, len) != 0) {
      response->Send(kXP_ServerError,
                     "Urgent: could not propagate request to proofsrv");
      return rc;
   }

   // Notify to user
   response->Send();
   TRACEP(this, respid, DBG, "Urgent: request propagated to proofsrv");

   // Over
   return rc;
}

//___________________________________________________________________________
int XrdProofdProtocol::Interrupt()
{
   // Handle an interrupt request

   int rc = 1;

   XPD_SETRESP(this, "Interrupt");

   // Unmarshall the data
   int psid = ntohl(fRequest.interrupt.sid);
   int type = ntohl(fRequest.interrupt.type);
   TRACEP(this, respid, REQ, "Interrupt: psid: "<<psid<<", type:"<<type);

   // Find server session
   XrdProofServProxy *xps = 0;
   if (!fPClient || !(xps = fPClient->GetProofServ(psid))) {
      TRACEP(this, respid, XERR, "Interrupt: session ID not found");
      response->Send(kXR_InvalidRequest,"nterrupt: session ID not found");
      return rc;
   }

   if (xps) {

      // Check ID matching
      if (!xps->Match(psid)) {
         response->Send(kXP_InvalidRequest,"Interrupt: IDs do not match - do nothing");
         return rc;
      }

      TRACEP(this, respid, DBG, "Interrupt: xps: "<<xps<<", internal link "<<xps->Link()<<
                  ", proofsrv ID: "<<xps->SrvPID());

      // Propagate the type as unsolicited
      if (xps->Response()->Send(kXR_attn, kXPD_interrupt, type) != 0) {
         response->Send(kXP_ServerError,
                        "Interrupt: could not propagate interrupt code to proofsrv");
         return rc;
      }

      // Notify to user
      response->Send();
      TRACEP(this, respid, DBG, "Interrupt: interrupt propagated to proofsrv");
   }

   // Over
   return rc;
}

//___________________________________________________________________________
int XrdProofdProtocol::Ping()
{
   // Handle a ping request

   int rc = 1;
   XPD_SETRESP(this, "Ping");

   // Unmarshall the data
   int psid = ntohl(fRequest.sendrcv.sid);
   int opt = ntohl(fRequest.sendrcv.opt);

   TRACEP(this, respid, REQ, "Ping: psid: "<<psid<<", opt: "<<opt);

   // Find server session
   XrdProofServProxy *xps = 0;
   if (!fPClient || !(xps = fPClient->GetProofServ(psid))) {
      TRACEP(this, respid,  XERR, "Ping: session ID not found");
      response->Send(kXR_InvalidRequest,"session ID not found");
      return rc;
   }

   kXR_int32 pingres = 0;
   if (xps) {
      TRACEP(this, respid,  DBG, "Ping: xps: "<<xps<<", status: "<<xps->Status());

      // Type of connection
      bool external = !(opt & kXPD_internal);

      if (external) {
         TRACEP(this, respid,  DBG, "Ping: EXT: psid: "<<psid);

         // Send the request
         if ((pingres = (kXR_int32) xps->VerifyProofServ(fgMgr->SessionMgr()->InternalWait())) == -1) {
            TRACEP(this, respid,  XERR, "Ping: EXT: could not verify proofsrv");
            response->Send(kXR_ServerError, "EXT: could not verify proofsrv");
            return rc;
         }

         // Notify the client
         TRACEP(this, respid,  DBG, "Ping: EXT: ping notified to client");
         response->Send(kXR_ok, pingres);
         return rc;

      } else {
         TRACEP(this, respid,  DBG, "Ping: INT: psid: "<<psid);

         // If a semaphore is waiting, post it
         xps->PingSem();

#if 0
         // Just notify to user
         pingres = 1;
         TRACEP(this, respid, DBG, "Ping: INT: ping notified to client");
         fResponse.Send(kXR_ok, pingres);
#endif
         return rc;
      }
   }

   // Failure
   TRACEP(this, respid,  XERR, "Ping: session ID not found");
   response->Send(kXR_ok, pingres);
   return rc;
}

//___________________________________________________________________________
void XrdProofdProtocol::PostSession(int on, const char *u, const char *g, int pid)
{
   // Post change of session status

   if (fgPriorityMgrFd > 0) {
      int type = 0;
      if (write(fgPriorityMgrFd, &type, sizeof(type)) !=  sizeof(type)) {
         TRACE(XERR, "PostSession: problem sending message type on the pipe");
         return;
      }
      char buf[1024];
      sprintf(buf, "%d %s %s %d", on, u, g, pid);
      int len = strlen(buf)+1;
      if (write(fgPriorityMgrFd, &len, sizeof(len)) !=  sizeof(len)) {
         TRACE(XERR, "PostSession: problem sending message length on the pipe");
         return;
      }
      if (write(fgPriorityMgrFd, buf, len) !=  len) {
         TRACE(XERR, "PostSession: problem sending message on the pipe");
         return;
      }
   }
   // Done
   return;
}

//___________________________________________________________________________
void XrdProofdProtocol::PostSessionRemoval(int pid)
{
   // Post removal of session 'pid'

   TRACE(DBG, "PostClientDisconnection: posting session removal to socket "<<fgProofServMgrFd);

   if (fgProofServMgrFd > 0) {
      int type = 0;
      if (write(fgProofServMgrFd, &type, sizeof(type)) !=  sizeof(type)) {
         TRACE(XERR, "PostSessionRemoval: problem sending message type on the pipe");
         return;
      }
      if (write(fgProofServMgrFd, &pid, sizeof(pid)) !=  sizeof(pid)) {
         TRACE(XERR, "PostSessionRemoval: problem sending pid on the pipe: "<<pid);
         return;
      }
   }
   // Done
   return;
}

//___________________________________________________________________________
void XrdProofdProtocol::PostClientDisconnection()
{
   // Post disconnection of a client

   TRACE(DBG, "PostClientDisconnection: posting client disconnection to socket "<<fgProofServMgrFd);

   if (fgProofServMgrFd > 0) {
      int type = 1;
      if (write(fgProofServMgrFd, &type, sizeof(type)) !=  sizeof(type)) {
         TRACE(XERR, "PostClientDisconnection: problem sending message type on the pipe");
         return;
      }
   }
   // Done
   return;
}
