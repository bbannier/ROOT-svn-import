// @(#)root/proofd:$Id:$
// Author: G. Ganis Feb 2008

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// XrdProofdAdmin                                                       //
//                                                                      //
// Author: G. Ganis, CERN, 2008                                         //
//                                                                      //
// Envelop class for admin services.                                    //
// Loaded as service by XrdProofdManager.                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
#include "XrdProofdPlatform.h"

#include "Xrd/XrdBuffer.hh"

#include "XrdProofdAdmin.h"
#include "XrdProofdClient.h"
#include "XrdProofdClientMgr.h"
#include "XrdProofdManager.h"
#include "XrdProofdNetMgr.h"
#include "XrdProofdPriorityMgr.h"
#include "XrdProofdProofServMgr.h"
#include "XrdProofdProtocol.h"
#include "XrdProofGroup.h"
#include "XrdProofSched.h"
#include "XrdProofServProxy.h"
#include "XrdROOT.h"

// Tracing utilities
#include "XrdProofdTrace.h"
static const char *gTraceID = "";
// extern XrdOucTrace *XrdProofdTrace;
#define TRACEID gTraceID

//______________________________________________________________________________
XrdProofdAdmin::XrdProofdAdmin(XrdProofdManager *mgr)
{
   // Constructor

   fMgr = mgr;
   fPriorityMgrFd = (fMgr && fMgr->PriorityMgr()) ? fMgr->PriorityMgr()->WriteFd() : -1;
}

//______________________________________________________________________________
int XrdProofdAdmin::Process(XrdProofdProtocol *p, int type)
{
   // Process admin request
   int rc = 1;
   XPD_SETRESP(p, "Process");

   TRACEP(p, respid, REQ, "Admin::Process: enter: req id: " << p->Request()->header.requestid);

   XrdOucString emsg;
   switch (type) {
      case kQuerySessions:
         return QuerySessions(p);
      case kQueryLogPaths:
         return QueryLogPaths(p);
      case kCleanupSessions:
         return CleanupSessions(p);
      case kSendMsgToUser:
         return SendMsgToUser(p);
      case kGroupProperties:
         return SetGroupProperties(p);
      case kGetWorkers:
         return GetWorkers(p);
      case kQueryWorkers:
         return QueryWorkers(p);
      case kQueryROOTVersions:
         return QueryROOTVersions(p);
      case kROOTVersion:
         return SetROOTVersion(p);
      case kSessionAlias:
         return SetSessionAlias(p);
      case kSessionTag:
         return SetSessionTag(p);
      default:
         emsg += "Invalid type: ";
         emsg += type;
         break;
   }

   // Notify invalid request
   response->Send(kXR_InvalidRequest, emsg.c_str());

   // Done
   return 0;
}

//______________________________________________________________________________
int XrdProofdAdmin::QueryROOTVersions(XrdProofdProtocol *p)
{
   // Handle request for list of ROOT versions

   int rc = 1;
   XPD_SETRESP(p, "QueryROOTVersions");

   XrdOucString msg = fMgr->ROOTMgr()->ExportVersions(p->Client()->ROOT());

   TRACEP(p, respid, DBG, "Admin::QueryROOTVersions: sending: "<<msg);

   // Send back to user
   response->Send((void *)msg.c_str(), msg.length()+1);

   // Over
   return rc;
}

//______________________________________________________________________________
int XrdProofdAdmin::SetROOTVersion(XrdProofdProtocol *p)
{
   // Handle request for changing the default ROOT version

   int rc = 1;
   XPD_SETRESP(p, "SetROOTVersion");

   // Change default ROOT version
   const char *t = p->Argp() ? (const char *) p->Argp()->buff : "default";
   int len = p->Argp() ? p->Request()->header.dlen : strlen("default");
   XrdOucString tag(t,len);

   // If a user name is given separate it out and check if
   // we can do the operation
   XrdOucString usr;
   if (tag.beginswith("u:")) {
      usr = tag;
      usr.erase(usr.rfind(' '));
      usr.replace("u:","");
      TRACEP(p, respid, DBG, "Admin::SetROOTVersion: request is for user: "<< usr);
      // Isolate the tag
      tag.erase(0,tag.find(' ') + 1);
   }
   TRACEP(p, respid, ADMIN, "Admin::SetROOTVersion: version tag: "<< tag);

   // If the action is requested for a user different from us we
   // must be 'superuser'
   XrdProofdClient *c = p->Client();
   XrdOucString grp;
   if (usr.length() > 0) {
      // Separate group info, if any
      if (usr.find(':') != STR_NPOS) {
         grp = usr;
         grp.erase(grp.rfind(':'));
         usr.erase(0,usr.find(':') + 1);
      } else {
         XrdProofGroup *g =
            (fMgr->GroupsMgr()) ? fMgr->GroupsMgr()->GetUserGroup(usr.c_str()) : 0;
         grp = g ? g->Name() : "default";
      }
      if (usr != p->Client()->User()) {
         if (!p->SuperUser()) {
            usr.insert("SetROOTVersion: not allowed to change settings for usr '", 0);
            usr += "'";
            TRACEP(p, respid, XERR, usr.c_str());
            response->Send(kXR_InvalidRequest, usr.c_str());
            return rc;
         }
         // Lookup the list
         if (!(c = fMgr->ClientMgr()->GetClient(usr.c_str(), grp.c_str()))) {
            // No: fail
            XrdOucString emsg("SetROOTVersion: user not found or not allowed: ");
            emsg += usr;
            TRACEP(p, respid, XERR, emsg.c_str());
            response->Send(kXR_InvalidRequest, emsg.c_str());
            return rc;
         }
      }
   }

   // Search in the list
   XrdROOT *r = fMgr->ROOTMgr()->GetVersion(tag.c_str());
   bool ok = r ? 1 : 0;
   if (!r && tag == "default") {
      // If not found we may have been requested to set the default version
      r = fMgr->ROOTMgr()->DefaultVersion();
      ok = r ? 1 : 0;
   }

   if (ok) {
      // Save the version in the client instance
      c->SetROOT(r);
      // Notify
      TRACEP(p, respid, ADMIN, "Admin::SetROOTVersion: default changed to "<<c->ROOT()->Tag()<<
                   " for {client, group} = {"<<usr<<", "<<grp<<"} ("<<c<<")");
      // Forward down the tree, if not leaf
      if (fMgr->SrvType() != kXPD_Worker) {
         XrdOucString buf("u:");
         buf += c->UI().fUser;
         buf += " ";
         buf += tag;
         int type = ntohl(p->Request()->proof.int1);
         fMgr->NetMgr()->Broadcast(type, buf.c_str(),response);
      }
      // Acknowledge user
      response->Send();
   } else {
      tag.insert("Admin::SetROOTVersion: tag '", 0);
      tag += "' not found in the list of available ROOT versions";
      TRACEP(p, respid, XERR, tag.c_str());
      response->Send(kXR_InvalidRequest, tag.c_str());
   }

   // Over
   return rc;
}

//______________________________________________________________________________
int XrdProofdAdmin::QueryWorkers(XrdProofdProtocol *p)
{
   // Handle request for getting the list of potential workers

   int rc = 1;
   XPD_SETRESP(p, "QueryWorkers");

   // Send back a list of potentially available workers
   XrdOucString sbuf(1024);
   fMgr->ProofSched()->ExportInfo(sbuf);

   // Send buffer
   char *buf = (char *) sbuf.c_str();
   int len = sbuf.length() + 1;
   TRACEP(p, respid, DBG, "Admin::QueryWorkers: sending: "<<buf);

   // Send back to user
   response->Send(buf, len);

   // Over
   return rc;
}

//______________________________________________________________________________
int XrdProofdAdmin::GetWorkers(XrdProofdProtocol *p)
{
   // Handle request for getting the best set of workers

   int rc = 1;
   XPD_SETRESP(p, "GetWorkers");

   // Unmarshall the data
   int psid = ntohl(p->Request()->proof.sid);

   // Find server session
   XrdProofServProxy *xps = 0;
   if (!p->Client() || !(xps = p->Client()->GetProofServ(psid))) {
      TRACEP(p, respid, XERR, "Admin::GetWorkers: session ID not found");
      response->Send(kXR_InvalidRequest,"session ID not found");
      return rc;
   }

   // We should query the chosen resource provider
   XrdOucString wrks;

   if (fMgr->GetWorkers(wrks, xps) !=0 ) {
      // Something wrong
      response->Send(kXR_InvalidRequest,"GetWorkers: failure");
      return rc;
   } else {
      // Send buffer
      char *buf = (char *) wrks.c_str();
      int len = wrks.length() + 1;
      TRACEP(p, respid, DBG, "Admin::GetWorkers: sending: "<<buf);

      // Send back to user
      response->Send(buf, len);
   }

   // Over
   return rc;
}

//______________________________________________________________________________
int XrdProofdAdmin::SetGroupProperties(XrdProofdProtocol *p)
{
   // Handle request for setting group properties

   int rc = 1;
   XPD_SETRESP(p, "SetGroupProperties");

   // User's group
   int   len = p->Request()->header.dlen;
   char *grp = new char[len+1];
   memcpy(grp, p->Argp()->buff, len);
   grp[len] = 0;
   TRACEP(p, respid, ADMIN, "Admin::SetGroupProperties: request to change priority for group '"<< grp<<"'");

   // Make sure is the current one of the user
   if (strcmp(grp, p->Client()->UI().fGroup.c_str())) {
      TRACEP(p, respid, XERR, "Admin::SetGroupProperties: received group does not match the user's one");
      response->Send(kXR_InvalidRequest,
                         "SetGroupProperties: received group does not match the user's one");
      return rc;
   }

   // The priority value
   int priority = ntohl(p->Request()->proof.int2);

   // Tell the priority manager
   if (fPriorityMgrFd > 0) {
      int type = 1;
      if (write(fPriorityMgrFd, &type, sizeof(type)) !=  sizeof(type)) {
         TRACEP(p, respid, XERR, "Admin::SetGroupProperties: problem sending message type on the pipe");
         response->Send(kXR_InvalidRequest,
                             "SetGroupProperties: problem sending message type on the pipe");
         return rc;
      }
      char buf[1024];
      sprintf(buf, "%s %d", grp, priority);
      int len = strlen(buf)+1;
      if (write(fPriorityMgrFd, &len, sizeof(len)) !=  sizeof(len)) {
         TRACEP(p, respid, XERR, "Admin::SetGroupProperties: problem sending message length on the pipe");
         response->Send(kXR_InvalidRequest,
                             "SetGroupProperties: problem sending message length on the pipe");
         return rc;
      }
      if (write(fPriorityMgrFd, buf, len) !=  len) {
         TRACEP(p, respid, XERR, "Admin::SetGroupProperties: problem sending message on the pipe");
         response->Send(kXR_InvalidRequest,
                             "SetGroupProperties: problem sending message on the pipe");
         return rc;
      }
   }

   // Notify
   TRACEP(p, respid, ADMIN, "Admin::SetGroupProperties: priority for group '"<< grp<<"' has been set to "<<priority);

   // Acknowledge user
   response->Send();

   // Over
   return rc;
}

//______________________________________________________________________________
int XrdProofdAdmin::SendMsgToUser(XrdProofdProtocol *p)
{
   // Handle request for sending a message to a user

   int rc = 1;
   XPD_SETRESP(p, "SendMsgToUser");

   // Target client (default us)
   XrdProofdClient *tgtclnt = p->Client();
   XrdProofdClient *c = 0;
   std::list<XrdProofdClient *>::iterator i;

   // Extract the user name, if any
   int len = p->Request()->header.dlen;
   if (len <= 0) {
      // No message: protocol error?
      TRACEP(p, respid, XERR, "Admin::SendMsgToUser: no message");
      response->Send(kXR_InvalidRequest,"SendMsgToUser: no message");
      return rc;
   }

   XrdOucString cmsg((const char *)p->Argp()->buff, len);
   XrdOucString usr;
   if (cmsg.beginswith("u:")) {
      // Extract user
      int isp = cmsg.find(' ');
      if (isp != STR_NPOS) {
         usr.assign(cmsg, 2, isp-1);
         cmsg.erase(0, isp+1);
      }
      if (usr.length() > 0) {
         TRACEP(p, respid, DBG, "Admin::SendMsgToUser: request for user: '"<<usr<<"'");
         // Find the client instance
         bool clntfound = 0;
         if ((c = fMgr->ClientMgr()->GetClient(usr.c_str(), 0))) {
            tgtclnt = c;
            clntfound = 1;
         }
         if (!clntfound) {
            // No user: protocol error?
            TRACEP(p, respid, XERR, "Admin::SendMsgToUser: target client not found");
            response->Send(kXR_InvalidRequest,
                           "SendMsgToUser: target client not found");
            return rc;
         }
      }
   }
   // Recheck message length
   if (cmsg.length() <= 0) {
      // No message: protocol error?
      TRACEP(p, respid, XERR, "Admin::SendMsgToUser: no message after user specification");
      response->Send(kXR_InvalidRequest,
                          "SendMsgToUser: no message after user specification");
      return rc;
   }

   // Check if allowed
   if (!p->SuperUser()) {
      if (usr.length() > 0) {
         if (tgtclnt != p->Client()) {
            TRACEP(p, respid, XERR, "Admin::SendMsgToUser: not allowed to send messages to usr '"<<usr<<"'");
            response->Send(kXR_InvalidRequest,
                                "SendMsgToUser: not allowed to send messages to specified usr");
            return rc;
         }
      } else {
         TRACEP(p, respid, XERR, "Admin::SendMsgToUser: not allowed to send messages to connected users");
         response->Send(kXR_InvalidRequest,
                             "SendMsgToUser: not allowed to send messages to connected users");
         return rc;
      }
   } else {
      if (usr.length() <= 0) tgtclnt = 0;
   }

   // The clients to notified
   fMgr->ClientMgr()->Broadcast(tgtclnt, cmsg.c_str());

   // Acknowledge user
   response->Send();

   // Over
   return rc;
}

//______________________________________________________________________________
int XrdProofdAdmin::QuerySessions(XrdProofdProtocol *p)
{
   // Handle request for list of sessions

   int rc = 1;
   XPD_SETRESP(p, "QuerySessions");

   XrdOucString msg = p->Client()->ExportSessions();

   TRACEP(p, respid, DBG, "Admin::QuerySessions: sending: "<<msg);

   // Send back to user
   response->Send((void *)msg.c_str(), msg.length()+1);

   // Over
   return rc;
}

//______________________________________________________________________________
int XrdProofdAdmin::QueryLogPaths(XrdProofdProtocol *p)
{
   // Handle request for log paths 

   int rc = 1;
   XPD_SETRESP(p, "QueryLogPaths");

   int ridx = ntohl(p->Request()->proof.int2);

   // Find out for which session is this request
   XrdOucString stag, master, user, buf;
   int len = p->Request()->header.dlen;
   if (len > 0) {
      buf.assign(p->Argp()->buff,0,len-1);
      int im = buf.find("|master:");
      int iu = buf.find("|user:");
      stag = buf;
      stag.erase(stag.find("|"));
      if (im != STR_NPOS) {
         master.assign(buf, im + strlen("|master:"));
         master.erase(master.find("|"));
         TRACEP(p, respid, DBG,"Admin::QueryLogPaths: master: "<<master);
      }
      if (iu != STR_NPOS) {
         user.assign(buf, iu + strlen("|user:"));
         user.erase(user.find("|"));
         TRACEP(p, respid, DBG,"Admin::QueryLogPaths: user: "<<user);
      }
      if (stag.beginswith('*'))
         stag = "";
   }

   XrdProofdClient *client = (user.length() > 0) ? 0 : p->Client();
   if (!client)
      // Find the client instance
      client = fMgr->ClientMgr()->GetClient(user.c_str(), 0);
   if (!client) {
      TRACEP(p, respid, XERR, "Admin::QueryLogPaths: query sess logs: client for '"<<user<<"' not found");
      response->Send(kXR_InvalidRequest,"QueryLogPaths: query log: client not found");
      return rc;
   }

   XrdOucString tag = (stag == "" && ridx >= 0) ? "last" : stag;
   if (stag == "" && client->Sandbox()->GuessTag(tag, ridx) != 0) {
      TRACEP(p, respid, XERR, "Admin::QueryLogPaths: query sess logs: session tag not found");
      response->Send(kXR_InvalidRequest,"QueryLogPaths: query log: session tag not found");
      return rc;
   }

   // Return message
   XrdOucString rmsg;

   if (master.length() <= 0) {
      // The session tag first
      rmsg += tag; rmsg += "|";
      // The pool URL second
      rmsg += fMgr->PoolURL(); rmsg += "|";
   }

   // Locate the local log file
   XrdOucString sdir(client->Sandbox()->Dir());
   sdir += "/session-";
   sdir += tag;

   // Open dir
   DIR *dir = opendir(sdir.c_str());
   if (!dir) {
      XrdOucString msg("Admin::QueryLogPaths: cannot open dir ");
      msg += sdir; msg += " (errno: "; msg += errno; msg += ")";
      TRACEP(p, respid, XERR, msg.c_str());
      response->Send(kXR_InvalidRequest, msg.c_str());
      return rc;
   }
   // Scan the directory to add the top master (only if top master)
   if (master.length() <= 0) {
      bool found = 0;
      struct dirent *ent = 0;
      while ((ent = (struct dirent *)readdir(dir))) {
         if (!strncmp(ent->d_name, "master-", 7) &&
            strstr(ent->d_name, ".log")) {
            rmsg += "|0 proof://"; rmsg += fMgr->Host(); rmsg += ':';
            rmsg += fMgr->Port(); rmsg += '/';
            rmsg += sdir; rmsg += '/'; rmsg += ent->d_name;
            found = 1;
            break;
         }
      }
   }
   // Close dir
   closedir(dir);

   // Now open the workers file
   XrdOucString wfile(sdir);
   wfile += "/.workers";
   FILE *f = fopen(wfile.c_str(), "r");
   if (f) {
      char ln[2048];
      while (fgets(ln, sizeof(ln), f)) {
         if (ln[strlen(ln)-1] == '\n')
            ln[strlen(ln)-1] = 0;
         // Locate status and url
         char *ps = strchr(ln, ' ');
         if (ps) {
            *ps = 0;
            ps++;
            // Locate ordinal
            char *po = strchr(ps, ' ');
            if (po) {
               po++;
               // Locate path
               char *pp = strchr(po, ' ');
               if (pp) {
                  *pp = 0;
                  pp++;
                  // Record now
                  rmsg += "|"; rmsg += po; rmsg += " ";
                  if (master.length() > 0) {
                     rmsg += master;
                     rmsg += ",";
                  }
                  rmsg += ln; rmsg += '/';
                  rmsg += pp;
                  // If the line is for a submaster, we have to get the info
                  // about its workers
                  bool ismst = (strstr(pp, "master-")) ? 1 : 0;
                  if (ismst) {
                     XrdOucString msg(stag);
                     msg += "|master:";
                     msg += ln;
                     msg += "|user:";
                     msg += XrdClientUrlInfo(ln).User;
                     char *bmst = fMgr->NetMgr()->ReadLogPaths((const char *)&ln[0], msg.c_str(), ridx);
                     if (bmst) {
                        rmsg += bmst;
                        free(bmst);
                     }
                  }
               }
            }
         }
      }
      fclose(f);
   }

   // Send back to user
   response->Send((void *) rmsg.c_str(), rmsg.length()+1);

   // Over
   return rc;
}

//______________________________________________________________________________
int XrdProofdAdmin::CleanupSessions(XrdProofdProtocol *p)
{
   // Handle request of 

   int rc = 1;
   XPD_SETRESP(p, "CleanupSessions");

   XrdOucString cmsg;

   // Target client (default us)
   XrdProofdClient *tgtclnt = p->Client();

   // If super user we may be requested to cleanup everything
   bool all = 0;
   char *usr = 0;
   bool clntfound = 1;
   if (p->SuperUser()) {
      int what = ntohl(p->Request()->proof.int2);
      all = (what == 1) ? 1 : 0;

      if (!all) {
         // Get a user name, if any.
         // A super user can ask cleaning for clients different from itself
         char *buf = 0;
         int len = p->Request()->header.dlen;
         if (len > 0) {
            clntfound = 0;
            buf = p->Argp()->buff;
            len = (len < 9) ? len : 8;
         } else {
            buf = (char *) p->Client()->User();
            len = strlen(p->Client()->User());
         }
         if (len > 0) {
            usr = new char[len+1];
            memcpy(usr, buf, len);
            usr[len] = '\0';
            // Group info, if any
            char *grp = strstr(usr, ":");
            if (grp)
               *grp++ = 0;
            // Find the client instance
            XrdProofdClient *c = fMgr->ClientMgr()->GetClient(usr, grp);
            if (c) {
               tgtclnt = c;
               clntfound = 1;
            }
            TRACEP(p, respid, ADMIN, "Admin::CleanupSessions: superuser, cleaning usr: "<< usr);
         }
      } else {
         tgtclnt = 0;
         TRACEP(p, respid, ADMIN, "Admin::CleanupSessions: superuser, all sessions cleaned");
      }
   } else {
      // Define the user name for later transactions (their executed under
      // the admin name)
      int len = strlen(tgtclnt->User()) + 1;
      usr = new char[len+1];
      memcpy(usr, tgtclnt->User(), len);
      usr[len] = '\0';
   }

   // We cannot continue if we do not have anything to clean
   if (!clntfound) {
      TRACEP(p, respid, ADMIN, "Admin::CleanupSessions: client '"<<usr<<"' has no sessions - do nothing");
   }

   // Asynchronous notification to requester
   if (fMgr->SrvType() != kXPD_Worker) {
      cmsg = "CleanupSessions: signalling active sessions for termination";
      response->Send(kXR_attn, kXPD_srvmsg, (char *) cmsg.c_str(), cmsg.length());
   }

   // Send a termination request to client sessions
   std::list<int> signalledpid;
   XrdOucString msg = "CleanupSessions: cleaning up client: requested by: ";
   msg += p->Link()->ID;
   int srvtype = ntohl(p->Request()->proof.int2);
   fMgr->ClientMgr()->TerminateSessions(tgtclnt, msg.c_str(), srvtype, signalledpid);

   // Asynchronous notification to requester
   if (fMgr->SrvType() != kXPD_Worker) {
      cmsg = "CleanupSessions: verifying termination status (may take up to 10 seconds)";
      response->Send(kXR_attn, kXPD_srvmsg, 0, (char *) cmsg.c_str(), cmsg.length());
   }

   // Now we give sometime to sessions to terminate (10 sec).
   // We check the status every second
   int nw = 10;
   int nleft = signalledpid.size();
   while (nw-- && nleft > 0) {

      // Loop over the list of processes requested to terminate
      std::list<int>::iterator ii;
      for (ii = signalledpid.begin(); ii != signalledpid.end(); )
         if (XrdProofdAux::VerifyProcessByID(*ii) == 0) {
            nleft--;
            ii = signalledpid.erase(ii);
         } else
            ++ii;

      // Wait a bit before retrying
      sleep(1);
   }

   // Lock the interested client mutexes (no action is allowed while
   // doing this
   fMgr->ClientMgr()->SetLock(1,tgtclnt);

   // Asynchronous notification to requester
   if (fMgr->SrvType() != kXPD_Worker) {
      cmsg = "CleanupSessions: terminating the remaining sessions ...";
      response->Send(kXR_attn, kXPD_srvmsg, 0, (char *) cmsg.c_str(), cmsg.length());
   }

   // Now we cleanup what left (any zombies or super resistent processes)
   int ncln = fMgr->SessionMgr()->CleanupProofServ(all, usr);
   if (ncln > 0) {
      // Asynchronous notification to requester
      cmsg = "CleanupSessions: wait 5 seconds for completion ...";
      response->Send(kXR_attn, kXPD_srvmsg, 0, (char *) cmsg.c_str(), cmsg.length());
      sleep(5);
   }

   // Cleanup all possible sessions around
   // (forward down the tree only if not leaf)
   if (fMgr->SrvType() != kXPD_Worker) {

      // Asynchronous notification to requester
      cmsg = "CleanupSessions: forwarding the reset request to next tier(s) ";
      response->Send(kXR_attn, kXPD_srvmsg, 0, (char *) cmsg.c_str(), cmsg.length());

      int type = ntohl(p->Request()->proof.int1);
      fMgr->NetMgr()->Broadcast(type, usr,response, 1);
   }

   // Unlock the locked client mutexes
   fMgr->ClientMgr()->SetLock(0,tgtclnt);

   // Cleanup usr
   SafeDelArray(usr);

   // Acknowledge user
   response->Send();

   // Over
   return rc;
}

//______________________________________________________________________________
int XrdProofdAdmin::SetSessionAlias(XrdProofdProtocol *p)
{
   // Handle request for setting the session alias

   int rc = 1;
   XPD_SETRESP(p, "SetSessionAlias");

   //
   // Specific info about a session
   int psid = ntohl(p->Request()->proof.sid);
   XrdProofServProxy *xps = 0;
   if (!p->Client() || !(xps = p->Client()->GetProofServ(psid))) {
      TRACEP(p, respid, XERR, "Admin::SetSessionAlias: session ID not found");
      response->Send(kXR_InvalidRequest,"SetSessionAlias: session ID not found");
      return rc;
   }

   // Set session alias
   const char *msg = (const char *) p->Argp()->buff;
   int   len = p->Request()->header.dlen;
   if (len > kXPROOFSRVALIASMAX - 1)
      len = kXPROOFSRVALIASMAX - 1;

   // Save tag
   if (len > 0 && msg) {
      xps->SetAlias(msg);
      if (TRACING(ADMIN)) {
         TRACEP(p, respid, DBG, "Admin::SetSessionAlias: session alias set to: "<<xps->Alias());
      }
   }

   // Acknowledge user
   response->Send();

   // Over
   return rc;
}

//______________________________________________________________________________
int XrdProofdAdmin::SetSessionTag(XrdProofdProtocol *p)
{
   // Handle request for setting the session tag

   int rc = 1;
   XPD_SETRESP(p, "SetSessionTag");
   //
   // Specific info about a session
   int psid = ntohl(p->Request()->proof.sid);
   XrdProofServProxy *xps = 0;
   if (!p->Client() || !(xps = p->Client()->GetProofServ(psid))) {
      TRACEP(p, respid, XERR, "Admin::SetSessionTag: session ID not found");
      response->Send(kXR_InvalidRequest,"SetSessionTag: session ID not found");
      return rc;
   }

   // Set session tag
   const char *msg = (const char *) p->Argp()->buff;
   int   len = p->Request()->header.dlen;
   if (len > kXPROOFSRVTAGMAX - 1)
      len = kXPROOFSRVTAGMAX - 1;

   // Save tag
   if (len > 0 && msg) {
      xps->SetTag(msg);
      if (TRACING(ADMIN)) {
         TRACEP(p, respid, DBG, "Admin::SetSessionTag: session tag set to: "<<xps->Tag());
      }
   }

   // Acknowledge user
   response->Send();

   // Over
   return rc;
}

