// @(#)root/proofd:$Id$
// Author: G. Ganis Jan 2008

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// XrdProofdClientMgr                                                   //
//                                                                      //
// Author: G. Ganis, CERN, 2008                                         //
//                                                                      //
// Class managing clients.                                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
#include "XrdProofdPlatform.h"

#ifdef OLDXRDOUC
#  include "XrdOuc/XrdOucError.hh"
#  define XPD_LOG_01 OUC_LOG_01
#else
#  include "XrdSys/XrdSysError.hh"
#  define XPD_LOG_01 SYS_LOG_01
#endif

#include "Xrd/XrdBuffer.hh"
#include "XrdOuc/XrdOucErrInfo.hh"
#include "XrdSec/XrdSecInterface.hh"

#include "XrdProofdClient.h"
#include "XrdProofdClientMgr.h"
#include "XrdProofdManager.h"
#include "XrdProofdProtocol.h"
#include "XrdProofGroup.h"
#include "XrdProofdProofServ.h"
#include "XrdProofdProofServMgr.h"
#include "XrdROOT.h"

// Tracing utilities
#include "XrdProofdTrace.h"
static const char *gTraceID = "";
extern XrdOucTrace *XrdProofdTrace;
#define TRACEID gTraceID

// Security handle
typedef XrdSecService *(*XrdSecServLoader_t)(XrdSysLogger *, const char *cfn);

//______________________________________________________________________________
XrdProofdClientMgr::XrdProofdClientMgr(XrdProofdManager *mgr,
                                       XrdProtocol_Config *pi, XrdSysError *e)
                  : XrdProofdConfig(pi->ConfigFN, e)
{
   // Constructor

   fMgr = mgr;
   fCIA = 0;
   fNDisconnected = 0;
   fReconnectTimeOut = 300;
}

//__________________________________________________________________________
void XrdProofdClientMgr::RegisterDirectives()
{
   // Register directives for configuration

   Register("seclib", new XrdProofdDirective("seclib",
                                   (void *)&fSecLib, &DoDirectiveString, 0));
   Register("reconnto", new XrdProofdDirective("reconnto",
                               (void *)&fReconnectTimeOut, &DoDirectiveInt));
}

//__________________________________________________________________________
int XrdProofdClientMgr::Config(bool rcf)
{
   // Run configuration and parse the entered config directives.
   // Return 0 on success, -1 on error

   // Run first the configurator
   if (XrdProofdConfig::Config(rcf) != 0) {
      fEDest->Say(0, "xpd: Config: ClientMgr: problems parsing file ");
      return -1;
   }

   XrdOucString msg;
   msg = (rcf) ? "xpd: Config: ClientMgr: re-configuring"
               : "xpd: Config: ClientMgr: configuring";
   fEDest->Say(0, msg.c_str());

   // Admin paths
   fClntAdminPath = fMgr->AdminPath();
   fClntAdminPath += "/clients";

   // Make sure they exist
   XrdProofUI ui;
   XrdProofdAux::GetUserInfo(fMgr->EffectiveUser(), ui);
   if (XrdProofdAux::AssertDir(fClntAdminPath.c_str(), ui, 1) != 0) {
      fEDest->Say(0, "xpd: Config: ClientMgr: unable to assert the clients admin path: ",
                     fClntAdminPath.c_str());
      fClntAdminPath = "";
      return -1;
   }
   fEDest->Say(0, "xpd: Config: ClientMgr: clients admin path set to: ", fClntAdminPath.c_str());

   // Init place holders for previous active clients, if any
   if (ParsePreviousClients(msg) != 0) {
      fEDest->Say(0, "xpd: Config: ClientMgr: problems parsing previous active"
                     " clients: ", msg.c_str());
   }

   // Initialize the security system if this is wanted
   if (!rcf) {
      if (fSecLib.length() <= 0)
         fEDest->Say(0, "XRD seclib not specified; strong authentication disabled");
      else {
         if (!(fCIA = LoadSecurity())) {
            fEDest->Emsg(0, "xpd: Config: ClientMgr: unable to load security system.");
            return -1;
         }
         fEDest->Emsg(0, "xpd: Config: ClientMgr: security library loaded");
      }
   }

   if (rcf) {
      // Re-assign groups
      if (fMgr->GroupsMgr() && fMgr->GroupsMgr()->Num() > 0) {
         std::list<XrdProofdClient *>::iterator pci;
         for (pci = fProofdClients.begin(); pci != fProofdClients.end(); ++pci)
            (*pci)->SetGroup(fMgr->GroupsMgr()->GetUserGroup((*pci)->User())->Name());
      }
   }

   // Initialize the security system if this is wanted
   if (!fCIA)
      fEDest->Say(0, "xpd: Config: ClientMgr: XRD seclib not specified; strong authentication disabled");

   // Done
   return 0;
}

//______________________________________________________________________________
int XrdProofdClientMgr::Login(XrdProofdProtocol *p)
{
   // Process a login request

   int rc = 1;
   XPD_SETRESP(p, "Login");

   TRACEP(p, respid, REQ, "Login: enter");

   // If this server is explicitely required to be a worker node or a
   // submaster, check whether the requesting host is allowed to connect
   if (p->Request()->login.role[0] != 'i' &&
      (fMgr->SrvType() == kXPD_MasterWorker || fMgr->SrvType() == kXPD_Master)) {
      if (!fMgr->CheckMaster(p->Link()->Host())) {
         TRACEP(p, respid, XERR,"Login: master not allowed to connect - "
                     "ignoring request ("<<p->Link()->Host()<<")");
         response->Send(kXR_InvalidRequest,
                            "Login: master not allowed to connect - request ignored");
         return rc;
      }
   }

   // If this is the second call (after authentication) we just need mapping
   if (p->Status() == XPD_NEED_MAP) {
      // Acknowledge the client
      response->Send();
      p->SetStatus(XPD_LOGGEDIN);
      return MapClient(p, 0);
   }

   // Make sure the user is not already logged in
   if ((p->Status() & XPD_LOGGEDIN)) {
      response->Send(kXR_InvalidRequest, "duplicate login; already logged in");
      return rc;
   }

   // Find out the connection type: 'i', internal, means this is a proofsrv calling back.
   bool needauth = 0;
   bool ismaster = (fMgr->SrvType() == kXPD_TopMaster || fMgr->SrvType() == kXPD_Master) ? 1 : 0;
   switch (p->Request()->login.role[0]) {
   case 'i':
      p->SetConnType(kXPD_Internal);
      response->Set("int: ");
      break;
   case 'M':
      if (fMgr->SrvType() == kXPD_AnyServer || ismaster) {
         p->SetConnType(kXPD_ClientMaster);
         needauth = 1;
         response->Set("m2c: ");
      } else {
         TRACEP(p, respid, XERR,"Login: top master mode not allowed - ignoring request");
         response->Send(kXR_InvalidRequest,
                            "Server not allowed to be top master - ignoring request");
         return rc;
      }
      break;
   case 'm':
      if (fMgr->SrvType() == kXPD_AnyServer || ismaster) {
         p->SetConnType(kXPD_MasterMaster);
         needauth = 1;
         response->Set("m2m: ");
      } else {
         TRACEP(p, respid, XERR,"Login: submaster mode not allowed - ignoring request");
         response->Send(kXR_InvalidRequest,
                             "Server not allowed to be submaster - ignoring request");
         return rc;
      }
      break;
   case 's':
      if (fMgr->SrvType() == kXPD_AnyServer || fMgr->SrvType() == kXPD_MasterWorker) {
         p->SetConnType(kXPD_MasterWorker);
         needauth = 1;
         response->Set("w2m: ");
      } else {
         TRACEP(p, respid, XERR,"Login: worker mode not allowed - ignoring request");
         response->Send(kXR_InvalidRequest,
                             "Server not allowed to be worker - ignoring request");
         return rc;
      }
      break;
   default:
      TRACEP(p, respid, XERR, "Login: unknown mode: '" << p->Request()->login.role[0] <<"'");
      response->Send(kXR_InvalidRequest, "Server type: invalide mode");
      return rc;
   }

   // Get user and group names for the entity requiring to login
   int i, pid;
   XrdOucString uname, gname;

   // Unmarshall the data
   pid = (int)ntohl(p->Request()->login.pid);
   char un[9];
   for (i = 0; i < (int)sizeof(un)-1; i++) {
      if (p->Request()->login.username[i] == '\0' || p->Request()->login.username[i] == ' ')
         break;
      un[i] = p->Request()->login.username[i];
   }
   un[i] = '\0';
   uname = un;

   // Longer usernames are in the attached buffer
   if (uname == "?>buf") {
      // Attach to buffer
      char *buf = p->Argp()->buff;
      int   len = p->Request()->login.dlen;
      // Extract username
      uname.assign(buf,0,len-1);
      int iusr = uname.find("|usr:");
      if (iusr == -1) {
         TRACEP(p, respid, XERR,"Login: long user name not found");
         response->Send(kXR_InvalidRequest,"Login: long user name not found");
         return rc;
      }
      uname.erase(0,iusr+5);
      uname.erase(uname.find("|"));
   }

   // Extract group name, if specified (syntax is uname[:gname])
   int ig = uname.find(":");
   if (ig != -1) {
      gname.assign(uname, ig+1);
      uname.erase(ig);
      TRACEP(p, respid, DBG,"Login: requested group: "<<gname);
   }

   // Here we check if the user is allowed to use the system
   // If not, we fail.
   XrdOucString emsg;
   XrdProofUI ui;
   bool su;
   if (fMgr->CheckUser(uname.c_str(), ui, emsg, su) != 0) {
      emsg.insert(": ", 0);
      emsg.insert(uname, 0);
      emsg.insert("Login: ClientID not allowed: ", 0);
      TRACEP(p, respid, XERR, emsg.c_str());
      response->Send(kXR_InvalidRequest, emsg.c_str());
      return rc;
   }
   if (su) {
      // Update superuser flag
      p->SetSuperUser(su);
      TRACEP(p, respid, LOGIN,"Login:"<<uname<<" is a privileged user ");
   }

   // Check if user belongs to a group
   XrdProofGroup *g = 0;
   if (fMgr->GroupsMgr() && fMgr->GroupsMgr()->Num() > 0) {
      if (gname.length() > 0) {
         g = fMgr->GroupsMgr()->GetGroup(gname.c_str());
         if (!g) {
            emsg = "Login: group unknown: ";
            emsg += gname;
            TRACEP(p, respid, XERR, emsg.c_str());
            response->Send(kXR_InvalidRequest, emsg.c_str());
            return rc;
         } else if (strncmp(g->Name(),"default",7) &&
                   !g->HasMember(uname.c_str())) {
            emsg = "Login: user ";
            emsg += uname;
            emsg += " is not member of group ";
            emsg += gname;
            TRACEP(p, respid, XERR, emsg.c_str());
            response->Send(kXR_InvalidRequest, emsg.c_str());
            return rc;
         } else {
            if (TRACING(DBG)) {
               TRACEP(p, respid, DBG,"Login: group: "<<gname<<" found");
               g->Print();
            }
         }
      } else {
         g = fMgr->GroupsMgr()->GetUserGroup(uname.c_str());
         gname = g ? g->Name() : "default";
      }
   }
   ui.fGroup = gname;

   // Attach-to / Create the XrdProofdClient instance for this user: if login
   // fails this will be removed at a later stage
   XrdProofdClient *c = GetClient(uname.c_str(), gname.c_str());
   if (c) {
      c->SetROOT(fMgr->ROOTMgr()->DefaultVersion());
      if (c->IsValid()) {
         // Set the group, if any
         c->SetGroup(g->Name());
         // Reference Stream ID
         unsigned short sid;
         memcpy((void *)&sid, (const void *)&(p->Request()->header.streamid[0]), 2);
         c->SetRefSid(sid);
      }
   } else {
      emsg = "Login: unable to instantiate object for client ";
      emsg += uname;
      TRACEP(p, respid, XERR, emsg.c_str());
      response->Send(kXR_InvalidRequest, emsg.c_str());
      return rc;
   }
   // Save into the protocol instance
   p->SetClient(c);

   // Atomic from now on
   XrdSysMutexHelper mh(p->Client()->Mutex());

   // Establish the ID for this link
   p->Link()->setID(uname.c_str(), pid);
   p->SetClntCapVer(p->Request()->login.capver[0]);

   // Get the security token for this link. We will either get a token, a null
   // string indicating host-only authentication, or a null indicating no
   // authentication. We can then optimize of each case.
   if (needauth && fCIA) {
      const char *pp = fCIA->getParms(i, p->Link()->Name());
      if (pp && i ) {
         response->Send((kXR_int32)XPROOFD_VERSBIN, (void *)pp, i);
         p->SetStatus((XPD_NEED_MAP | XPD_NEED_AUTH));
         return rc;
      } else {
         response->Send((kXR_int32)XPROOFD_VERSBIN);
         p->SetStatus(XPD_LOGGEDIN);
         if (pp)
            p->SetAuthEntity();
      }
   } else {
      rc = response->Send((kXR_int32)XPROOFD_VERSBIN);
      p->SetStatus(XPD_LOGGEDIN);
   }

   // Map the client
   return MapClient(p, 1);
}

//______________________________________________________________________________
int XrdProofdClientMgr::MapClient(XrdProofdProtocol *p, bool all)
{
   // Process a login request
   int rc = 1;
   XPD_SETRESP(p, "MapClient");

   XrdOucString msg;

   TRACEP(p, respid, REQ,"MapClient: enter");

   // Map the existing session, if found
   if (!p->Client() || !p->Client()->IsValid()) {
      if (p->Client()) {
         // Remove from the list
         fProofdClients.remove(p->Client());
         delete p->Client();
         p->SetClient(0);
      }
      TRACEP(p, respid, DBG,"MapClient: cannot find valid instance of XrdProofdClient");
      response->Send(kXP_ServerError,
                           "MapClient: cannot find valid instance of XrdProofdClient");
      return rc;
   }

   // Atomic from now on
   XrdSysMutexHelper mh(p->Client()->Mutex());

   // Flag for internal connections
   bool proofsrv = ((p->ConnType() == kXPD_Internal) && all) ? 1 : 0;

   // If call back from proofsrv, find out the target session
   short int psid = -1;
   char protver = -1;
   short int clientvers = -1;
   if (proofsrv) {
      memcpy(&psid, (const void *)&(p->Request()->login.reserved[0]), 2);
      if (psid < 0) {
         TRACEP(p, respid, XERR,"MapClient: proofsrv callback: sent invalid session id");
         response->Send(kXR_InvalidRequest,
                            "MapClient: proofsrv callback: sent invalid session id");
         return rc;
      }
      protver = p->Request()->login.capver[0];
      TRACEP(p, respid, DBG,"MapClient: proofsrv callback for session: " <<psid);
   } else {
      // Get PROOF version run by client
      memcpy(&clientvers, (const void *)&(p->Request()->login.reserved[0]), 2);
      TRACEP(p, respid, DBG,"MapClient: PROOF version run by client: " <<clientvers);
   }

   // If proofsrv, locate the target session
   if (proofsrv) {
      XrdProofdProofServ *psrv = p->Client()->GetServer(psid);
      if (!psrv) {
         TRACEP(p, respid, XERR, "MapClient: proofsrv callback:"
                     " wrong target session: protocol error");
         response->Send(kXP_nosession, "MapClient: proofsrv callback:"
                                             " wrong target session: protocol error");
         return -1;
      } else {
         // Set the protocol version
         psrv->SetProtVer(protver);
         // Assign this link to it
         psrv->SetConnection(p);
         // Set Trace ID
         XrdOucString tid(" : xrd->");
         tid += psrv->Ordinal();
         tid += " ";
         psrv->Response()->Set(tid.c_str());
         TRACEP(p, psrv->Response()->ID(), DBG,"MapClient: proofsrv callback:"
                     " link assigned to target session "<<psid);
      }
   } else {

      // Make sure that the version is filled correctly (if an admin operation
      // was run before this may still be -1 on workers)
      p->ProofProtocol(clientvers);

      // Check if we have already an ID for this client from a previous connection
      XrdOucString cpath;
      int cid = -1;
      if ((cid = CheckAdminPath(p, cpath, msg)) >= 0) {
         // Assign the slot
         p->Client()->SetClientID(cid, p);
         // The index of the next free slot will be the unique ID
         p->SetCID(cid);
         // Remove the file indicating that this client was still disconnected
         cpath.replace("/cid", "/disconnected");
         if (unlink(cpath.c_str()) != 0) {
            XrdProofdAux::Form(msg, "MapClient: warning: could not remove %s (errno: %d)",
                                    cpath.c_str(), errno);
            TRACEP(p, respid, XERR, msg.c_str());
         }
         cpath.replace("/disconnected", "");
         // Update counters
         fNDisconnected--;
         // Notify and of reconnection time if so
         if (fNDisconnected <= 0)
            PostEndOfReconnection();
      } else {
         // The index of the next free slot will be the unique ID
         p->SetCID(p->Client()->GetClientID(p));
         // Create the client directory in the admin path
         if (CreateAdminPath(p, cpath, msg) != 0) {
            TRACEP(p, respid, XERR, msg.c_str());
            // Remove from the list
            fProofdClients.remove(p->Client());
            delete p->Client();
            p->SetClient(0);
            response->Send(kXP_ServerError, msg.c_str());
         }
      }
      p->SetAdminPath(cpath.c_str());
      XrdProofdAux::Form(msg, "MapClient: client admin path created: %s", cpath.c_str());
      TRACEP(p, respid, DBG, msg.c_str());

      TRACEP(p, respid, DBG,"MapClient: CID: "<<p->CID()<<", size: "<<p->Client()->Size());
   }

   // Document this login
   if (!(p->Status() & XPD_NEED_AUTH)) {

      fEDest->Log(XPD_LOG_01, ":MapClient", p->Link()->ID, "login");
   }

   return rc;
}

//___________________________________________________________________________
void XrdProofdClientMgr::PostEndOfReconnection()
{
   // Post end of reconnection phase

   int sock = fMgr->SessionMgr() ? fMgr->SessionMgr()->WriteFd() : -1;
   TRACE(DBG, "PostEndOfReconnection: socket "<<sock);

   if (sock > 0) {
      int type = 2;
      if (write(sock, &type, sizeof(type)) !=  sizeof(type)) {
         TRACE(XERR, "PostEndOfReconnection: problem sending message type on the pipe");
         return;
      }
   }
   // Done
   return;
}

//_____________________________________________________________________________
int XrdProofdClientMgr::CreateAdminPath(XrdProofdProtocol *p,
                                        XrdOucString &cpath, XrdOucString &emsg)
{
   // Create the client directory in the admin path

   if (!p) {
      XrdProofdAux::Form(emsg, "CreateAdminPath: invalid inputs (p: %p)", p);
      return -1;
   }

   // Create link ID
   XrdOucString lid(p->Link()->ID);
   XrdOucString sid;
   XrdProofdAux::Form(sid, ":%d", p->Link()->FDnum());
   lid.replace(sid, "");
   lid.erase(0, lid.find('.') + 1);

   // Create the path now
   XrdProofdAux::Form(cpath, "%s/%s", p->Client()->AdminPath(), lid.c_str());
   XrdProofUI ui;
   XrdProofdAux::GetUserInfo(fMgr->EffectiveUser(), ui);
   if (XrdProofdAux::AssertDir(cpath.c_str(), ui, 1) != 0) {
      XrdProofdAux::Form(emsg, "CreateAdminPath: error creating client admin path: %s",
                               cpath.c_str());
      return -1;
   }
   // Save client ID for full recovery
   XrdOucString cidpath;
   XrdProofdAux::Form(cidpath, "%s/cid", cpath.c_str());
   FILE *fcid = fopen(cidpath.c_str(), "w");
   if (fcid) {
      fprintf(fcid, "%d", p->CID());
      fclose(fcid);
   } else {
      XrdProofdAux::Form(emsg, "CreateAdminPath: error creating file for client id: %s",
                               cidpath.c_str());
      return -1;
   }
   // Done
   return 0;
}

//_____________________________________________________________________________
int XrdProofdClientMgr::CheckAdminPath(XrdProofdProtocol *p,
                                       XrdOucString &cidpath, XrdOucString &emsg)
{
   // Check the old-clients admin for an existing entry for this client and
   // read the client ID;

   emsg = "";

   if (!p) {
      XrdProofdAux::Form(emsg, "CheckAdminPath: invalid inputs (p: %p)", p);
      return -1;
   }

   // Create link ID
   XrdOucString lid(p->Link()->ID);
   XrdOucString sid;
   XrdProofdAux::Form(sid, ":%d", p->Link()->FDnum());
   lid.replace(sid, "");
   lid.erase(0, lid.find('.') + 1);

   // Create the path now
   XrdProofdAux::Form(cidpath, "%s/%s/cid", p->Client()->AdminPath(), lid.c_str());

   // Check last access time
   struct stat st;
   if (stat(cidpath.c_str(), &st) != 0 ||
      (int)(time(0) - st.st_atime) > fReconnectTimeOut) {
      cidpath.replace("/cid", "");
      XrdProofdAux::Form(emsg, "CheckAdminPath: reconnection timeout"
                               " expired: remove %s ", cidpath.c_str());
      if (XrdProofdAux::RmDir(cidpath.c_str()) != 0)
         emsg += ": failure!";
      return -1;
   }

   // Get the client ID for full recovery
   return GetIDFromAdminPath(cidpath.c_str(), emsg);
}

//_____________________________________________________________________________
int XrdProofdClientMgr::GetIDFromAdminPath(const char *cidpath, XrdOucString &emsg)
{
   // Check the old-clients admin for an existing entry for this client and
   // read the client ID;

   emsg = "";
   // Get the client ID for full recovery
   int cid = -1;
   FILE *fcid = fopen(cidpath, "r");
   if (fcid) {
      char line[64];
      if (fgets(line, sizeof(line), fcid))
         sscanf(line, "%d", &cid);
      fclose(fcid);
   } else if (errno != ENOENT) {
      XrdProofdAux::Form(emsg, "GetIDFromAdminPath: error reading client id"
                               " from: %s (errno: %d)", cidpath, errno);
   }
   // Done
   return cid;
}

//_____________________________________________________________________________
int XrdProofdClientMgr::ParsePreviousClients(XrdOucString &emsg)
{
   // Client entries for the clients still connected when the daemon terminated

   emsg = "";

   // Open dir
   DIR *dir = opendir(fClntAdminPath.c_str());
   if (!dir) {
      TRACE(XERR, "ParsePreviousClients: cannot open dir "<<fClntAdminPath<<
                  " ; error: "<<errno);
      return -1;
   }
   TRACE(DBG, "ParsePreviousClients: creating holders for active clients ...");

   // <adminpath>/usr.grp/xpdsock
   // <adminpath>/usr.grp/<pid>@<host>/cid
   // <adminpath>/usr.grp/<pid>@<host>/disconnected

   // Scan the active sessions admin path
   XrdOucString usrpath, cidpath, discpath, usr, grp;
   struct dirent *ent = 0;
   while ((ent = (struct dirent *)readdir(dir))) {
      // Skip the basic entries
      if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;
      XrdProofdAux::Form(usrpath, "%s/%s", fClntAdminPath.c_str(), ent->d_name);
      bool rm = 0;
      struct stat st;
      if (stat(usrpath.c_str(), &st) == 0) {
         usr = ent->d_name;
         grp = usr;
         usr.erase(usr.find('.'));
         grp.erase(0, grp.find('.')+1);
         TRACE(DBG, "ParsePreviousClients: found usr: "<<usr<<", grp: "<<grp);
         // Get client instance
         XrdProofdClient *c = GetClient(usr.c_str(), grp.c_str());
         if (!c) {
            XrdProofdAux::Form(emsg, "ParsePreviousClients: could not get client instance"
                                     " for {%s, %s}", usr.c_str(), grp.c_str());
            rm = 1;
         }
         // Open user sub-dir
         DIR *subdir = 0;
         if (!rm && !(subdir = opendir(usrpath.c_str()))) {
            TRACE(XERR, "ParsePreviousClients: cannot open dir "<<usrpath<<
                        " ; error: "<<errno);
            rm = 1;
         }
         if (!rm) {
            bool xrm = 0;
            struct dirent *sent = 0;
            while ((sent = (struct dirent *)readdir(subdir))) {
               // Skip the basic entries
               if (!strcmp(sent->d_name, ".") || !strcmp(sent->d_name, "..")) continue;
               if (!strcmp(sent->d_name, "xpdsock")) continue;
               XrdProofdAux::Form(cidpath, "%s/%s/cid", usrpath.c_str(), sent->d_name);
               // Check last access time
               if (stat(cidpath.c_str(), &st) != 0 ||
                  (int)(time(0) - st.st_atime) > fReconnectTimeOut) {
                  xrm = 1;
               }
               // Read the client ID and and reserve an entry in the related vector
               int cid = (!xrm) ? GetIDFromAdminPath(cidpath.c_str(), emsg) : -1;
               if (cid < 0)
                  xrm = 1;
               // Reserve an entry in the related vector
               if (!xrm && c->ReserveClientID(cid) != 0)
                  xrm = 1;
               // Flag this as disconnected
               if (!xrm) {
                  XrdProofdAux::Form(discpath, "%s/%s/disconnected", usrpath.c_str(), sent->d_name);
                  int fd = 0;
                  if ((fd = open(discpath.c_str(), O_EXCL | O_RDWR | O_CREAT)) < 0) {
                     TRACE(XERR,"ParsePreviousClients: unable to create path: " <<discpath);
                     xrm = 1;
                  }
                  close(fd);
                  if (!xrm)
                     fNDisconnected++;
               }
               // If it did not work remove the entry
               if (xrm) {
                  TRACE(DBG,"ParsePreviousClients: removing path: " <<cidpath);
                  cidpath.replace("/cid", "");
                  XrdProofdAux::Form(emsg, "ParsePreviousClients: failure: remove %s ", cidpath.c_str());
                  if (XrdProofdAux::RmDir(cidpath.c_str()) != 0)
                     emsg += ": failure!";
               }
            }
         }
         if (subdir)
            closedir(subdir);
      } else
         rm = 1;
      // If it did not work remove the entry
      if (rm) {
         TRACE(DBG,"ParsePreviousClients: removing path: " <<cidpath);
         cidpath.replace("/cid", "");
         XrdProofdAux::Form(emsg, "ParsePreviousClients: failure: remove %s ", cidpath.c_str());
         if (XrdProofdAux::RmDir(cidpath.c_str()) != 0)
            emsg += ": failure!";
      }
   }
   // Close the directory
   closedir(dir);

   // Notify the number of previously active clients now offline
   TRACE(DBG, "ParsePreviousClients: found "<<fNDisconnected<<" active clients");

   // Done
   return 0;
}

//_____________________________________________________________________________
int XrdProofdClientMgr::Auth(XrdProofdProtocol *p)
{
   // Analyse client authentication info

   struct sockaddr netaddr;
   XrdSecCredentials cred;
   XrdSecParameters *parm = 0;
   XrdOucErrInfo     eMsg;
   const char *eText;
   int rc = 1;
   XPD_SETRESP(p, "Auth");

   TRACEP(p, respid, REQ,"Auth: enter");

   // Ignore authenticate requests if security turned off
   if (!fCIA)
      return response->Send();
   cred.size   = p->Request()->header.dlen;
   cred.buffer = p->Argp()->buff;

   // If we have no auth protocol, try to get it
   if (!p->AuthProt()) {
      p->Link()->Name(&netaddr);
      XrdSecProtocol *ap = 0;
      if (!(ap = fCIA->getProtocol(p->Link()->Host(), netaddr, &cred, &eMsg))) {
         eText = eMsg.getErrText(rc);
         TRACEP(p, respid, XERR,"Auth: user authentication failed; "<<eText);
         response->Send(kXR_NotAuthorized, eText);
         return -EACCES;
      }
      p->SetAuthProt(ap);
      p->AuthProt()->Entity.tident = p->Link()->ID;
   }

   // Now try to authenticate the client using the current protocol
   if (!(rc = p->AuthProt()->Authenticate(&cred, &parm, &eMsg))) {
      const char *msg = (p->Status() & XPD_ADMINUSER ? "admin login as" : "login as");
      rc = response->Send();
      char status = p->Status();
      status &= ~XPD_NEED_AUTH;
      p->SetStatus(status);
      p->SetAuthEntity(&(p->AuthProt()->Entity));
      if (p->AuthProt()->Entity.name)
         fEDest->Log(XPD_LOG_01, ":Auth", p->Link()->ID, msg, p->AuthProt()->Entity.name);
      else
         fEDest->Log(XPD_LOG_01, ":Auth", p->Link()->ID, msg, " nobody");
      return rc;
   }

   // If we need to continue authentication, tell the client as much
   if (rc > 0) {
      TRACEP(p, respid, DBG, "Auth: more auth requested; sz: " <<(parm ? parm->size : 0));
      if (parm) {
         rc = response->Send(kXR_authmore, parm->buffer, parm->size);
         delete parm;
         return rc;
      }
      if (p->AuthProt()) {
         p->AuthProt()->Delete();
         p->SetAuthProt(0);
      }
      TRACEP(p, respid, XERR,"Auth: security requested additional auth w/o parms!");
      response->Send(kXP_ServerError,"invalid authentication exchange");
      return -EACCES;
   }

   // We got an error, bail out
   if (p->AuthProt()) {
      p->AuthProt()->Delete();
      p->SetAuthProt(0);
   }
   eText = eMsg.getErrText(rc);
   TRACEP(p, respid, XERR, "Auth: user authentication failed; "<<eText);
   response->Send(kXR_NotAuthorized, eText);
   return -EACCES;
}

//_____________________________________________________________________________
XrdSecService *XrdProofdClientMgr::LoadSecurity()
{
   // Load security framework and plugins, if not already done

   TRACE(ACT, "LoadSecurity: enter");

   const char *cfn = CfgFile();
   const char *seclib = fSecLib.c_str();

   // Make sure the input config file is defined
   if (!cfn) {
      if (fEDest) fEDest->Emsg("LoadSecurity","config file not specified");
      return 0;
   }

   // Open the security library
   void *lh = 0;
   if (!(lh = dlopen(seclib, RTLD_NOW))) {
      if (fEDest) fEDest->Emsg("LoadSecurity",dlerror(),"opening shared library",seclib);
      return 0;
   }

   // Get the server object creator
   XrdSecServLoader_t ep = 0;
   if (!(ep = (XrdSecServLoader_t)dlsym(lh, "XrdSecgetService"))) {
      if (fEDest) fEDest->Emsg("LoadSecurity", dlerror(),
                             "finding XrdSecgetService() in", seclib);
      return 0;
   }

   // Extract in a temporary file the directives prefixed "xpd.sec..." (filtering
   // out the prefix), "sec.protocol" and "sec.protparm"
   int nd = 0;
   char *rcfn = FilterSecConfig(nd);
   if (!rcfn) {
      if (nd == 0) {
         // No directives to be processed
         if (fEDest) fEDest->Emsg("LoadSecurity",
                                "no security directives: strong authentication disabled");
         return 0;
      }
      // Failure
      if (fEDest) fEDest->Emsg("LoadSecurity", "creating temporary config file");
      return 0;
   }

   // Get the server object
   XrdSecService *cia = 0;
   if (!(cia = (*ep)((fEDest ? fEDest->logger() : (XrdSysLogger *)0), rcfn))) {
      if (fEDest) fEDest->Emsg("LoadSecurity",
                             "Unable to create security service object via", seclib);
      return 0;
   }
   // Notify
   if (fEDest) fEDest->Emsg("LoadSecurity", "strong authentication enabled");

   // Unlink the temporary file and cleanup its path
   unlink(rcfn);
   delete[] rcfn;

   // All done
   return cia;
}

//__________________________________________________________________________
char *XrdProofdClientMgr::FilterSecConfig(int &nd)
{
   // Grep directives of the form "xpd.sec...", "sec.protparm" and
   // "sec.protocol" from file 'cfn' and save them in a temporary file,
   // stripping off the prefix "xpd." when needed.
   // If any such directory is found, the full path of the temporary file
   // is returned, with the number of directives found in 'nd'.
   // Otherwise 0 is returned and '-errno' specified in nd.
   // The caller has the responsability to unlink the temporary file and
   // to release the memory allocated for the path.

   static const char *pfx[] = { "xpd.sec.", "sec.protparm", "sec.protocol" };
   char *rcfn = 0;

   TRACE(ACT, "FilterSecConfig: enter");

   const char *cfn = CfgFile();

   // Make sure that we got an input file path and that we can open the
   // associated path.
   FILE *fin = 0;
   if (!cfn || !(fin = fopen(cfn,"r"))) {
      nd = (errno > 0) ? -errno : -1;
      return rcfn;
   }

   // Read the directives: if an interesting one is found, we create
   // the output temporary file
   int fd = -1;
   char lin[2048];
   while (fgets(lin,sizeof(lin),fin)) {
      if (!strncmp(lin, pfx[0], strlen(pfx[0])) ||
          !strncmp(lin, pfx[1], strlen(pfx[1])) ||
          !strncmp(lin, pfx[2], strlen(pfx[2]))) {
         // Target directive found
         nd++;
         // Create the output file, if not yet done
         if (!rcfn) {
            rcfn = new char[strlen(fMgr->TMPdir()) + strlen("/xpdcfn_XXXXXX") + 2];
            sprintf(rcfn, "%s/xpdcfn_XXXXXX", fMgr->TMPdir());
            if ((fd = mkstemp(rcfn)) < 0) {
               delete[] rcfn;
               nd = (errno > 0) ? -errno : -1;
               fclose(fin);
               rcfn = 0;
               return rcfn;
            }
         }
         XrdOucString slin = lin;
         // Strip the prefix "xpd."
         slin.replace("xpd.","");
         // Make keyword substitution
         fMgr->ResolveKeywords(slin, 0);
         // Write the line to the output file
         XrdProofdAux::Write(fd, slin.c_str(), slin.length());
      }
   }

   // Close files
   fclose(fin);
   close(fd);

   return rcfn;
}

//______________________________________________________________________________
XrdProofdClient *XrdProofdClientMgr::GetClient(const char *usr,
                                               const char *grp, const char *sock)
{
   // Handle request for localizing a client instance for {usr, grp} from the list.
   // Create a new instance, if required; for new instances, use the path at 'sock'
   // for the unix socket, or generate a new one, if sock = 0.

   XrdProofdClient *c = 0;
   std::list<XrdProofdClient *>::iterator i;
   for (i = fProofdClients.begin(); i != fProofdClients.end(); ++i) {
      if ((c = *i) && c->Match(usr,grp))
         break;
      c = 0;
   }

   if (!c) {
      // Is this a potential user?
      XrdProofUI ui;
      XrdOucString emsg;
      bool su;
      if (fMgr->CheckUser(usr, ui, emsg, su) == 0) {
         // Yes: create an (invalid) instance of XrdProofdClient:
         // It would be validated on the first valid login
         ui.fUser = usr;
         ui.fGroup = grp;
         bool full = (fMgr->SrvType() != kXPD_Worker)  ? 1 : 0;
         XrdOucString tmp(fMgr->TMPdir());
         if (sock)
            // Use existing unix socket
            XrdProofdAux::Form(tmp, "sock:%s", sock);
         c = new XrdProofdClient(ui, full, fMgr->ChangeOwn(), fEDest, fClntAdminPath.c_str());
         if (c && c->IsValid()) {
            // Locate and set the group, if any
            if (fMgr->GroupsMgr() && fMgr->GroupsMgr()->Num() > 0) {
               XrdProofGroup *g = fMgr->GroupsMgr()->GetUserGroup(usr, grp);
               if (g)
                  c->SetGroup(g->Name());
               else
                  TRACE(XERR, "XrdProofdClientMgr::GetClient: group = "<<grp<<" nor found");
            }
            // Add to the list
            fProofdClients.push_back(c);
            TRACE(DBG, "XrdProofdClientMgr::GetClient: instance for {client, group} = {"<<usr<<", "<<
                        grp<<"} created and added to the list ("<<c<<")");
         } else {
            TRACE(XERR, "XrdProofdClientMgr::GetClient: instance for {client, group} = {"<<usr<<", "<<
                        grp<<"} is invalid");
            SafeDelete(c);
         }
      } else {
         TRACE(XERR, "XrdProofdClientMgr::GetClient: instance for {client, group} = {"<<usr<<", "<<
                      grp<<"} could not be created: "<<emsg);
      }
   }

   // Over
   return c;
}

//______________________________________________________________________________
void XrdProofdClientMgr::Broadcast(XrdProofdClient *clnt, const char *msg)
{
   // Broadcast message 'msg' to the connected instances of client 'clnt' or to all
   // connected instances if clnt == 0.

   // The clients to notified
   std::list<XrdProofdClient *> *clnts;
   if (!clnt) {
      // The full list
      clnts = &fProofdClients;
   } else {
      clnts = new std::list<XrdProofdClient *>;
      clnts->push_back(clnt);
   }

   // Loop over them
   XrdProofdClient *c = 0;
   std::list<XrdProofdClient *>::iterator i;
   for (i = clnts->begin(); i != clnts->end(); ++i) {
      if ((c = *i))
         c->Broadcast(msg);
   }

   // Cleanup, if needed
   if (clnt) delete clnts;
}

//______________________________________________________________________________
void XrdProofdClientMgr::TerminateSessions(XrdProofdClient *clnt, const char *msg,
                                           int srvtype)
{
   // Terminate sessions of client 'clnt' or to of all clients if clnt == 0.
   // The list of process IDs having been signalled is returned.

   // The clients to notified
   std::list<XrdProofdClient *> *clnts;
   if (!clnt) {
      // The full list
      clnts = &fProofdClients;
   } else {
      clnts = new std::list<XrdProofdClient *>;
      clnts->push_back(clnt);
   }

   int wfd = fMgr->SessionMgr() ? fMgr->SessionMgr()->WriteFd() : -1;

   // Loop over them
   XrdProofdClient *c = 0;
   std::list<XrdProofdClient *>::iterator i;
   for (i = clnts->begin(); i != clnts->end(); ++i) {
      if ((c = *i)) {

         // This part may be not thread safe
         XrdSysMutexHelper mh(c->Mutex());

         // Notify the attached clients that we are going to cleanup
         c->Broadcast(msg);

         // Loop over client sessions and terminate cliant sessions
         c->TerminateSessions(srvtype, 0, 0, wfd);
      }
   }

   // Cleanup, if needed
   if (clnt) delete clnts;
}

//______________________________________________________________________________
void XrdProofdClientMgr::SetLock(bool on, XrdProofdClient *clnt)
{
   // Lock / Unlock client 'clnt' or all clients (clnt == 0)

   // The clients to notified
   std::list<XrdProofdClient *> *clnts;
   if (!clnt) {
      // The full list
      clnts = &fProofdClients;
   } else {
      clnts = new std::list<XrdProofdClient *>;
      clnts->push_back(clnt);
   }

   // Loop over them
   XrdProofdClient *c = 0;
   std::list<XrdProofdClient *>::iterator i;
   for (i = clnts->begin(); i != clnts->end(); ++i) {
      if ((c = *i))
         if (on)
            c->Mutex()->Lock();
         else
            c->Mutex()->UnLock();
   }

   // Cleanup, if needed
   if (clnt) delete clnts;
}
