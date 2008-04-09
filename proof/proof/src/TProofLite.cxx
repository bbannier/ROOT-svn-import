// @(#)root/proof:$Id$
// Author: G. Ganis March 2008

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofLite                                                           //
//                                                                      //
// This class starts a PROOF session on the local machine: no daemons,  //
// client and master merged, communications via UNIX-like sockets.      //
// By default the number of workers started is NumberOfCores+1; a       //
// different number can be forced on construction.                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofLite.h"

#include "TDSet.h"
#include "TEnv.h"
#include "TError.h"
#include "TFile.h"
#include "THashList.h"
#include "TMessage.h"
#include "TMonitor.h"
#include "TObjString.h"
#include "TPluginManager.h"
#include "TProofQueryResult.h"
#include "TProofServ.h"
#include "TQueryResultManager.h"
#include "TROOT.h"
#include "TServerSocket.h"
#include "TSlave.h"
#include "TSortedList.h"
#include "TVirtualProofPlayer.h"

ClassImp(TProofLite)

//______________________________________________________________________________
TProofLite::TProofLite(const char *url, const char *conffile, const char *confdir,
                       Int_t loglevel, const char *alias, TProofMgr *mgr)
{
   // Create a PROOF environment. Starting PROOF involves either connecting
   // to a master server, which in turn will start a set of slave servers, or
   // directly starting as master server (if master = ""). Masterurl is of
   // the form: [proof[s]://]host[:port]. Conffile is the name of the config
   // file describing the remote PROOF cluster (this argument alows you to
   // describe different cluster configurations).
   // The default is proof.conf. Confdir is the directory where the config
   // file and other PROOF related files are (like motd and noproof files).
   // Loglevel is the log level (default = 1). User specified custom config
   // files will be first looked for in $HOME/.conffile.

   fUrl.SetUrl(url);

   // This may be needed during init
   fManager = mgr;

   // Default server type
   fServType = TProofMgr::kProofLite;

   // Default query mode
   fQueryMode = kSync;

   // Client and master are merged
   fMasterServ = kTRUE;
   SetBit(TProof::kIsClient);
   SetBit(TProof::kIsMaster);

   // Protocol and Host
   fUrl.SetProtocol("proof");
   fUrl.SetHost("__lite__");

   // User
   if (strlen(fUrl.GetUser()) <= 0) {
      // Get user logon name
      UserGroup_t *pw = gSystem->GetUserInfo();
      if (pw) {
         fUrl.SetUser(pw->fUser);
         delete pw;
      }
   }
   fMaster = gSystem->HostName();

   // Determine the number of workers giving priority to users request.
   // Otherwise use the system information, if available, or just start
   // the minimal number, i.e. 2 .
   fNWorkers = -1;
   if (url && strlen(url)) {
      TString o(url);
      Int_t in = o.Index("workers=");
      if (in != kNPOS) {
         o.Remove(0, in + strlen("workers="));
         while (!o.IsDigit())
            o.Remove(o.Length()-1);
         fNWorkers = (!o.IsNull()) ? o.Atoi() : fNWorkers;
      }
   }
   if (fNWorkers < 0) {
      SysInfo_t si;
      if (gSystem->GetSysInfo(&si) == 0) {
         fNWorkers = si.fCpus + 1;
      } else {
         // Two workers by default
         fNWorkers = 2;
      }
   }
   Printf(" +++ Starting PROOF-Lite with %d workers +++", fNWorkers);

   // Init the session now
   Init(url, conffile, confdir, loglevel, alias);

   // Still needed by the packetizers: needs to be changed
   gProof = this;
}

//______________________________________________________________________________
Int_t TProofLite::Init(const char *, const char *conffile,
                       const char *confdir, Int_t loglevel, const char *)
{
   // Start the PROOF environment. Starting PROOF involves either connecting
   // to a master server, which in turn will start a set of slave servers, or
   // directly starting as master server (if master = ""). For a description
   // of the arguments see the TProof ctor. Returns the number of started
   // master or slave servers, returns 0 in case of error, in which case
   // fValid remains false.

   R__ASSERT(gSystem);

   fValid = kFALSE;

   if (TestBit(TProof::kIsMaster)) {
      // Fill default conf file and conf dir
      if (!conffile || strlen(conffile) == 0)
         fConfFile = kPROOF_ConfFile;
      if (!confdir  || strlen(confdir) == 0)
         fConfDir  = kPROOF_ConfDir;
   } else {
      fConfDir     = confdir;
      fConfFile    = conffile;
   }

   // The sandbox for this session
   if (CreateSandbox() != 0) {
      Error("Init", "could not create/assert sandbox for this session");
      return 0;
   }

   // UNIX path for communication with workers
   fSockPath       = Form("%s/sockpath", fWorkDir.Data());

   fLogLevel       = loglevel;
   fProtocol       = kPROOF_Protocol;
   fSendGroupView  = kTRUE;
   fImage          = "<local>";
   fIntHandler     = 0;
   fStatus         = 0;
   fSlaveInfo      = 0;
   fChains         = new TList;
   fAvailablePackages = 0;
   fEnabledPackages = 0;
   fEndMaster      = TestBit(TProof::kIsMaster) ? kTRUE : kFALSE;

   // Timeout for some collect actions
   fCollectTimeout = gEnv->GetValue("Proof.CollectTimeout", -1);

   fProgressDialog        = 0;
   fProgressDialogStarted = kFALSE;

   // Client logging of messages from the workers
   fRedirLog = kFALSE;
   if (TestBit(TProof::kIsClient)) {
      fLogFileName = Form("%s/session-%s.log", fWorkDir.Data(), GetName());
      if ((fLogFileW = fopen(fLogFileName.Data(), "w")) == 0)
         Error("Init", "could not create temporary logfile %s", fLogFileName.Data());
      if ((fLogFileR = fopen(fLogFileName.Data(), "r")) == 0)
         Error("Init", "could not open logfile %s for reading", fLogFileName.Data());
   }
   fLogToWindowOnly = kFALSE;

   // Create 'queries' locker instance and lock it
   fQueryLock      = new TProofLockPath(Form("%s/%s%s-%s",
                     gSystem->TempDirectory(),
                     kPROOF_QueryLockFile, GetName(),
                     TString(fQueryDir).ReplaceAll("/","%").Data()));
   fQueryLock->Lock();
   // Create the query manager
   fQMgr = new TQueryResultManager(fQueryDir, GetName(), fWorkDir,
                                   fQueryLock, fLogFileW);

   // Status of cluster
   fIdle = kTRUE;

   // Query type
   fSync = kTRUE;

   // List of queries
   fQueries = 0;
   fOtherQueries = 0;
   fDrawQueries = 0;
   fMaxDrawQueries = 1;
   fSeqNum = 0;

   // Remote ID of the session
   fSessionID = -1;

   // Part of active query
   fWaitingSlaves = 0;

   // Make remote PROOF player
   fPlayer = 0;
   MakePlayer("lite");

   fFeedback = new TList;
   fFeedback->SetOwner();
   fFeedback->SetName("FeedbackList");
   AddInput(fFeedback);

   // Sort workers by descending performance index
   fSlaves           = new TSortedList(kSortDescending);
   fActiveSlaves     = new TList;
   fInactiveSlaves   = new TList;
   fUniqueSlaves     = new TList;
   fAllUniqueSlaves  = new TList;
   fNonUniqueMasters = new TList;
   fBadSlaves        = new TList;
   fAllMonitor       = new TMonitor;
   fActiveMonitor    = new TMonitor;
   fUniqueMonitor    = new TMonitor;
   fAllUniqueMonitor = new TMonitor;
   fCurrentMonitor   = 0;

   fPackageLock             = 0;
   fEnabledPackagesOnClient = 0;
   fGlobalPackageDirList    = 0;
   if (TestBit(TProof::kIsClient)) {

      // List of directories where to look for global packages
      TString globpack = gEnv->GetValue("Proof.GlobalPackageDirs","");
      if (globpack.Length() > 0) {
         Int_t ng = 0;
         Int_t from = 0;
         TString ldir;
         while (globpack.Tokenize(ldir, from, ":")) {
            if (gSystem->AccessPathName(ldir, kReadPermission)) {
               Warning("Init", "directory for global packages %s does not"
                               " exist or is not readable", ldir.Data());
            } else {
               // Add to the list, key will be "G<ng>", i.e. "G0", "G1", ...
               TString key = Form("G%d", ng++);
               if (!fGlobalPackageDirList) {
                  fGlobalPackageDirList = new THashList();
                  fGlobalPackageDirList->SetOwner();
               }
               fGlobalPackageDirList->Add(new TNamed(key,ldir));
            }
         }
      }

      UserGroup_t *ug = gSystem->GetUserInfo();
      fPackageLock = new TProofLockPath(Form("%s%s", kPROOF_PackageLockFile, ug->fUser.Data()));
      delete ug;

      fEnabledPackagesOnClient = new TList;
      fEnabledPackagesOnClient->SetOwner();
   }

   // Start workers
   if (SetupWorkers() != 0) {
      Error("Init", "problems setting up workers");
      return 0;
   }

   // we are now properly initialized
   fValid = kTRUE;

   // De-activate monitor (will be activated in Collect)
   fAllMonitor->DeActivateAll();

   // By default go into parallel mode
   GoParallel(9999, kFALSE);

   // Send relevant initial state to slaves
   SendInitialState();

   SetActive(kFALSE);

   if (IsValid()) {
      // Activate input handler
      ActivateAsyncInput();
      // Set PROOF to running state
      SetRunStatus(TProof::kRunning);
   }
   return fActiveSlaves->GetSize();
}
//______________________________________________________________________________
TProofLite::~TProofLite()
{
   // Destructor

   if (!(fQMgr && fQMgr->Queries() && fQMgr->Queries()->GetSize())) {
      // needed in case fQueryDir is on NFS ?!
      gSystem->MakeDirectory(fQueryDir+"/.delete");
      gSystem->Exec(Form("%s %s", kRM, fQueryDir.Data()));
   }

   // Remove lock file
   if (fQueryLock) {
      gSystem->Unlink(fQueryLock->GetName());
      fQueryLock->Unlock();
   }
}

//______________________________________________________________________________
Int_t TProofLite::SetupWorkers()
{
   // Start up PROOF workers.

   // Create server socket on the assigned UNIX sock path
   TServerSocket *ss = new TServerSocket(fSockPath);
   if (!ss || !ss->IsValid()) {
      Error("StartWorkers",
            "unable to create server socket for internal communications");
      SetBit(kInvalidObject);
      return -1;
   }

   // Create a monitor and add the socket to it
   TMonitor *mon = new TMonitor;
   mon->Add(ss);

   TList started;
   TSlave *wrk = 0;
   Int_t nWrksDone = 0, nWrksTot = fNWorkers;
   // Now we create the worker applications which will call us back to finalize
   // the setup
   Int_t ord = 0;
   for (; ord < fNWorkers; ord++) {

      // Ordinal for this worker server
      TString fullord = Form("0.%d", ord);

      // Create environment files
      SetProofServEnv(fullord);

      // Create worker server and add to the list
      if ((wrk = CreateSlave("lite", fullord, 100, fImage, fWorkDir)))
         started.Add(wrk);

      // Notify
      NotifyStartUp("Opening connections to workers", ++nWrksDone, nWrksTot);

   } //end of worker loop

   // Wait for call backs
   nWrksDone = 0;
   nWrksTot = started.GetSize();
   Int_t nSelects = 0;
   Int_t to = gEnv->GetValue("ProofLite.StartupTimeOut", 5) * 1000;
   while (started.GetSize() > 0 && nSelects < nWrksTot) {

      // Wait for activity on the socket for max 5 secs
      TSocket *xs = mon->Select(to);

      // Count attempts and check
      nSelects++;
      if (xs == (TSocket *) -1) continue;

      // Get the connection
      TSocket *s = ss->Accept();
      if (s && s->IsValid()) {
         // Receive ordinal
         TMessage *msg = 0;
         s->Recv(msg);
         if (msg) {
            TString ord;
            *msg >> ord;
            // Find who is calling back
            TSlave *wrk = (TSlave *) started.FindObject(ord);
            if (wrk) {
               // Remove it from the started list
               started.Remove(wrk);

               // Assign tis socket the selected worker
               wrk->SetSocket(s);
               // Remove socket from global TROOT socket list. Only the TProof object,
               // representing all worker sockets, will be added to this list. This will
               // ensure the correct termination of all proof servers in case the
               // root session terminates.
               {  R__LOCKGUARD2(gROOTMutex);
                  gROOT->GetListOfSockets()->Remove(s);
               }
               if (wrk->IsValid()) {
                  // Set the input handler
                  wrk->SetInputHandler(new TProofInputHandler(this, wrk->GetSocket()));
                  // Set fParallel to 1 for workers since they do not
                  // report their fParallel with a LOG_DONE message
                  wrk->fParallel = 1;
                  // Finalize setup of the server
                  wrk->SetupServ(TSlave::kSlave, 0);
               }

               // Monitor good workers
               if (wrk->IsValid()) {
                  fSlaves->Add(wrk);
                  fAllMonitor->Add(wrk->GetSocket());
                  // Notify startup operations
                  NotifyStartUp("Setting up worker servers", ++nWrksDone, nWrksTot);
               } else {
                  // Flag as bad
                  fBadSlaves->Add(wrk);
               }
            }
         }
      }
   }

   // Cleanup the monitor and the server socket
   mon->DeActivateAll();
   delete mon;
   delete ss;

   // Create Progress dialog, if needed
   if (!gROOT->IsBatch()) {
      if ((fProgressDialog =
         gROOT->GetPluginManager()->FindHandler("TProofProgressDialog")))
         if (fProgressDialog->LoadPlugin() == -1)
            fProgressDialog = 0;
   }

   // Done
   return 0;
}

//______________________________________________________________________________
void TProofLite::NotifyStartUp(const char *action, Int_t done, Int_t tot)
{
   // Notify setting-up operation message

   Int_t frac = (Int_t) (done*100.)/tot;
   char msg[512] = {0};
   if (frac >= 100) {
      sprintf(msg, "%s: OK (%d workers)                 \n",
                   action, tot);
   } else {
      sprintf(msg, "%s: %d out of %d (%d %%)\r",
                   action, done, tot, frac);
   }
   fprintf(stderr,"%s", msg);
}

//______________________________________________________________________________
Int_t TProofLite::SetProofServEnv(const char *ord)
{
   // Create environment files for worker 'ord'

   // Check input
   if (!ord || strlen(ord) <= 0) {
      Error("SetProofServEnv", "ordinal string undefined");
      return -1;
   }

   // ROOT env file
   TString rcfile(Form("%s/worker-%s.rootrc", fWorkDir.Data(), ord));
   FILE *frc = fopen(rcfile.Data(), "w");
   if (!frc) {
      Error("SetProofServEnv", "cannot open rc file %s", rcfile.Data());
      return -1;
   }

   // The session working dir depends on the role
   fprintf(frc,"# The session working dir\n");
   fprintf(frc,"ProofServ.SessionDir: %s/worker-%s\n", fWorkDir.Data(), ord);

   // Log / Debug level
   fprintf(frc,"# Proof Log/Debug level\n");
   fprintf(frc,"Proof.DebugLevel: %d\n", gDebug);

   // Ordinal number
   fprintf(frc,"# Ordinal number\n");
   fprintf(frc,"ProofServ.Ordinal: %s\n", ord);

   // ROOT Version tag
   fprintf(frc,"# ROOT Version tag\n");
   fprintf(frc,"ProofServ.RootVersionTag: %s\n", gROOT->GetVersion());

   // Work dir
   TString sandbox = gEnv->GetValue("ProofLite.Sandbox", Form("%s/%s",
                                     gSystem->WorkingDirectory(), kPROOF_WorkDir));
   fprintf(frc,"# Users sandbox\n");
   fprintf(frc, "ProofServ.Sandbox: %s\n", sandbox.Data());

   // Image
   fprintf(frc,"# Server image\n");
   fprintf(frc, "ProofServ.Image: %s\n", fImage.Data());

   // Set Open socket
   fprintf(frc,"# Open socket\n");
   fprintf(frc, "ProofServ.OpenSock: %s\n", fSockPath.Data());

   // Client Protocol
   fprintf(frc,"# Client Protocol\n");
   fprintf(frc, "ProofServ.ClientVersion: %d\n", kPROOF_Protocol);

   // ROOT env file created
   fclose(frc);

   // System env file
   TString envfile(Form("%s/worker-%s.env", fWorkDir.Data(), ord));
   FILE *fenv = fopen(envfile.Data(), "w");
   if (!fenv) {
      Error("SetProofServEnv", "cannot open env file %s", envfile.Data());
      return -1;
   }
   // ROOTSYS
   fprintf(fenv, "ROOTSYS=%s\n", gSystem->Getenv("ROOTSYS"));
   // Conf dir
   fprintf(fenv, "ROOTCONFDIR=%s\n", gSystem->Getenv("ROOTSYS"));
   // TMPDIR
   fprintf(fenv, "TMPDIR=%s\n", gSystem->TempDirectory());
   // Log file in the log dir
   TString logfile(Form("%s/worker-%s.log", fWorkDir.Data(), ord));
   fprintf(fenv, "ROOTPROOFLOGFILE=%s\n", logfile.Data());
   // RC file
   fprintf(fenv, "ROOTRCFILE=%s\n", rcfile.Data());
   // ROOT version tag (needed in building packages)
   fprintf(fenv, "ROOTVERSIONTAG=%s\n", gROOT->GetVersion());
   // Set the user envs
   if (fgProofEnvList) {
      TString namelist;
      TIter nxenv(fgProofEnvList);
      TNamed *env = 0;
      while ((env = (TNamed *)nxenv())) {
         fprintf(fenv, "%s=%s\n", env->GetName(), env->GetTitle());
         if (namelist.Length() > 0)
            namelist += ',';
         namelist += env->GetName();
      }
      fprintf(fenv, "PROOF_ALLVARS=%s\n", namelist.Data());
   }

   // System env file created
   fclose(fenv);

   // Done
   return 0;
}

//______________________________________________________________________________
Int_t TProofLite::AssertPath(const char *path, Bool_t writable)
{
   // Make sure that 'path' exists; if 'writable' is kTRUE, make also sure
   // that the path is writable

   if (!path || strlen(path) <= 0) {
      Error("AssertPath", "undefined input path");
      return -1;
   }

   if (gSystem->AccessPathName(path)) {
      if (gSystem->mkdir(path, kTRUE) != 0) {
         Error("AssertPath", "could not create path %s", path);
         return -1;
      }
   } else {
      // It must be writable
      if (gSystem->AccessPathName(path, kWritePermission) && writable) {
         if (gSystem->Chmod(path, 0666) != 0) {
            Error("AssertPath", "could not make path %s writable", path);
            return -1;
         }
      }
   }

   // Done
   return 0;
}

//______________________________________________________________________________
Int_t TProofLite::CreateSandbox()
{
   // Create the sandbox for this session

   // Make sure the sandbox area exist and is writable
   Bool_t needsymlink = kFALSE;
   TString sandbox = gEnv->GetValue("ProofLite.Sandbox", "");
   if (sandbox.IsNull()) {
      sandbox = Form("~/%s", kPROOF_WorkDir);
      needsymlink = kTRUE;
   }
   if (AssertPath(sandbox, kTRUE) != 0) return -1;

   // Package Dir
   fPackageDir = gEnv->GetValue("Proof.PackageDir", "");
   if (fPackageDir.IsNull())
      fPackageDir = Form("%s/%s", sandbox.Data(), kPROOF_PackDir);
   if (AssertPath(fPackageDir, kTRUE) != 0) return -1;

   // Cache Dir
   fCacheDir = gEnv->GetValue("Proof.CacheDir", "");
   if (fCacheDir.IsNull())
      fCacheDir = Form("%s/%s", sandbox.Data(), kPROOF_CacheDir);
   if (AssertPath(fCacheDir, kTRUE) != 0) return -1;

   // Data Set Dir
   fDataSetDir = gEnv->GetValue("Proof.DataSetDir", "");
   if (fDataSetDir.IsNull())
      fDataSetDir = Form("%s/%s", sandbox.Data(), kPROOF_DataSetDir);
   if (AssertPath(fDataSetDir, kTRUE) != 0) return -1;

   // Session unique tag (name of this TProof instance)
   TString stag(Form("%s-%d-%d", gSystem->HostName(), (int)time(0), gSystem->GetPid()));
   SetName(stag.Data());

   // Session working dir
   fWorkDir = Form("%s/%s/session-%s", gSystem->WorkingDirectory(), kPROOF_WorkDir, stag.Data());
   if (AssertPath(fWorkDir, kTRUE) != 0) return -1;

   // Create symlink to the last session
   TString lastsess(Form("%s/%s/last-lite-session", gSystem->WorkingDirectory(), kPROOF_WorkDir));
   gSystem->Unlink(lastsess);
   gSystem->Symlink(fWorkDir, lastsess);

   // Queries Dir: local to the working dir, unless required differently
   fQueryDir = gEnv->GetValue("Proof.QueryDir", "");
   if (fQueryDir.IsNull())
      fQueryDir = Form("%s/%s/%s", gSystem->WorkingDirectory(), kPROOF_WorkDir, kPROOF_QueryDir);
   if (AssertPath(fQueryDir, kTRUE) != 0) return -1;

   // Done
   return 0;
}

//______________________________________________________________________________
void TProofLite::Print(Option_t *option) const
{
   // Print status of PROOF-Lite cluster.

   if (IsParallel())
      Printf("*** PROOF-Lite cluster (parallel mode, %d workers):", GetParallel());
   else
      Printf("*** PROOF-Lite cluster (sequential mode)");

   Printf("Host name:                  %s", gSystem->HostName());
   Printf("User:                       %s", GetUser());
   TString ver(gROOT->GetVersion());
   if (gROOT->GetSvnRevision() > 0)
      ver += Form("|r%d", gROOT->GetSvnRevision());
   if (gSystem->Getenv("ROOTVERSIONTAG"))
      ver += Form("|%s", gSystem->Getenv("ROOTVERSIONTAG"));
   Printf("ROOT version|rev|tag:       %s", ver.Data());
   Printf("Architecture-Compiler:      %s-%s", gSystem->GetBuildArch(),
                                               gSystem->GetBuildCompilerVersion());
   Printf("Protocol version:           %d", GetClientProtocol());
   Printf("Working directory:          %s", gSystem->WorkingDirectory());
   Printf("Communication path:         %s", fSockPath.Data());
   Printf("Log level:                  %d", GetLogLevel());
   Printf("Number of workers:          %d", GetNumberOfSlaves());
   Printf("Number of active workers:   %d", GetNumberOfActiveSlaves());
   Printf("Number of unique workers:   %d", GetNumberOfUniqueSlaves());
   Printf("Number of inactive workers: %d", GetNumberOfInactiveSlaves());
   Printf("Number of bad workers:      %d", GetNumberOfBadSlaves());
   Printf("Total MB's processed:       %.2f", float(GetBytesRead())/(1024*1024));
   Printf("Total real time used (s):   %.3f", GetRealTime());
   Printf("Total CPU time used (s):    %.3f", GetCpuTime());
   if (TString(option).Contains("a", TString::kIgnoreCase) && GetNumberOfSlaves()) {
      Printf("List of workers:");
      TIter nextslave(fSlaves);
      while (TSlave* sl = dynamic_cast<TSlave*>(nextslave())) {
         if (sl->IsValid())
            sl->Print(option);
      }
   }
}

//______________________________________________________________________________
TProofQueryResult *TProofLite::MakeQueryResult(Long64_t nent, const char *opt,
                                               Long64_t fst, TDSet *dset,
                                               const char *selec)
{
   // Create a TProofQueryResult instance for this query.

   // Increment sequential number
   if (fQMgr) fQMgr->IncrementSeqNum();

   // Create the instance and add it to the list
   TProofQueryResult *pqr = new TProofQueryResult(fQMgr->SeqNum(), opt,
                                                  fPlayer->GetInputList(), nent,
                                                  fst, dset, selec, dset->GetEntryList());
   // Title is the session identifier
   pqr->SetTitle(GetName());

   return pqr;
}

//______________________________________________________________________________
void TProofLite::SetQueryRunning(TProofQueryResult *pq)
{
   // Set query in running state.

   // Record current position in the log file at start
   fflush(fLogFileW);
   Int_t startlog = lseek(fileno(fLogFileW), (off_t) 0, SEEK_END);

   // Add some header to logs
   Printf(" ");
   Info("SetQueryRunning", "starting query: %d", pq->GetSeqNum());

   // Build the list of loaded PAR packages
   TString parlist = "";
   TIter nxp(fEnabledPackagesOnClient);
   TObjString *os= 0;
   while ((os = (TObjString *)nxp())) {
      if (parlist.Length() <= 0)
         parlist = os->GetName();
      else
         parlist += Form(";%s",os->GetName());
   }

   // Set in running state
   pq->SetRunning(startlog, parlist);

   // Bytes and CPU at start (we will calculate the differential at end)
   pq->SetProcessInfo(pq->GetEntries(), GetCpuTime(), GetBytesRead());
}

//______________________________________________________________________________
TList *TProofLite::GetDataSet(const char *name)
{
   // Utility function used in various methods for user dataset upload.

   TString fileListPath;
   if (strchr(name, '~') == name) {
      char *nameCopy = new char[strlen(name)];
      strcpy(nameCopy, name + 1);
      char *userName = strtok(nameCopy, "/");
      if (strcmp(strtok(0, "/"), "public"))
         return 0;
      fileListPath = fWorkDir + "/../" + userName + "/"
                     + kPROOF_DataSetDir + "/public/";
      delete[] nameCopy;
   } else if (strchr(name, '/') && strstr(name, "public") != name) {
      Printf("Dataset name should be of form [[~user/]public/]dataset");
      return 0;
   } else
      fileListPath = fDataSetDir + "/" + name + ".root";
   TList *fileList = 0;
   if (gSystem->AccessPathName(fileListPath.Data(), kFileExists) == kFALSE) {
      TFile *f = TFile::Open(fileListPath);
      f->cd();
      fileList = (TList *) f->Get("fileList");
      f->Close();
      delete f;
      if (strchr(name, '~') == name)  // not when allocated with Form
         delete[] fileListPath;
   }
   return fileList;
}

//______________________________________________________________________________
Long64_t TProofLite::Process(TDSet *dset, const char *selector, Option_t *option,
                             Long64_t nentries, Long64_t first)
{
   // Process a data set (TDSet) using the specified selector (.C) file.
   // Entry- or event-lists should be set in the data set object using
   // TDSet::SetEntryList.
   // The return value is -1 in case of error and TSelector::GetStatus() in
   // in case of success.

   if (!IsValid() || !fQMgr) {
      Error("Process", "invalid sesion or query-result manager undefined!");
      return -1;
   }

   Bool_t hasNoData = (dset->TestBit(TDSet::kEmpty)) ? kTRUE : kFALSE;

   // If just a name was given to identify the dataset, retrieve it from the
   // local files
   if ((!hasNoData) && dset->GetListOfElements()->GetSize() == 0) {
      TList *files = GetDataSet(dset->GetName());
      if (!files) {
         Error("Process", "no such dataset: %s", dset->GetName());
         return -1;
      }
      files->SetOwner();
      if (!dset->Add(files)) {
         Error("Process", "Error retrieving dataset %s", dset->GetName());
         delete files;
         return -1;
      }
      delete files;
   }

   // Create instance of query results
   TProofQueryResult *pq = MakeQueryResult(nentries, option, first, dset, selector);

   // If not a draw action add the query to the main list
   if (!(pq->IsDraw())) {
      if (fQMgr->Queries()) fQMgr->Queries()->Add(pq);
      // Also save it to queries dir
      fQMgr->SaveQuery(pq);
   }

   // Expand selector files
   if (pq->GetSelecImp()) {
      gSystem->Exec(Form("%s %s", kRM, pq->GetSelecImp()->GetName()));
      pq->GetSelecImp()->SaveSource(pq->GetSelecImp()->GetName());
   }
   if (pq->GetSelecHdr() &&
         !strstr(pq->GetSelecHdr()->GetName(), "TProofDrawHist")) {
      gSystem->Exec(Form("%s %s", kRM, pq->GetSelecHdr()->GetName()));
      pq->GetSelecHdr()->SaveSource(pq->GetSelecHdr()->GetName());
   }

   // Set in running state
   SetQueryRunning(pq);

   // Save to queries dir, if not standard draw
   if (!(pq->IsDraw()))
      fQMgr->SaveQuery(pq);
   else
      fQMgr->IncrementDrawQueries();


   // Start or reset the progress dialog
   if (!gROOT->IsBatch()) {
      Int_t dsz = dset->GetListOfElements()->GetSize();
      if (fProgressDialog && !TestBit(kUsingSessionGui)) {
         if (!fProgressDialogStarted) {
            fProgressDialog->ExecPlugin(5, this, selector, dsz,
                                           first, nentries);
            fProgressDialogStarted = kTRUE;
         } else {
            ResetProgressDialog(selector, dsz, first, nentries);
         }
      }
      ResetBit(kUsingSessionGui);
   }

   // Add query results to the player lists
   fPlayer->AddQueryResult(pq);

   // Set query currently processed
   fPlayer->SetCurrentQuery(pq);

   // Make sure the unique query tag is available as TNamed object in the
   // input list so that it can be used in TSelectors for monitoring
   TNamed *qtag = (TNamed *) fPlayer->GetInputList()->FindObject("PROOF_QueryTag");
   if (qtag) {
      qtag->SetTitle(Form("%s:%s",pq->GetTitle(),pq->GetName()));
   } else {
      fPlayer->AddInput(new TNamed("PROOF_QueryTag",
                                   Form("%s:%s",pq->GetTitle(),pq->GetName())));
   }

   // Set PROOF to running state
   SetRunStatus(TProof::kRunning);

   // Resolve query mode
   fSync = (GetQueryMode(option) == kSync);

   if (fSync && !IsIdle()) {
      Info("Process","not idle, cannot submit synchronous query");
      return -1;
   }

   // deactivate the default application interrupt handler
   // ctrl-c's will be forwarded to PROOF to stop the processing
   TSignalHandler *sh = 0;
   if (fSync) {
      if (gApplication)
         sh = gSystem->RemoveSignalHandler(gApplication->GetSignalHandler());
   }

   Long64_t rv = fPlayer->Process(dset, selector, option, nentries, first);

   if (fSync) {
      // reactivate the default application interrupt handler
      if (sh)
         gSystem->AddSignalHandler(sh);
   }

   // Return number of events processed
   if (fPlayer->GetExitStatus() != TVirtualProofPlayer::kFinished) {
      Bool_t abort = (fPlayer->GetExitStatus() == TVirtualProofPlayer::kAborted)
                   ? kTRUE : kFALSE;
      if (abort) fPlayer->StopProcess(kTRUE);
      Emit("StopProcess(Bool_t)", abort);
   }

   // Complete filling of the TQueryResult instance
   AskStatistics();
   if (fQMgr->FinalizeQuery(pq, this, fPlayer))
      fQMgr->SaveQuery(pq, -1);

   // Remove aborted queries from the list
   if (fPlayer->GetExitStatus() == TVirtualProofPlayer::kAborted) {
      if (fQMgr) fQMgr->RemoveQuery(pq);
   } else {
      // If the last object, notify the GUI that the result arrived
      QueryResultReady(Form("%s:%s", pq->GetTitle(), pq->GetName()));
      // Keep in memory only light infor about a query
      if (!(pq->IsDraw())) {
         if (fQMgr->Queries()) {
            TQueryResult *pqr = pq->CloneInfo();
            if (pqr)
               fQMgr->Queries()->Add(pqr);
            // Remove from the fQueries list
            fQMgr->Queries()->Remove(pq);
         }
      }
   }

   // Done
   return rv;
}
