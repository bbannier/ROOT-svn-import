// @(#)root/proofd:$Id$
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
#include "XrdProofdProofServ.h"

class XrdOucStream;
class XrdProtocol_Config;
class XrdProofdManager;
class XrdROOTMgr;
class XrdScheduler;

class XpdClientSessions {
public:
   XrdProofdClient *fClient;
   std::list<XrdProofdProofServ *> fProofServs;
   XpdClientSessions(XrdProofdClient *c) : fClient(c) { }
};

class XrdProofSessionInfo {
public:
   time_t         fLastAccess;
   int            fPid;
   int            fID;
   int            fSrvType;
   int            fStatus;
   XrdOucString   fUser;
   XrdOucString   fGroup;
   XrdOucString   fUnixPath;
   XrdOucString   fTag;
   XrdOucString   fAlias;
   XrdOucString   fLogFile;
   XrdOucString   fOrdinal;
   XrdOucString   fUserEnvs;
   XrdOucString   fROOTTag;
   int            fSrvProtVers;

   XrdProofSessionInfo(XrdProofdClient *c, XrdProofdProofServ *s);
   XrdProofSessionInfo(const char *file) { ReadFromFile(file); }

   void FillProofServ(XrdProofdProofServ &s, XrdROOTMgr *rmgr);
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
   int                fReconnectTime;
   int                fReconnectTimeOut;
   int                fRecoverTimeOut;

   XrdOucString       fActiAdminPath; // Active sessions admin area
   XrdOucString       fTermAdminPath; // Terminated sessions admin area

   XrdOucHash<XrdProofdProofServ> fSessions; // List of sessions
   std::list<XrdProofdProofServ *> fActiveSessions;     // List of active sessions (non-idle)

   int                DoDirectiveProofServMgr(char *, XrdOucStream *, bool);
   int                DoDirectivePutEnv(char *, XrdOucStream *, bool);
   int                DoDirectivePutRc(char *, XrdOucStream *, bool);
   int                DoDirectiveShutdown(char *, XrdOucStream *, bool);

   int                RecoverActiveSessions();
   int                ResolveSession(int pid, std::list<XpdClientSessions> *cls);

   // Session Admin path management
   int                GetSessionInfo(int pid, XrdProofSessionInfo &info);
   int                AddSession(XrdProofdClient *c, XrdProofdProofServ *s);
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

   bool              IsReconnecting();
   void              SetReconnectTime(bool on = 1);

   int               Process(XrdProofdProtocol *p);

   XrdProofdProofServ *Accept(XrdProofdClient *c, int to, XrdOucString &e);
   int               Attach(XrdProofdProtocol *p);
   int               Create(XrdProofdProtocol *p);
   int               Destroy(XrdProofdProtocol *p);
   int               Detach(XrdProofdProtocol *p);
   int               Recover(XpdClientSessions *cl);

   int               BroadcastPriorities();

   std::list<XrdProofdProofServ *> *ActiveSessions() { return &fActiveSessions; }
   XrdProofdProofServ *GetActiveSession(int pid);

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
