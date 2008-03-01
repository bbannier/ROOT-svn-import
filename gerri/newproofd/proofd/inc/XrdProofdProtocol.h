// @(#)root/proofd:$Id$
// Author: G. Ganis  June 2005

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_XrdProofdProtocol
#define ROOT_XrdProofdProtocol

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// XrdProofdProtocol                                                    //
//                                                                      //
// Authors: G. Ganis, CERN, 2005                                        //
//                                                                      //
// XrdProtocol implementation to coordinate 'proofserv' applications.   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <vector>

// Version index: start from 1001 (0x3E9) to distinguish from 'proofd'
// To be increment when non-backward compatible changes are introduced
//  1001 (0x3E9) -> 1002 (0x3EA) : support for flexible env setting
//  1002 (0x3EA) -> 1003 (0x3EB) : many new features
//  1003 (0x3EB) -> 1004 (0x3EC) : restructuring
//  1004 (0x3EC) -> 1005 (0x3ED) : deeper restructuring
#define XPROOFD_VERSBIN 0x000003ED
#define XPROOFD_VERSION "0.5"

#include "Xrd/XrdLink.hh"
#include "Xrd/XrdObject.hh"
#include "Xrd/XrdProtocol.hh"
#include "XrdOuc/XrdOucString.hh"
#include "XrdSec/XrdSecInterface.hh"
#include "XProofProtocol.h"

class XrdBuffer;
class XrdProofdClient;
class XrdProofdManager;
class XrdProofdResponse;
class XrdProofServProxy;
class XrdSrvBuffer;

class XrdProofdProtocol : XrdProtocol {

public:
   XrdProofdProtocol();
   virtual ~XrdProofdProtocol() {} // Never gets destroyed

   void          DoIt() {}
   XrdProtocol  *Match(XrdLink *lp);
   int           Process(XrdLink *lp);
   void          Recycle(XrdLink *lp, int x, const char *y);
   int           Stats(char *buff, int blen, int do_sync);

   static int    Configure(char *parms, XrdProtocol_Config *pi);

   // Getters
   inline kXR_int32 CID() const { return fCID; }
   inline XrdProofdClient *Client() const { return fPClient; }
   inline int    ConnType() const { return fConnType; }
   inline char   Status() const { return fStatus; }
   inline short int ProofProtocol() const { return fProofProtocol; }
   inline bool   SuperUser() const { return fSuperUser; }

#if 0
   inline XrdProofdResponse *Response() const { return (XrdProofdResponse *)&fResponse; }
#else
   XrdProofdResponse *Response(kXR_unt16 rid);
#endif
   inline XPClientRequest *Request() const { return (XPClientRequest *)&fRequest; }
   inline XrdBuffer *Argp() const { return fArgp; }
   inline XrdLink *Link() const { return fLink; }
   inline XrdSecProtocol *AuthProt() const { return fAuthProt; }

   // Setters
   inline void   SetAdminPath(const char *p) { fAdminPath = p; }
   inline void   SetAuthEntity(XrdSecEntity *se = 0) { fSecEntity.tident = fLink->ID;
                                                       fSecClient = (se) ? se : &fSecEntity; }
   inline void   SetAuthProt(XrdSecProtocol *p) { fAuthProt = p; }
   inline void   SetClient(XrdProofdClient *c) { fPClient = c; }
   inline void   SetClntCapVer(unsigned char c) { fClntCapVer = c; }
   inline void   SetCID(kXR_int32 cid) { fCID = cid; }
   inline void   SetConnType(int ct) { fConnType = ct; }
   inline void   ProofProtocol(short int pp) { fProofProtocol = pp; }
   inline void   SetStatus(char s) { fStatus = s; }
   inline void   SetSuperUser(bool su = 1) { fSuperUser = su; }

   static XrdProofdManager *Mgr() { return fgMgr; }

 private:

   int           GetBuff(int quantum);
   int           GetData(const char *dtype, char *buff, int blen);
   XrdProofdResponse *GetNewResponse(kXR_unt16 rid);
   int           Interrupt();
   int           Ping();
   int           Process2();
   void          Reset();
   int           SendData(XrdProofServProxy *xps, kXR_int32 sid = -1, XrdSrvBuffer **buf = 0);
   int           SendDataN(XrdProofServProxy *xps, XrdSrvBuffer **buf = 0);
   int           SendMsg();
   int           Urgent();

   //
   // Local area
   //
   XrdObject<XrdProofdProtocol>  fProtLink;
   XrdLink                      *fLink;
   XrdBuffer                    *fArgp;

   char                          fStatus;

   unsigned char                 fClntCapVer;
   short int                     fProofProtocol;   // PROOF protocol version run by client

   kXR_int32                     fConnType;        // Type of connection: Clnt-Mst, Mst-Mst, Mst-Wrk
   bool                          fSuperUser;       // TRUE for privileged clients (admins)
   //
   XrdProofdClient              *fPClient;         // Our reference XrdProofdClient
   kXR_int32                     fCID;             // Reference ID of this client
   XrdOucString                  fAdminPath;       // Admin path for this client
   //
   XrdSecEntity                 *fSecClient;
   XrdSecProtocol               *fAuthProt;
   XrdSecEntity                  fSecEntity;
   //
   int                           fhcPrev;
   int                           fhcMax;
   int                           fhcNext;
   int                           fhcNow;
   int                           fhalfBSize;
   //
   XPClientRequest               fRequest;  // handle client requests
#if 0
   XrdProofdResponse             fResponse; // Response to incoming request
#else
   std::vector<XrdProofdResponse *> fResponses; // One per each logical connection
#endif
   XrdSysRecMutex                fMutex;    // Local mutex

   //
   // Static area: general protocol managing section
   //
   static bool                   fgConfigDone;
   static int                    fgCount;
   static XrdObjectQ<XrdProofdProtocol> fgProtStack;
   static XrdBuffManager        *fgBPool;     // Buffer manager
   static int                    fgMaxBuffsz;    // Maximum buffer size we can have

   static XrdSysError            fgEDest;     // Error message handler
   static XrdSysLogger           fgMainLogger; // Error logger


   //
   // Static area: protocol configuration section
   static int                    fgPriorityMgrFd; // communication to priority manager
   static int                    fgProofServMgrFd; // communication to proofserv manager
   static int                    fgReadWait;
   static XrdProofdManager      *fgMgr;       // Cluster manager

   static void                   PostSession(int on, const char *u, const char *g, int pid);
   static void                   PostSessionRemoval(int pid);
   static void                   PostClientDisconnection();
};

#define XPD_SETRESP(p, x) \
   kXR_unt16 rid; \
   memcpy((void *)&rid, (const void *)&(p->Request()->header.streamid[0]), 2); \
   XrdProofdResponse *response = p->Response(rid); \
   if (!response) { \
      TRACEI(p, XERR, x << ": could not get Response instance for requid:"<< rid); \
      return rc; \
   } \
   const char *respid = response->ID();

#endif
