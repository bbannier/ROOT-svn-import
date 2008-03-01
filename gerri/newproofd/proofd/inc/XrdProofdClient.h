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
class XrdProofServProxy;
class XrdROOT;

class XrdProofdClient {

 public:
   XrdProofdClient(XrdProofUI ui,
                   bool master, bool changeown, XrdSysError *edest, const char *tmp);

   virtual ~XrdProofdClient();

   inline const char      *Group() const { return fUI.fGroup.c_str(); }
   inline const char      *User() const { return fUI.fUser.c_str(); }
   inline bool             IsValid() const { return fIsValid; }
   bool                    Match(const char *id, const char *grp = 0);
   inline XrdSysRecMutex  *Mutex() const { return (XrdSysRecMutex *)&fMutex; }
   inline unsigned short   RefSid() const { return fRefSid; }
   inline XrdROOT         *ROOT() const { return fROOT; }
   inline XrdProofdSandbox *Sandbox() const { return (XrdProofdSandbox *)&fSandbox; }
   inline XrdProofUI       UI() const { return fUI; }

   XrdProofServProxy      *GetServer(int psid);
   XrdProofServProxy      *GetServer(XrdProofdProtocol *p);
   void                    EraseServer(int psid);
   XrdProofServProxy      *GetProofServ(int psid);

   void                    DisconnectFromProofServ(XrdProofdProtocol *p);
   int                     ResetClientSlot(XrdProofdProtocol *p);

   int                     GetClientID(XrdProofdProtocol *p);
   XrdProofServProxy      *GetFreeServObj();

   void                    Broadcast(const char *msg, bool closelink = 0);

   XrdOucString            ExportSessions();
   void                    TerminateSessions(bool kill, int srvtype, XrdProofServProxy *ref,
                                             const char *msg, std::list<int> &sigpid);

   void                    SetGroup(const char *g) { fUI.fGroup = g; }
   void                    SetROOT(XrdROOT *r) { fROOT = r; }

   void                    SetRefSid(unsigned short sid) { fRefSid = sid; }
   void                    SetValid(bool valid = 1) { fIsValid = valid; }

   int                     Size() const { return fClients.size(); }

   int                     CreateUNIXSock(XrdSysError *edest,
                                          const char *tmpdir, bool changeown);
   XrdNet                 *UNIXSock() const { return fUNIXSock; }
   char                   *UNIXSockPath() const { return fUNIXSockPath; }
   void                    SaveUNIXPath(); // Save path in the sandbox
   void                    SetUNIXSockSaved() { fUNIXSockSaved = 1;}

 private:

   XrdSysRecMutex          fMutex; // Local mutex

   bool                    fChangeOwn; // TRUE if ownership must be changed where relevant
   bool                    fIsValid; // TRUE if the instance is complete

   unsigned short          fRefSid;     // Reference stream ID for this client
   XrdProofUI              fUI;         // user info
   XrdROOT                *fROOT;        // ROOT vers instance to be used for proofserv

   XrdProofdSandbox        fSandbox;     // Clients sandbox

   XrdNet                 *fUNIXSock;     // UNIX server socket for internal connections
   char                   *fUNIXSockPath; // UNIX server socket path
   bool                    fUNIXSockSaved; // TRUE if the socket path has been saved


   std::vector<XrdProofServProxy *> fProofServs; // Allocated ProofServ sessions
   std::vector<XrdProofdProtocol *> fClients;    // Attached Client sessions
};

#endif
