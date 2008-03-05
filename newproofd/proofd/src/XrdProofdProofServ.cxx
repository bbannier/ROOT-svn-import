// @(#)root/proofd:$Id$
// Author: Gerardo Ganis  12/12/2005

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "XrdProofdProofServ.h"
#include "XrdProofWorker.h"

// Tracing utils
#include "XrdProofdTrace.h"
static const char *TraceID = "";
#define TRACEID TraceID
#ifndef SafeDelete
#define SafeDelete(x) { if (x) { delete x; x = 0; } }
#endif
#ifndef SafeDelArray
#define SafeDelArray(x) { if (x) { delete[] x; x = 0; } }
#endif

//__________________________________________________________________________
XrdProofdProofServ::XrdProofdProofServ()
{
   // Constructor

   fMutex = new XrdSysRecMutex;
   fConn = 0;
   fParent = 0;
   fPingSem = 0;
   fQueryNum = 0;
   fStartMsg = 0;
   fStatus = kXPD_idle;
   fSrvPID = -1;
   fSrvType = kXPD_AnyServer;
   fID = -1;
   fIsShutdown = false;
   fIsValid = true;  // It is created for a valid server ...
   fProtVer = -1;
   fNClients = 0;
   fClients.reserve(10);
   fDisconnectTime = -1;
   fSetIdleTime = time(0);
   fROOT = 0;
   // Strings
   fAlias = "";
   fClient = "";
   fFileout = "";
   fGroup = "";
   fOrdinal = "";
   fTag = "";
   fUserEnvs = "";
}

//__________________________________________________________________________
XrdProofdProofServ::~XrdProofdProofServ()
{
   // Destructor

   SafeDelete(fQueryNum);
   SafeDelete(fStartMsg);
   SafeDelete(fPingSem);

   std::vector<XrdClientID *>::iterator i;
   for (i = fClients.begin(); i != fClients.end(); i++)
       if (*i)
          delete (*i);
   fClients.clear();

   // Cleanup worker info
   ClearWorkers();

   SafeDelete(fMutex);
}

//__________________________________________________________________________
void XrdProofdProofServ::ClearWorkers()
{
   // Decrease worker counters and clean-up the list

   XrdSysMutexHelper mhp(fMutex);

   // Decrease worker counters
   std::list<XrdProofWorker *>::iterator i;
   for (i = fWorkers.begin(); i != fWorkers.end(); i++)
       if (*i)
          (*i)->fActive--;
   fWorkers.clear();
}

//__________________________________________________________________________
void XrdProofdProofServ::Reset()
{
   // Reset this instance
   XrdSysMutexHelper mhp(fMutex);

   fConn = 0;
   fParent = 0;
   SafeDelete(fQueryNum);
   SafeDelete(fStartMsg);
   SafeDelete(fPingSem);
   fStatus = kXPD_idle;
   fSrvPID = -1;
   fSrvType = kXPD_AnyServer;
   fID = -1;
   fIsShutdown = false;
   fIsValid = 0;
   fProtVer = -1;
   fNClients = 0;
   fClients.clear();
   fDisconnectTime = -1;
   fSetIdleTime = -1;
   fROOT = 0;
   // Cleanup worker info
   ClearWorkers();
   // Strings
   fAlias = "";
   fClient = "";
   fFileout = "";
   fGroup = "";
   fOrdinal = "";
   fTag = "";
   fUserEnvs = "";
}

//__________________________________________________________________________
XrdClientID *XrdProofdProofServ::GetClientID(int cid)
{
   // Get instance corresponding to cid
   //

   XrdSysMutexHelper mhp(fMutex);

   XrdClientID *csid = 0;
   TRACE(ACT,"XrdProofdProofServ::GetClientID: cid: "<<cid<<
             ", size: "<<fClients.size());

   if (cid < 0) {
      TRACE(XERR,"XrdProofdProofServ::GetClientID: negative ID: protocol error!");
      return csid;
   }

   // Count new attached client
   fNClients++;

   // If in the allocate range reset the corresponding instance and
   // return it
   if (cid < (int)fClients.size()) {
      csid = fClients.at(cid);
      csid->Reset();
      return csid;
   }

   // If not, allocate a new one; we need to resize (double it)
   if (cid >= (int)fClients.capacity())
      fClients.reserve(2*fClients.capacity());

   // Allocate new elements (for fast access we need all of them)
   int ic = (int)fClients.size();
   for (; ic <= cid; ic++)
      fClients.push_back((csid = new XrdClientID()));

   TRACE(DBG,"XrdProofdProofServ::GetClientID: cid: "<<cid<<
             ", new size: "<<fClients.size());

   // We are done
   return csid;
}

//__________________________________________________________________________
int XrdProofdProofServ::FreeClientID(XrdProofdProtocol *p)
{
   // Free instance corresponding to protocol 'p'
   //

   XrdSysMutexHelper mhp(fMutex);

   TRACE(ACT,"ProofServ::FreeClientID: p: "<<p<<", session status: "<<
              fStatus<<", # clients: "<< fClients.size());

   if (!p) {
      TRACE(XERR,"ProofServ::FreeClientID: undefined protocol!");
      return -1;
   }

   // Remove this from the list of clients
   std::vector<XrdClientID *>::iterator i;
   for (i = fClients.begin(); i != fClients.end(); ++i) {
      if ((*i) && (*i)->P() == p) {
         (*i)->Reset();
         fNClients--;
         // Record time of last disconnection
         if (fNClients <= 0)
            fDisconnectTime = time(0);
         return 0;
      }
   }

   // Out of range
   return -1;
}

//__________________________________________________________________________
int XrdProofdProofServ::DisconnectTime()
{
   // Return the time (in secs) all clients have been disconnected.
   // Return -1 if the session is running

   XrdSysMutexHelper mhp(fMutex);

   int disct = -1;
   if (fDisconnectTime > 0)
      disct = time(0) - fDisconnectTime;
   return ((disct > 0) ? disct : -1);
}

//__________________________________________________________________________
int XrdProofdProofServ::IdleTime()
{
   // Return the time (in secs) the session has been idle.
   // Return -1 if the session is running

   XrdSysMutexHelper mhp(fMutex);

   int idlet = -1;
   if (fStatus == kXPD_idle)
      idlet = time(0) - fSetIdleTime;
   return ((idlet > 0) ? idlet : -1);
}

//__________________________________________________________________________
void XrdProofdProofServ::SetIdle()
{
   // Set status to idle and update the related time stamp
   //

   XrdSysMutexHelper mhp(fMutex);

   fStatus = kXPD_idle;
   fSetIdleTime = time(0);
}

//__________________________________________________________________________
void XrdProofdProofServ::SetRunning()
{
   // Set status to running and reset the related time stamp
   //

   XrdSysMutexHelper mhp(fMutex);

   fStatus = kXPD_running;
   fSetIdleTime = -1;
}

//______________________________________________________________________________
void XrdProofdProofServ::Broadcast(const char *msg)
{
   // Broadcast message 'msg' to the attached clients

   XrdSysMutexHelper mhp(fMutex);

   int len = 0;
   if (msg && (len = strlen(msg)) > 0) {

      int ic = 0;
      XrdProofdProtocol *p = 0;
      for (ic = 0; ic < (int) fClients.size(); ic++) {
         // Send message
         if ((p = fClients.at(ic)->P())) {
            XrdProofdResponse *response = p ? p->Response(fClients.at(ic)->Sid()) : 0;
            response->Send(kXR_attn, kXPD_errmsg, (void *)msg, len);
         }
      }
   }
}

//______________________________________________________________________________
int XrdProofdProofServ::TerminateProofServ()
{
   // Terminate the associated process.
   // A shutdown interrupt message is forwarded.
   // If add is TRUE (default) the pid is added to the list of processes
   // requested to terminate.
   // Return the pid of tyhe terminated process on success, -1 if not allowed
   // or other errors occured.

   TRACE(ACT, "ProofServ::TerminateProofServ: ord: " << Ordinal() << ", pid: " << fSrvPID);

   // Send a terminate signal to the proofserv
   int pid = fSrvPID;
   if (pid > -1) {

      int type = 3;
      if (Response()->Send(kXR_attn, kXPD_interrupt, type) != 0)
         // Could not send: signal failure
         return -1;
      // For registration/monitoring purposes
      return pid;
   }

   // Failed
   return -1;
}

//______________________________________________________________________________
int XrdProofdProofServ::VerifyProofServ(int timeout)
{
   // Check if the associated proofserv process is alive.
   // A ping message is sent and the reply waited for the internal timeout.
   // Return 1 if successful, 0 if reply was not received within the
   // internal timeout, -1 in case of error.
   int rc = -1;

   TRACE(ACT, "ProofServ::VerifyProofServ: ord: " << Ordinal()<< ", pid: " << fSrvPID);

   XrdSysMutexHelper mhp(fMutex);

   // Create semaphore
   CreatePingSem();

   // Propagate the ping request
   if (Response()->Send(kXR_attn, kXPD_ping, 0, 0) != 0) {
      TRACE(XERR, "ProofServ::VerifyProofServ: could not propagate ping to proofsrv");
      DeletePingSem();
      return rc;
   }

   // Wait for reply
   rc = 1;
   if (fPingSem && fPingSem->Wait(timeout) != 0) {
      XrdOucString msg = "ProofServ::VerifyProofServ: did not receive ping reply after ";
      msg += timeout;
      msg += " secs";
      TRACE(XERR, msg.c_str());
      rc = 0;
   }
   TRACE(ACT, "ProofServ::VerifyProofServ: " << fSrvPID << " ping: "<<rc);

   // Cleanup
   DeletePingSem();

   // Done
   return rc;
}

//__________________________________________________________________________
int XrdProofdProofServ::BroadcastPriority(int priority)
{
   // Broadcast a new group priority value to the worker servers.
   // Called by masters.

   XrdSysMutexHelper mhp(fMutex);

   // Prepare buffer
   int len = sizeof(kXR_int32);
   char *buf = new char[len];
   kXR_int32 itmp = priority;
   itmp = static_cast<kXR_int32>(htonl(itmp));
   memcpy(buf, &itmp, sizeof(kXR_int32));
   // Send over
   if (Response()->Send(kXR_attn, kXPD_priority, buf, len) != 0) {
      // Failure
      TRACE(XERR,"XrdProofdProofServ::BroadcastPriorities: problems telling proofserv");
      return -1;
   }
   TRACE(DBG,"XrdProofdProofServ::BroadcastPriorities: priority "<<priority<<" sent over");
   // Done
   return 0;
}

//______________________________________________________________________________
int XrdProofdProofServ::SendData(int cid, void *buff, int len)
{
   // Send data to client cid.

   TRACE(HDBG, "XrdProofdProofServ::SendData: length: "<<len<<" bytes (cid: "<<cid<<
                  ", size: "<<fClients.size()<<")");

   // Get corresponding instance
   XrdClientID *csid = 0;
   if (cid < 0 || cid > (int)(fClients.size() - 1) || !(csid = fClients.at(cid))) {
      TRACE(XERR, "XrdProofdProofServ::SendData: client ID not found (cid: "<<cid<<
                  ", size: "<<fClients.size()<<")");
      return -1;
   }
   if (!(csid->P())) {
      TRACE(XERR, "XrdProofdProofServ::SendData: client not connected: csid: "<<csid<<
                  ", cid: "<<cid<<", fSid: " << csid->Sid());
      return -1;
   }

   //
   // The message is strictly for the client requiring it
   int rs = -1;
   XrdProofdResponse *response = csid->P() ? csid->P()->Response(csid->Sid()) : 0;
   if (response)
      if (!response->Send(kXR_attn, kXPD_msg, buff, len))
         rs = 0;

   // Done
   return rs;
}

//______________________________________________________________________________
int XrdProofdProofServ::SendDataN(void *buff, int len)
{
   // Send data over the open client links of this session.
   // Used when all the connected clients are eligible to receive the message.

   TRACE(HDBG, "SendDataN: enter: length: "<<len<<" bytes");

   // Send to connected clients
   XrdClientID *csid = 0;
   int ic = 0;
   for (ic = 0; ic < (int) fClients.size(); ic++) {
      if ((csid = fClients.at(ic)) && csid->P()) {
         XrdProofdResponse *resp = csid->P()->Response(csid->Sid());
         int rs = 0;
         TRACE(HDBG, "SendDataN: INTERNAL: client sid:"<<csid->Sid());
         rs = resp->Send(kXR_attn, kXPD_msg, buff, len);
         if (rs)
            return 1;
      }
   }

   // Done
   return 0;
}
