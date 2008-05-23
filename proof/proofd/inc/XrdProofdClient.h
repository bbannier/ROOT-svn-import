// @(#)root/proofd:$Id$
// Author: G. Ganis June 2007

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_XrdProofdClient
#define ROOT_XrdProofdClient

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// XrdProofdClient                                                      //
//                                                                      //
// Author: G. Ganis, CERN, 2007                                         //
//                                                                      //
// Auxiliary class describing a PROOF client.                           //
// Used by XrdProofdProtocol.                                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
#include <list>
#include <vector>

#ifdef OLDXRDOUC
#  include "XrdSysToOuc.h"
#  include "XrdOuc/XrdOucPthread.hh"
#else
#  include "XrdSys/XrdSysPthread.hh"
#endif
#include "XrdOuc/XrdOucString.hh"

#include "XrdProofdAux.h"
#include "XrdProofdSandbox.h"

#define XPC_DEFMAXOLDLOGS 10

class XrdNet;
class XrdProofdProtocol;
class XrdProofdProofServ;
class XrdROOT;

class XrdProofdClient {

 public:
   XrdProofdClient(XrdProofUI ui,
                   bool master, bool changeown, XrdSysError *edest, const char *tmp);

   virtual ~XrdProofdClient();

   inline const char      *Group() const { return fUI.fGroup.c_str(); }
   inline const char      *User() const { return fUI.fUser.c_str(); }
   inline bool             IsValid() const { return fIsValid; }
   bool                    Match(const char *usr, const char *grp = 0);
   inline XrdSysRecMutex  *Mutex() const { return (XrdSysRecMutex *)&fMutex; }
   inline unsigned short   RefSid() const { return fRefSid; }
   inline XrdROOT         *ROOT() const { return fROOT; }
   inline XrdProofdSandbox *Sandbox() const { return (XrdProofdSandbox *)&fSandbox; }
   inline XrdProofUI       UI() const { return fUI; }

   XrdProofdProofServ      *GetServer(int psid);
   XrdProofdProofServ      *GetServer(XrdProofdProtocol *p);
   void                    EraseServer(int psid);
   XrdProofdProofServ      *GetProofServ(int psid);

   int                     ResetClientSlot(int ic);

   int                     GetClientID(XrdProofdProtocol *p);
   int                     ReserveClientID(int cid);
   int                     SetClientID(int cid, XrdProofdProtocol *p);
   XrdProofdProofServ     *GetFreeServObj();
   XrdProofdProofServ     *GetServObj(int id);

   void                    Broadcast(const char *msg);

   XrdOucString            ExportSessions();
   void                    TerminateSessions(int srvtype, XrdProofdProofServ *ref,
                                             const char *msg, XrdProofdPipe *pipe, bool changeown);

   void                    SetGroup(const char *g) { fUI.fGroup = g; }
   void                    SetROOT(XrdROOT *r) { fROOT = r; }

   void                    SetRefSid(unsigned short sid) { fRefSid = sid; }
   void                    SetValid(bool valid = 1) { fIsValid = valid; }

   int                     Size() const { return fClients.size(); }

   int                     CreateUNIXSock(XrdSysError *edest);
   XrdNet                 *UNIXSock() const { return fUNIXSock; }
   const char             *UNIXSockPath() const { return fUNIXSockPath.c_str(); }
   void                    SaveUNIXPath(); // Save path in the sandbox
   void                    SetUNIXSockSaved() { fUNIXSockSaved = 1;}

   const char             *AdminPath() const { return fAdminPath.c_str(); }

 private:

   XrdSysRecMutex          fMutex; // Local mutex

   bool                    fChangeOwn; // TRUE if ownership must be changed where relevant
   bool                    fIsValid; // TRUE if the instance is complete

   unsigned short          fRefSid;     // Reference stream ID for this client
   XrdProofUI              fUI;         // user info
   XrdROOT                *fROOT;        // ROOT vers instance to be used for proofserv

   XrdProofdSandbox        fSandbox;     // Clients sandbox

   XrdNet                 *fUNIXSock;     // UNIX server socket for internal connections
   XrdOucString            fUNIXSockPath; // UNIX server socket path
   bool                    fUNIXSockSaved; // TRUE if the socket path has been saved

   XrdOucString            fAdminPath;    // Admin path for this client

   std::vector<XrdProofdProofServ *> fProofServs; // Allocated ProofServ sessions
   std::vector<XrdProofdProtocol *> fClients;    // Attached Client sessions

   void                    PostSessionRemoval(int fd, int pid);
};

#endif
