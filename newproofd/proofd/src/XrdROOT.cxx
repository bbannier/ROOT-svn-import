// @(#)root/proofd:$Id$
// Author: Gerardo Ganis  June 2007

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// XrdROOT                                                              //
//                                                                      //
// Authors: G. Ganis, CERN, 2007                                        //
//                                                                      //
// Class describing a ROOT version                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "XrdProofdPlatform.h"

#include "XrdROOT.h"
#include "XrdProofdManager.h"
#include "XrdProofdProtocol.h"
#include "XrdProofdProofServMgr.h"
#include "XrdOuc/XrdOucStream.hh"
#include "XrdSys/XrdSysPriv.hh"

// Tracing
#include "XrdProofdTrace.h"
static const char *gTraceID = "";
extern XrdOucTrace *XrdProofdTrace;
#define TRACEID gTraceID

//__________________________________________________________________________
XrdROOT::XrdROOT(const char *dir, const char *tag)
{
   // Constructor: validates 'dir', gets the version and defines the tag.

   fStatus = -1;
   fSrvProtVers = -1;

   // 'dir' must make sense
   if (!dir || strlen(dir) <= 0)
      return;
   if (tag && strlen(tag) > 0) {
      fExport = tag;
      fExport += " "; fExport += dir;
   } else
      fExport += dir;

   // The path should exist and be statable
   struct stat st;
   if (stat(dir, &st) == -1) {
      XPDERR("XrdROOT: unable to stat path "<<dir);
      return;
   }
   // ... and be a directory
   if (!S_ISDIR(st.st_mode)) {
      XPDERR("XrdROOT: path "<<dir<<" is not a directory");
      return;
   }
   fDir = dir;

   // Get the version
   XrdOucString version;
   if (GetROOTVersion(dir, version) == -1) {
      XPDERR("XrdROOT: unable to extract ROOT version from path "<<dir);
      return;
   }

   // Default tag is the version
   fTag = (!tag || strlen(tag) <= 0) ? version : tag;

   // The application to be run
   fPrgmSrv = dir;
   fPrgmSrv += "/bin/proofserv";

   // Export string
   fExport = fTag;
   fExport += " "; fExport += version;
   fExport += " "; fExport += dir;

   // First step OK
   fStatus = 0;
}

//__________________________________________________________________________
void XrdROOT::SetValid(kXR_int16 vers)
{
   // Set valid, save protocol and finalize the export string

   fStatus = 1;

   if (vers > 0) {
      // Cleanup export, if needed
      if (fSrvProtVers > 0) {
         XrdOucString vs(" ");
         vs += fSrvProtVers;
         fExport.replace(vs,XrdOucString(""));
      }
      fSrvProtVers = vers;

      // Finalize export string
      fExport += " ";
      fExport += (int)fSrvProtVers;
   }
}

//__________________________________________________________________________
int XrdROOT::GetROOTVersion(const char *dir, XrdOucString &version)
{
   // Get ROOT version associated with 'dir'.

   int rc = -1;

   XrdOucString versfile = dir;
   versfile += "/include/RVersion.h";

   // Open file
   FILE *fv = fopen(versfile.c_str(), "r");
   if (!fv) {
      XPDERR("XrdROOT::GetROOTVersion: unable to open "<<versfile);
      return rc;
   }

   // Read the file
   char line[1024];
   while (fgets(line, sizeof(line), fv)) {
      char *pv = (char *) strstr(line, "ROOT_RELEASE");
      if (pv) {
         if (line[strlen(line)-1] == '\n')
            line[strlen(line)-1] = 0;
         pv += strlen("ROOT_RELEASE") + 1;
         version = pv;
         version.replace("\"","");
         rc = 0;
         break;
      }
   }

   // Close the file
   fclose(fv);

   // Done
   return rc;
}

//
// Manager

//______________________________________________________________________________
XrdROOTMgr::XrdROOTMgr(XrdProofdManager *mgr,
                       XrdProtocol_Config *pi, XrdSysError *e)
          : XrdProofdConfig(pi->ConfigFN, e)
{
   // Constructor

   fMgr = mgr;
   fSched = pi->Sched;
   fROOT.clear();

   // Configuration directives
   RegisterDirectives();
}

//__________________________________________________________________________
int XrdROOTMgr::Config(bool rcf)
{
   // Run configuration and parse the entered config directives.
   // Return 0 on success, -1 on error

   // Run first the configurator
   if (XrdProofdConfig::Config(rcf) != 0) {
      fEDest->Say(0, "xpd: Config: ROOTMgr: problems parsing file ");
      return -1;
   }

   XrdOucString msg;
   msg = (rcf) ? "xpd: Config: ROOTMgr: re-configuring"
               : "xpd: Config: ROOTMgr: configuring";
   fEDest->Say(0, msg.c_str());

   // ROOT dirs
   if (rcf) {
      // Remove parked ROOT sys entries
      std::list<XrdROOT *>::iterator tri;
      if (fROOT.size() > 0) {
         for (tri = fROOT.begin(); tri != fROOT.end();) {
            if ((*tri)->IsParked()) {
               delete (*tri);
               tri = fROOT.erase(tri);
            } else {
               tri++;
            }
         }
      }
   } else {
      // Check the ROOT dirs
      if (fROOT.size() <= 0) {
         // None defined: use ROOTSYS as default, if any; otherwise we fail
         if (getenv("ROOTSYS")) {
            XrdROOT *rootc = new XrdROOT(getenv("ROOTSYS"), "");
            msg = "ROOTMgr : Config: ROOT dist: \"";
            msg += rootc->Export();
            if (Validate(rootc, fSched) == 0) {
               msg += "\" validated";
               fROOT.push_back(rootc);
            } else {
               msg += "\" could not be validated";
            }
            fEDest->Say(0, msg.c_str());
        }
         if (fROOT.size() <= 0) {
            fEDest->Say(0, "ROOTMgr : Config: no ROOT dir defined;"
                           " ROOTSYS location missing - unloading");
            return 0;
         }
      }
   }

   // Done
   return 0;
}

//__________________________________________________________________________
void XrdROOTMgr::RegisterDirectives()
{
   // Register directives for configuration

   Register("rootsys", new XrdProofdDirective("rootsys", this, &DoDirectiveClass));
}

//______________________________________________________________________________
int XrdROOTMgr::DoDirective(XrdProofdDirective *d,
                            char *val, XrdOucStream *cfg, bool rcf)
{
   // Update the priorities of the active sessions.

   if (!d)
      // undefined inputs
      return -1;

   if (d->fName == "rootsys") {
      return DoDirectiveRootSys(val, cfg, rcf);
   }
   TRACE(XERR,"DoDirective: unknown directive: "<<d->fName);
   return -1;
}

//______________________________________________________________________________
int XrdROOTMgr::DoDirectiveRootSys(char *val, XrdOucStream *cfg, bool)
{
   // Process 'rootsys' directive

   if (!val || !cfg)
      // undefined inputs
      return -1;

   // Two tokens may be meaningful
   XrdOucString dir = val;
   val = cfg->GetToken();
   XrdOucString tag = val;
   bool ok = 1;
   if (tag == "if") {
      tag = "";
      // Conditional
      cfg->RetToken();
      ok = (XrdProofdAux::CheckIf(cfg, fMgr->Host()) > 0) ? 1 : 0;
   }
   if (ok) {
      XrdROOT *rootc = new XrdROOT(dir.c_str(), tag.c_str());
      // Check if already validated
      std::list<XrdROOT *>::iterator ori;
      for (ori = fROOT.begin(); ori != fROOT.end(); ori++) {
         if ((*ori)->Match(rootc->Dir(), rootc->Tag())) {
            if ((*ori)->IsParked()) {
               (*ori)->SetValid();
               SafeDelete(rootc);
               break;
            }
         }
      }
      // If not, try validation
      if (rootc) {
         if (Validate(rootc, fSched) == 0) {
            XPDPRT("DoDirectiveRootSys: validation OK for: "<<rootc->Export());
            // Add to the list
            fROOT.push_back(rootc);
         } else {
            XPDPRT("DoDirectiveRootSys: could not validate "<<rootc->Export());
            SafeDelete(rootc);
         }
      }
   }
   return 0;
}

//__________________________________________________________________________
int XrdROOTMgr::Validate(XrdROOT *r, XrdScheduler *sched)
{
   // Start a trial server application to test forking and get the version
   // of the protocol run by the PROOF server.
   // Return 0 if everything goes well, -1 in case of any error.

   XPDPRT("XrdROOTMgr::Validate: forking test and protocol retrieval");

   if (r->IsInvalid()) {
      // Cannot be validated
      XPDERR("XrdROOTMgr::Validate: invalid instance - cannot be validated");
      return -1;
   }

   // Make sure the application path has been defined
   if (!r->PrgmSrv() || strlen(r->PrgmSrv()) <= 0) {
      XPDERR("XrdROOTMgr::Validate: "
            " path to PROOF server application undefined - exit");
      return -1;
   }

   // Make sure the scheduler is defined
   if (!sched) {
      XPDERR("XrdROOTMgr::Validate: "
            " scheduler undefined - exit");
      return -1;
   }

   // Pipe to communicate the protocol number
   int fp[2];
   if (pipe(fp) != 0) {
      XPDERR("XrdROOTMgr::Validate: unable to generate pipe for"
            " PROOT protocol number communication");
      return -1;
   }

   // Fork a test agent process to handle this session
   TRACE(FORK,"XrdROOTMgr::Validate: forking external proofsrv");
   int pid = -1;
   if (!(pid = sched->Fork("proofsrv"))) {

      char *argvv[5] = {0};

      // start server
      argvv[0] = (char *)r->PrgmSrv();
      argvv[1] = (char *)"proofserv";
      argvv[2] = (char *)"xpd";
      argvv[3] = (char *)"test";
      argvv[4] = 0;

      // Set basic environment for proofserv
      if (XrdProofdProofServMgr::SetProofServEnv(fMgr, r) != 0) {
         TRACE(XERR, "XrdROOTMgr::Validate:"
                       " SetProofServEnv did not return OK - EXIT");
         exit(1);
      }

      // Set Open socket
      char *ev = new char[25];
      sprintf(ev, "ROOTOPENSOCK=%d", fp[1]);
      putenv(ev);

      // Prepare for execution: we need to acquire the identity of
      // a normal user
      if (!getuid()) {
         XrdProofUI ui;
         if (XrdProofdAux::GetUserInfo(geteuid(), ui) != 0) {
            TRACE(XERR, "XrdROOTMgr::Validate:"
                          " could not get info for user-id: "<<geteuid());
            exit(1);
         }

         // acquire permanently target user privileges
         if (XrdSysPriv::ChangePerm((uid_t)ui.fUid, (gid_t)ui.fGid) != 0) {
            TRACE(XERR, "XrdROOTMgr::Validate: can't acquire "<<
                          ui.fUser <<" identity");
            exit(1);
         }

      }

      // Run the program
      execv(r->PrgmSrv(), argvv);

      // We should not be here!!!
      TRACE(XERR, "XrdROOTMgr::Validate:"
                    " returned from execv: bad, bad sign !!!");
      exit(1);
   }

   // parent process
   if (pid < 0) {
      XPDERR("XrdROOTMgr::Validate: forking failed - exit");
      close(fp[0]);
      close(fp[1]);
      return -1;
   }

   // now we wait for the callback to be (successfully) established
   TRACE(FORK, "XrdROOTMgr::Validate:"
               " test server launched: wait for protocol ");

   // Read protocol
   int proto = -1;
   struct pollfd fds_r;
   fds_r.fd = fp[0];
   fds_r.events = POLLIN;
   int pollRet = 0;
   // We wait for 60 secs max (30 x 2000 millisecs): this is enough to
   // cover possible delays due to heavy load
   int ntry = 30;
   while (pollRet == 0 && ntry--) {
      while ((pollRet = poll(&fds_r, 1, 2000)) < 0 &&
             (errno == EINTR)) { }
      if (pollRet == 0)
         TRACE(DBG,"XrdROOTMgr::Validate: "
                   "receiving PROOF server protocol number: waiting 2 s ...");
   }
   if (pollRet > 0) {
      if (read(fp[0], &proto, sizeof(proto)) != sizeof(proto)) {
         XPDERR("Validate: "
               " XrdROOT::problems receiving PROOF server protocol number");
         return -1;
      }
   } else {
      if (pollRet == 0) {
         XPDERR("XrdROOTMgr::Validate: "
               " timed-out receiving PROOF server protocol number");
      } else {
         XPDERR("XrdROOTMgr::Validate: "
               " failed to receive PROOF server protocol number");
      }
      return -1;
   }

   // Set valid, record protocol and update export string
   r->SetValid((kXR_int16) ntohl(proto));

   // Cleanup
   close(fp[0]);
   close(fp[1]);

   // We are done
   return 0;
}

//______________________________________________________________________________
XrdOucString XrdROOTMgr::ExportVersions(XrdROOT *def)
{
   // Return a string describing the available versions, with the default
   // version 'def' markde with a '*'

   XrdOucString out;

   // Generic info about all known sessions
   std::list<XrdROOT *>::iterator ip;
   for (ip = fROOT.begin(); ip != fROOT.end(); ++ip) {
      // Flag the default one
      if (def == *ip)
         out += "  * ";
      else
         out += "    ";
      out += (*ip)->Export();
      out += "\n";
   }

   // Over
   return out;
}

//______________________________________________________________________________
XrdROOT *XrdROOTMgr::GetVersion(const char *tag)
{
   // Return pointer to the ROOT version corresponding to 'tag'
   // or 0 if not found.

   XrdROOT *r = 0;

   std::list<XrdROOT *>::iterator ip;
   for (ip = fROOT.begin(); ip != fROOT.end(); ++ip) {
      if ((*ip)->MatchTag(tag)) {
         r = (*ip);
         break;
      }
   }

   // Over
   return r;
}
