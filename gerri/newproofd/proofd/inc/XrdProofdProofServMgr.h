// @(#)root/proofd:$Id:$
// Author: G. Ganis Jan 2008

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_XrdProofdProofServMgr
#define ROOT_XrdProofdProofServMgr

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// XrdProofdProofServMgr                                                  //
//                                                                      //
// Author: G. Ganis, CERN, 2008                                         //
//                                                                      //
// Class managing proofserv sessions manager.                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <list>

#ifdef OLDXRDOUC
#  include "XrdSysToOuc.h"
#  include "XrdOuc/XrdOucSemWait.hh"
#else
#  include "XrdSys/XrdSysSemWait.hh"
#endif

#include "XrdOuc/XrdOucHash.hh"
#include "XrdOuc/XrdOucString.hh"

#include "XrdProofdConfig.h"
#include "XrdProofServProxy.h"

class XrdOucStream;
class XrdProtocol_Config;
class XrdProofdManager;
class XrdScheduler;

class XrdProofSessionInfo {
public:
   time_t         fLastAccess;
   XrdOucString   fUser;
   XrdOucString   fGroup;
   XrdOucString   fUnixPath;
   XrdProofSessionInfo(const char *u, const char *g, const char *p) :
                       fLastAccess(0), fUser(u), fGroup(g), fUnixPath(p) { }
   XrdProofSessionInfo(const char *file) { ReadFromFile(file); }

   int ReadFromFile(const char *file);
   void Reset();
   int SaveToFile(const char *file);
};

class XrdProofdProofServMgr : public XrdProofdConfig {

   XrdProofdManager  *fMgr;
   XrdSysSemWait      fForkSem;   // To serialize fork requests
   XrdScheduler      *fSched;     // System scheduler
   int                fInternalWait;   // Timeout on replies from proofsrv
   XrdOucString       fProofServEnvs;  // Additional envs to be exported before proofserv
   XrdOucString       fProofServRCs;   // Additional rcs to be passed to proofserv

   int                fShutdownOpt;    // What to do when a client disconnects
   int                fShutdownDelay;  // Delay shutdown by this (if enabled)

   int                fPipe[2]; // pipe for the poller

   int                fCheckFrequency;
   int                fTerminationTimeOut;
   int                fVerifyTimeOut;

   XrdOucString       fActiAdminPath; // Active sessions admin area
   XrdOucString       fTermAdminPath; // Terminated sessions admin area

   XrdOucHash<XrdProofServProxy> fSessions; // List of sessions
   std::list<XrdProofServProxy *> fActiveSessions;     // List of active sessions (non-idle)

   int                DoDirectiveProofServMgr(char *, XrdOucStream *, bool);
   int                DoDirectivePutEnv(char *, XrdOucStream *, bool);
   int                DoDirectivePutRc(char *, XrdOucStream *, bool);
   int                DoDirectiveShutdown(char *, XrdOucStream *, bool);

   // Session Admin path management
   XrdOucString       GetSessionPath(const char *root, int pid);
   int                GetSessionInfo(int pid, XrdProofSessionInfo &info);
   int                AddSession(int pid, XrdProofdClient *c);
   int                RmSession(int pid);
   int                TouchSession(int pid, const char *path = 0);

public:
   XrdProofdProofServMgr(XrdProofdManager *mgr, XrdProtocol_Config *pi, XrdSysError *e);
   virtual ~XrdProofdProofServMgr() { }

   int               Config(bool rcf = 0);
   int               DoDirective(XrdProofdDirective *d,
                                 char *val, XrdOucStream *cfg, bool rcf);
   void              RegisterDirectives();

   int               CheckFrequency() const { return fCheckFrequency; }
   int               InternalWait() const { return fInternalWait; }

   int               Process(XrdProofdProtocol *p);

   int               Accept(XrdProofdProtocol *p, XrdProofServProxy *xps);
   int               Attach(XrdProofdProtocol *p);
   int               Create(XrdProofdProtocol *p);
   int               Destroy(XrdProofdProtocol *p);
   int               Detach(XrdProofdProtocol *p);

   int               BroadcastPriorities();

#if 0
   void              AddActiveSession(XrdProofServProxy *p) { fActiveSessions.push_back(p); }
   void              RemoveActiveSession(XrdProofServProxy *p) { fActiveSessions.remove(p); }
#else
   std::list<XrdProofServProxy *> *ActiveSessions() { return &fActiveSessions; }
   XrdProofServProxy *GetActiveSession(int pid);
#endif

   int               CleanupProofServ(bool all = 0, const char *usr = 0);

   int               SetProofServEnv(XrdProofdProtocol *p,
                                     int psid = -1, int loglevel = -1, const char *cfg = 0);
   int               SetProofServEnvOld(XrdProofdProtocol *p,
                                        int psid = -1, int loglevel = -1, const char *cfg = 0);
   int               SaveAFSkey(XrdSecCredentials *c, const char *fn);
   int               SetUserEnvironment(XrdProofdProtocol *p);

   static int        SetProofServEnv(XrdProofdManager *m, XrdROOT *r);

   int               ReadFd() const { return fPipe[0]; }
   int               WriteFd() const { return fPipe[1]; }

   // Checks run periodically by the cron job
   int               DeleteFromSessions(int pid);
   int               MvSession(int pid);
   int               CheckActiveSessions();
   int               CheckTerminatedSessions();

};
#endif
