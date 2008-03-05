// @(#)root/proofd:$Id$
// Author: G. Ganis June 2007

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// XrdProofdManager                                                     //
//                                                                      //
// Author: G. Ganis, CERN, 2007                                         //
//                                                                      //
// Class mapping manager fonctionality.                                 //
// On masters it keeps info about the available worker nodes and allows //
// communication with them.                                             //
// On workers it handles the communication with the master.             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
#include "XrdProofdPlatform.h"

#include "XrdProofdManager.h"

#ifdef OLDXRDOUC
#  include "XrdOuc/XrdOucPlugin.hh"
#  include "XrdOuc/XrdOucTimer.hh"
#else
#  include "XrdSys/XrdSysPlugin.hh"
#  include "XrdSys/XrdSysTimer.hh"
#endif
#include "XrdNet/XrdNetDNS.hh"
#include "XrdOuc/XrdOucStream.hh"

#include "XrdProofdAdmin.h"
#include "XrdProofdClient.h"
#include "XrdProofdClientMgr.h"
#include "XrdProofdConfig.h"
#include "XrdProofdNetMgr.h"
#include "XrdProofdPriorityMgr.h"
#include "XrdProofdProofServMgr.h"
#include "XrdProofdProtocol.h"
#include "XrdProofGroup.h"
#include "XrdProofSched.h"
#include "XrdProofdProofServ.h"
#include "XrdProofWorker.h"
#include "XrdROOT.h"

// Tracing utilities
#include "XrdProofdTrace.h"
static const char *gTraceID = "";
extern XrdOucTrace *XrdProofdTrace;
#define TRACEID gTraceID

//--------------------------------------------------------------------------
//
// XrdProofdCron
//
// Function run in separate thread to run periodic checks, ... at a tunable
// frequency
//
//--------------------------------------------------------------------------
void *XrdProofdCron(void *p)
{
   // This is an endless loop to periodically check the system

   XrdProofdManager *mgr = (XrdProofdManager *)p;
   if (!(mgr)) {
      TRACE(REQ, "XrdProofdCron: undefined manager: cannot start");
      return (void *)0;
   }

   TRACE(REQ, "XrdProofdCron: started with frequency "<<mgr->CronFrequency()<<" sec");

   while(1) {
      // Wait a while
      XrdSysTimer::Wait(mgr->CronFrequency() * 1000);
      // Do something here
      TRACE(REQ, "XrdProofdCron: running periodical checks");
      // Reconfigure the manager
      mgr->Config(1);
   }

   // Should never come here
   return (void *)0;
}

//__________________________________________________________________________
XrdProofdManager::XrdProofdManager(XrdProtocol_Config *pi, XrdSysError *edest)
                : XrdProofdConfig(pi->ConfigFN, edest)
{
   // Constructor

   fSrvType  = kXPD_AnyServer;
   fEffectiveUser = "";
   fHost = "";
   fPort = XPD_DEF_PORT;
   fImage = "";        // image name for these servers
   fTMPdir = "/tmp";
   fWorkDir = "";
   fDataSetDir = "";
   fSuperMst = 0;
   fNamespace = "/proofpool";
   fMastersAllowed.clear();
   fCron = 1;
   fCronFrequency = 60;
   fOperationMode = kXPD_OpModeOpen;
#if 0
   fMultiUser = (!getuid()) ? 1 : 0;
#else
   fMultiUser = 0;
#endif
   fChangeOwn = 0;

   // Proof admin path
   fAdminPath = pi->AdmPath;
   fAdminPath += "/.xprood.";

   // Services
   fAdmin = 0;
   fClientMgr = 0;
   fGroupsMgr = 0;
   fNetMgr = 0;
   fPriorityMgr = 0;
   fProofSched = 0;
   fSessionMgr = 0;

   // Configuration directives
   RegisterDirectives();

   // Admin request handler
   fAdmin = new XrdProofdAdmin(this);

   // Client manager
   fClientMgr = new XrdProofdClientMgr(this, pi, edest);

   // Network manager
   fNetMgr = new XrdProofdNetMgr(this, pi, edest);

   // Priority manager
   fPriorityMgr = new XrdProofdPriorityMgr(this, pi, edest);

   // ROOT versions manager
   fROOTMgr = new XrdROOTMgr(this, pi, edest);

   // Session manager
   fSessionMgr = new XrdProofdProofServMgr(this, pi, edest);
}

//__________________________________________________________________________
XrdProofdManager::~XrdProofdManager()
{
   // Destructor

   // Destroy the configuration handler
   SafeDelete(fAdmin);
   SafeDelete(fClientMgr);
   SafeDelete(fNetMgr);
   SafeDelete(fPriorityMgr);
   SafeDelete(fProofSched);
   SafeDelete(fROOTMgr);
   SafeDelete(fSessionMgr);
}

//__________________________________________________________________________
static int CreateGroupDataSetDir(const char *, XrdProofGroup *g, void *rd)
{
   // Create dataset dir for group 'g' under the root dataset dir 'rd'

   const char *dsetroot = (const char *)rd;

   if (!dsetroot || strlen(dsetroot) <= 0)
      // Dataset root dir undefined: we cannot continue
      return 1;

   XrdOucString gdsetdir = dsetroot;
   gdsetdir += '/';
   gdsetdir += g->Name();

   XrdProofUI ui;
   XrdProofdAux::GetUserInfo(geteuid(), ui);

   if (XrdProofdAux::AssertDir(gdsetdir.c_str(), ui, 1) != 0) {
      MERROR(MHEAD, "CreateGroupDataSetDir: could not assert " << gdsetdir);
   }

   // Process next
   return 0;
}

//______________________________________________________________________________
bool XrdProofdManager::CheckMaster(const char *m)
{
   // Check if master 'm' is allowed to connect to this host
   bool rc = 1;

   if (fMastersAllowed.size() > 0) {
      rc = 0;
      XrdOucString wm(m);
      std::list<XrdOucString *>::iterator i;
      for (i = fMastersAllowed.begin(); i != fMastersAllowed.end(); ++i) {
         if (wm.matches((*i)->c_str())) {
            rc = 1;
            break;
         }
      }
   }

   // We are done
   return rc;
}

//_____________________________________________________________________________
int XrdProofdManager::CheckUser(const char *usr,
                                 XrdProofUI &ui, XrdOucString &e, bool &su)
{
   // Check if the user is allowed to use the system
   // Return 0 if OK, -1 if not.

   // No 'root' logins
   if (!usr || strlen(usr) <= 0) {
      e = "CheckUser: 'usr' string is undefined ";
      return -1;
   }

   // No 'root' logins
   if (strlen(usr) == 4 && !strcmp(usr, "root")) {
      e = "CheckUser: 'root' logins not accepted ";
      return -1;
   }

   // Here we check if the user is known locally.
   // If not, we fail for now.
   // In the future we may try to get a temporary account
   if (fChangeOwn && !fMultiUser) {
      if (XrdProofdAux::GetUserInfo(usr, ui) != 0) {
         e = "CheckUser: unknown ClientID: ";
         e += usr;
         return -1;
      }
   } else {
      if (XrdProofdAux::GetUserInfo(geteuid(), ui) != 0) {
         e = "CheckUser: problems getting user info for id: ";
         e += (int)geteuid();
         return -1;
      }
   }

   XrdOucString us;
   int from = 0;
   // If we are in controlled mode we have to check if the user in the
   // authorized list; otherwise we fail. Privileged users are always
   // allowed to connect.
   bool notok = 1;
   if (fOperationMode == kXPD_OpModeControlled) {
      while ((from = fAllowedUsers.tokenize(us, from, ',')) != -1) {
         if (us == usr) {
            notok = 0;
            break;
         }
      }
      if (notok) {
         e = "CheckUser: controlled operations:"
             " user not currently authorized to log in: ";
         e += usr;
         return -1;
      }
   }

   // Check if this is a priviliged client
   from = 0;
   su = 0;
   while ((from = fSuperUsers.tokenize(us, from, ',')) != -1) {
      if (us == usr) {
         su = 1;
         break;
      }
   }

   // OK
   return 0;
}

//_____________________________________________________________________________
XrdProofSched *XrdProofdManager::LoadScheduler()
{
   // Load PROOF scheduler

   XrdProofSched *sched = 0;
   XrdOucString name, lib;

   const char *cfn = CfgFile();

   // Locate first the relevant directives in the config file
   if (cfn && strlen(cfn) > 0) {
      XrdOucStream cfg(fEDest, getenv("XRDINSTANCE"));
      // Open and attach the config file
      int cfgFD;
      if ((cfgFD = open(cfn, O_RDONLY, 0)) >= 0) {
         cfg.Attach(cfgFD);
         // Process items
         char *val = 0, *var = 0;
         while ((var = cfg.GetMyFirstWord())) {
            if (!(strcmp("xpd.sched", var))) {
               // Get the name
               val = cfg.GetToken();
               if (val && val[0]) {
                  name = val;
                  // Get the lib
                  val = cfg.GetToken();
                  if (val && val[0])
                     lib = val;
                  // We are done
                  break;
               }
            }
         }
      } else {
         XrdOucString m("failure opening config file (errno:");
         m += errno;
         m += "): ";
         TRACE(XERR, "LoadScheduler: "<< m);
      }
   }

   // If undefined or default init a default instance
   if (name == "default" || !(name.length() > 0 && lib.length() > 0)) {
      if ((name.length() <= 0 && lib.length() > 0) ||
          (name.length() > 0 && lib.length() <= 0)) {
         XrdOucString m("LoadScheduler: missing or incomplete info (name:");
         m += name;
         m += ", lib:";
         m += lib;
         m += ")";
         TRACE(DBG, m.c_str());
      }
      TRACE(DBG,"LoadScheduler: instantiating default scheduler");
      sched = new XrdProofSched("default", this, fGroupsMgr, cfn, fEDest);
   } else {
      // Load the required plugin
      if (lib.beginswith("~") || lib.beginswith("$"))
         XrdProofdAux::Expand(lib);
      XrdSysPlugin *h = new XrdSysPlugin(fEDest, lib.c_str());
      if (!h)
         return (XrdProofSched *)0;
      // Get the scheduler object creator
      XrdProofSchedLoader_t ep = (XrdProofSchedLoader_t) h->getPlugin("XrdgetProofSched", 1);
      if (!ep) {
         delete h;
         return (XrdProofSched *)0;
      }
      // Get the scheduler object
      if (!(sched = (*ep)(cfn, this, fGroupsMgr, fEDest))) {
         TRACE(XERR, "LoadScheduler: unable to create scheduler object from " << lib);
         return (XrdProofSched *)0;
      }
   }
   // Check result
   if (!(sched->IsValid())) {
      TRACE(XERR, "LoadScheduler:"
                  " unable to instantiate the "<<sched->Name()<<" scheduler using "<< cfn);
      delete sched;
      return (XrdProofSched *)0;
   }
   // Notify
   XPDPRT("LoadScheduler: scheduler loaded: type: " << sched->Name());

   // All done
   return sched;
}

//__________________________________________________________________________
int XrdProofdManager::GetWorkers(XrdOucString &lw, XrdProofdProofServ *xps)
{
   // Get a list of workers from the available resource broker
   int rc = 0;

   TRACE(ACT, "GetWorkers: enter");

   // We need the scheduler at this point
   if (!fProofSched) {
      fEDest->Emsg("GetWorkers", "Scheduler undefined");
      return -1;
   }

   // Query the scheduler for the list of workers
   std::list<XrdProofWorker *> wrks;
   fProofSched->GetWorkers(xps, &wrks);
   TRACE(DBG, "GetWorkers: list size: " << wrks.size());

   // The full list
   std::list<XrdProofWorker *>::iterator iw;
   for (iw = wrks.begin(); iw != wrks.end() ; iw++) {
      XrdProofWorker *w = *iw;
      // Add separator if not the first
      if (lw.length() > 0)
         lw += '&';
      // Add export version of the info
      lw += w->Export();
      // Count
      xps->AddWorker(w);
      w->fProofServs.push_back(xps);
      w->fActive++;
   }

   return rc;
}

//__________________________________________________________________________
int XrdProofdManager::Config(bool rcf)
{
   // Run configuration and parse the entered config directives.
   // Return 0 on success, -1 on error

   XrdSysMutexHelper mtxh(fMutex);

   // Run first the configurator
   if (XrdProofdConfig::Config(rcf) != 0) {
      fEDest->Say(0, "xpd: Config: Manager: problems parsing file ");
      return -1;
   }

   XrdOucString msg;
   msg = (rcf) ? "xpd: Config: Manager: re-configuring"
               : "xpd: Config: Manager: configuring";
   fEDest->Say(0, msg.c_str());

   // Change/DonotChange ownership when logging clients
   fChangeOwn = (fMultiUser && getuid()) ? 0 : 1;

   // Notify port
   msg = "xpd: Config: Manager: listening on port ";
   msg += fPort;
   fEDest->Say(0, msg.c_str());

   XrdProofUI ui;
   if (!rcf) {
      // Effective user
      if (XrdProofdAux::GetUserInfo(geteuid(), ui) == 0) {
         fEffectiveUser = ui.fUser;
      } else {
         msg = "xpd: Config: Manager: could not resolve effective user (getpwuid, errno: ";
         msg += errno;
         msg += ")";
         fEDest->Say(0, msg.c_str());
         return -1;
      }

      // Local FQDN
      char *host = XrdNetDNS::getHostName();
      fHost = host ? host : "";
      SafeFree(host);

      // Notify temporary directory
      fEDest->Say(0, "xpd: Config: Manager: using temp dir: ", fTMPdir.c_str());

      // Notify role
      const char *roles[] = { "any", "worker", "submaster", "master" };
      fEDest->Say(0, "xpd: Config: Manager: role set to: ", roles[fSrvType+1]);

      // Admin path
      fAdminPath += fPort;
      if (XrdProofdAux::AssertDir(fAdminPath.c_str(), ui, fChangeOwn) != 0) {
         fEDest->Say(0, "xpd: Config: Manager: unable to assert the admin path: ",
                        fAdminPath.c_str());
         return -1;
      }
      fEDest->Say(0, "xpd: Config: Manager: admin path set to: ", fAdminPath.c_str());

      // Create / Update the process ID file under the admin path
      XrdOucString pidfile(fAdminPath);
      pidfile += "/xrootd.pid";
      FILE *fpid = fopen(pidfile.c_str(), "w");
      if (!fpid) {
         msg = "xpd: Config: Manager: unable to open pid file: ";
         msg += pidfile.c_str();
         msg = " ; errno: ";
         msg += errno;
         fEDest->Say(0, msg.c_str());
         return -1;
      }
      fprintf(fpid, "%d", getpid());
      fclose(fpid);
   } else {
      if (XrdProofdAux::GetUserInfo(fEffectiveUser.c_str(), ui) == 0) {
         fEDest->Say(0, "xpd: Config: Manager: unable to get user info for ",
                        fEffectiveUser.c_str());
      }
   }

   // Work directory, if specified
   if (fWorkDir.length() > 0) {
      // Make sure it exists
      if (XrdProofdAux::AssertDir(fWorkDir.c_str(), ui, fChangeOwn) != 0) {
         fEDest->Say(0, "xpd: Config: Manager: unable to assert working dir: ",
                        fWorkDir.c_str());
         return -1;
      }
      fEDest->Say(0, "xpd: Config: Manager: working directories under: ",
                     fWorkDir.c_str());
      // Communicate it to the sandbox service
      XrdProofdSandbox::SetWorkdir(fWorkDir.c_str());
   }

   // Dataset directory, if specified
   if (fDataSetDir.length() > 0) {
      // Make sure it exists
      if (XrdProofdAux::AssertDir(fDataSetDir.c_str(), ui, fChangeOwn) != 0) {
         fEDest->Say(0, "xpd: Config: Manager: unable to assert dataset dir: ",
                        fDataSetDir.c_str());
         return -1;
      }
      fEDest->Say(0, "xpd: Config: Manager: dataset directories under: ",
                     fDataSetDir.c_str());
      // Communicate it to the sandbox service
      XrdProofdSandbox::SetDSetdir(fDataSetDir.c_str());
   }

   // Notify allow rules
   if (fSrvType == kXPD_Worker) {
      if (fMastersAllowed.size() > 0) {
         std::list<XrdOucString *>::iterator i;
         for (i = fMastersAllowed.begin(); i != fMastersAllowed.end(); ++i)
            fEDest->Say(0, "xpd: Config: Manager: masters allowed to connect: ", (*i)->c_str());
      } else {
            fEDest->Say(0, "xpd: Config: Manager: masters allowed to connect: any");
      }
   }

   // Pool and namespace
   if (fPoolURL.length() <= 0) {
      // Default pool entry point is this host
      fPoolURL = "root://";
      fPoolURL += fHost;
   }
   fEDest->Say(0, "xpd: Config: Manager: PROOF pool: ", fPoolURL.c_str());
   fEDest->Say(0, "xpd: Config: Manager: PROOF pool namespace: ", fNamespace.c_str());

   // Initialize resource broker (if not worker)
   if (fSrvType != kXPD_Worker) {

      // Scheduler instance
      if (!(fProofSched = LoadScheduler())) {
         fEDest->Say(0, "xpd: Config: Manager: scheduler initialization failed");
         return 0;
      }
      const char *st[] = { "disabled", "enabled" };
      fEDest->Say(0, "xpd: Config: Manager: user config files are ", st[fNetMgr->WorkerUsrCfg()]);
   }

   // Superusers: add default
   if (fSuperUsers.length() > 0)
      fSuperUsers += ",";
   fSuperUsers += fEffectiveUser;
   msg = "xpd: Config: Manager: list of superusers: ";
   msg += fSuperUsers;
   fEDest->Say(0, msg.c_str());

   // Notify controlled mode, if such
   if (fOperationMode == kXPD_OpModeControlled) {
      fAllowedUsers += ',';
      fAllowedUsers += fSuperUsers;
      msg = "xpd: Config: Manager: running in controlled access mode: users allowed: ";
      msg += fAllowedUsers;
      fEDest->Say(0, msg.c_str());
   }

   // Bare lib path
   if (getenv(XPD_LIBPATH)) {
      // Try to remove existing ROOT dirs in the path
      XrdOucString paths = getenv(XPD_LIBPATH);
      XrdOucString ldir;
      int from = 0;
      while ((from = paths.tokenize(ldir, from, ':')) != STR_NPOS) {
         bool isROOT = 0;
         if (ldir.length() > 0) {
            // Check this dir
            DIR *dir = opendir(ldir.c_str());
            if (dir) {
               // Scan the directory
               struct dirent *ent = 0;
               while ((ent = (struct dirent *)readdir(dir))) {
                  if (!strncmp(ent->d_name, "libCore", 7)) {
                     isROOT = 1;
                     break;
                  }
               }
               // Close the directory
               closedir(dir);
            }
            if (!isROOT) {
               if (fBareLibPath.length() > 0)
                  fBareLibPath += ":";
               fBareLibPath += ldir;
            }
         }
      }
      fEDest->Say(0, "xpd: Config: Manager: bare lib path for proofserv: ",
                     fBareLibPath.c_str());
   }

   // Groups
   if (!fGroupsMgr)
      // Create default group, if none explicitely requested
      fGroupsMgr = new XrdProofGroupMgr;

   if (fGroupsMgr)
      fGroupsMgr->Print(0);

   // Dataset dir
   if (fGroupsMgr && fDataSetDir.length() > 0) {
      // Make sure that the group dataset dirs exists
      fGroupsMgr->Apply(CreateGroupDataSetDir, (void *)fDataSetDir.c_str());
   }

   // Config the network manager
   if (fNetMgr && fNetMgr->Config(rcf) != 0) {
      fEDest->Say(0, "xpd: Config: Manager: problems configuring the network manager");
      return -1;
   }

   // Config the priority manager
   if (fPriorityMgr && fPriorityMgr->Config(rcf) != 0) {
      fEDest->Say(0, "xpd: Config: Manager: problems configuring the priority manager");
      return -1;
   }

   // Config the ROOT versions manager
   if (fROOTMgr && fROOTMgr->Config(rcf) != 0) {
      fEDest->Say(0, "xpd: Config: Manager: problems configuring the ROOT versions manager");
      return -1;
   }

   // Config the client manager
   if (fClientMgr && fClientMgr->Config(rcf) != 0) {
      fEDest->Say(0, "xpd: Config: Manager: problems configuring the client manager");
      return -1;
   }

   // Config the session manager
   if (fSessionMgr && fSessionMgr->Config(rcf) != 0) {
      fEDest->Say(0, "xpd: Config: Manager: problems configuring the session manager");
      return -1;
   }

   // Start cron thread, if required
   if (fCron == 1) {
      pthread_t tid;
      if (XrdSysThread::Run(&tid, XrdProofdCron,
                            (void *)this, 0, "Proof Manager cron thread") != 0) {
         fEDest->Say(0, "xpd: Config: Manager: could not start cron thread");
         return 0;
      }
      fEDest->Say(0, "xpd: Config: Manager: cron thread started");
   }

   // Done
   return 0;
}

//__________________________________________________________________________
void XrdProofdManager::RegisterDirectives()
{
   // Register directives for configuration

   // Register special config directives
   Register("trace", new XrdProofdDirective("trace", this, &DoDirectiveClass));
   Register("groupfile", new XrdProofdDirective("groupfile", this, &DoDirectiveClass));
   Register("multiuser", new XrdProofdDirective("multiuser", this, &DoDirectiveClass));
   Register("shutdown", new XrdProofdDirective("shutdown", this, &DoDirectiveClass));
   Register("maxoldlogs", new XrdProofdDirective("maxoldlogs", this, &DoDirectiveClass));
   Register("allow", new XrdProofdDirective("allow", this, &DoDirectiveClass));
   Register("allowedusers", new XrdProofdDirective("allowedusers", this, &DoDirectiveClass));
   Register("role", new XrdProofdDirective("role", this, &DoDirectiveClass));
   Register("cron", new XrdProofdDirective("cron", this, &DoDirectiveClass));
   Register("xrd.protocol", new XrdProofdDirective("xrd.protocol", this, &DoDirectiveClass));
   // Register config directives for strings
   Register("tmp", new XrdProofdDirective("tmp", (void *)&fTMPdir, &DoDirectiveString));
   Register("poolurl", new XrdProofdDirective("poolurl", (void *)&fPoolURL, &DoDirectiveString));
   Register("namespace", new XrdProofdDirective("namespace", (void *)&fNamespace, &DoDirectiveString));
   Register("superusers", new XrdProofdDirective("superusers", (void *)&fSuperUsers, &DoDirectiveString));
   Register("image", new XrdProofdDirective("image", (void *)&fImage, &DoDirectiveString));
   Register("workdir", new XrdProofdDirective("workdir", (void *)&fWorkDir, &DoDirectiveString));
   Register("datasetdir", new XrdProofdDirective("datasetdir", (void *)&fDataSetDir, &DoDirectiveString));
   Register("proofplugin", new XrdProofdDirective("proofplugin", (void *)&fProofPlugin, &DoDirectiveString));
}

//______________________________________________________________________________
int XrdProofdManager::ResolveKeywords(XrdOucString &s, XrdProofdClient *pcl)
{
   // Resolve special keywords in 's' for client 'pcl'. Recognized keywords
   //     <workdir>          root for working dirs
   //     <host>             local host name
   //     <user>             user name
   // Return the number of keywords resolved.

   int nk = 0;

   TRACE(HDBG,"ResolveKeywords: enter: "<<s<<" - WorkDir(): "<<WorkDir());

   // Parse <workdir>
   if (s.replace("<workdir>", WorkDir()))
      nk++;

   TRACE(HDBG,"ResolveKeywords: after <workdir>: "<<s);

   // Parse <host>
   if (s.replace("<host>", Host()))
      nk++;

   TRACE(HDBG,"ResolveKeywords: after <host>: "<<s);

   // Parse <user>
   if (pcl)
      if (s.replace("<user>", pcl->User()))
         nk++;

   TRACE(HDBG,"ResolveKeywords: exit: "<<s);

   // We are done
   return nk;
}

//
// Special directive processors

//______________________________________________________________________________
int XrdProofdManager::DoDirective(XrdProofdDirective *d,
                                  char *val, XrdOucStream *cfg, bool rcf)
{
   // Update the priorities of the active sessions.

   if (!d)
      // undefined inputs
      return -1;

   if (d->fName == "trace") {
      return DoDirectiveTrace(val, cfg, rcf);
   } else if (d->fName == "groupfile") {
      return DoDirectiveGroupfile(val, cfg, rcf);
   } else if (d->fName == "maxoldlogs") {
      return DoDirectiveMaxOldLogs(val, cfg, rcf);
   } else if (d->fName == "allow") {
      return DoDirectiveAllow(val, cfg, rcf);
   } else if (d->fName == "allowedusers") {
      return DoDirectiveAllowedUsers(val, cfg, rcf);
   } else if (d->fName == "role") {
      return DoDirectiveRole(val, cfg, rcf);
   } else if (d->fName == "multiuser") {
      return DoDirectiveMultiUser(val, cfg, rcf);
   } else if (d->fName == "cron") {
      return DoDirectiveCron(val, cfg, rcf);
   } else if (d->fName == "xrd.protocol") {
      return DoDirectivePort(val, cfg, rcf);
   }
   TRACE(XERR,"DoDirective: unknown directive: "<<d->fName);
   return -1;
}

//______________________________________________________________________________
int XrdProofdManager::DoDirectiveTrace(char *val, XrdOucStream *cfg, bool)
{
   // Scan the config file for tracing settings

   if (!val || !cfg)
   // undefined inputs
   return -1;

   // Specifies tracing options. Valid keywords are:
   //   req            trace protocol requests             [on]*
   //   login          trace details about login requests  [on]*
   //   act            trace internal actions              [off]
   //   rsp            trace server replies                [off]
   //   fork           trace proofserv forks               [on]*
   //   dbg            trace details about actions         [off]
   //   hdbg           trace more details about actions    [off]
   //   err            trace errors                        [on]
   //   sched          trace details about scheduling      [off]
   //   admin          trace admin requests                [on]*
   //   all            trace everything
   //
   // Defaults are shown in brackets; '*' shows the default when the '-d'
   // option is passed on the command line. Each option may be
   // optionally prefixed by a minus sign to turn off the setting.
   // Order matters: 'all' in last position enables everything; in first
   // position is corrected by subsequent settings
   //
   while (val && val[0]) {
      bool on = 1;
      if (val[0] == '-') {
         on = 0;
         val++;
      }
      if (!strcmp(val,"req")) {
         TRACESET(REQ, on);
      } else if (!strcmp(val,"login")) {
         TRACESET(LOGIN, on);
      } else if (!strcmp(val,"act")) {
         TRACESET(ACT, on);
      } else if (!strcmp(val,"rsp")) {
         TRACESET(RSP, on);
      } else if (!strcmp(val,"fork")) {
         TRACESET(FORK, on);
      } else if (!strcmp(val,"dbg")) {
         TRACESET(DBG, on);
      } else if (!strcmp(val,"hdbg")) {
         TRACESET(HDBG, on);
      } else if (!strcmp(val,"err")) {
         TRACESET(XERR, on);
      } else if (!strcmp(val,"sched")) {
         TRACESET(SCHED, on);
      } else if (!strcmp(val,"admin")) {
         TRACESET(ADMIN, on);
      } else if (!strcmp(val,"all")) {
         // Everything
         XPDPRT("Setting trace: "<<on);
         XrdProofdTrace->What = (on) ? TRACE_ALL : 0;
      }
      // Next
      val = cfg->GetToken();
   }
         bool dbg = TRACING(DBG);
         XPDPRT("dbg trace: "<<dbg);

   return 0;
}

//______________________________________________________________________________
int XrdProofdManager::DoDirectiveGroupfile(char *val, XrdOucStream *cfg, bool rcf)
{
   // Process 'groupfile' directive

   if (!val)
      // undefined inputs
      return -1;

   // Check deprecated 'if' directive
   if (Host() && cfg)
      if (XrdProofdAux::CheckIf(cfg, Host()) == 0)
         return 0;

   // Defines file with the group info
   if (rcf) {
      SafeDelete(fGroupsMgr);
   } else if (fGroupsMgr) {
      TRACE(XERR,"DoDirectiveGroupfile: groups manager already initialized: ignoring ");
      return -1;
   }
   fGroupsMgr = new XrdProofGroupMgr;
   fGroupsMgr->Config(val);
   return 0;
}

//______________________________________________________________________________
int XrdProofdManager::DoDirectiveMaxOldLogs(char *val, XrdOucStream *cfg, bool)
{
   // Process 'maxoldlogs' directive

   if (!val)
      // undefined inputs
      return -1;

   // Check deprecated 'if' directive
   if (Host() && cfg)
      if (XrdProofdAux::CheckIf(cfg, Host()) == 0)
         return 0;

   // Max number of sessions per user
   int maxoldlogs = strtol(val, 0, 10);
   XrdProofdSandbox::SetMaxOldSessions(maxoldlogs);
   return 0;
}

//______________________________________________________________________________
int XrdProofdManager::DoDirectiveAllow(char *val, XrdOucStream *cfg, bool)
{
   // Process 'allow' directive

   if (!val)
      // undefined inputs
      return -1;

   // Check deprecated 'if' directive
   if (Host() && cfg)
      if (XrdProofdAux::CheckIf(cfg, Host()) == 0)
         return 0;

   // Masters allowed to connect
   fMastersAllowed.push_back(new XrdOucString(val));
   return 0;
}

//______________________________________________________________________________
int XrdProofdManager::DoDirectiveAllowedUsers(char *val, XrdOucStream *cfg, bool)
{
   // Process 'allowedusers' directive

   if (!val)
      // undefined inputs
      return -1;

   // Check deprecated 'if' directive
   if (Host() && cfg)
      if (XrdProofdAux::CheckIf(cfg, Host()) == 0)
         return 0;

   // Users allowed to use the cluster
   fAllowedUsers = val;
   fOperationMode = kXPD_OpModeControlled;
   return 0;
}

//______________________________________________________________________________
int XrdProofdManager::DoDirectiveRole(char *val, XrdOucStream *cfg, bool)
{
   // Process 'allowedusers' directive

   if (!val)
      // undefined inputs
      return -1;

   // Check deprecated 'if' directive
   if (Host() && cfg)
      if (XrdProofdAux::CheckIf(cfg, Host()) == 0)
         return 0;

   // Role this server
   XrdOucString tval(val);
   if (tval == "supermaster") {
      fSrvType = kXPD_TopMaster;
      fSuperMst = 1;
   } else if (tval == "master") {
      fSrvType = kXPD_TopMaster;
   } else if (tval == "submaster") {
      fSrvType = kXPD_Master;
   } else if (tval == "worker") {
      fSrvType = kXPD_Worker;
   }

   return 0;
}

//______________________________________________________________________________
int XrdProofdManager::DoDirectivePort(char *, XrdOucStream *cfg, bool)
{
   // Process 'xrd.protocol' directive to find the port

   if (!cfg)
      // undefined inputs
      return -1;

   // Get the value
   XrdOucString proto = cfg->GetToken();
   if (proto.length() > 0 && proto.beginswith("xproofd:")) {
      proto.replace("xproofd:","");
      fPort = strtol(proto.c_str(), 0, 10);
      fPort = (fPort < 0) ? XPD_DEF_PORT : fPort;
   }
   return 0;
}

//______________________________________________________________________________
int XrdProofdManager::DoDirectiveMultiUser(char *val, XrdOucStream *cfg, bool)
{
   // Process 'multiuser' directive

   if (!val)
      // undefined inputs
      return -1;

   // Check deprecated 'if' directive
   if (Host() && cfg)
      if (XrdProofdAux::CheckIf(cfg, Host()) == 0)
         return 0;

   // Multi-user option
   int mu = strtol(val,0,10);
   fMultiUser = (mu == 1) ? 1 : fMultiUser;
   return 0;
}

//______________________________________________________________________________
int XrdProofdManager::DoDirectiveCron(char *val, XrdOucStream *, bool)
{
   // Process 'cron' directive

   if (!val)
      // undefined inputs
      return -1;

   // Cron frequency
   int freq = strtol(val, 0, 10);
   if (freq > 0) {
      XPDPRT("DoDirectiveCron: setting frequency to "<<freq<<" sec");
      fCronFrequency = freq;
   }

   return 0;
}

//______________________________________________________________________________
int XrdProofdManager::Process(XrdProofdProtocol *p)
{
   // Process manager request

   int rc = 1;
   XPD_SETRESP(p, "Process");

   TRACEP(p, respid, REQ, "Process: enter: req id: " << p->Request()->header.requestid);

   // Atomic
   XrdSysMutexHelper mhp(fMutex);

   // If the user is not yet logged in, restrict what the user can do
   if (!p->Status() || !(p->Status() & XPD_LOGGEDIN)) {
      switch(p->Request()->header.requestid) {
      case kXP_auth:
         return fClientMgr->Auth(p);
      case kXP_login:
         return fClientMgr->Login(p);
      default:
         TRACEP(p, respid, XERR,"Process: invalid request: " <<p->Request()->header.requestid);
         response->Send(kXR_InvalidRequest,"Invalid request; user not logged in");
         return p->Link()->setEtext("protocol sequence error 1");
      }
   }

   // Once logged-in, the user can request the real actions
   XrdOucString emsg;
   switch(p->Request()->header.requestid) {
      case kXP_admin:
         {  int type = ntohl(p->Request()->proof.int1);
            return fAdmin->Process(p, type);
         }
      case kXP_readbuf:
         return fNetMgr->ReadBuffer(p);
      case kXP_create:
      case kXP_destroy:
      case kXP_attach:
      case kXP_detach:
         return fSessionMgr->Process(p);
      default:
         emsg += "Invalid request: ";
         emsg += p->Request()->header.requestid;
         break;
   }

   // Notify invalid request
   response->Send(kXR_InvalidRequest, emsg.c_str());

   // Done
   return 0;
}
