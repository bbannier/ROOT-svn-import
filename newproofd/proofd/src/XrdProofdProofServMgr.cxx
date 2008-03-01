// @(#)root/proofd:$Id:$
// Author: G. Ganis Jan 2008

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// XrdProofdProofServMgr                                                  //
//                                                                      //
// Author: G. Ganis, CERN, 2008                                         //
//                                                                      //
// Class managing proofserv sessions manager.                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
#include "XrdProofdPlatform.h"

#include "Xrd/XrdBuffer.hh"
#include "Xrd/XrdPoll.hh"
#include "XrdNet/XrdNet.hh"
#include "XrdNet/XrdNetDNS.hh"
#include "XrdNet/XrdNetPeer.hh"
#include "XrdOuc/XrdOucStream.hh"
#include "XrdSys/XrdSysPriv.hh"
#include "XrdSut/XrdSutAux.hh"

#include "XrdProofdClient.h"
#include "XrdProofdManager.h"
#include "XrdProofdNetMgr.h"
#include "XrdProofdPriorityMgr.h"
#include "XrdProofdProofServMgr.h"
#include "XrdProofdProtocol.h"
#include "XrdProofGroup.h"
#include "XrdROOT.h"

// Aux structures for scan through operations
typedef struct {
   XrdProofGroupMgr *fGroupMgr;
   int *fNBroadcast;
} XpdBroadcastPriority_t;

#define LONGOK(x) (x >= 0 && x < LONG_MAX)

// Tracing utilities
#include "XrdProofdTrace.h"
static const char *gTraceID = "";
extern XrdOucTrace *XrdProofdTrace;
#define TRACEID gTraceID

//--------------------------------------------------------------------------
//
// XrdProofdSessionPoller
//
// Function run in separate thread watching changes in session status
// frequency
//
//--------------------------------------------------------------------------
void *XrdProofdProofServCron(void *p)
{
   // This is an endless loop to check the system periodically or when
   // triggered via a message in a dedicated pipe

   XrdProofdProofServMgr *mgr = (XrdProofdProofServMgr *)p;
   if (!(mgr)) {
      TRACE(REQ, "XrdProofdProofServCron: undefined manager: cannot start");
      return (void *)0;
   }

   // Read protocol
   struct pollfd fds_r;
   fds_r.fd = mgr->ReadFd();
   fds_r.events = POLLIN;

   // Check frequency in milli secs
   int checkfq = (mgr->CheckFrequency() > 0) ? mgr->CheckFrequency() * 1000 : -1;

   while(1) {
      // We wait for processes to communicate a session status change
      int pollRet = 0;
      while ((pollRet = poll(&fds_r, 1, checkfq)) < 0 &&
            (errno == EINTR)) { }
      if (pollRet > 0) {
         // Read message type
         int type = -1;
         if (read(mgr->ReadFd(), &type, sizeof(type)) != sizeof(type)) {
            XPDERR("XrdProofdProofServCron: problems receiving message type");
            continue;
         }
         // only one type is supported currently
         if (type == 0) {
            // A session has just gone: read process id
            int pid = 0;
            if (read(mgr->ReadFd(), &pid, sizeof(pid)) != sizeof(pid)) {
               XPDERR("XrdProofdProofServCron: problems receiving process ID");
               continue;
            }
            // Remove it from the hash list
            mgr->DeleteFromSessions(pid);
            // Move the netry to the terminated sessions area
            mgr->MvSession(pid);
         } else if (type == 1) {
            // A client just disconnected: we check the sessions status to see
            // if any of them must be terminated
         } else {
            XPDERR("XrdProofdProofServCron: unknown type: "<<type);
            continue;
         }
      }
      // Run periodical checks
      mgr->CheckActiveSessions();
      mgr->CheckTerminatedSessions();
   }

   // Should never come here
   return (void *)0;
}

//______________________________________________________________________________
XrdProofdProofServMgr::XrdProofdProofServMgr(XrdProofdManager *mgr,
                                             XrdProtocol_Config *pi, XrdSysError *e)
                  : XrdProofdConfig(pi->ConfigFN, e)
{
   // Constructor

   fMgr = mgr;
   fSched = pi->Sched;
   fInternalWait = 10;
   fActiveSessions.clear();
   fShutdownOpt = 1;
   fShutdownDelay = 0;
   // Defaults can be changed via 'proofservmgr'
   fCheckFrequency = 20;
   fTerminationTimeOut = fCheckFrequency - 5;
   fVerifyTimeOut = 5;

   // Init pipe for the poller
   if (pipe(fPipe) != 0) {
      XPDERR("XrdProofdProofServMgr: unable to generate pipe for"
            " the session poller");
      return;
   }

   // Configuration directives
   RegisterDirectives();
}

//__________________________________________________________________________
int XrdProofdProofServMgr::Config(bool rcf)
{
   // Run configuration and parse the entered config directives.
   // Return 0 on success, -1 on error

   // Run first the configurator
   if (XrdProofdConfig::Config(rcf) != 0) {
      fEDest->Say(0, "xpd: Config: ProofServMgr: problems parsing file ");
      return -1;
   }

   XrdOucString msg;
   msg = (rcf) ? "xpd: Config: ProofServMgr: re-configuring"
               : "xpd: Config: ProofServMgr: configuring";
   fEDest->Say(0, msg.c_str());

   // Notify timeout on internal communications
   msg = "xpd: Config: ProofServMgr: setting internal timeout to (secs): ";
   msg += fInternalWait;
   fEDest->Say(0, msg.c_str());

   // Shutdown options
   msg = "xpd: Config: ProofServMgr: client sessions shutdown after disconnection";
   if (fShutdownOpt > 0) {
      if (fShutdownOpt == 1)
         msg = "xpd: Config: ProofServMgr: client sessions kept idle for ";
      else if (fShutdownOpt == 2)
         msg = "xpd: Config: ProofServMgr: client sessions kept for ";
      msg += fShutdownDelay;
      msg += " secs after disconnection";
   }
   fEDest->Say(0, msg.c_str());

   // Admin paths
   fActiAdminPath = fMgr->AdminPath();
   fActiAdminPath += "/activesessions";
   fTermAdminPath = fMgr->AdminPath();
   fTermAdminPath += "/terminatedsessions";

   // Make sure they exist
   XrdProofUI ui;
   XrdProofdAux::GetUserInfo(fMgr->EffectiveUser(), ui);
   if (XrdProofdAux::AssertDir(fActiAdminPath.c_str(), ui, 1) != 0) {
      fEDest->Say(0, "xpd: Config: ProofServMgr: unable to assert the admin path: ",
                     fActiAdminPath.c_str());
      fActiAdminPath = "";
      return -1;
   }
   fEDest->Say(0, "xpd: Config: ProofServMgr: active sessions admin path set to: ", fActiAdminPath.c_str());

   if (XrdProofdAux::AssertDir(fTermAdminPath.c_str(), ui, 1) != 0) {
      fEDest->Say(0, "xpd: Config: ProofServMgr: unable to assert the admin path: ",
                     fTermAdminPath.c_str());
      fTermAdminPath = "";
      return -1;
   }
   fEDest->Say(0, "xpd: Config: ProofServMgr: terminated sessions admin path set to: ", fTermAdminPath.c_str());

   // Start cron thread
   pthread_t tid;
   if (XrdSysThread::Run(&tid, XrdProofdProofServCron,
                           (void *)this, 0, "ProofServMgr cron thread") != 0) {
      fEDest->Say(0, "xpd: Config: ProofServMgr: could not start cron thread");
      return 0;
   }
   fEDest->Say(0, "xpd: Config: ProofServMgr: cron thread started");

   // Done
   return 0;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::AddSession(int pid, XrdProofdClient *c)
{
   // Add new active session

   // Check inputs
   if (pid <= 0 || !c) {
      TRACE(XERR,"AddSession: invalid inputs: "<<pid<<", "<<c);
      return -1;
   }

   // Path
   XrdOucString path = GetSessionPath(fActiAdminPath.c_str(), pid);

   // Save session info to file
   XrdProofSessionInfo info(c->User(), c->Group(), c->UNIXSockPath());
   return info.SaveToFile(path.c_str());
}

//______________________________________________________________________________
XrdOucString XrdProofdProofServMgr::GetSessionPath(const char *root, int pid)
{
   // Get the path for 'pid' under 'root'

   XrdOucString p;

   // Check inputs
   if (pid > 0 && root) {
      p = root;
      p += "/";
      p += pid;
   } else {
      TRACE(XERR,"GetSessionPath: invalid inputs: "<<pid<<", "<<root);
   }

   // Done
   return p;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::MvSession(int pid)
{
   // Move session file from the active to the terminated areas 

   // Check inputs
   if (pid <= 0) {
      TRACE(XERR,"MvSession: invalid input: "<<pid);
      return -1;
   }

   // Paths
   XrdOucString opath = GetSessionPath(fActiAdminPath.c_str(), pid);
   XrdOucString npath = GetSessionPath(fTermAdminPath.c_str(), pid);

   // Move the file
   errno = 0;
   int rc = 0;
   if ((rc = rename(opath.c_str(), npath.c_str()) == 0) || (errno == ENOENT)) {
      if (!rc)
         // Record the time when we did this
         TouchSession(pid, npath.c_str());
      return 0;
   }

   TRACE(XERR,"MvSession: session pid file cannot be moved: "<<opath<<
              "; target file: "<<npath<<"; errno: "<<errno);
   return -1;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::RmSession(int pid)
{
   // Remove session file from the terminated sessions area

   // Check inputs
   if (pid <= 0) {
      TRACE(XERR,"RmSession: invalid input: "<<pid);
      return -1;
   }

   // Path
   XrdOucString path = GetSessionPath(fTermAdminPath.c_str(), pid);

   // remove the file
   if (unlink(path.c_str()) == 0)
      return 0;

   TRACE(XERR,"RmSession: session pid file cannot be unlinked: "<<
              path<<"; error: "<<errno);
   return -1;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::TouchSession(int pid, const char *fpath)
{
   // Update the access time for the session pid file to the current time

   // Check inputs
   if (pid <= 0) {
      TRACE(XERR,"TouchSession: invalid input: "<<pid);
      return -1;
   }

   // Path
   XrdOucString path = (fpath && strlen(fpath) > 0)
                     ? fpath : GetSessionPath(fActiAdminPath.c_str(), pid);

   // Current settings
   struct stat st;
   if (stat(path.c_str(), &st)) {
      TRACE(XERR,"TouchSession: session pid file cannot be stat'ed: "<<
              path<<"; error: "<<errno);
      return -1;
   }

   // utime wanted by utime
   struct utimbuf ut;
   ut.modtime = st.st_mtime;
   if ((ut.actime = time(0)) < st.st_atime)
      ut.actime = st.st_atime;

   // Update file time stamps
   if (utime(path.c_str(), &ut) == 0)
      return 0;

   TRACE(XERR,"TouchSession: time stamps for session pid file cannot be updated: "<<
              path<<"; error: "<<errno);
   return -1;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::DeleteFromSessions(int pid)
{
   // Delete from the hash list the session with ID pid.
   // Return -ENOENT if not found, or 0.

   XrdOucString key; key += pid;
   XrdProofServProxy *xps = fSessions.Find(key.c_str());
   if (xps) fActiveSessions.remove(xps);
   return fSessions.Del(key.c_str());
}

//______________________________________________________________________________
int XrdProofdProofServMgr::CheckActiveSessions()
{
   // Go through the active sessions admin path and make sure sessions are alive.
   // Move those not responding in the terminated sessions admin path.

   // Open dir
   DIR *dir = opendir(fActiAdminPath.c_str());
   if (!dir) {
      TRACE(XERR, "CheckActiveSessions: cannot open dir "<<fActiAdminPath<<
                  " ; error: "<<errno);
      return -1;
   }

   // Scan the active sessions admin path
   struct dirent *ent = 0;
   while ((ent = (struct dirent *)readdir(dir))) {
      // Get the session instance (skip non-digital entries)
      int pid = strtol(ent->d_name, 0, 10);
      if (!LONGOK(pid) || pid == 0) continue;
      //
      XrdProofServProxy *xps = fSessions.Find(ent->d_name);
      bool rmsession = (xps && xps->IsValid()) ? 0 : 1;
      if (xps && xps->IsValid()) {
         // Verify the session
         if (xps->VerifyProofServ(fVerifyTimeOut) != 1) {
            rmsession = 1;
         } else {
            // Touch it
            TouchSession(pid);
         }
         // Check if we need to shutdown it
         XrdSysMutexHelper mh(xps->Mutex());
         if (xps->GetNClients() <= 0) {
            if ((fShutdownOpt == 1 && (xps->IdleTime() >= fShutdownDelay)) ||
                (fShutdownOpt == 2 && (xps->DisconnectTime() >= fShutdownDelay))) {
               xps->TerminateProofServ();
            }
         }
      }
      // Remove the session, if needed
      if (rmsession)
         MvSession(pid);
   }
   // Close the directory
   closedir(dir);

   // Done
   return 0;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::CheckTerminatedSessions()
{
   // Go through the terminated sessions admin path and make sure sessions they
   // are gone.
   // Hard-kill those still alive.

   // Open dir
   DIR *dir = opendir(fTermAdminPath.c_str());
   if (!dir) {
      TRACE(XERR, "CheckTerminatedSessions: cannot open dir "<<fTermAdminPath<<
                  " ; error: "<<errno);
      return -1;
   }

   // Scan the terminated sessions admin path
   int now = -1;
   struct dirent *ent = 0;
   while ((ent = (struct dirent *)readdir(dir))) {
      // Get the session instance (skip non-digital entries)
      int pid = strtol(ent->d_name, 0, 10);
      if (!LONGOK(pid) || pid == 0) continue;

      // Current time
      now = (now > 0) ? now : time(0);

      // Full path
      XrdOucString path = GetSessionPath(fTermAdminPath.c_str(), pid);
      // Check termination time
      struct stat st;
      int rcst = stat(path.c_str(), &st);
      if ((now - st.st_mtime) > fTerminationTimeOut || rcst != 0) { 
         // Check if the process is still alive
         if (XrdProofdAux::VerifyProcessByID(pid) != 0) {
            // Send again an hard-kill signal
            XrdProofSessionInfo info(path.c_str());
            XrdProofUI ui;
            XrdProofdAux::GetUserInfo(info.fUser.c_str(), ui);
            XrdProofdAux::KillProcess(pid, 1, ui, fMgr->ChangeOwn());
         } else {
            // Delete the entry
            RmSession(pid);
         }
      }
   }
   // Close the directory
   closedir(dir);

   // Done
   return 0;
}

//__________________________________________________________________________
void XrdProofdProofServMgr::RegisterDirectives()
{
   // Register directives for configuration

   // Register special config directives
   Register("proofservmgr", new XrdProofdDirective("proofservmgr", this, &DoDirectiveClass));
   Register("putenv", new XrdProofdDirective("putenv", this, &DoDirectiveClass));
   Register("putrc", new XrdProofdDirective("putrc", this, &DoDirectiveClass));
   Register("shutdown", new XrdProofdDirective("shutdown", this, &DoDirectiveClass));
   // Register config directives for ints
   Register("intwait",
                  new XrdProofdDirective("intwait", (void *)&fInternalWait, &DoDirectiveInt));
}

//______________________________________________________________________________
int XrdProofdProofServMgr::DoDirective(XrdProofdDirective *d,
                                       char *val, XrdOucStream *cfg, bool rcf)
{
   // Update the priorities of the active sessions.

   if (!d)
      // undefined inputs
      return -1;

   if (d->fName == "proofservmgr") {
      return DoDirectiveProofServMgr(val, cfg, rcf);
   } else if (d->fName == "putenv") {
      return DoDirectivePutEnv(val, cfg, rcf);
   } else if (d->fName == "putrc") {
      return DoDirectivePutRc(val, cfg, rcf);
   } else if (d->fName == "shutdown") {
      return DoDirectiveShutdown(val, cfg, rcf);
   }
   TRACE(XERR,"DoDirective: unknown directive: "<<d->fName);
   return -1;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::DoDirectiveProofServMgr(char *val, XrdOucStream *cfg, bool)
{
   // Process 'proofsrvmgr' directive
   // eg: xpd.proofsrvmgr checkfq:120 termto:100 verifyto:5

   if (!val || !cfg)
      // undefined inputs
      return -1;

   int checkfq = -1;
   int termto = -1;
   int verifyto = -1;

   while (val) {
      XrdOucString tok(val);
      if (tok.beginswith("checkfq:")) {
         tok.replace("checkfq:", "");
         checkfq = strtol(tok.c_str(), 0, 10);
      } else if (tok.beginswith("termto:")) {
         tok.replace("termto:", "");
         termto = strtol(tok.c_str(), 0, 10);
      } else if (tok.beginswith("verifyto:")) {
         tok.replace("verifyto:", "");
         verifyto = strtol(tok.c_str(), 0, 10);
      }
      // Get next
      val = cfg->GetToken();
   }

   // Check deprecated 'if' directive
   if (fMgr->Host() && cfg)
      if (XrdProofdAux::CheckIf(cfg, fMgr->Host()) == 0)
         return 0;

   // Set the values
   fCheckFrequency = LONGOK(checkfq) ? checkfq : fCheckFrequency;
   fTerminationTimeOut = LONGOK(termto) ? termto : fTerminationTimeOut;
   fVerifyTimeOut = LONGOK(verifyto) ? verifyto : fVerifyTimeOut;

   return 0;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::DoDirectivePutEnv(char *val, XrdOucStream *, bool)
{
   // Process 'putenv' directives

   if (!val)
      // undefined inputs
      return -1;

   // Env variable to exported to 'proofserv'
   if (fProofServEnvs.length() > 0)
      fProofServEnvs += ',';
   fProofServEnvs += val;

   return 0;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::DoDirectivePutRc(char *val, XrdOucStream *cfg, bool)
{
   // Process 'putenv' directives

   if (!val || !cfg)
      // undefined inputs
      return -1;

   // rootrc variable to be passed to 'proofserv':
   if (fProofServRCs.length() > 0)
      fProofServRCs += ',';
   fProofServRCs += val;
   while ((val = cfg->GetToken()) && val[0]) {
      fProofServRCs += ' ';
      fProofServRCs += val;
   }

   return 0;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::DoDirectiveShutdown(char *val, XrdOucStream *cfg, bool)
{
   // Process 'shutdown' directive

   if (!val || !cfg)
      // undefined inputs
      return -1;

   int opt = -1;
   int delay = -1;

   // Shutdown option
   int dp = strtol(val,0,10);
   if (dp >= 0 && dp <= 2)
      opt = dp;
   // Shutdown delay
   if ((val = cfg->GetToken())) {
      int l = strlen(val);
      int f = 1;
      XrdOucString tval = val;
      // Parse
      if (val[l-1] == 's') {
         val[l-1] = 0;
      } else if (val[l-1] == 'm') {
         f = 60;
         val[l-1] = 0;
      } else if (val[l-1] == 'h') {
         f = 3600;
         val[l-1] = 0;
      } else if (val[l-1] < 48 || val[l-1] > 57) {
         f = -1;
      }
      if (f > 0) {
         int de = strtol(val,0,10);
         if (de > 0)
            delay = de * f;
      }
   }

   // Check deprecated 'if' directive
   if (fMgr->Host() && cfg)
      if (XrdProofdAux::CheckIf(cfg, fMgr->Host()) == 0)
         return 0;

   // Set the values
   fShutdownOpt = (opt > -1) ? opt : fShutdownOpt;
   fShutdownDelay = (delay > -1) ? delay : fShutdownDelay;

   return 0;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::Process(XrdProofdProtocol *p)
{
   // Process manager request

   int rc = 1;
   XPD_SETRESP(p, "Process");

   TRACEP(p, respid, REQ, "Process: enter: req id: " << p->Request()->header.requestid);

   // Once logged-in, the user can request the real actions
   XrdOucString emsg("Invalid request code: ");

   switch(p->Request()->header.requestid) {
   case kXP_create:
      return Create(p);
   case kXP_destroy:
      return Destroy(p);
   case kXP_attach:
      return Attach(p);
   case kXP_detach:
      return Detach(p);
   default:
      emsg += p->Request()->header.requestid;
      break;
   }

   // Whatever we have, it's not valid
   response->Send(kXR_InvalidRequest, emsg.c_str());
   return 0;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::Attach(XrdProofdProtocol *p)
{
   // Handle a request to attach to an existing session

   int psid = -1, rc = 1;
   XPD_SETRESP(p, "Attach");

   // Unmarshall the data
   psid = ntohl(p->Request()->proof.sid);
   TRACEP(p, respid, REQ, "Attach: psid: "<<psid<<", CID = "<<p->CID());

   // Find server session
   XrdProofServProxy *xps = 0;
   if (!p->Client() || !(xps = p->Client()->GetProofServ(psid))) {
      TRACEP(p, respid, XERR, "Attach: session ID not found");
      response->Send(kXR_InvalidRequest,"session ID not found");
      return rc;
   }
   TRACEP(p, respid, DBG, "Attach: xps: "<<xps<<", status: "<< xps->Status());

   // Stream ID
   unsigned short sid;
   memcpy((void *)&sid, (const void *)&(p->Request()->header.streamid[0]), 2);

   // We associate this instance to the corresponding slot in the
   // session vector of attached clients
   XrdClientID *csid = xps->GetClientID(p->CID());
   csid->SetP(p);
   csid->SetSid(sid);

   // Take parentship, if orphalin
   if (!(xps->Parent()))
      xps->SetParent(csid);

   // Notify to user
   if (p->ConnType() == kXPD_ClientMaster) {
      // Send also back the data pool url
      XrdOucString dpu = fMgr->PoolURL();
      if (!dpu.endswith('/'))
         dpu += '/';
      dpu += fMgr->NameSpace();
      response->Send(psid, xps->ROOT()->SrvProtVers(), (kXR_int16)XPROOFD_VERSBIN,
                         (void *) dpu.c_str(), dpu.length());
   } else
      response->Send(psid, xps->ROOT()->SrvProtVers(), (kXR_int16)XPROOFD_VERSBIN);

   // Send saved query num message
   if (xps->QueryNum()) {
      TRACEP(p, respid, XERR, "Attach: sending query num message ("<<
                  xps->QueryNum()->fSize<<" bytes)");
      response->Send(kXR_attn, kXPD_msg,
                          xps->QueryNum()->fBuff, xps->QueryNum()->fSize);
   }
   // Send saved start processing message, if not idle
   if (xps->Status() == kXPD_running && xps->StartMsg()) {
      TRACEP(p, respid, XERR, "Attach: sending start process message ("<<
                  xps->StartMsg()->fSize<<" bytes)");
      response->Send(kXR_attn, kXPD_msg,
                          xps->StartMsg()->fBuff, xps->StartMsg()->fSize);
   }

   // Over
   return rc;
}

//_________________________________________________________________________________
int XrdProofdProofServMgr::Create(XrdProofdProtocol *p)
{
   // Handle a request to create a new session

   int psid = -1, rc = 1;
   XPD_SETRESP(p, "Create");

   TRACEP(p, respid, REQ, "Create: enter");

   // Allocate next free server ID and fill in the basic stuff
   XrdProofServProxy *xps = p->Client()->GetFreeServObj();
   xps->SetClient(p->Client()->User());
   xps->SetSrvType(p->ConnType());
   psid = xps->ID();

   // Prepare the stream identifier
   unsigned short sid;
   memcpy((void *)&sid, (const void *)&(p->Request()->header.streamid[0]), 2);
   // We associate this instance to the corresponding slot in the
   // session vector of attached clients
   XrdClientID *csid = xps->GetClientID(p->CID());
   csid->SetP(p);
   csid->SetSid(sid);
   // Take parentship, if orphalin
   xps->SetParent(csid);

   // Unmarshall log level
   int loglevel = ntohl(p->Request()->proof.int1);

   // Parse buffer
   char *buf = p->Argp()->buff;
   int   len = p->Request()->proof.dlen;

   // Extract session tag
   XrdOucString tag(buf,len);

   TRACEP(p, respid, DBG, "Create: received buf: "<<tag);

   tag.erase(tag.find('|'));
   xps->SetTag(tag.c_str());
   TRACEP(p, respid, DBG, "Create: tag: "<<tag);

   // Extract ordinal number
   XrdOucString ord = "0";
   if ((p->ConnType() == kXPD_MasterWorker) || (p->ConnType() == kXPD_MasterMaster)) {
      ord.assign(buf,0,len-1);
      int iord = ord.find("|ord:");
      if (iord != STR_NPOS) {
         ord.erase(0,iord+5);
         ord.erase(ord.find("|"));
      } else
         ord = "0";
   }
   xps->SetOrdinal(ord.c_str());

   // Extract config file, if any (for backward compatibility)
   XrdOucString cffile;
   cffile.assign(buf,0,len-1);
   int icf = cffile.find("|cf:");
   if (icf != STR_NPOS) {
      cffile.erase(0,icf+4);
      cffile.erase(cffile.find("|"));
   } else
      cffile = "";

   // Extract user envs, if any
   XrdOucString uenvs;
   uenvs.assign(buf,0,len-1);
   int ienv = uenvs.find("|envs:");
   if (ienv != STR_NPOS) {
      uenvs.erase(0,ienv+6);
      uenvs.erase(uenvs.find("|"));
      xps->SetUserEnvs(uenvs.c_str());
   } else
      uenvs = "";

   // The ROOT version to be used
   xps->SetROOT(p->Client()->ROOT());
   XPDPRT("Create: using ROOT version: "<<xps->ROOT()->Export());
   if (p->ConnType() == kXPD_ClientMaster) {
      // Notify the client if using a version different from the default one
      if (p->Client()->ROOT() != fMgr->ROOTMgr()->DefaultVersion()) {
         XrdOucString msg("++++ Using NON-default ROOT version: ");
         msg += xps->ROOT()->Export();
         msg += " ++++\n";
         response->Send(kXR_attn, kXPD_srvmsg, (char *) msg.c_str(), msg.length());
      }
   }

   // Notify
   TRACEP(p, respid, DBG, "Create: {ord,cfg,psid,cid,log}: {"<<ord<<","<<cffile<<","<<psid
                                                  <<","<<p->CID()<<","<<loglevel<<"}");
   if (uenvs.length() > 0)
      TRACEP(p, respid, DBG, "Create: user envs: "<<uenvs);

   // Here we fork: for some weird problem on SMP machines there is a
   // non-zero probability for a deadlock situation in system mutexes.
   // The semaphore seems to have solved the problem.
   if (fForkSem.Wait(10) != 0) {
      xps->Reset();
      // Timeout acquire fork semaphore
      response->Send(kXR_ServerError, "timed-out acquiring fork semaphore");
      return rc;
   }

   // Pipe to communicate status of setup
   int fp[2];
   if (pipe(fp) != 0) {
      xps->Reset();
      // Failure creating pipe
      response->Send(kXR_ServerError,
                          "unable to create pipe for status-of-setup communication");
      return rc;
   }

   // Fork an agent process to handle this session
   int pid = -1;
   TRACEP(p, respid, FORK,"Forking external proofsrv: UNIX sock: "<<p->Client()->UNIXSockPath());
   if (!(pid = fSched->Fork("proofsrv"))) {

      p->Client()->Mutex()->UnLock();

      int setupOK = 0;

      XrdOucString pmsg = "child process ";
      pmsg += (int) getpid();
      MTRACE(FORK, "xpd: ", pmsg.c_str());

      // We set to the user environment
      if (SetUserEnvironment(p) != 0) {
         MTRACE(XERR, "xpd:child: ",
                      "Create: SetUserEnvironment did not return OK - EXIT");
         write(fp[1], &setupOK, sizeof(setupOK));
         close(fp[0]);
         close(fp[1]);
         exit(1);
      }

      char *argvv[6] = {0};

      // We add our PID to be able to identify processes coming from us
      char cpid[10] = {0};
      sprintf(cpid, "%d", getppid());

      // Log level
      char clog[10] = {0};
      sprintf(clog, "%d", loglevel);

      // start server
      argvv[0] = (char *) xps->ROOT()->PrgmSrv();
      argvv[1] = (char *)((p->ConnType() == kXPD_MasterWorker) ? "proofslave"
                       : "proofserv");
      argvv[2] = (char *)"xpd";
      argvv[3] = (char *)cpid;
      argvv[4] = (char *)clog;
      argvv[5] = 0;

      // Set environment for proofserv
      if (SetProofServEnv(p, psid, loglevel, cffile.c_str()) != 0) {
         MTRACE(XERR, "xpd:child: ",
                      "Create: SetProofServEnv did not return OK - EXIT");
         write(fp[1], &setupOK, sizeof(setupOK));
         close(fp[0]);
         close(fp[1]);
         exit(1);
      }

      // Setup OK: now we go
      // Communicate the logfile path
      int lfout = strlen(xps->Fileout());
      write(fp[1], &lfout, sizeof(lfout));
      if (lfout > 0) {
         int n, ns = 0;
         char *buf = (char *) xps->Fileout();
         for (n = 0; n < lfout; n += ns) {
            if ((ns = write(fp[1], buf + n, lfout - n)) <= 0) {
               MTRACE(XERR, "xpd:child: ",
                            "Create: SetProofServEnv did not return OK - EXIT");
               write(fp[1], &setupOK, sizeof(setupOK));
               close(fp[0]);
               close(fp[1]);
               exit(1);
            }
         }
      }

      // Cleanup
      close(fp[0]);
      close(fp[1]);

      MTRACE(LOGIN,"xpd:child: ", "Create: user: "<<p->Client()->User()<<
                         ", uid: "<<getuid()<<", euid:"<<geteuid());
      // Run the program
      execv(xps->ROOT()->PrgmSrv(), argvv);

      // We should not be here!!!
      MERROR("xpd:child: ", "Create: returned from execv: bad, bad sign !!!");
      exit(1);
   }

   TRACEP(p, respid, FORK,"Parent process: child is "<<pid);

   // Wakeup colleagues
   fForkSem.Post();

   // parent process
   if (pid < 0) {
      xps->Reset();
      // Failure in forking
      response->Send(kXR_ServerError, "could not fork agent");
      close(fp[0]);
      close(fp[1]);
      return rc;
   }

   // Read status-of-setup from pipe
   XrdOucString emsg;
   int setupOK = 0;
   struct pollfd fds_r;
   fds_r.fd = fp[0];
   fds_r.events = POLLIN;
   int pollRet = 0;
   // We wait for 14 secs max (7 x 2000 millisecs): this is enough to
   // cover possible delays due to heavy load; the client will anyhow
   // retry a few times
   int ntry = 7;
   while (pollRet == 0 && ntry--) {
      while ((pollRet = poll(&fds_r, 1, 2000)) < 0 &&
             (errno == EINTR)) { }
      if (pollRet == 0)
         TRACEP(p, respid, FORK,"Create: "
                    "receiving status-of-setup from pipe: waiting 2 s ..."<<pid);
   }
   if (pollRet > 0) {
      if (read(fp[0], &setupOK, sizeof(setupOK)) == sizeof(setupOK)) {
         // now we wait for the callback to be (successfully) established
         if (setupOK > 0) {
            // Receive path of the log file
            int lfout = setupOK;
            char *buf = new char[lfout + 1];
            int n, nr = 0;
            for (n = 0; n < lfout; n += nr) {
               while ((nr = read(fp[0], buf + n, lfout - n)) == -1 && errno == EINTR)
                  errno = 0;   // probably a SIGCLD that was caught
               if (nr == 0)
                  break;          // EOF
               if (nr < 0) {
                  // Failure
                  setupOK= -1;
                  emsg += ": failure receiving logfile path";
                  break;
               }
            }
            if (setupOK > 0) {
               buf[lfout] = 0;
               xps->SetFileout(buf);
               // Set also the session tag
               XrdOucString stag(buf);
               stag.erase(stag.rfind('/'));
               stag.erase(0, stag.find("session-") + strlen("session-"));
               xps->SetTag(stag.c_str());
            }
            delete[] buf;
         } else {
            emsg += ": proofserv startup failed";
         }
      } else {
         emsg += ": problems receiving status-of-setup after forking";
      }
   } else {
      if (pollRet == 0) {
         emsg += ": timed-out receiving status-of-setup from pipe";
      } else {
         emsg += ": failed to receive status-of-setup from pipe";
      }
   }

   // Cleanup
   close(fp[0]);
   close(fp[1]);

   // Notify to user
   if (setupOK > 0) {
      if (p->ConnType() == kXPD_ClientMaster) {
         // Send also back the data pool url
         XrdOucString dpu = fMgr->PoolURL();
         if (!dpu.endswith('/'))
            dpu += '/';
         dpu += fMgr->NameSpace();
         response->Send(psid, xps->ROOT()->SrvProtVers(), (kXR_int16)XPROOFD_VERSBIN,
                             (void *) dpu.c_str(), dpu.length());
      } else
         response->Send(psid, xps->ROOT()->SrvProtVers(), (kXR_int16)XPROOFD_VERSBIN);
   } else {
      // Failure
      emsg += ": failure setting up proofserv" ;
      xps->Reset();
      XrdProofdAux::KillProcess(pid, 1, p->Client()->UI(), fMgr->ChangeOwn());
      response->Send(kXR_ServerError, emsg.c_str());
      return rc;
   }
   // UNIX Socket is saved now
   p->Client()->SetUNIXSockSaved();

   // now we wait for the callback to be (successfully) established
   TRACEP(p, respid, FORK, "Create: server launched: wait for callback ");

#if 0
   // We will get back a peer to initialize a link
   XrdNetPeer peerpsrv;
   XrdLink   *linkpsrv = 0;
   int lnkopts = 0;

   // Perform regular accept
   if (!(p->Client()->UNIXSock()->Accept(peerpsrv, XRDNET_NODNTRIM, fInternalWait))) {

      // We need the right privileges to do this
      XrdOucString msg("did not receive callback: ");
      if (XrdProofdAux::KillProcess(pid, 1, p->Client()->UI(), fMgr->ChangeOwn()) != 0)
         msg += "process could not be killed";
      else
         msg += "process killed";
      response->Send(kXR_attn, kXPD_errmsg, (char *) msg.c_str(), msg.length());

      xps->Reset();
      return rc;
   }
   // Make sure we have the full host name
   if (peerpsrv.InetName) {
      char *ptmp = peerpsrv.InetName;
      peerpsrv.InetName = XrdNetDNS::getHostName("localhost");
      free(ptmp);
   }

   // Allocate a new network object
   if (!(linkpsrv = XrdLink::Alloc(peerpsrv, lnkopts))) {

      // We need the right privileges to do this
      XrdOucString msg("could not allocate network object: ");
      if (XrdProofdAux::KillProcess(pid, 0, p->Client()->UI(), fMgr->ChangeOwn()) != 0)
         msg += "process could not be killed";
      else
         msg += "process killed";
      response->Send(kXR_attn, kXPD_errmsg, (char *) msg.c_str(), msg.length());

      xps->Reset();
      return rc;

   } else {

      // Keep buffer after object goes away
      peerpsrv.InetBuff = 0;
      TRACEP(p, respid, DBG, "Accepted connection from " << peerpsrv.InetName);

      // Get a protocol object off the stack (if none, allocate a new one)
      XrdProtocol *xp = p->Match(linkpsrv);
      if (!xp) {

         // We need the right privileges to do this
         XrdOucString msg("match failed: protocol error: ");
         if (XrdProofdAux::KillProcess(pid, 0, p->Client()->UI(), fMgr->ChangeOwn()) != 0)
            msg += "process could not be killed";
         else
            msg += "process killed";
         response->Send(kXR_attn, kXPD_errmsg, (char *) msg.c_str(), msg.length());

         linkpsrv->Close();
         xps->Reset();
         return rc;
      }

      // Take a short-cut and process the initial request as a sticky request
      xp->Process(linkpsrv);
      if (xp->Process(linkpsrv) != 1) {
         // We need the right privileges to do this
         XrdOucString msg("handshake with internal link failed: ");
         if (XrdProofdAux::KillProcess(pid, 0, p->Client()->UI(), fMgr->ChangeOwn()) != 0)
            msg += "process could not be killed";
         else
            msg += "process killed";
         response->Send(kXR_attn, kXPD_errmsg, (char *) msg.c_str(), msg.length());

         linkpsrv->Close();
         xps->Reset();
         return rc;
      }

      // Attach this link to the appropriate poller and enable it.
      if (!XrdPoll::Attach(linkpsrv)) {

         // We need the right privileges to do this
         XrdOucString msg("could not attach new internal link to poller: ");
         if (XrdProofdAux::KillProcess(pid, 0, p->Client()->UI(), fMgr->ChangeOwn()) != 0)
            msg += "process could not be killed";
         else
            msg += "process killed";
         response->Send(kXR_attn, kXPD_errmsg, (char *) msg.c_str(), msg.length());

         linkpsrv->Close();
         xps->Reset();
         return rc;
      }

      // Tight this protocol instance to the link
      linkpsrv->setProtocol(xp);

      // Schedule it
      fSched->Schedule((XrdJob *)linkpsrv);
   }

   // Set ID
   xps->SetSrv(pid);
#else

   // Set ID
   xps->SetSrv(pid);

   if (Accept(p, xps) != 0) {
      TRACEP(p, respid, XERR, "Create: problems accepting callback");
      return rc;
   }

#endif

   // Set the group, if any
   xps->SetGroup(p->Client()->Group());

   // Change child process priority, if required
   int dp = 0;
   if (fMgr->PriorityMgr()->SetProcessPriority(xps->SrvPID(),
                                                        p->Client()->User(), dp) != 0) {
      TRACEP(p, respid, XERR, "Create: problems changing child process priority");
   } else if (dp > 0) {
      TRACEP(p, respid, DBG, "Create: priority of the child process changed by " << dp << " units");
   }

   XrdClientID *cid = xps->Parent();
   TRACEP(p, respid, DBG, "Create: xps: "<<xps<<", ClientID: "<<(int *)cid<<" (sid: "<<sid<<")");

   // Record this session in the client sandbox
   if (p->Client()->Sandbox()->AddSession(xps->Tag()) == -1)
      TRACEP(p, respid, REQ, "Create: problems recording session in sandbox");

#if 0
   // Add to the list
   fSessions.push_back(xps);
#else
   // Update the global session handlers
   XrdOucString key; key += pid;
   fSessions.Add(key.c_str(), xps, 0, Hash_keepdata);
   fActiveSessions.push_back(xps);
   AddSession(pid, p->Client());
#endif

   // Over
   return rc;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::Accept(XrdProofdProtocol *p, XrdProofServProxy *xps)
{
   // Accept a callback from a starting-up server

   int rc = 1;
   XPD_SETRESP(p, "Accept");

   // We will get back a peer to initialize a link
   XrdNetPeer peerpsrv;
   XrdLink   *linkpsrv = 0;
   int lnkopts = 0;

   // Check inputs
   if (!p || !xps || !xps->IsValid()) {
      return -1;
   }

   int pid = xps->SrvPID();
 
   // Perform regular accept
   if (!(p->Client()->UNIXSock()->Accept(peerpsrv, XRDNET_NODNTRIM, fInternalWait))) {

      // We need the right privileges to do this
      XrdOucString msg("did not receive callback: ");
      if (XrdProofdAux::KillProcess(pid, 1, p->Client()->UI(), fMgr->ChangeOwn()) != 0)
         msg += "process could not be killed";
      else
         msg += "process killed";
      response->Send(kXR_attn, kXPD_errmsg, (char *) msg.c_str(), msg.length());

      xps->Reset();
      return -1;
   }
   // Make sure we have the full host name
   if (peerpsrv.InetName) {
      char *ptmp = peerpsrv.InetName;
      peerpsrv.InetName = XrdNetDNS::getHostName("localhost");
      free(ptmp);
   }

   // Allocate a new network object
   if (!(linkpsrv = XrdLink::Alloc(peerpsrv, lnkopts))) {

      // We need the right privileges to do this
      XrdOucString msg("could not allocate network object: ");
      if (XrdProofdAux::KillProcess(pid, 0, p->Client()->UI(), fMgr->ChangeOwn()) != 0)
         msg += "process could not be killed";
      else
         msg += "process killed";
      response->Send(kXR_attn, kXPD_errmsg, (char *) msg.c_str(), msg.length());

      xps->Reset();
      return -1;

   } else {

      // Keep buffer after object goes away
      peerpsrv.InetBuff = 0;
      TRACEP(p, respid, DBG, "Accepted connection from " << peerpsrv.InetName);

      // Get a protocol object off the stack (if none, allocate a new one)
      XrdProtocol *xp = p->Match(linkpsrv);
      if (!xp) {

         // We need the right privileges to do this
         XrdOucString msg("match failed: protocol error: ");
         if (XrdProofdAux::KillProcess(pid, 0, p->Client()->UI(), fMgr->ChangeOwn()) != 0)
            msg += "process could not be killed";
         else
            msg += "process killed";
         response->Send(kXR_attn, kXPD_errmsg, (char *) msg.c_str(), msg.length());

         linkpsrv->Close();
         xps->Reset();
         return -1;
      }

      // Take a short-cut and process the initial request as a sticky request
      xp->Process(linkpsrv);
      if (xp->Process(linkpsrv) != 1) {
         // We need the right privileges to do this
         XrdOucString msg("handshake with internal link failed: ");
         if (XrdProofdAux::KillProcess(pid, 0, p->Client()->UI(), fMgr->ChangeOwn()) != 0)
            msg += "process could not be killed";
         else
            msg += "process killed";
         response->Send(kXR_attn, kXPD_errmsg, (char *) msg.c_str(), msg.length());

         linkpsrv->Close();
         xps->Reset();
         return -1;
      }

      // Attach this link to the appropriate poller and enable it.
      if (!XrdPoll::Attach(linkpsrv)) {

         // We need the right privileges to do this
         XrdOucString msg("could not attach new internal link to poller: ");
         if (XrdProofdAux::KillProcess(pid, 0, p->Client()->UI(), fMgr->ChangeOwn()) != 0)
            msg += "process could not be killed";
         else
            msg += "process killed";
         response->Send(kXR_attn, kXPD_errmsg, (char *) msg.c_str(), msg.length());

         linkpsrv->Close();
         xps->Reset();
         return -1;
      }

      // Tight this protocol instance to the link
      linkpsrv->setProtocol(xp);

      // Schedule it
      fSched->Schedule((XrdJob *)linkpsrv);
   }

   // Done
   return 0;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::Detach(XrdProofdProtocol *p)
{
   // Handle a request to detach from an existing session

   int psid = -1, rc = 1;
   XPD_SETRESP(p, "Detach");

   // Unmarshall the data
   psid = ntohl(p->Request()->proof.sid);
   TRACEP(p, respid, REQ, "Detach: psid: "<<psid);

   // Find server session
   XrdProofServProxy *xps = 0;
   if (!p->Client() || !(xps = p->Client()->GetProofServ(psid))) {
      TRACEP(p, respid, XERR, "Detach: session ID not found");
      response->Send(kXR_InvalidRequest,"session ID not found");
      return rc;
   }
#if 0
   TRACEP(p, respid, DBG, "Detach: xps: "<<xps<<", status: "<< xps->Status()<<
               ", # clients: "<< xps->Clients()->size());

   // Remove this from the list of clients
   std::vector<XrdClientID *>::iterator i;
   for (i = xps->Clients()->begin(); i != xps->Clients()->end(); ++i) {
      if (*i) {
         if ((*i)->P() == p) {
            delete (*i);
            xps->Clients()->erase(i);
            break;
         }
      }
   }
#else
   xps->FreeClientID(p);
#endif

   // Notify to user
   response->Send();

   return rc;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::Destroy(XrdProofdProtocol *p)
{
   // Handle a request to shutdown an existing session

   int psid = -1, rc = 1;
   XPD_SETRESP(p, "Destroy");

   // Unmarshall the data
   psid = ntohl(p->Request()->proof.sid);
   TRACEP(p, respid, REQ, "Destroy: psid: "<<psid);

   // Find server session
   XrdProofServProxy *xpsref = 0;
   if (psid > -1) {
      // Request for a specific session
      if (!p->Client() || !(xpsref = p->Client()->GetProofServ(psid))) {
         TRACEP(p, respid, XERR, "Destroy: reference session ID not found");
         response->Send(kXR_InvalidRequest,"reference session ID not found");
         return rc;
      }
   }

   // Terminate the servers
   std::list<int> sigpid;
   XrdOucString msg("Destroy: sessions destroyed by: ");
   msg += p->Link()->ID;
   p->Client()->Broadcast(msg.c_str());
   p->Client()->TerminateSessions(1, kXPD_AnyServer, xpsref, msg.c_str(), sigpid);

   // Notify to user
   response->Send();

   // Over
   return rc;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::SetProofServEnvOld(XrdProofdProtocol *p,
                                              int psid, int loglevel, const char *cfg)
{
   // Set environment for proofserv; old version preparing the environment for
   // proofserv protocol version <= 13. Needed for backward compatibility.

   char *ev = 0;

   MTRACE(REQ,  "xpd:child: ", "SetProofServEnv: enter: psid: "<<psid<<
                      ", log: "<<loglevel);

   // Make sure the principal client is defined
   if (!p->Client()) {
      MTRACE(XERR, "xpd:child: ",
                   "SetProofServEnv: principal client undefined - cannot continue");
      return -1;
   }

   // Set basic environment for proofserv
   if (SetProofServEnv(fMgr, p->Client()->ROOT()) != 0) {
      MTRACE(XERR, "xpd:child: ",
                   "SetProofServEnvOld: problems setting basic environment - exit");
      return -1;
   }

   // Session proxy
   XrdProofServProxy *xps = p->Client()->GetProofServ(psid);
   if (!xps) {
      MTRACE(XERR, "xpd:child: ",
                   "SetProofServEnvOld: unable to get instance of proofserv proxy");
      return -1;
   }

   // Work directory
   XrdOucString udir = p->Client()->Sandbox()->Dir();
   MTRACE(DBG, "xpd:child: ",
               "SetProofServEnvOld: working dir for "<<p->Client()->User()<<" is: "<<udir);

   // Session tag
   char hn[64], stag[512];
#if defined(XPD__SUNCC)
   sysinfo(SI_HOSTNAME, hn, sizeof(hn));
#else
   gethostname(hn, sizeof(hn));
#endif
   XrdOucString host = hn;
   if (host.find(".") != STR_NPOS)
      host.erase(host.find("."));
   sprintf(stag,"%s-%d-%d",host.c_str(),(int)time(0),getpid());

   // Session dir
   XrdOucString logdir = udir;
   if (p->ConnType() == kXPD_ClientMaster) {
      logdir += "/session-";
      logdir += stag;
      xps->SetTag(stag);
   } else {
      logdir += "/";
      logdir += xps->Tag();
   }
   MTRACE(DBG, "xpd:child: ", "SetProofServEnvOld: log dir "<<logdir);
   // Make sure the directory exists
   if (XrdProofdAux::AssertDir(logdir.c_str(), p->Client()->UI(), fMgr->ChangeOwn()) == -1) {
      MTRACE(XERR, "xpd:child: ",
                   "SetProofServEnvOld: unable to create log dir: "<<logdir);
      return -1;
   }
   // The session dir (sandbox) depends on the role
   XrdOucString sessdir = logdir;
   if (p->ConnType() == kXPD_MasterWorker)
      sessdir += "/worker-";
   else
      sessdir += "/master-";
   sessdir += xps->Ordinal();
   sessdir += "-";
   sessdir += stag;
   ev = new char[strlen("ROOTPROOFSESSDIR=")+sessdir.length()+2];
   sprintf(ev, "ROOTPROOFSESSDIR=%s", sessdir.c_str());
   putenv(ev);
   MTRACE(DBG,  "xpd:child: ", "SetProofServEnvOld: "<<ev);

   // Log level
   ev = new char[strlen("ROOTPROOFLOGLEVEL=")+5];
   sprintf(ev, "ROOTPROOFLOGLEVEL=%d", loglevel);
   putenv(ev);
   MTRACE(DBG, "xpd:child: ", "SetProofServEnvOld: "<<ev);

   // Ordinal number
   ev = new char[strlen("ROOTPROOFORDINAL=")+strlen(xps->Ordinal())+2];
   sprintf(ev, "ROOTPROOFORDINAL=%s", xps->Ordinal());
   putenv(ev);
   MTRACE(DBG, "xpd:child: ", "SetProofServEnvOld: "<<ev);

   // ROOT Version tag if not the default one
   ev = new char[strlen("ROOTVERSIONTAG=")+strlen(p->Client()->ROOT()->Tag())+2];
   sprintf(ev, "ROOTVERSIONTAG=%s", p->Client()->ROOT()->Tag());
   putenv(ev);
   MTRACE(DBG, "xpd:child: ", "SetProofServEnvOld: "<<ev);

   // Create the env file
   MTRACE(DBG, "xpd:child: ", "SetProofServEnvOld: creating env file");
   XrdOucString envfile = sessdir;
   envfile += ".env";
   FILE *fenv = fopen(envfile.c_str(), "w");
   if (!fenv) {
      MTRACE(XERR, "xpd:child: ",
                  "SetProofServEnvOld: unable to open env file: "<<envfile);
      return -1;
   }
   MTRACE(DBG, "xpd:child: ",
               "SetProofServEnvOld: environment file: "<< envfile);

   // Forwarded sec credentials, if any
   if (p->AuthProt()) {

      // Additional envs possibly set by the protocol for next application
      XrdOucString secenvs(getenv("XrdSecENVS"));
      if (secenvs.length() > 0) {
         // Go through the list
         XrdOucString env;
         int from = 0;
         while ((from = secenvs.tokenize(env, from, ',')) != -1) {
            if (env.length() > 0) {
               // Set the env now
               ev = new char[env.length()+1];
               strncpy(ev, env.c_str(), env.length());
               ev[env.length()] = 0;
               putenv(ev);
               fprintf(fenv, "%s\n", ev);
               MTRACE(DBG, "xpd:child: ", "SetProofServEnvOld: "<<ev);
            }
         }
      }

      // The credential buffer, if any
      XrdSecCredentials *creds = p->AuthProt()->getCredentials();
      if (creds) {
         int lev = strlen("XrdSecCREDS=")+creds->size;
         ev = new char[lev+1];
         strcpy(ev, "XrdSecCREDS=");
         memcpy(ev+strlen("XrdSecCREDS="), creds->buffer, creds->size);
         ev[lev] = 0;
         putenv(ev);
         MTRACE(DBG, "xpd:child: ", "SetProofServEnvOld: XrdSecCREDS set");

         // If 'pwd', save AFS key, if any
         if (!strncmp(p->AuthProt()->Entity.prot, "pwd", 3)) {
            XrdOucString credsdir = udir;
            credsdir += "/.creds";
            // Make sure the directory exists
            if (!XrdProofdAux::AssertDir(credsdir.c_str(), p->Client()->UI(), fMgr->ChangeOwn())) {
               if (SaveAFSkey(creds, credsdir.c_str()) == 0) {
                  ev = new char[strlen("ROOTPROOFAFSCREDS=")+credsdir.length()+strlen("/.afs")+2];
                  sprintf(ev, "ROOTPROOFAFSCREDS=%s/.afs", credsdir.c_str());
                  putenv(ev);
                  fprintf(fenv, "ROOTPROOFAFSCREDS has been set\n");
                  MTRACE(DBG, "xpd:child: ", "SetProofServEnvOld: " << ev);
               } else {
                  MTRACE(DBG, "xpd:child: ", "SetProofServEnvOld: problems in saving AFS key");
               }
            } else {
               MTRACE(XERR, "xpd:child: ",
                            "SetProofServEnvOld: unable to create creds dir: "<<credsdir);
               return -1;
            }
         }
      }
   }

   // Set ROOTSYS
   fprintf(fenv, "ROOTSYS=%s\n", xps->ROOT()->Dir());

   // Set conf dir
   fprintf(fenv, "ROOTCONFDIR=%s\n", xps->ROOT()->Dir());

   // Set TMPDIR
   fprintf(fenv, "ROOTTMPDIR=%s\n", fMgr->TMPdir());

   // Port (really needed?)
   fprintf(fenv, "ROOTXPDPORT=%d\n", fMgr->Port());

   // Work dir
   fprintf(fenv, "ROOTPROOFWORKDIR=%s\n", udir.c_str());

   // Session tag
   fprintf(fenv, "ROOTPROOFSESSIONTAG=%s\n", stag);

   // Whether user specific config files are enabled
   if (fMgr->NetMgr()->WorkerUsrCfg())
      fprintf(fenv, "ROOTUSEUSERCFG=1\n");

   // Set Open socket
   fprintf(fenv, "ROOTOPENSOCK=%s\n", p->Client()->UNIXSockPath());

   // Entity
   fprintf(fenv, "ROOTENTITY=%s@%s\n", p->Client()->User(), p->Link()->Host());

   // Session ID
   fprintf(fenv, "ROOTSESSIONID=%d\n", psid);

   // Client ID
   fprintf(fenv, "ROOTCLIENTID=%d\n", p->CID());

   // Client Protocol
   fprintf(fenv, "ROOTPROOFCLNTVERS=%d\n", p->ProofProtocol());

   // Ordinal number
   fprintf(fenv, "ROOTPROOFORDINAL=%s\n", xps->Ordinal());

   // ROOT version tag if different from the default one
   if (getenv("ROOTVERSIONTAG"))
      fprintf(fenv, "ROOTVERSIONTAG=%s\n", getenv("ROOTVERSIONTAG"));

   // Config file
   if (cfg && strlen(cfg) > 0)
      fprintf(fenv, "ROOTPROOFCFGFILE=%s\n", cfg);

   // Log file in the log dir
   XrdOucString logfile = sessdir;
   logfile += ".log";
   fprintf(fenv, "ROOTPROOFLOGFILE=%s\n", logfile.c_str());
   xps->SetFileout(logfile.c_str());

   // Additional envs (xpd.putenv directive)
   if (fProofServEnvs.length() > 0) {
      // Go through the list
      XrdOucString env;
      int from = 0;
      while ((from = fProofServEnvs.tokenize(env, from, ',')) != -1) {
         if (env.length() > 0) {
            // Resolve keywords
            fMgr->ResolveKeywords(env, p->Client());
            // Set the env now
            ev = new char[env.length()+1];
            strncpy(ev, env.c_str(), env.length());
            ev[env.length()] = 0;
            putenv(ev);
            fprintf(fenv, "%s\n", ev);
            MTRACE(DBG, "xpd:child: ", "SetProofServEnvOld: "<<ev);
         }
      }
   }

   // Set the user envs
   if (xps->UserEnvs() &&
       strlen(xps->UserEnvs()) && strstr(xps->UserEnvs(),"=")) {
      // The single components
      XrdOucString ue = xps->UserEnvs();
      XrdOucString env, namelist;
      int from = 0, ieq = -1;
      while ((from = ue.tokenize(env, from, ',')) != -1) {
         if (env.length() > 0 && (ieq = env.find('=')) != -1) {
            ev = new char[env.length()+1];
            strncpy(ev, env.c_str(), env.length());
            ev[env.length()] = 0;
            putenv(ev);
            fprintf(fenv, "%s\n", ev);
            MTRACE(DBG, "xpd:child: ", "SetProofServEnvOld: "<<ev);
            env.erase(ieq);
            if (namelist.length() > 0)
               namelist += ',';
            namelist += env;
         }
      }
      // The list of names, ','-separated
      ev = new char[strlen("PROOF_ALLVARS=") + namelist.length() + 2];
      sprintf(ev, "PROOF_ALLVARS=%s", namelist.c_str());
      putenv(ev);
      fprintf(fenv, "%s\n", ev);
      MTRACE(DBG, "xpd:child: ", "SetProofServEnvOld: "<<ev);
   }

   // Close file
   fclose(fenv);

   // Create or Update symlink to last session
   TRACE(DBG, "SetProofServEnvOld: creating symlink");
   XrdOucString syml = udir;
   if (p->ConnType() == kXPD_MasterWorker)
      syml += "/last-worker-session";
   else
      syml += "/last-master-session";
   if (XrdProofdAux::SymLink(logdir.c_str(), syml.c_str()) != 0) {
      MTRACE(XERR, "xpd:child: ",
                   "SetProofServEnvOld: problems creating symlink to "
                    " last session (errno: "<<errno<<")");
   }

   // We are done
   MTRACE(DBG, "xpd:child: ", "SetProofServEnvOld: done");
   return 0;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::SetProofServEnv(XrdProofdManager *mgr, XrdROOT *r)
{
   // Set basic environment accordingly to 'r'

   char *ev = 0;

   MTRACE(REQ, "xpd:child: ",
               "SetProofServEnv: enter: ROOT dir: "<< (r ? r->Dir() : "*** undef ***"));

   if (r) {
      char *rootsys = (char *) r->Dir();
#ifndef ROOTLIBDIR
      char *ldpath = 0;
      if (mgr->BareLibPath() && strlen(mgr->BareLibPath()) > 0) {
         ldpath = new char[32 + strlen(rootsys) + strlen(mgr->BareLibPath())];
         sprintf(ldpath, "%s=%s/lib:%s", XPD_LIBPATH, rootsys, mgr->BareLibPath());
      } else {
         ldpath = new char[32 + strlen(rootsys)];
         sprintf(ldpath, "%s=%s/lib", XPD_LIBPATH, rootsys);
      } 
      putenv(ldpath);
#endif
      // Set ROOTSYS
      ev = new char[15 + strlen(rootsys)];
      sprintf(ev, "ROOTSYS=%s", rootsys);
      putenv(ev);

      // Set conf dir
      ev = new char[20 + strlen(rootsys)];
      sprintf(ev, "ROOTCONFDIR=%s", rootsys);
      putenv(ev);

      // Set TMPDIR
      ev = new char[20 + strlen(mgr->TMPdir())];
      sprintf(ev, "TMPDIR=%s", mgr->TMPdir());
      putenv(ev);

      // Done
      return 0;
   }

   // Bad input
   MTRACE(REQ,  "xpd:child: ", "SetProofServEnv: XrdROOT instance undefined!");
   return -1;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::SetProofServEnv(XrdProofdProtocol *p,
                                           int psid, int loglevel, const char *cfg)
{
   // Set environment for proofserv

   char *ev = 0;

   MTRACE(REQ,  "xpd:child: ", "SetProofServEnv: enter: psid: "<<psid<<
                      ", log: "<<loglevel);

   // Make sure the principal client is defined
   if (!p->Client()) {
      MTRACE(XERR, "xpd:child: ",
                   "SetProofServEnv: principal client undefined - cannot continue");
      return -1;
   }

   // Old proofservs expect different settings
   int rootvers = p->Client()->ROOT() ? p->Client()->ROOT()->SrvProtVers() : -1;
   MTRACE(DBG,  "xpd:child: ", "SetProofServEnv: rootvers: "<< rootvers);
   if (rootvers < 14 && rootvers > -1)
      return SetProofServEnvOld(p, psid, loglevel, cfg);

   // Session proxy
   XrdProofServProxy *xps = p->Client()->GetProofServ(psid);
   if (!xps) {
      MTRACE(XERR, "xpd:child: ",
                   "SetProofServEnv: unable to get instance of proofserv proxy");
      return -1;
   }

   // Client sandbox
   XrdOucString udir = p->Client()->Sandbox()->Dir();
   MTRACE(DBG, "xpd:child: ",
               "SetProofServEnv: sandbox for "<<p->Client()->User()<<" is: "<<udir);

   // Create and log into the directory reserved to this session:
   // the unique tag will identify it
   char hn[64], stag[512];
#if defined(XPD__SUNCC)
   sysinfo(SI_HOSTNAME, hn, sizeof(hn));
#else
   gethostname(hn, sizeof(hn));
#endif
   XrdOucString host = hn;
   if (host.find(".") != STR_NPOS)
      host.erase(host.find("."));
   sprintf(stag,"%s-%d-%d",host.c_str(),(int)time(0),getpid());

   // Session dir
   XrdOucString topstag = stag;
   XrdOucString sessiondir = udir;
   if (p->ConnType() == kXPD_ClientMaster) {
      sessiondir += "/session-";
      sessiondir += stag;
      xps->SetTag(stag);
   } else {
      sessiondir += "/";
      sessiondir += xps->Tag();
      topstag = xps->Tag();
      topstag.replace("session-","");
   }
   MTRACE(DBG, "xpd:child: ", "SetProofServEnv: session dir "<<sessiondir);
   // Make sure the directory exists ...
   if (XrdProofdAux::AssertDir(sessiondir.c_str(), p->Client()->UI(), fMgr->ChangeOwn()) == -1) {
      MTRACE(XERR, "xpd:child: ",
                   "SetProofServEnv: unable to create log dir: "<<sessiondir);
      return -1;
   }
   // ... and log into it
   if (XrdProofdAux::ChangeToDir(sessiondir.c_str(), p->Client()->UI(), fMgr->ChangeOwn()) != 0) {
      MTRACE(XERR, "xpd:child: ", "SetProofServEnv: couldn't change directory to "<<
                   sessiondir);
      return -1;
   }

   // Set basic environment for proofserv
   if (SetProofServEnv(fMgr, p->Client()->ROOT()) != 0) {
      MTRACE(XERR, "xpd:child: ",
                   "SetProofServEnv: problems setting basic environment - exit");
      return -1;
   }

   // The session working dir depends on the role
   XrdOucString swrkdir = sessiondir;
   if (p->ConnType() == kXPD_MasterWorker)
      swrkdir += "/worker-";
   else
      swrkdir += "/master-";
   swrkdir += xps->Ordinal();
   swrkdir += "-";
   swrkdir += stag;

   // Create the rootrc and env files
   MTRACE(DBG, "xpd:child: ", "SetProofServEnv: creating env file");
   XrdOucString rcfile = swrkdir;
   rcfile += ".rootrc";
   FILE *frc = fopen(rcfile.c_str(), "w");
   if (!frc) {
      MTRACE(XERR, "xpd:child: ",
                  "SetProofServEnv: unable to open rootrc file: "<<rcfile);
      return -1;
   }
   // Symlink to session.rootrc
   if (XrdProofdAux::SymLink(rcfile.c_str(), "session.rootrc") != 0) {
      MTRACE(XERR, "xpd:child: ",
                   "SetProofServEnv: problems creating symlink to"
                    "'session.rootrc' (errno: "<<errno<<")");
   }
   MTRACE(DBG, "xpd:child: ",
               "SetProofServEnv: session rootrc file: "<< rcfile);

   // Port
   fprintf(frc,"# XrdProofdProtocol listening port\n");
   fprintf(frc, "ProofServ.XpdPort: %d\n", fMgr->Port());

   // Local root prefix
   if (fMgr->LocalROOT() && strlen(fMgr->LocalROOT()) > 0) {
      fprintf(frc,"# Prefix to be prepended to local paths\n");
      fprintf(frc, "Path.Localroot: %s\n", fMgr->LocalROOT());
   }

   // Data pool entry-point URL
   if (fMgr->PoolURL() && strlen(fMgr->PoolURL()) > 0) {
      XrdOucString purl(fMgr->PoolURL());
      if (!purl.endswith("/"))
         purl += "/";
      fprintf(frc,"# URL for the data pool entry-point\n");
      fprintf(frc, "ProofServ.PoolUrl: %s\n", purl.c_str());
   }

   // The session working dir depends on the role
   fprintf(frc,"# The session working dir\n");
   fprintf(frc,"ProofServ.SessionDir: %s\n", swrkdir.c_str());

   // Log / Debug level
   fprintf(frc,"# Proof Log/Debug level\n");
   fprintf(frc,"Proof.DebugLevel: %d\n", loglevel);

   // Ordinal number
   fprintf(frc,"# Ordinal number\n");
   fprintf(frc,"ProofServ.Ordinal: %s\n", xps->Ordinal());

   // ROOT Version tag
   if (p->Client()->ROOT()) {
      fprintf(frc,"# ROOT Version tag\n");
      fprintf(frc,"ProofServ.RootVersionTag: %s\n", p->Client()->ROOT()->Tag());
   }
   // Proof group
   if (p->Client()->Group()) {
      fprintf(frc,"# Proof group\n");
      fprintf(frc,"ProofServ.ProofGroup: %s\n", p->Client()->Group());
      // On master we should have the data set dir
      if ((fMgr->SrvType() == kXPD_ClientMaster || fMgr->SrvType() == kXPD_AnyServer)) {
         const char *dsetdir = p->Client()->Sandbox()->DSetDir();
         if (dsetdir && strlen(dsetdir) > 0) {
            fprintf(frc,"# User's dataset dir\n");
            fprintf(frc,"ProofServ.DataSetDir: %s\n", dsetdir);
         }
         // Export also the dataset root (for location purposes)
         fprintf(frc,"# Global root for datasets\n");
         fprintf(frc,"ProofServ.DataSetRoot: %s\n", fMgr->DataSetDir());
      }
   }

   //  Path to file with group information
   if (fMgr->GroupsMgr() && fMgr->GroupsMgr()->GetCfgFile()) {
      fprintf(frc,"# File with group information\n");
      fprintf(frc, "ProofDataSetManager.GroupFile: %s\n", fMgr->GroupsMgr()->GetCfgFile());
   }

   // Work dir
   fprintf(frc,"# Users sandbox\n");
   fprintf(frc, "ProofServ.Sandbox: %s\n", udir.c_str());

   // Image
   if (fMgr->Image() && strlen(fMgr->Image()) > 0) {
      fprintf(frc,"# Server image\n");
      fprintf(frc, "ProofServ.Image: %s\n", fMgr->Image());
   }

   // Session tag
   fprintf(frc,"# Session tag\n");
   fprintf(frc, "ProofServ.SessionTag: %s\n", topstag.c_str());

   // Whether user specific config files are enabled
   if (fMgr->NetMgr()->WorkerUsrCfg()) {
      fprintf(frc,"# Whether user specific config files are enabled\n");
      fprintf(frc, "ProofServ.UseUserCfg: 1\n");
   }
   // Set Open socket
   fprintf(frc,"# Open socket\n");
   fprintf(frc, "ProofServ.OpenSock: %s\n", p->Client()->UNIXSockPath());
   // Entity
   fprintf(frc,"# Entity\n");
   if (p->Client()->UI().fGroup.length() > 0)
      fprintf(frc, "ProofServ.Entity: %s:%s@%s\n",
              p->Client()->User(), p->Client()->UI().fGroup.c_str(), p->Link()->Host());
   else
      fprintf(frc, "ProofServ.Entity: %s@%s\n", p->Client()->User(), p->Link()->Host());


   // Session ID
   fprintf(frc,"# Session ID\n");
   fprintf(frc, "ProofServ.SessionID: %d\n", psid);

   // Client ID
   fprintf(frc,"# Client ID\n");
   fprintf(frc, "ProofServ.ClientID: %d\n", p->CID());

   // Client Protocol
   fprintf(frc,"# Client Protocol\n");
   fprintf(frc, "ProofServ.ClientVersion: %d\n", p->ProofProtocol());

   // Config file
   if (cfg && strlen(cfg) > 0) {
      fprintf(frc,"# Config file\n");
      // User defined
      fprintf(frc, "ProofServ.ProofConfFile: %s\n", cfg);
   } else {
      fprintf(frc,"# Config file\n");
      if (fMgr->IsSuperMst()) {
         fprintf(frc,"# Config file\n");
         fprintf(frc, "ProofServ.ProofConfFile: sm:\n");
      } else if (fMgr->ProofPlugin() && strlen(fMgr->ProofPlugin())) {
         fprintf(frc,"# Config file\n");
         fprintf(frc, "ProofServ.ProofConfFile: %s\n", fMgr->ProofPlugin());
      }
   }

   // Additional rootrcs (xpd.putrc directive)
   if (fProofServRCs.length() > 0) {
      fprintf(frc,"# Additional rootrcs (xpd.putrc directives)\n");
      // Go through the list
      XrdOucString rc;
      int from = 0;
      while ((from = fProofServRCs.tokenize(rc, from, ',')) != -1)
         if (rc.length() > 0)
            fprintf(frc, "%s\n", rc.c_str());
   }

   // Done with this
   fclose(frc);

   // Now save the exported env variables, for the record
   XrdOucString envfile = swrkdir;
   envfile += ".env";
   FILE *fenv = fopen(envfile.c_str(), "w");
   if (!fenv) {
      MTRACE(XERR, "xpd:child: ",
                  "SetProofServEnv: unable to open env file: "<<envfile);
      return -1;
   }
   MTRACE(DBG, "xpd:child: ", "SetProofServEnv: environment file: "<< envfile);

   // Forwarded sec credentials, if any
   if (p->AuthProt()) {

      // Additional envs possibly set by the protocol for next application
      XrdOucString secenvs(getenv("XrdSecENVS"));
      if (secenvs.length() > 0) {
         // Go through the list
         XrdOucString env;
         int from = 0;
         while ((from = secenvs.tokenize(env, from, ',')) != -1) {
            if (env.length() > 0) {
               // Set the env now
               ev = new char[env.length()+1];
               strncpy(ev, env.c_str(), env.length());
               ev[env.length()] = 0;
               putenv(ev);
               fprintf(fenv, "%s\n", ev);
               MTRACE(DBG, "xpd:child: ", "SetProofServEnv: "<<ev);
            }
         }
      }

      // The credential buffer, if any
      XrdSecCredentials *creds = p->AuthProt()->getCredentials();
      if (creds) {
         int lev = strlen("XrdSecCREDS=")+creds->size;
         ev = new char[lev+1];
         strcpy(ev, "XrdSecCREDS=");
         memcpy(ev+strlen("XrdSecCREDS="), creds->buffer, creds->size);
         ev[lev] = 0;
         putenv(ev);
         MTRACE(DBG, "xpd:child: ", "SetProofServEnv: XrdSecCREDS set");

         // If 'pwd', save AFS key, if any
         if (!strncmp(p->AuthProt()->Entity.prot, "pwd", 3)) {
            XrdOucString credsdir = udir;
            credsdir += "/.creds";
            // Make sure the directory exists
            if (!XrdProofdAux::AssertDir(credsdir.c_str(), p->Client()->UI(), fMgr->ChangeOwn())) {
               if (SaveAFSkey(creds, credsdir.c_str()) == 0) {
                  ev = new char[strlen("ROOTPROOFAFSCREDS=")+credsdir.length()+strlen("/.afs")+2];
                  sprintf(ev, "ROOTPROOFAFSCREDS=%s/.afs", credsdir.c_str());
                  putenv(ev);
                  fprintf(fenv, "ROOTPROOFAFSCREDS has been set\n");
                  MTRACE(DBG, "xpd:child: ", "SetProofServEnv: " << ev);
               } else {
                  MTRACE(DBG, "xpd:child: ", "SetProofServEnv: problems in saving AFS key");
               }
            } else {
               MTRACE(XERR, "xpd:child: ",
                            "SetProofServEnv: unable to create creds dir: "<<credsdir);
               return -1;
            }
         }
      }
   }

   // Library path
   fprintf(fenv, "%s=%s\n", XPD_LIBPATH, getenv(XPD_LIBPATH));

   // ROOTSYS
   fprintf(fenv, "ROOTSYS=%s\n", xps->ROOT()->Dir());

   // Conf dir
   fprintf(fenv, "ROOTCONFDIR=%s\n", xps->ROOT()->Dir());

   // TMPDIR
   fprintf(fenv, "TMPDIR=%s\n", fMgr->TMPdir());

   // ROOT version tag (needed in building packages)
   ev = new char[strlen("ROOTVERSIONTAG=")+strlen(p->Client()->ROOT()->Tag())+2];
   sprintf(ev, "ROOTVERSIONTAG=%s", p->Client()->ROOT()->Tag());
   putenv(ev);
   fprintf(fenv, "%s\n", ev);

   // Log file in the log dir
   XrdOucString logfile = swrkdir;
   logfile += ".log";
   ev = new char[strlen("ROOTPROOFLOGFILE=")+logfile.length()+2];
   sprintf(ev, "ROOTPROOFLOGFILE=%s", logfile.c_str());
   putenv(ev);
   fprintf(fenv, "%s\n", ev);
   xps->SetFileout(logfile.c_str());

   // Xrootd config file
   ev = new char[strlen("XRDCF=")+strlen(CfgFile())+2];
   sprintf(ev, "XRDCF=%s", CfgFile());
   putenv(ev);
   fprintf(fenv, "%s\n", ev);

   // Additional envs (xpd.putenv directive)
   if (fProofServEnvs.length() > 0) {
      // Go through the list
      XrdOucString env;
      int from = 0;
      while ((from = fProofServEnvs.tokenize(env, from, ',')) != -1) {
         if (env.length() > 0) {
            // Resolve keywords
            fMgr->ResolveKeywords(env, p->Client());
            // Set the env now
            ev = new char[env.length()+1];
            strncpy(ev, env.c_str(), env.length());
            ev[env.length()] = 0;
            putenv(ev);
            fprintf(fenv, "%s\n", ev);
            MTRACE(DBG, "xpd:child: ", "SetProofServEnv: "<<ev);
         }
      }
   }

   // Set the user envs
   if (xps->UserEnvs() &&
       strlen(xps->UserEnvs()) && strstr(xps->UserEnvs(),"=")) {
      // The single components
      XrdOucString ue = xps->UserEnvs();
      XrdOucString env, namelist;
      int from = 0, ieq = -1;
      while ((from = ue.tokenize(env, from, ',')) != -1) {
         if (env.length() > 0 && (ieq = env.find('=')) != -1) {
            ev = new char[env.length()+1];
            strncpy(ev, env.c_str(), env.length());
            ev[env.length()] = 0;
            putenv(ev);
            fprintf(fenv, "%s\n", ev);
            MTRACE(DBG, "xpd:child: ", "SetProofServEnv: "<<ev);
            env.erase(ieq);
            if (namelist.length() > 0)
               namelist += ',';
            namelist += env;
         }
      }
      // The list of names, ','-separated
      ev = new char[strlen("PROOF_ALLVARS=") + namelist.length() + 2];
      sprintf(ev, "PROOF_ALLVARS=%s", namelist.c_str());
      putenv(ev);
      fprintf(fenv, "%s\n", ev);
      MTRACE(DBG, "xpd:child: ", "SetProofServEnv: "<<ev);
   }

   // Close file
   fclose(fenv);

   // Create or Update symlink to last session
   TRACE(DBG, "SetProofServEnv: creating symlink");
   XrdOucString syml = udir;
   if (p->ConnType() == kXPD_MasterWorker)
      syml += "/last-worker-session";
   else
      syml += "/last-master-session";
   if (XrdProofdAux::SymLink(sessiondir.c_str(), syml.c_str()) != 0) {
      MTRACE(XERR, "xpd:child: ",
                   "SetProofServEnv: problems creating symlink to "
                    " last session (errno: "<<errno<<")");
   }

   // We are done
   MTRACE(DBG, "xpd:child: ", "SetProofServEnv: done");
   return 0;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::CleanupProofServ(bool all, const char *usr)
{
   // Cleanup (kill) all 'proofserv' processes from the process table.
   // Only the processes associated with 'usr' are killed,
   // unless 'all' is TRUE, in which case all 'proofserv' instances are
   // terminated (this requires superuser privileges).
   // Super users can also terminated all processes fo another user (specified
   // via usr).
   // Return number of process notified for termination on success, -1 otherwise

   TRACE(ACT, "CleanupProofServ: enter: all: "<<all<<
               ", usr: " << (usr ? usr : "undef"));
   int nk = 0;

   // Name
   const char *pn = "proofserv";

   // Uid
   XrdProofUI ui;
   int refuid = -1;
   if (!all) {
      if (!usr) {
         TRACE(DBG, "CleanupProofServ: usr must be defined for all = FALSE");
         return -1;
      }
      if (XrdProofdAux::GetUserInfo(usr, ui) != 0) {
         TRACE(DBG, "CleanupProofServ: problems getting info for user " << usr);
         return -1;
      }
      refuid = ui.fUid;
   }

#if defined(linux)
   // Loop over the "/proc" dir
   DIR *dir = opendir("/proc");
   if (!dir) {
      XrdOucString emsg("CleanupProofServ: cannot open /proc - errno: ");
      emsg += errno;
      TRACE(DBG, emsg.c_str());
      return -1;
   }

   struct dirent *ent = 0;
   while ((ent = readdir(dir))) {
      if (DIGIT(ent->d_name[0])) {
         XrdOucString fn("/proc/", 256);
         fn += ent->d_name;
         fn += "/status";
         // Open file
         FILE *ffn = fopen(fn.c_str(), "r");
         if (!ffn) {
            XrdOucString emsg("CleanupProofServ: cannot open file ");
            emsg += fn; emsg += " - errno: "; emsg += errno;
            TRACE(HDBG, emsg.c_str());
            continue;
         }
         // Read info
         bool xname = 1, xpid = 1, xppid = 1;
         bool xuid = (all) ? 0 : 1;
         int pid = -1;
         int ppid = -1;
         char line[2048] = { 0 };
         while (fgets(line, sizeof(line), ffn) &&
               (xname || xpid || xppid || xuid)) {
            // Check name
            if (xname && strstr(line, "Name:")) {
               if (!strstr(line, pn))
                  break;
               xname = 0;
            }
            if (xpid && strstr(line, "Pid:")) {
               pid = (int) XrdProofdAux::GetLong(&line[strlen("Pid:")]);
               xpid = 0;
            }
            if (xppid && strstr(line, "PPid:")) {
               ppid = (int) XrdProofdAux::GetLong(&line[strlen("PPid:")]);
               // Parent process must be us or be dead
               if (ppid != getpid() &&
                   XrdProofdAux::VerifyProcessByID(ppid, "xrootd"))
                  // Process created by another running xrootd
                  break;
               xppid = 0;
            }
            if (xuid && strstr(line, "Uid:")) {
               int uid = (int) XrdProofdAux::GetLong(&line[strlen("Uid:")]);
               if (refuid == uid)
                  xuid = 0;
            }
         }
         // Close the file
         fclose(ffn);
         // If this is a good candidate, kill it
         if (!xname && !xpid && !xppid && !xuid) {

            bool muok = 1;
            if (fMgr->MultiUser() && !all) {
               // We need to check the user name: we may be the owner of somebody
               // else process; if not session is attached, we kill it
               muok = 0;
               XrdProofServProxy *srv = GetActiveSession(pid);
               if (!srv || (srv && !strcmp(usr, srv->Client())))
                  muok = 1;
            }
            if (muok)
               if (XrdProofdAux::KillProcess(pid, 1, ui, fMgr->ChangeOwn()) == 0)
                  nk++;
         }
      }
   }
   // Close the directory
   closedir(dir);

#elif defined(__sun)

   // Loop over the "/proc" dir
   DIR *dir = opendir("/proc");
   if (!dir) {
      XrdOucString emsg("CleanupProofServ: cannot open /proc - errno: ");
      emsg += errno;
      TRACE(DBG, emsg.c_str());
      return -1;
   }

   struct dirent *ent = 0;
   while ((ent = readdir(dir))) {
      if (DIGIT(ent->d_name[0])) {
         XrdOucString fn("/proc/", 256);
         fn += ent->d_name;
         fn += "/psinfo";
         // Open file
         int ffd = open(fn.c_str(), O_RDONLY);
         if (ffd <= 0) {
            XrdOucString emsg("CleanupProofServ: cannot open file ");
            emsg += fn; emsg += " - errno: "; emsg += errno;
            TRACE(HDBG, emsg.c_str());
            continue;
         }
         // Read info
         bool xname = 1;
         bool xuid = (all) ? 0 : 1;
         bool xppid = 1;
         // Get the information
         psinfo_t psi;
         if (read(ffd, &psi, sizeof(psinfo_t)) != sizeof(psinfo_t)) {
            XrdOucString emsg("CleanupProofServ: cannot read ");
            emsg += fn; emsg += ": errno: "; emsg += errno;
            TRACE(XERR, emsg.c_str());
            close(ffd);
            continue;
         }
         // Close the file
         close(ffd);

         // Check name
         if (xname) {
            if (!strstr(psi.pr_fname, pn))
               continue;
            xname = 0;
         }
         // Check uid, if required
         if (xuid) {
            if (refuid == psi.pr_uid)
               xuid = 0;
         }
         // Parent process must be us or be dead
         int ppid = psi.pr_ppid;
         if (ppid != getpid() &&
             fMgr->VerifyProcessByID(ppid, "xrootd")) {
             // Process created by another running xrootd
             continue;
             xppid = 0;
         }

         // If this is a good candidate, kill it
         if (!xname && !xppid && !xuid) {
            bool muok = 1;
            if (fMgr->MultiUser() && !all) {
               // We need to check the user name: we may be the owner of somebody
               // else process; if no session is attached , we kill it
               muok = 0;
               XrdProofServProxy *srv = fMgr->GetActiveSession(psi.pr_pid);
               if (!srv || (srv && !strcmp(usr, srv->Client())))
                  muok = 1;
            }
            if (muok)
               if (XrdProofdAux::KillProcess(psi.pr_pid, 1, p->Client()->UI(), fMgr->ChangeOwn()) == 0)
                  nk++;
         }
      }
   }
   // Close the directory
   closedir(dir);

#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__APPLE__)

   // Get the proclist
   kinfo_proc *pl = 0;
   int np;
   int ern = 0;
   if ((ern = XrdProofdAux::GetMacProcList(&pl, np)) != 0) {
      XrdOucString emsg("CleanupProofServ: cannot get the process list: errno: ");
      emsg += ern;
      TRACE(XERR, emsg.c_str());
      return -1;
   }

   // Loop over the list
   int ii = np;
   while (ii--) {
      if (strstr(pl[ii].kp_proc.p_comm, pn)) {
         if (all || (int)(pl[ii].kp_eproc.e_ucred.cr_uid) == refuid) {
            // Parent process must be us or be dead
            int ppid = pl[ii].kp_eproc.e_ppid;
            bool xppid = 0;
            if (ppid != getpid()) {
               int jj = np;
               while (jj--) {
                  if (strstr(pl[jj].kp_proc.p_comm, "xrootd") &&
                      pl[jj].kp_proc.p_pid == ppid) {
                      xppid = 1;
                      break;
                  }
               }
            }
            if (!xppid) {
               bool muok = 1;
               if (fMgr->MultiUser() && !all) {
                  // We need to check the user name: we may be the owner of somebody
                  // else process; if no session is attached, we kill it
                  muok = 0;
                  XrdProofServProxy *srv = fMgr->GetActiveSession(pl[np].kp_proc.p_pid);
                  if (!srv || (srv && !strcmp(usr, srv->Client())))
                     muok = 1;
               }
               if (muok)
                  // Good candidate to be shot
                  if (XrdProofdAux::KillProcess(pl[np].kp_proc.p_pid, 1, p->Client()->UI(), fMgr->ChangeOwn()))
                     nk++;
            }
         }
      }
   }
   // Cleanup
   free(pl);
#else
   // For the remaining cases we use 'ps' via popen to localize the processes

   // Build command
   XrdOucString cmd = "ps ";
   bool busr = 0;
   const char *cusr = (usr && strlen(usr) && fSuperUser) ? usr : fPClient->ID();
   if (all) {
      cmd += "ax";
   } else {
      cmd += "-U ";
      cmd += cusr;
      cmd += " -u ";
      cmd += cusr;
      cmd += " -f";
      busr = 1;
   }
   cmd += " | grep proofserv 2>/dev/null";

   // Our parent ID as a string
   char cpid[10];
   sprintf(cpid, "%d", getpid());

   // Run it ...
   XrdOucString pids = ":";
   FILE *fp = popen(cmd.c_str(), "r");
   if (fp != 0) {
      char line[2048] = { 0 };
      while (fgets(line, sizeof(line), fp)) {
         // Parse line: make sure that we are the parent
         char *px = strstr(line, "xpd");
         if (!px)
            // Not xpd: old proofd ?
            continue;
         char *pi = strstr(px+3, cpid);
         if (!pi) {
            // Not started by us: check if the parent is still running
            pi = px + 3;
            int ppid = (int) XrdProofdAux::GetLong(pi);
            TRACE(HDBG, "CleanupProofServ: found alternative parent ID: "<< ppid);
            // If still running then skip
            if (fMgr->VerifyProcessByID(ppid, "xrootd"))
               continue;
         }
         // Get pid now
         int from = 0;
         if (busr)
            from += strlen(cusr);
         int pid = (int) XrdProofdAux::GetLong(&line[from]);
         bool muok = 1;
         if (fMgr->MultiUser() && !all) {
            // We need to check the user name: we may be the owner of somebody
            // else process; if no session is attached, we kill it
            muok = 0;
            XrdProofServProxy *srv = fMgr->GetActiveSession(pid);
            if (!srv || (srv && !strcmp(usr, srv->Client())))
               muok = 1;
         }
         if (muok)
            // Kill it
            if (XrdProofdAux::KillProcess(pid, 1, p->Client()->UI(), fMgr->ChangeOwn()) == 0)
               nk++;
      }
      pclose(fp);
   } else {
      // Error executing the command
      return -1;
   }
#endif

   // Done
   return nk;
}

//___________________________________________________________________________
int XrdProofdProofServMgr::SetUserEnvironment(XrdProofdProtocol *p)
{
   // Set user environment: set effective user and group ID of the process
   // to the ones of the owner of this protocol instnace and change working
   // dir to the sandbox.
   // Return 0 on success, -1 if enything goes wrong.

   MTRACE(ACT, "xpd:child: ", "SetUserEnvironment: enter");

   if (XrdProofdAux::ChangeToDir(p->Client()->Sandbox()->Dir(),
                                 p->Client()->UI(), fMgr->ChangeOwn()) != 0) {
      MTRACE(XERR, "xpd:child: ", "SetUserEnvironment: couldn't change directory to "<<
                   p->Client()->Sandbox()->Dir());
      return -1;
   }

   // set HOME env
   char *h = new char[8 + strlen(p->Client()->Sandbox()->Dir())];
   sprintf(h, "HOME=%s", p->Client()->Sandbox()->Dir());
   putenv(h);
   MTRACE(XERR, "xpd:child: ", "SetUserEnvironment: set "<<h);

   // Set access control list from /etc/initgroup
   // (super-user privileges required)
   MTRACE(DBG, "xpd:child: ", "SetUserEnvironment: setting ACLs");
   if (fMgr->ChangeOwn() && (int) geteuid() != p->Client()->UI().fUid) {

      XrdSysPrivGuard pGuard((uid_t)0, (gid_t)0);
      if (XpdBadPGuard(pGuard, p->Client()->UI().fUid)) {
         MTRACE(XERR, "xpd:child: ", "SetUserEnvironment: could not get privileges");
         return -1;
      }

      initgroups(p->Client()->UI().fUser.c_str(), p->Client()->UI().fGid);
   }

   if (fMgr->ChangeOwn()) {
      // acquire permanently target user privileges
      MTRACE(DBG, "xpd:child: ", "SetUserEnvironment: acquire target user identity");
      if (XrdSysPriv::ChangePerm((uid_t)p->Client()->UI().fUid,
                                 (gid_t)p->Client()->UI().fGid) != 0) {
         MTRACE(XERR, "xpd:child: ",
                      "SetUserEnvironment: can't acquire "<< p->Client()->UI().fUser <<" identity");
         return -1;
      }
   }

   // Save UNIX path in the sandbox for later cleaning
   // (it must be done after sandbox login)
   p->Client()->SaveUNIXPath();

   // We are done
   MTRACE(DBG, "xpd:child: ", "SetUserEnvironment: done");
   return 0;
}

//______________________________________________________________________________
int XrdProofdProofServMgr::SaveAFSkey(XrdSecCredentials *c, const char *dir)
{
   // Save the AFS key, if any, for usage in proofserv in file 'dir'/.afs .
   // Return 0 on success, -1 on error.

   // Check file name
   if (!dir || strlen(dir) <= 0) {
      TRACE(XERR, "SaveAFSkey: dir name undefined");
      return -1;
   }

   // Check credentials
   if (!c) {
      TRACE(XERR, "SaveAFSkey: credentials undefined");
      return -1;
   }

   // Decode credentials
   int lout = 0;
   char *out = new char[c->size];
   if (XrdSutFromHex(c->buffer, out, lout) != 0) {
      TRACE(XERR, "SaveAFSkey: problems unparsing hex string");
      delete [] out;
      return -1;
   }

   // Locate the key
   char *key = out + 5;
   if (strncmp(key, "afs:", 4)) {
      TRACE(DBG, "SaveAFSkey: string does not contain an AFS key");
      delete [] out;
      return 0;
   }
   key += 4;

   // Filename
   XrdOucString fn = dir;
   fn += "/.afs";
   // Open the file, truncatin g if already existing
   int fd = open(fn.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
   if (fd <= 0) {
      TRACE(XERR, "SaveAFSkey: problems creating file - errno: " << errno);
      delete [] out;
      return -1;
   }
   // Make sure it is protected
   if (fchmod(fd, 0600) != 0) {
      TRACE(XERR, "SaveAFSkey: problems setting file permissions to 0600 - errno: " << errno);
      delete [] out;
      close(fd);
      return -1;
   }
   // Write out the key
   int rc = 0;
   int lkey = lout - 9;
   if (XrdProofdAux::Write(fd, key, lkey) != lkey) {
      TRACE(XERR, "SaveAFSkey: problems writing to file - errno: " << errno);
      rc = -1;
   }

   // Cleanup
   delete [] out;
   close(fd);
   return rc;
}

//__________________________________________________________________________
XrdProofServProxy *XrdProofdProofServMgr::GetActiveSession(int pid)
{
   // Return active session with process ID pid, if any.

   XrdOucString key; key += pid;
   return fSessions.Find(key.c_str());
}

//__________________________________________________________________________
static int BroadcastPriority(const char *, XrdProofServProxy *ps, void *s)
{
   // Run thorugh entries to broadcast the relevant priority

   XpdBroadcastPriority_t *bp = (XpdBroadcastPriority_t *)s;

   int nb = *(bp->fNBroadcast);

   XrdOucString emsg;
   if (ps) {
      if (ps->IsValid() && (ps->Status() == kXPD_running) &&
        !(ps->SrvType() == kXPD_Master)) {
         XrdProofGroup *g = (ps->Group() && bp->fGroupMgr)
                          ? bp->fGroupMgr->GetGroup(ps->Group()) : 0;
         TRACE(SCHED,"BroadcastPriority: group: "<<  g<<", client: "<<ps->Client());
         if (g && g->Active() > 0) {
            TRACE(SCHED,"BroadcastPriority: priority: "<< g->Priority()<<" active: "<<g->Active());
            int prio = (int) (g->Priority() * 100);
            ps->BroadcastPriority(prio);
            nb++;
         }
      }
      // Go to next
      return 0;
   } else {
      emsg = "input entry undefined";
   }

   // Some problem
   TRACE(XERR,"BroadcastPriority: protocol error: "<<emsg);
   return 1;
}

//__________________________________________________________________________
int XrdProofdProofServMgr::BroadcastPriorities()
{
   // Broadcast priorities to the active sessions.
   // Returns the number of sessions contacted.

   int nb = 0;

#if 0
   // Communicate them to the sessions
   std::list<XrdProofServProxy *>::iterator svi;
   for (svi = fSessions.begin(); svi != fSessions.end(); svi++) {
      TRACE(SCHED,"BroadcastPriorities: server type: "<<(*svi)->SrvType());
      if ((*svi)->IsValid() && ((*svi)->Status() == kXPD_running) &&
        !((*svi)->SrvType() == kXPD_Master)) {
         XrdProofGroup *g = (*svi)->Group();
         TRACE(SCHED,"UpdatePriorities: group: "<<  g<<", client: "<<(*svi)->Client());
         if (g && g->Active() > 0) {
            TRACE(SCHED,"UpdatePriorities: Priority: "<< g->Priority()<<" Active: "<<g->Active());
            int prio = (int) (g->Priority() * 100);
            (*svi)->BroadcastPriority(prio);
            nb++;
         }
      }
   }
#else
   XpdBroadcastPriority_t bp = { (fMgr ? fMgr->GroupsMgr() : 0), &nb };
   fSessions.Apply(BroadcastPriority, (void *)&bp);
#endif
   // Done
   return nb;
}

//
// Auxilliary class to handle session pid files
//

//______________________________________________________________________________
int XrdProofSessionInfo::SaveToFile(const char *file)
{
   // Save content to 'file'

   // Check inputs
   if (!file || strlen(file) <= 0) {
      TRACE(XERR,"SafeToFile: invalid input: "<<file);
      return -1;
   }

   // Create the file
   FILE *fpid = fopen(file,"w");
   if (fpid) {
      fprintf(fpid, "%s %s\n", fUser.c_str(), fGroup.c_str());
      fprintf(fpid, "%s", fUnixPath.c_str());
      fclose(fpid);
      return 0;
   }

   TRACE(XERR,"SaveToFile: session pid file cannot be (re-)created: "<<
              file<<"; error: "<<errno);
   return -1;
}

//______________________________________________________________________________
void XrdProofSessionInfo::Reset()
{
   // Reset the content

   fLastAccess = 0;
   fUser = "";
   fGroup = "";
   fUnixPath = "";
}

//______________________________________________________________________________
int XrdProofSessionInfo::ReadFromFile(const char *file)
{
   // Read content from 'file'

   // Check inputs
   if (!file || strlen(file) <= 0) {
      TRACE(XERR,"ReadFromFile: invalid input: "<<file);
      return -1;
   }

   Reset();

   // Open the file
   FILE *fpid = fopen(file,"r");
   if (fpid) {
      char line[4096];
      if (fgets(line, sizeof(line), fpid)) {
         char usr[128], grp[128];
         sscanf(line, "%s %s", usr, grp);
         fUser = usr;
         fGroup = grp;
      }
      if (fgets(line, sizeof(line), fpid)) {
         fUnixPath = line;
      }
      fclose(fpid);
      // Fill access time
      struct stat st;
      if (!stat(file, &st))
         fLastAccess = st.st_atime;
      return 0;
   }

   TRACE(XERR,"ReadFromFile: session pid file cannot be open: "<<
              file<<"; error: "<<errno);
   return -1;
}
