// @(#)root/proofd:$Id$
// Author: Gerardo Ganis  12/12/2005

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "XrdProofServProxy.h"
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
XrdProofServProxy::XrdProofServProxy()
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
XrdProofServProxy::~XrdProofServProxy()
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
void XrdProofServProxy::ClearWorkers()
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
void XrdProofServProxy::Reset()
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
XrdClientID *XrdProofServProxy::GetClientID(int cid)
{
   // Get instance corresponding to cid
   //

   XrdSysMutexHelper mhp(fMutex);

   XrdClientID *csid = 0;
   TRACE(ACT,"XrdProofServProxy::GetClientID: cid: "<<cid<<
             ", size: "<<fClients.size());

   if (cid < 0) {
      TRACE(XERR,"XrdProofServProxy::GetClientID: negative ID: protocol error!");
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

   TRACE(DBG,"XrdProofServProxy::GetClientID: cid: "<<cid<<
             ", new size: "<<fClients.size());

   // We are done
   return csid;
}

//__________________________________________________________________________
int XrdProofServProxy::FreeClientID(XrdProofdProtocol *p)
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
int XrdProofServProxy::DisconnectTime()
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
int XrdProofServProxy::IdleTime()
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
void XrdProofServProxy::SetIdle()
{
   // Set status to idle and update the related time stamp
   //

   XrdSysMutexHelper mhp(fMutex);

   fStatus = kXPD_idle;
   fSetIdleTime = time(0);
}

//__________________________________________________________________________
void XrdProofServProxy::SetRunning()
{
   // Set status to running and reset the related time stamp
   //

   XrdSysMutexHelper mhp(fMutex);

   fStatus = kXPD_running;
   fSetIdleTime = -1;
}

//______________________________________________________________________________
void XrdProofServProxy::Broadcast(const char *msg)
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
#if 0
            unsigned short sid;
            p->Response()->GetSID(sid);
            p->Response()->Set(fClients.at(ic)->Sid());
            p->Response()->Send(kXR_attn, kXPD_errmsg, (void *)msg, len);
            p->Response()->Set(sid);
#else
            XrdProofdResponse *response = p ? p->Response(fClients.at(ic)->Sid()) : 0;
            response->Send(kXR_attn, kXPD_errmsg, (void *)msg, len);
#endif
         }
      }
   }
}

//______________________________________________________________________________
int XrdProofServProxy::TerminateProofServ()
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
int XrdProofServProxy::VerifyProofServ(int timeout)
{
   // Check if the associated proofserv process is alive.
   // A ping message is sent and the reply waited for the internal timeout.
   // Return 1 if successful, 0 if reply was not received within the
   // internal timeout, -1 in case of error.
   int rc = -1;

   TRACE(ACT, "ProofServ::VerifyProofServ: ord: " << Ordinal()<< ", pid: " << fSrvPID);

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

#if 0
//__________________________________________________________________________
int XrdProofServProxy::GetDefaultProcessPriority()
{
   // Get the default nice value for a process

   if (fDefSchedPriority == -99999)
      fDefSchedPriority = getpriority(PRIO_PROCESS, fSrvPID);
   return fDefSchedPriority;
}

//__________________________________________________________________________
int XrdProofServProxy::SetProcessPriority(int priority)
{
   // Set priority of the server process to priority (positive or negative)
   // If 'priority' is -99999 restore the default value.
   // Returns 0 in case of success, -errno in case of error.

   TRACE(SCHED, "SetProcessPriority: enter: pid: " << fSrvPID <<
              ", priority: " << priority);

   int newpriority = priority;
   // Restore defaults if requested
   if (priority == -99999) {
      newpriority = GetDefaultProcessPriority();
   }

   // Set the priority
   {  XrdProofUI ui;
      XrdProofdAux::GetUserInfo(geteuid(), ui);
      XrdSysPrivGuard pGuard((uid_t)0, (gid_t)0);
      if (XpdBadPGuard(pGuard, ui.fUid)) {
         TRACE(XERR, "SetProcessPriority: could not get privileges");
         return -1;
      }
      TRACE(SCHED, "SetProcessPriority: got privileges ");
      errno = 0;
      if (setpriority(PRIO_PROCESS, fSrvPID, newpriority) != 0) {
         TRACE(XERR, "SetProcessPriority:"
                     " setpriority: errno: " << errno);
         return ((errno != 0) ? -errno : -1);
      }
      TRACE(SCHED, "SetProcessPriority: new priority set ");
   }

   // Check that it worked out
   errno = 0;
   if ((priority = getpriority(PRIO_PROCESS, fSrvPID)) == -1 && errno != 0) {
      TRACE(XERR, "SetProcessPriority:"
                 " getpriority: errno: " << errno);
      return -errno;
   }
   if (priority != newpriority) {
      TRACE(XERR, "SetProcessPriority:"
                 " unexpected result of action: found " << priority <<
                 ", expected "<<newpriority);
      errno = EPERM;
      return -errno;
   }

   TRACE(SCHED, "SetProcessPriority: done: pid: " << fSrvPID <<
              ", priority: " << priority);

   // We are done
   return 0;
}

//__________________________________________________________________________
int XrdProofServProxy::SetInflate(int inflate, bool sendover)
{
   // Set the inflate factor for this session; this factor is used to
   // artificially inflate the processing time (by delaying new packet
   // requests) to control resource sharing.
   // If 'sendover' is TRUE the factor is communicated to proofserv,
   // otherwise is just stored.

   XrdSysMutexHelper mhp(fMutex);
   fInflate = inflate;

   if (sendover) {
      // Prepare buffer
      int len = sizeof(kXR_int32);
      char *buf = new char[len];
      kXR_int32 itmp = inflate;
      itmp = static_cast<kXR_int32>(htonl(itmp));
      memcpy(buf, &itmp, sizeof(kXR_int32));
      // Send over
      if (fResponse.Send(kXR_attn, kXPD_inflate, buf, len) != 0) {
         // Failure
         TRACE(XERR,"XrdProofServProxy::SetInflate: problems telling proofserv");
         return -1;
      }
      TRACE(DBG,"XrdProofServProxy::SetInflate: inflate factor set to "<<inflate);
   }
   // Done
   return 0;
}
#endif

//__________________________________________________________________________
int XrdProofServProxy::BroadcastPriority(int priority)
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
      TRACE(XERR,"XrdProofServProxy::BroadcastPriorities: problems telling proofserv");
      return -1;
   }
   TRACE(DBG,"XrdProofServProxy::BroadcastPriorities: priority "<<priority<<" sent over");
   // Done
   return 0;
}

//__________________________________________________________________________
void XrdProofServProxy::SetSrv(int pid)
{
   // Set the server PID. Also find the scheduling policy

   XrdSysMutexHelper mhp(fMutex);

   // The PID
   fSrvPID = pid;

   // Done
   return;
}

//______________________________________________________________________________
int XrdProofServProxy::SendData(int cid, void *buff, int len)
{
   // Send data to client cid.

   TRACE(HDBG, "XrdProofServProxy::SendData: enter: length: "<<len<<" bytes");

   // Get corresponding instance
   XrdClientID *csid = 0;
   if (cid < 0 || cid > (int)(fClients.size() - 1) || !(csid = fClients.at(cid))) {
      TRACE(XERR, "XrdProofServProxy::SendData: client ID not found (cid: "<<cid<<
                  ", size: "<<fClients.size()<<")");
      return -1;
   }
   if (!(csid->P())) {
      TRACE(XERR, "XrdProofServProxy::SendData: client not connected: csid: "<<csid<<
                  ", cid: "<<cid<<", fSid: " << csid->Sid());
      return -1;
   }

   //
   // The message is strictly for the client requiring it
#if 0
   int rs = 0;
   {  XrdSysMutexHelper mhp(csid->P()->Response()->fMutex);
      unsigned short sid;
      csid->P()->Response()->GetSID(sid);
      TRACE(HDBG, "XrdProofServProxy::SendData: this sid: "<<sid<<
                  ", client sid: "<<csid->Sid());
      csid->P()->Response()->Set(csid->Sid());
      if (csid->P()->Response()->Send(kXR_attn, kXPD_msg, buff, len))
         rs = -1;
      csid->P()->Response()->Set(sid);
   }
#else
   int rs = -1;
   XrdProofdResponse *response = csid->P() ? csid->P()->Response(csid->Sid()) : 0;
   if (response)
      if (!response->Send(kXR_attn, kXPD_msg, buff, len))
         rs = 0;
#endif

   // Done
   return rs;
}

//______________________________________________________________________________
int XrdProofServProxy::SendDataN(void *buff, int len)
{
   // Send data over the open client links of this session.
   // Used when all the connected clients are eligible to receive the message.

   TRACE(HDBG, "SendDataN: enter: length: "<<len<<" bytes");

   // Send to connected clients
   XrdClientID *csid = 0;
   int ic = 0;
   for (ic = 0; ic < (int) fClients.size(); ic++) {
      if ((csid = fClients.at(ic)) && csid->P()) {
#if 0
         XrdProofdResponse *resp = csid->P()->Response();
         int rs = 0;
         {  XrdSysMutexHelper mhp(resp->fMutex);
            unsigned short sid;
            resp->GetSID(sid);
            TRACE(HDBG, "SendDataN: INTERNAL: this sid: "<<sid<<
                           "; client sid:"<<csid->Sid());
            resp->Set(csid->Sid());
            rs = resp->Send(kXR_attn, kXPD_msg, buff, len);
            resp->Set(sid);
         }
#else
         XrdProofdResponse *resp = csid->P()->Response(csid->Sid());
         int rs = 0;
         TRACE(HDBG, "SendDataN: INTERNAL: client sid:"<<csid->Sid());
         rs = resp->Send(kXR_attn, kXPD_msg, buff, len);
#endif
         if (rs)
            return 1;
      }
   }

   // Done
   return 0;
}
