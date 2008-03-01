// @(#)root/proofd:$Id$
// Author: G. Ganis  June 2007

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_XrdProofdAux
#define ROOT_XrdProofdAux

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// XrdProofdAux                                                          //
//                                                                      //
// Authors: G. Ganis, CERN, 2007                                        //
//                                                                      //
// Small auxilliary classes used in XrdProof                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
#include <stdarg.h>

#ifdef OLDXRDOUC
#  include "XrdSysToOuc.h"
#  include "XrdOuc/XrdOucSemWait.hh"
#else
#  include "XrdSys/XrdSysSemWait.hh"
#endif

#include "Xrd/XrdProtocol.hh"
#include "XProofProtocol.h"
#include "XrdOuc/XrdOucHash.hh"
#include "XrdOuc/XrdOucString.hh"

//
// User Info class
//
class XrdProofUI {
public:
   XrdOucString fUser;       // User name
   XrdOucString fGroup;      // PROOF group name
   XrdOucString fHomeDir;    // Unix home
   int          fUid;        // Unix user ID
   int          fGid;        // Unix group ID

   XrdProofUI() { fUid = -1; fGid = -1; }
   XrdProofUI(const XrdProofUI &ui) { fUser = ui.fUser;
                                      fGroup = ui.fGroup;
                                      fHomeDir = ui.fHomeDir;
                                      fUid = ui.fUid; fGid = ui.fGid; }
   ~XrdProofUI() { }

   void Reset() { fUser = ""; fHomeDir = ""; fGroup = ""; fUid = -1; fGid = -1; }
};

//
// File container (e.g. for config files)
//
class XrdProofdFile {
public:
   XrdOucString  fName;  // File name
   time_t        fMtime; // File mofification time last time we accessed it
   XrdProofdFile(const char *fn = 0, time_t mtime = 0) : fName(fn), fMtime(mtime) { }
};

//
// User priority
//
class XrdProofdPriority {
public:
   XrdOucString            fUser;          // User to who this applies (wild cards accepted)
   int                     fDeltaPriority; // Priority change
   XrdProofdPriority(const char *usr, int dp) : fUser(usr), fDeltaPriority(dp) { }
};

//
// Small class to describe a process
//
class XrdProofdPInfo {
public:
   int pid;
   XrdOucString pname;
   XrdProofdPInfo(int i, const char *n) : pid(i) { pname = n; }
};

//
// Class to handle configuration directives
//
class XrdProofdDirective;
class XrdOucStream;
typedef int (*XrdFunDirective_t)(XrdProofdDirective *, char *,
                                 XrdOucStream *cfg, bool reconfig);
class XrdProofdDirective {
public:
   void              *fVal;
   XrdOucString       fName;
   XrdFunDirective_t  fFun;
   bool               fRcf;
   const char        *fHost; // needed to support old 'if' construct

   XrdProofdDirective(const char *n, void *v, XrdFunDirective_t f, bool rcf = 1) :
                      fVal(v), fName(n), fFun(f), fRcf(rcf) { }

   int DoDirective(char *val, XrdOucStream *cfg, bool reconfig)
                      { return (*fFun)(this, val, cfg, reconfig); }
};
// Function of general interest
int DoDirectiveClass(XrdProofdDirective *, char *val, XrdOucStream *cfg, bool rcf);
int DoDirectiveInt(XrdProofdDirective *, char *val, XrdOucStream *cfg, bool rcf);
int DoDirectiveString(XrdProofdDirective *, char *val, XrdOucStream *cfg, bool rcf);
// To set the host field in a loop over the hash list
int SetHostInDirectives(const char *, XrdProofdDirective *d, void *h);

//
// Static methods
//
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
typedef struct kinfo_proc kinfo_proc;
#endif
class XrdOucStream;
class XrdProofdAux {
   static XrdSysRecMutex fgFormMutex;
public:
   XrdProofdAux() { }

   static int AssertDir(const char *path, XrdProofUI ui, bool changeown);
   static int ChangeToDir(const char *dir, XrdProofUI ui, bool changeown);
   static int CheckIf(XrdOucStream *s, const char *h);
   static char *Expand(char *p);
   static void Expand(XrdOucString &path);
   static int Form(XrdOucString &str, const char *fmt, ...);
   static long int GetLong(char *str);
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
   static int GetMacProcList(kinfo_proc **plist, int &nproc);
#endif
   static int GetNumCPUs();
   static int GetUserInfo(const char *usr, XrdProofUI &ui);
   static int GetUserInfo(int uid, XrdProofUI &ui);
   static int KillProcess(int pid, bool forcekill, XrdProofUI ui, bool changeown);
   static char *ReadMsg(int fd);
   static int SymLink(const char *path, const char *link);
   static int VerifyProcessByID(int pid, const char *pname = "proofserv");
   static int Write(int fd, const void *buf, size_t nb);
};

// Useful definitions
#ifndef SafeDel
#define SafeDel(x) { if (x) { delete x; x = 0; } }
#endif
#ifndef SafeDelArray
#define SafeDelArray(x) { if (x) { delete[] x; x = 0; } }
#endif
#ifndef SafeFree
#define SafeFree(x) { if (x) free(x); x = 0; }
#endif

#ifndef INRANGE
#define INRANGE(x,y) ((x >= 0) && (x < (int)y->size()))
#endif

#ifndef DIGIT
#define DIGIT(x) (x >= 48 && x <= 57)
#endif

#ifndef XPDSWAP
#define XPDSWAP(a,b,t) { t = a ; a = b; b = t; }
#endif

#ifndef XpdBadPGuard
#define XpdBadPGuard(g,u) (!(g.Valid()) && (geteuid() != (uid_t)u))
#endif

#undef MHEAD
#define MHEAD "--- Proofd: "

#undef  TRACELINK
#define TRACELINK fLink

#undef  RESPONSE
#define RESPONSE fResponse

#endif
