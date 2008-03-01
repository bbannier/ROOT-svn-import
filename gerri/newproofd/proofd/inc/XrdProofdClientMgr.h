// @(#)root/proofd:$Id:$
// Author: G. Ganis Jan 2008

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_XrdProofdClientMgr
#define ROOT_XrdProofdClientMgr

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// XrdProofdClientMgr                                                   //
//                                                                      //
// Author: G. Ganis, CERN, 2008                                         //
//                                                                      //
// Class managing clients.                                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <list>

#include "XrdProofdConfig.h"

#include "XrdOuc/XrdOucString.hh"

#define XPD_LOGGEDIN       1
#define XPD_NEED_AUTH      2
#define XPD_ADMINUSER      4
#define XPD_NEED_MAP       8

class XrdProofdClient;
class XrdProofdConfig;
class XrdProofdManager;
class XrdProofdProtocol;
class XrdProtocol_Config;
class XrdSysError;
class XrdSecService;


class XrdProofdClientMgr : public XrdProofdConfig {

   XrdProofdManager  *fMgr;
   XrdOucString       fSecLib;
   XrdSecService     *fCIA;            // Authentication Server

   XrdOucString       fClntAdminPath;  // Client admin area

   std::list<XrdProofdClient *> fProofdClients;        // keeps track of all users

   int                CreateAdminPath(XrdProofdProtocol *p,
                                      XrdOucString &path, XrdOucString &e);
   int                MapClient(XrdProofdProtocol *xp, bool all = 1);
   char              *FilterSecConfig(int &nd);
   void               RegisterDirectives();

   // Security service
   XrdSecService     *LoadSecurity();

public:
   XrdProofdClientMgr(XrdProofdManager *mgr, XrdProtocol_Config *pi, XrdSysError *e);
   virtual ~XrdProofdClientMgr() { }

   int               Config(bool rcf = 0);

   XrdProofdClient  *GetClient(const char *usr, const char *grp = 0, const char *sock = 0);

   void              Broadcast(XrdProofdClient *c, const char *msg, bool closelink = 0);
   void              TerminateSessions(XrdProofdClient *c,
                                       const char *msg, int srvtype, std::list<int> &sigpid);

   void              SetLock(bool on = 1, XrdProofdClient *c = 0);

   int               Process(XrdProofdProtocol *p);

   int               Auth(XrdProofdProtocol *xp);
   int               Login(XrdProofdProtocol *xp);
};
#endif
