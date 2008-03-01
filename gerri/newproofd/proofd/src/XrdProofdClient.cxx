// @(#)root/proofd:$Id$
// Author: G. Ganis  June 2007

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

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
#include "XrdNet/XrdNet.hh"
#include "XrdSys/XrdSysPriv.hh"

#include "XrdProofdClient.h"
#include "XrdProofdProtocol.h"
#include "XrdProofServProxy.h"

#include "XrdProofdTrace.h"
static const char *gTraceID = "";
extern XrdOucTrace *XrdProofdTrace;
#define TRACEID gTraceID

//__________________________________________________________________________
XrdProofdClient::XrdProofdClient(XrdProofUI ui, bool master, bool changeown,
                                 XrdSysError *edest, const char *tmp)
                : fSandbox(ui, master, changeown)
{
   // Constructor

   fProofServs.reserve(10);
   fClients.reserve(10);
   fUI = ui;
   fUNIXSock = 0;
   fUNIXSockPath = 0;
   fUNIXSockSaved = 0;
   if (tmp && !strncmp(tmp, "sock:", 5)) {
      // Using previously defined socket path
      fUNIXSockPath = new char[strlen(tmp)-4];
      sprintf(fUNIXSockPath,"%s", tmp + 5);
      fUNIXSockSaved = 1;
   }
   fROOT = 0;
   fIsValid = 0;
   fRefSid = 0;
   fChangeOwn = changeown;

   // Create the UNIX socket
   if (fSandbox.IsValid())
      if (CreateUNIXSock(edest, tmp, changeown) == 0)
         fIsValid = 1;
}

//__________________________________________________________________________
XrdProofdClient::~XrdProofdClient()
{
   // Destructor

   // Unix socket
   SafeDel(fUNIXSock);
   SafeDelArray(fUNIXSockPath);
}

//__________________________________________________________________________
bool XrdProofdClient::Match(const char *id, const char *grp)
{
   // return TRUE if this instance matches 'id' (and 'grp', if defined) 

   bool rc = (id && !strcmp(id, User())) ? 1 : 0;
   if (rc && grp && strlen(grp) > 0)
      rc = (grp && !strcmp(grp, Group())) ? 1 : 0;

   return rc;
}

//__________________________________________________________________________
int XrdProofdClient::GetClientID(XrdProofdProtocol *p)
{
   // Get next free client ID. If none is found, increase the vector size
   // and get the first new one

   XrdSysMutexHelper mh(fMutex);

   int ic = 0;
   // Search for free places in the existing vector
   for (ic = 0; ic < (int)fClients.size() ; ic++) {
      if (!fClients[ic]) {
         fClients[ic] = p;
         return ic;
      }
   }

   // We need to resize (double it)
   if (ic >= (int)fClients.capacity())
      fClients.reserve(2*fClients.capacity());

   // Fill in new element
   fClients.push_back(p);

   TRACE(DBG, "XrdProofdClient::GetClientID: size: "<<fClients.size());

   // We are done
   return ic;
}

//__________________________________________________________________________
int XrdProofdClient::CreateUNIXSock(XrdSysError *edest,
                                    const char *tmpdir, bool changeown)
{
   // Create UNIX socket for internal connections

   TRACE(ACT, "CreateUNIXSock: enter");

#if 0
   // Make sure we do not have already a socket
   if (fUNIXSock && fUNIXSockPath) {
       TRACE(DBG,"CreateUNIXSock: UNIX socket exists already! (" <<
             fUNIXSockPath<<")");
       return 0;
   }

   // Make sure we do not have inconsistencies
   if (fUNIXSock || fUNIXSockPath) {
       TRACE(XERR,"CreateUNIXSock: inconsistent values: corruption? (sock: " <<
                 fUNIXSock<<", path: "<< fUNIXSockPath);
       return -1;
   }
#else

   // Make sure we do not have inconsistencies
   if (fUNIXSock || !fUNIXSockPath) {
       TRACE(XERR,"CreateUNIXSock: inconsistent values: corruption? (sock: " <<
                 fUNIXSock<<", path: "<< fUNIXSockPath);
       return -1;
   }
#endif

   // Inputs must make sense
   if (!edest || !tmpdir) {
       TRACE(XERR,"CreateUNIXSock: invalid inputs: edest: " <<
                 (int *)edest <<", tmpdir: "<< (int *)tmpdir);
       return -1;
   }

   // Create socket
   fUNIXSock = new XrdNet(edest);

   // Create path if needed
   int fd = 0;
   if (!fUNIXSockPath) {
      fUNIXSockPath = new char[strlen(tmpdir)+strlen("/xpdsock_XXXXXX")+2];
      sprintf(fUNIXSockPath,"%s/xpdsock_XXXXXX", tmpdir);
      if ((fd = mkstemp(fUNIXSockPath)) > -1)
         close(fd);
   }
   if (fd > -1) {
      if (fUNIXSock->Bind(fUNIXSockPath)) {
         TRACE(XERR,"CreateUNIXSock: warning:"
                   " problems binding to UNIX socket; path: " <<fUNIXSockPath);
         return -1;
      } else
         TRACE(DBG, "CreateUNIXSock: path for UNIX for socket is " <<fUNIXSockPath);
   } else {
      TRACE(XERR,"CreateUNIXSock: unable to generate unique"
            " path for UNIX socket; tried path " << fUNIXSockPath);
      return -1;
   }

   // Set ownership of the socket file to the client
   XrdSysPrivGuard pGuard((uid_t)0, (gid_t)0);
   if (XpdBadPGuard(pGuard, fUI.fUid) && changeown) {
      TRACE(XERR, "CreateUNIXSock: could not get privileges");
      return -1;
   }
   if (changeown) {
      if (chown(fUNIXSockPath, fUI.fUid, fUI.fGid) == -1) {
         TRACE(XERR, "CreateUNIXSock: cannot set user ownership"
                      " on UNIX socket (errno: "<<errno<<")");
         return -1;
      }
      // Make sure that it worked out
      struct stat st;
      if ((stat(fUNIXSockPath, &st) != 0) || 
            (int) st.st_uid != fUI.fUid || (int) st.st_gid != fUI.fGid) {
         TRACE(XERR, "CreateUNIXSock: problems setting user ownership"
                      " on UNIX socket");
         return -1;
      }
   }

   // We are done
   return 0;
}

//__________________________________________________________________________
void XrdProofdClient::SaveUNIXPath()
{
   // Save UNIX path in <SandBox>/.unixpath

   TRACE(ACT,"SaveUNIXPath: enter: saved? "<<fUNIXSockSaved);

   // Make sure we do not have already a socket
   if (fUNIXSockSaved) {
      TRACE(DBG,"SaveUNIXPath: UNIX path saved already");
      return;
   }

   // Make sure we do not have already a socket
   if (!fUNIXSockPath) {
       TRACE(XERR,"SaveUNIXPath: UNIX path undefined!");
       return;
   }

   // File name
   XrdOucString fn = fSandbox.Dir();
   fn += "/.unixpath";

   // Open the file for appending
   FILE *fup = fopen(fn.c_str(), "a+");
   if (!fup) {
      TRACE(XERR, "SaveUNIXPath: cannot open file "<<fn<<
            " for appending (errno: "<<errno<<")");
      return;
   }

   // Lock the file
   lseek(fileno(fup), 0, SEEK_SET);
   if (lockf(fileno(fup), F_LOCK, 0) == -1) {
      TRACE(XERR, "SaveUNIXPath: cannot lock file "<<fn<<
            " (errno: "<<errno<<")");
      fclose(fup);
      return;
   }

   // Read content, if any
   char ln[1024], path[1024];
   int pid = -1;
   std::list<XrdOucString *> actln;
   while (fgets(ln, sizeof(ln), fup)) {
      // Get rid of '\n'
      if (ln[strlen(ln)-1] == '\n')
         ln[strlen(ln)-1] = '\0';
      // Skip empty or comment lines
      if (strlen(ln) <= 0 || ln[0] == '#')
         continue;
      // Get PID and path
      sscanf(ln, "%d %s", &pid, path);
      // Verify if still running
      int vrc = -1;
      if ((vrc = XrdProofdAux::VerifyProcessByID(pid, "xrootd")) != 0) {
         // Still there
         actln.push_back(new XrdOucString(ln));
      } else if (vrc == 0) {
         // Not running: remove the socket path
         TRACE(DBG, "SaveUNIXPath: unlinking socket path "<< path);
         if (unlink(path) != 0 && errno != ENOENT) {
            TRACE(XERR, "SaveUNIXPath: problems unlinking socket path "<< path<<
                    " (errno: "<<errno<<")");
         }
      }
   }

   // Truncate the file
   if (ftruncate(fileno(fup), 0) == -1) {
      TRACE(XERR, "SaveUNIXPath: cannot truncate file "<<fn<<
                 " (errno: "<<errno<<")");
      lseek(fileno(fup), 0, SEEK_SET);
      lockf(fileno(fup), F_ULOCK, 0);
      fclose(fup);
      return;
   }

   // If active sockets still exist, write out new composition
   if (actln.size() > 0) {
      std::list<XrdOucString *>::iterator i;
      for (i = actln.begin(); i != actln.end(); ++i) {
         fprintf(fup, "%s\n", (*i)->c_str());
         delete (*i);
      }
   }

   // Append the path and our process ID
   lseek(fileno(fup), 0, SEEK_END);
   fprintf(fup, "%d %s\n", getppid(), fUNIXSockPath);

   // Unlock the file
   lseek(fileno(fup), 0, SEEK_SET);
   if (lockf(fileno(fup), F_ULOCK, 0) == -1)
      TRACE(XERR, "SaveUNIXPath: cannot unlock file "<<fn<<
                 " (errno: "<<errno<<")");

   // Close the file
   fclose(fup);

   // Path saved
   fUNIXSockSaved = 1;
}

#if 0
//__________________________________________________________________________
int XrdProofdClient::GetFreeServID()
{
   // Get next free server ID. If none is found, increase the vector size
   // and get the first new one

   TRACE(ACT,"GetFreeServID: enter");

   XrdSysMutexHelper mh(fMutex);

   TRACE(DBG,"GetFreeServID: size = "<<fProofServs.size()<<
              "; capacity = "<<fProofServs.capacity());
   int ic = 0;
   // Search for free places in the existing vector
   for (ic = 0; ic < (int)fProofServs.size() ; ic++) {
      if (fProofServs[ic] && !(fProofServs[ic]->IsValid())) {
         fProofServs[ic]->SetValid();
         return ic;
      }
   }

   // We may need to resize (double it)
   if (ic >= (int)fProofServs.capacity()) {
      int newsz = 2 * fProofServs.capacity();
      fProofServs.reserve(newsz);
   }

   // Allocate new element
   fProofServs.push_back(new XrdProofServProxy());

   TRACE(DBG,"GetFreeServID: size = "<<fProofServs.size()<<
              "; new capacity = "<<fProofServs.capacity()<<"; ic = "<<ic);

   // We are done
   return ic;
}
#else

//__________________________________________________________________________
XrdProofServProxy *XrdProofdClient::GetFreeServObj()
{
   // Get next free server ID. If none is found, increase the vector size
   // and get the first new one

   TRACE(ACT,"GetFreeServObj: enter");

   XrdSysMutexHelper mh(fMutex);

   TRACE(DBG,"GetFreeServObj: size = "<<fProofServs.size()<<
              "; capacity = "<<fProofServs.capacity());
   int ic = 0;
   // Search for free places in the existing vector
   for (ic = 0; ic < (int)fProofServs.size() ; ic++) {
      if (fProofServs[ic] && !(fProofServs[ic]->IsValid())) {
         fProofServs[ic]->SetValid();
         break;
      }
   }

   // If we did not find it, we resize the vector (double it)
   if (ic >= (int)fProofServs.capacity()) {
      int newsz = 2 * fProofServs.capacity();
      fProofServs.reserve(newsz);

      TRACE(DBG,"GetFreeServID: new capacity = "<<fProofServs.capacity());
   }

   // Allocate new element
   fProofServs.push_back(new XrdProofServProxy());

   XrdProofServProxy *xps = fProofServs[ic];
   xps->SetValid();
   xps->SetID(ic);

   TRACE(DBG,"GetFreeServID: size = "<<fProofServs.size()<<"; ic = "<<ic);

   // We are done
   return xps;
}


#endif

//______________________________________________________________________________
XrdProofServProxy *XrdProofdClient::GetServer(int psid)
{
   // Get from the vector server instance with id psid

   TRACE(ACT,"GetServer: enter: psid: " << psid);

   XrdSysMutexHelper mh(fMutex);

   XrdProofServProxy *xps = 0;
   std::vector<XrdProofServProxy *>::iterator ip;
   for (ip = fProofServs.begin(); ip != fProofServs.end(); ++ip) {
      xps = *ip;
      if (xps && xps->Match(psid))
         break;
      xps = 0;
   }
   // Done
   return xps;
}

//______________________________________________________________________________
XrdProofServProxy *XrdProofdClient::GetServer(XrdProofdProtocol *p)
{
   // Get server instance connected via 'p'

   TRACE(ACT,"GetServer: enter: p: " << p);

   XrdSysMutexHelper mh(fMutex);

   XrdProofServProxy *xps = 0;
   std::vector<XrdProofServProxy *>::iterator ip;
   for (ip = fProofServs.begin(); ip != fProofServs.end(); ++ip) {
      xps = (*ip);
      if (xps->Link() == p->Link())
         break;
      xps = 0;
   }
   // Done
   return xps;
}

//______________________________________________________________________________
XrdProofServProxy *XrdProofdClient::GetProofServ(int psid)
{
   // Get from the vector server instance with ID psid

   if (psid > -1 && psid < (int) fProofServs.size())
      return fProofServs.at(psid);
   // Done
   return (XrdProofServProxy *)0;
}

//______________________________________________________________________________
void XrdProofdClient::EraseServer(int psid)
{
   // Erase server with id psid from the list

   TRACE(ACT,"EraseServer: enter: psid: " << psid);

   XrdSysMutexHelper mh(fMutex);

   XrdProofServProxy *xps = 0;
   std::vector<XrdProofServProxy *>::iterator ip;
   for (ip = fProofServs.begin(); ip != fProofServs.end(); ++ip) {
      xps = *ip;
      if (xps && xps->Match(psid)) {
         fProofServs.erase(ip);
         break;
      }
   }
}

//______________________________________________________________________________
void XrdProofdClient::DisconnectFromProofServ(XrdProofdProtocol *p)
{
   // Disconnect instance 'p' from the attached proofservs

   TRACE(ACT,"DisconnectFromProofServ: enter: p: " << p);

   XrdSysMutexHelper mh(fMutex);

   std::vector<XrdProofServProxy *>::iterator ip;
   for (ip = fProofServs.begin(); ip != fProofServs.end(); ++ip)
      (*ip)->FreeClientID(p);

   // Done
   return;
}

//______________________________________________________________________________
int XrdProofdClient::ResetClientSlot(XrdProofdProtocol *p)
{
   // Reset slot reserved to instance 'p'

   TRACE(ACT,"ResetClientSlot: enter: p: " << p);

   XrdSysMutexHelper mh(fMutex);

   int rc = -1;
   int ic = 0;
   for (ic = 0; ic < (int) fClients.size(); ic++) {
      if (fClients.at(ic) == p) {
         fClients[ic] = 0;
         rc = 0;
      }
   }

   // Done
   return rc;
}

//______________________________________________________________________________
void XrdProofdClient::Broadcast(const char *msg,  bool closelink)
{
   // Broadcast message 'msg' to the connected clients

   int len = 0;
   if (msg && (len = strlen(msg)) > 0) {

      // Notify the attached clients
      int ic = 0;
      XrdProofdProtocol *p = 0;
      for (ic = 0; ic < (int) fClients.size(); ic++) {
         if ((p = fClients.at(ic)) && p->ConnType() == kXPD_ClientMaster) {
#if 0
            unsigned short sid;
            p->Response()->GetSID(sid);
            p->Response()->Set(fRefSid);
            p->Response()->Send(kXR_attn, kXPD_srvmsg, (char *) msg, len);
            p->Response()->Set(sid);
#else
            XrdProofdResponse *response = p ? p->Response(fRefSid) : 0;
            if (response)
               response->Send(kXR_attn, kXPD_srvmsg, (char *) msg, len);
#endif
            if (closelink)
               // Close the link, so that the associated protocol instance can be recycled
               p->Link()->Close();
         }
      }
   }
}

#if 0
//______________________________________________________________________________
int XrdProofdClient::StopShutdownTimers(XrdOucString &emsg)
{
   // Stop shutdown timers on sessions, if any

   int rc = 0;
   emsg = "StopShutdownTimers: could not stop shutdown timer in proofsrv";
   if (fProofServs.size() > 0) {
      XrdProofServProxy *psrv = 0;
      int is = 0;
      for (is = 0; is < (int) fProofServs.size(); is++) {
         if ((psrv = fProofServs.at(is)) &&
              psrv->IsValid() && (psrv->SrvType() == kXPD_TopMaster) &&
              psrv->IsShutdown()) {
            if (psrv->SetShutdownTimer(-1, 0, 0) != 0) {
               emsg += ": ";
               emsg += psrv->SrvPID();
               emsg += "; status: ";
               emsg += psrv->StatusAsString();
               rc = -1;
            }
         }
      }
   }
   if (!rc)
      emsg = "";
   // Done
   return rc;
}
#endif

//______________________________________________________________________________
XrdOucString XrdProofdClient::ExportSessions()
{
   // Return a string describing the existing sessions

   XrdOucString out;

   XrdProofServProxy *xps = 0;
   int ns = 0;
   std::vector<XrdProofServProxy *>::iterator ip;
   for (ip = fProofServs.begin(); ip != fProofServs.end(); ++ip)
      if ((xps = *ip) && xps->IsValid() && (xps->SrvType() == kXPD_TopMaster))
         ns++;
   TRACE(HDBG, "Client::ExportSessions: found: " << ns << " sessions");

   // Fill info
   out += ns;
   char buf[kXPROOFSRVTAGMAX+kXPROOFSRVALIASMAX+30];
   for (ip = fProofServs.begin(); ip != fProofServs.end(); ++ip) {
      if ((xps = *ip) && xps->IsValid() && (xps->SrvType() == kXPD_TopMaster)) {
         sprintf(buf," | %d %s %s %d %d", xps->ID(), xps->Tag(), xps->Alias(),
                                          xps->Status(), xps->GetNClients());
         out += buf;
         strcpy(buf,"");
      }
   }

   // Over
   return out;
}

//______________________________________________________________________________
void XrdProofdClient::TerminateSessions(bool kill, int srvtype,
                                        XrdProofServProxy *ref,
                                        const char *msg, std::list<int> &sigpid)
{
   // Terminate client sessions; IDs of signalled processes are added to sigpid.

   // Loop over client sessions and terminated them
   int is = 0;
   XrdProofServProxy *s = 0;
   for (is = 0; is < (int) fProofServs.size(); is++) {
      if ((s = fProofServs.at(is)) && s->IsValid() && (!ref || ref == s) &&
          (s->SrvType() == srvtype || (srvtype == kXPD_AnyServer))) {
         TRACE(HDBG, "Client::TerminateSessions: terminating " << s->SrvPID());

         if (msg && strlen(msg) > 0)
            // Tell other attached clients, if any, that this session is gone
            Broadcast(msg);

         bool siged = 1;
         if (s->TerminateProofServ() < 0)
            if (XrdProofdAux::KillProcess(s->SrvPID(), kill, fUI, fChangeOwn) != 0)
               siged = 0;
         // Add to the list
         if (siged) {
            sigpid.push_back(s->SrvPID());
            // Record this session in the sandbox as old session
            XrdOucString tag = "-";
            tag += s->SrvPID();
            if (fSandbox.GuessTag(tag, 1) == 0)
               fSandbox.RemoveSession(tag.c_str());
         }
         // Reset session proxy
         s->Reset();
      }
   }
}


