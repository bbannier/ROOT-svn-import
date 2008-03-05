// @(#)root/proofd:$Id$
// Author: G. Ganis  June 2005

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_XrdProofdProofServ
#define ROOT_XrdProofdProofServ

#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#if !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__APPLE__)
#include <sched.h>
#endif

#include <list>
#include <vector>

#ifdef OLDXRDOUC
#  include "XrdSysToOuc.h"
#  include "XrdOuc/XrdOucPthread.hh"
#  include "XrdOuc/XrdOucSemWait.hh"
#else
#  include "XrdSys/XrdSysPthread.hh"
#  include "XrdSys/XrdSysSemWait.hh"
#endif

#include "Xrd/XrdLink.hh"

#include "XrdProofdProtocol.h"
#include "XrdProofdResponse.h"

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// XrdSrvBuffer                                                         //
//                                                                      //
// Authors: G. Ganis, CERN, 2005                                        //
//                                                                      //
// The following structure is used to store buffers to be sent or       //
// received from clients                                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
class XrdSrvBuffer {
public:
   int   fSize;
   char *fBuff;

   XrdSrvBuffer(char *bp=0, int sz=0, bool dup=0) {
      if (dup && bp && sz > 0) {
         fMembuf = (char *)malloc(sz);
         if (fMembuf) {
            memcpy(fMembuf, bp, sz);
            fBuff = fMembuf;
            fSize = sz;
         }
      } else {
         fBuff = fMembuf = bp;
         fSize = sz;
      }}
   ~XrdSrvBuffer() {if (fMembuf) free(fMembuf);}

private:
   char *fMembuf;
};


class XrdROOT;

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// XrdClientID                                                          //
//                                                                      //
// Authors: G. Ganis, CERN, 2005                                        //
//                                                                      //
// Mapping of clients and stream IDs                                    //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
class XrdClientID {
private:
   XrdProofdProtocol *fP;
   unsigned short     fSid;

public:
   XrdClientID(XrdProofdProtocol *pt = 0, unsigned short id = 0)
            { fP = pt; fSid = id; }
   ~XrdClientID() { }

   XrdProofdClient   *C() const { return fP->Client(); }
   bool               IsValid() const { return (fP != 0); }
   XrdProofdProtocol *P() const { return fP; }
   void               Reset() { fP = 0; fSid = 0; }
   void               SetP(XrdProofdProtocol *p) { fP = p; }
   void               SetSid(unsigned short sid) { fSid = sid; }
   unsigned short     Sid() const { return fSid; }
};

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// XrdProofdProofServ                                                    //
//                                                                      //
// Authors: G. Ganis, CERN, 2005                                        //
//                                                                      //
// This class represent an instance of TProofServ                       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
#define kXPROOFSRVTAGMAX   64
#define kXPROOFSRVALIASMAX 256

class XrdProofGroup;
class XrdProofWorker;
class XrdSysSemWait;

class XrdProofdProofServ
{

public:
   XrdProofdProofServ();
   ~XrdProofdProofServ();

   void                AddWorker(XrdProofWorker *w) { XrdSysMutexHelper mhp(fMutex); fWorkers.push_back(w); }
   inline const char  *Alias() const { XrdSysMutexHelper mhp(fMutex); return fAlias.c_str(); }
   void                Broadcast(const char *msg);
   int                 BroadcastPriority(int priority);
   inline const char  *Client() const { XrdSysMutexHelper mhp(fMutex); return fClient.c_str(); }
   void                DeleteStartMsg()
                       { XrdSysMutexHelper mhp(fMutex); if (fStartMsg) delete fStartMsg; fStartMsg = 0;}
   int                 DisconnectTime();
   inline const char  *Fileout() const { XrdSysMutexHelper mhp(fMutex); return fFileout.c_str(); }
   int                 FreeClientID(XrdProofdProtocol *p);
   XrdClientID        *GetClientID(int cid);
   inline int          GetNClients() { XrdSysMutexHelper mhp(fMutex); return fNClients;}
   inline const char  *Group() const { XrdSysMutexHelper mhp(fMutex); return fGroup.c_str(); }
   int                 IdleTime();
   inline short int    ID() const { XrdSysMutexHelper mhp(fMutex); return fID; }
   inline bool         IsShutdown() const { XrdSysMutexHelper mhp(fMutex); return fIsShutdown; }
   inline bool         IsValid() const { XrdSysMutexHelper mhp(fMutex); return fIsValid; }
   inline XrdLink     *Link() const { XrdSysMutexHelper mhp(fMutex); return (fConn ? fConn->Link() : 0); }
   inline bool         Match(short int id) const { XrdSysMutexHelper mhp(fMutex); return (id == fID); }
   inline XrdSysRecMutex *Mutex() const { return fMutex; }
   inline const char  *Ordinal() const { XrdSysMutexHelper mhp(fMutex); return fOrdinal.c_str(); }
   inline XrdClientID *Parent() const { XrdSysMutexHelper mhp(fMutex); return fParent; }
   inline void         PingSem() const { if (fPingSem) fPingSem->Post(); }
   inline XrdSrvBuffer *QueryNum() const { XrdSysMutexHelper mhp(fMutex); return fQueryNum; }

   void                Reset();
   inline XrdROOT     *ROOT() const { XrdSysMutexHelper mhp(fMutex); return fROOT; }
   inline XrdProofdResponse *Response() const
                      { XrdSysMutexHelper mhp(fMutex); return (fConn ? fConn->Response(1) : 0);}
   int                 SendData(int cid, void *buff, int len);
   int                 SendDataN(void *buff, int len);
   void                SetAlias(const char *a) { XrdSysMutexHelper mhp(fMutex); fAlias = a; }
   void                SetClient(const char *c) { XrdSysMutexHelper mhp(fMutex); fClient = c; }
   inline void         SetConnection(XrdProofdProtocol *p) { XrdSysMutexHelper mhp(fMutex); fConn = p;}

   void                SetFileout(const char *f) { XrdSysMutexHelper mhp(fMutex); fFileout = f; }
   inline void         SetGroup(const char *g) { XrdSysMutexHelper mhp(fMutex); fGroup = g; }
   void                SetIdle();
   inline void         SetID(short int id) { XrdSysMutexHelper mhp(fMutex); fID = id;}
   void                SetOrdinal(const char *o) { XrdSysMutexHelper mhp(fMutex); fOrdinal = o; }
   inline void         SetParent(XrdClientID *cid) { XrdSysMutexHelper mhp(fMutex); fParent = cid; }
   inline void         SetProtVer(int pv) { XrdSysMutexHelper mhp(fMutex); fProtVer = pv; }
   inline void         SetROOT(XrdROOT *r) { XrdSysMutexHelper mhp(fMutex); fROOT = r; }
   void                SetRunning();
   void                SetSrvPID(int pid) { XrdSysMutexHelper mhp(fMutex); fSrvPID = pid; }
   inline void         SetSrvType(int id) { XrdSysMutexHelper mhp(fMutex); fSrvType = id; }
   inline void         SetStartMsg(XrdSrvBuffer *sm) { XrdSysMutexHelper mhp(fMutex); fStartMsg = sm; }
   inline void         SetStatus(int st) { XrdSysMutexHelper mhp(fMutex); fStatus = st; }
   void                SetTag(const char *t) { XrdSysMutexHelper mhp(fMutex); fTag = t; }
   void                SetUserEnvs(const char *t) { XrdSysMutexHelper mhp(fMutex); fUserEnvs = t; }
   inline void         SetValid(bool valid = 1) { XrdSysMutexHelper mhp(fMutex); fIsValid = valid; }
   inline int          SrvPID() const { XrdSysMutexHelper mhp(fMutex); return fSrvPID; }
   inline int          SrvType() const { XrdSysMutexHelper mhp(fMutex); return fSrvType; }
   inline XrdSrvBuffer *StartMsg() const { XrdSysMutexHelper mhp(fMutex); return fStartMsg; }
   inline int          Status() const { XrdSysMutexHelper mhp(fMutex); return fStatus;}
   inline const char  *Tag() const { XrdSysMutexHelper mhp(fMutex); return fTag.c_str(); }
   int                 TerminateProofServ();
   inline const char  *UserEnvs() const { XrdSysMutexHelper mhp(fMutex); return fUserEnvs.c_str(); }
   int                 VerifyProofServ(int timeout);
   inline std::list<XrdProofWorker *> *Workers() const
                      { XrdSysMutexHelper mhp(fMutex); return (std::list<XrdProofWorker *> *)&fWorkers; }

 private:

   XrdSysRecMutex           *fMutex;
   XrdProofdProtocol        *fConn;      // Protocol instance attached to this session

   XrdClientID              *fParent;    // Parent creating this session
   int                      fNClients;   // Number of attached clients
   std::vector<XrdClientID *> fClients;  // Attached clients stream ids
   std::list<XrdProofWorker *> fWorkers; // Workers assigned to the session

   XrdSysSemWait            *fPingSem;   // To sychronize ping requests

   XrdSrvBuffer             *fQueryNum;  // Msg with sequential number of currebt query
   XrdSrvBuffer             *fStartMsg;  // Msg with start processing info

   time_t                    fDisconnectTime; // Time at which all clients disconnected
   time_t                    fSetIdleTime; // Time at which the session went idle

   int                       fStatus;
   int                       fSrvPID;     // Srv process ID
   int                       fSrvType;
   short int                 fID;
   char                      fProtVer;
   XrdOucString              fFileout;

   bool                      fIsShutdown; // Whether asked to shutdown
   bool                      fIsValid;   // Validity flag

   XrdOucString              fAlias;     // Session alias
   XrdOucString              fClient;    // Client name
   XrdOucString              fTag;       // Session unique tag
   XrdOucString              fOrdinal;   // Session ordinal number
   XrdOucString              fUserEnvs;  // List of envs received from the user

   XrdROOT                  *fROOT;      // ROOT version run by this session

   XrdOucString              fGroup;     // Group, if any, to which the owner belongs

   void                      ClearWorkers();

   void                      CreatePingSem()
                             { XrdSysMutexHelper mhp(fMutex); fPingSem = new XrdSysSemWait(0);}
   void                      DeletePingSem()
                             { XrdSysMutexHelper mhp(fMutex); if (fPingSem) delete fPingSem; fPingSem = 0;}
};
#endif
