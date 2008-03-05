// @(#)root/proofd:$Id$
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
// XrdProofdPriorityMgr                                                 //
//                                                                      //
// Author: G. Ganis, CERN, 2007                                         //
//                                                                      //
// Class managing session priorities.                                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
#include "XrdProofdPlatform.h"

#include "XrdOuc/XrdOucStream.hh"
#include "XrdSys/XrdSysPriv.hh"

#include "XrdProofdAux.h"
#include "XrdProofdManager.h"
#include "XrdProofdPriorityMgr.h"
#include "XrdProofGroup.h"

// Tracing utilities
#include "XrdProofdTrace.h"
static const char *gTraceID = "";
extern XrdOucTrace *XrdProofdTrace;
#define TRACEID gTraceID

// Aux structures for scan through operations
typedef struct {
   XrdProofGroupMgr *fGroupMgr;
   std::list<XrdProofdSessionEntry *> *fSortedList;
} XpdCreateActiveList_t;

//--------------------------------------------------------------------------
//
// XrdProofdPriorityPoller
//
// Function run in separate thread watching changes in session status
// frequency
//
//--------------------------------------------------------------------------
void *XrdProofdPriorityPoller(void *p)
{
   // This is an endless loop to periodically check the system

   XrdProofdPriorityMgr *mgr = (XrdProofdPriorityMgr *)p;
   if (!(mgr)) {
      TRACE(REQ, "XrdProofdPriorityPoller: undefined manager: cannot start");
      return (void *)0;
   }

   // Read protocol
   struct pollfd fds_r;
   fds_r.fd = mgr->ReadFd();
   fds_r.events = POLLIN;

   while(1) {
      // We wait for processes to communicate a session status change
      int pollRet = 0;
      while ((pollRet = poll(&fds_r, 1, -1)) < 0 &&
            (errno == EINTR)) { }
      if (pollRet > 0) {
         // Read message type
         int type = 0;
         if (read(mgr->ReadFd(), &type, sizeof(type)) != sizeof(type)) {
            XPDERR("XrdProofdPriorityPoller: problems receiving message type");
            continue;
         }
         // Read message
         char *buf = XrdProofdAux::ReadMsg(mgr->ReadFd());
         if (buf) {
            char user[64], group[64];
            // Parse the buffer
            if (type == 0) {
               // Change session status
               int opt = 0, pid= -1;
               sscanf(buf, "%d %s %s %d", &opt, user, group, &pid);
               if (opt < 0)
                  // Remove
                  mgr->RemoveSession(pid);
               else if (opt > 0)
                  // Add
                  mgr->AddSession(user, group, pid);
            } else if (type == 1) {
               // Change group priority
               int prio = -1;
               sscanf(buf, "%s %d", group, &prio);
               mgr->SetGroupPriority(group, prio);
            } else {
               XPDERR("XrdProofdPriorityPoller: unknown message type: "<< type);
            }
            // Communicate new priorities
            if (mgr->SetNiceValues() != 0) {
               XPDERR("XrdProofdPriorityPoller: problem setting nice values ");
            }
         }
         delete [] buf;
      }
   }

   // Should never come here
   return (void *)0;
}

//______________________________________________________________________________
XrdProofdPriorityMgr::XrdProofdPriorityMgr(XrdProofdManager *mgr,
                                           XrdProtocol_Config *pi, XrdSysError *e)
                    : XrdProofdConfig(pi->ConfigFN, e)
{
   // Constructor

   fMgr = mgr;
   fSchedOpt = kXPD_sched_off;

   // Init pipe for the poller
   if (pipe(fPipe) != 0) {
      XPDERR("XrdProofdPriorityMgr: unable to generate pipe for"
            " the priority poller");
      return;
   }

   // Configuration directives
   RegisterDirectives();
}

//__________________________________________________________________________
static int DumpPriorityChanges(const char *, XrdProofdPriority *p, void *s)
{
   // Reset the priority on entries

   XrdSysError *e = (XrdSysError *)s;

   if (p && e) {
      XrdOucString msg("priority will be changed by ");
      msg += p->fDeltaPriority;
      msg += " for user(s): ";
      msg += p->fUser;
      e->Say(0, "xpd: Config: PriorityMgr: ", msg.c_str());
      // Check next
      return 0;
   }

   // Not enough info: stop
   return 1;
}

//__________________________________________________________________________
int XrdProofdPriorityMgr::Config(bool rcf)
{
   // Run configuration and parse the entered config directives.
   // Return 0 on success, -1 on error

   // Run first the configurator
   if (XrdProofdConfig::Config(rcf) != 0) {
      fEDest->Say(0, "xpd: Config: PriorityMgr: problems parsing file ");
      return -1;
   }

   XrdOucString msg;
   msg = (rcf) ? "xpd: Config: PriorityMgr: re-configuring"
               : "xpd: Config: PriorityMgr: configuring";
   fEDest->Say(0, msg.c_str());

   // Notify change priority rules
   if (fPriorities.Num() > 0) {
      fPriorities.Apply(DumpPriorityChanges, (void *)fEDest);
   } else {
      fEDest->Say(0, "xpd: Config: PriorityMgr: no priority changes requested");
   }

   // Scheduling option
   if (fMgr->GroupsMgr() && fMgr->GroupsMgr()->Num() > 1 && fSchedOpt != kXPD_sched_off) {
      msg = "xpd: Config: PriorityMgr: worker sched based on: ";
      msg += (fSchedOpt == kXPD_sched_central) ? "central" : "local";
      msg += " priorities";
      fEDest->Say(0, msg.c_str());
   }

   // Start poller thread
   pthread_t tid;
   if (XrdSysThread::Run(&tid, XrdProofdPriorityPoller,
                           (void *)this, 0, "PriorityMgr poller thread") != 0) {
      fEDest->Say(0, "xpd: Config: PriorityMgr: could not start poller thread");
      return 0;
   }
   fEDest->Say(0, "xpd: Config: PriorityMgr: poller thread started");

   // Done
   return 0;
}

//__________________________________________________________________________
void XrdProofdPriorityMgr::RegisterDirectives()
{
   // Register directives for configuration

   Register("schedopt", new XrdProofdDirective("schedopt", this, &DoDirectiveClass));
   Register("priority", new XrdProofdDirective("priority", this, &DoDirectiveClass));
}

//______________________________________________________________________________
int XrdProofdPriorityMgr::DoDirective(XrdProofdDirective *d,
                                  char *val, XrdOucStream *cfg, bool rcf)
{
   // Update the priorities of the active sessions.

   if (!d)
      // undefined inputs
      return -1;

   if (d->fName == "priority") {
      return DoDirectivePriority(val, cfg, rcf);
   } else if (d->fName == "schedopt") {
      return DoDirectiveSchedOpt(val, cfg, rcf);
   }
   TRACE(XERR,"DoDirective: unknown directive: "<<d->fName);
   return -1;
}

//______________________________________________________________________________
void XrdProofdPriorityMgr::SetGroupPriority(const char *grp, int priority)
{
   // Change group priority. Used when a master pushes a priority to a worker.

   XrdProofGroup *g = fMgr->GroupsMgr()->GetGroup(grp);
   if (g)
      g->SetPriority((float)priority);

   // Make sure scheduling is ON
   SetSchedOpt(kXPD_sched_central);

   // Done
   return;
}

//__________________________________________________________________________
static int ResetEntryPriority(const char *, XrdProofdSessionEntry *e, void *)
{
   // Reset the priority on entries

   if (e) {
      e->SetPriority();
      // Check next
      return 0;
   }

   // Not enough info: stop
   return 1;
}

//__________________________________________________________________________
static int CreateActiveList(const char *, XrdProofdSessionEntry *e, void *s)
{
   // Run thorugh entries to create the sorted list of active entries

   XpdCreateActiveList_t *cal = (XpdCreateActiveList_t *)s;

   XrdOucString emsg;
   if (e && cal) {
      XrdProofGroupMgr *gm = cal->fGroupMgr;
      std::list<XrdProofdSessionEntry *> *sorted = cal->fSortedList;
      if (gm) {
         XrdProofGroup *g = gm->GetGroup(e->fGroup.c_str());
         if (g) {
            float ef = g->FracEff() / g->Active();
            int nsrv = g->Active(e->fUser.c_str());
            if (nsrv > 0) {
               ef /= nsrv;
               e->fFracEff = ef;
               bool pushed = 0;
               std::list<XrdProofdSessionEntry *>::iterator ssvi;
               for (ssvi = sorted->begin() ; ssvi != sorted->end(); ssvi++) {
                  if (ef >= (*ssvi)->fFracEff) {
                     sorted->insert(ssvi, e);
                     pushed = 1;
                     break;
                  }
               }
               if (!pushed)
                  sorted->push_back(e);
               // Go to next
               return 0;

            } else {
               emsg = "no srv sessions for active client";
            }
         } else {
            emsg = "group not found: "; emsg += e->fGroup.c_str();
         }
      } else {
         emsg = "group manager undefined";
      }
   } else {
      emsg = "input structure or entry undefined";
   }

   // Some problem
   TRACE(XERR,"CreateActiveList: "<< (e ? e->fUser : "---") << ": Protocol error: "<<emsg);
   return 1;
}

//______________________________________________________________________________
int XrdProofdPriorityMgr::SetNiceValues(int opt)
{
   // Recalculate nice values taking into account all active users
   // and their priorities.
   // The type of sessions considered depend on 'opt':
   //    0          all active sessions
   //    1          master sessions
   //    2          worker sessionsg21
   // Return 0 on success, -1 otherwise.

   TRACE(SCHED,"---------------- SetNiceValues ("<<opt<<") ---------------------------");

   if (!fMgr->GroupsMgr() || fMgr->GroupsMgr()->Num() <= 1 || !IsSchedOn())
      // Nothing to do
      return 0;

   // At least two active session
   int nact = fSessions.Num();
   TRACE(SCHED,"enter: "<< fMgr->GroupsMgr()->Num()<<" groups, " << nact<<" active sessions");
   if (nact <= 1) {
      // Restore default values
      if (nact == 1)
         fSessions.Apply(ResetEntryPriority, 0);
      // Nothing else to do
      TRACE(SCHED,"------------ End of SetNiceValues ------------------------");
      return 0;
   }

   XrdSysMutexHelper mtxh(&fMutex);

   // Determine which groups are active and their effective fractions
   int rc = 0;
   if ((rc = fMgr->GroupsMgr()->SetEffectiveFractions(IsSchedOn())) != 0) {
      // Failure
      TRACE(XERR,"SetNiceValues: failure from SetEffectiveFractions");
      TRACE(SCHED,"------------ End of SetNiceValues ------------------------");
      return -1;
   }

   // Now create a list of active sessions sorted by decreasing effective fraction
   TRACE(SCHED,"--> creating a list of active sessions sorted by decreasing effective fraction ");
   std::list<XrdProofdSessionEntry *> sorted;
   XpdCreateActiveList_t cal = { fMgr->GroupsMgr(), &sorted };
   fSessions.Apply(CreateActiveList, (void *)&cal);

   // Notify
   int i = 0;
   std::list<XrdProofdSessionEntry *>::iterator ssvi;
   if (TRACING(SCHED) && TRACING(HDBG)) {
      for (ssvi = sorted.begin() ; ssvi != sorted.end(); ssvi++)
         XPDPRT("SetNiceValues: "<< i++ <<" eff: "<< (*ssvi)->fFracEff);
   }

   TRACE(SCHED,"SetNiceValues: calculating nice values");

   // The first has the max priority
   ssvi = sorted.begin();
   float xmax = (*ssvi)->fFracEff;
   if (xmax <= 0.) {
      TRACE(XERR,"SetNiceValues: negative or null max effective fraction: "<<xmax);
      return -1;
   }
   // This is for Unix
   int nice = 20 - fPriorityMax;
   (*ssvi)->SetPriority(nice);
   // The others have priorities rescaled wrt their effective fractions
   ssvi++;
   while (ssvi != sorted.end()) {
      int xpri = (int) ((*ssvi)->fFracEff / xmax * (fPriorityMax - fPriorityMin))
                                                 + fPriorityMin;
      nice = 20 - xpri;
      TRACE(SCHED, "    --> nice value for client "<< (*ssvi)->fUser<<" is "<<nice);
      (*ssvi)->SetPriority(nice);
      ssvi++;
   }
   TRACE(SCHED,"------------ End of SetNiceValues ------------------------");

   // Done
   return 0;
}

//______________________________________________________________________________
int XrdProofdPriorityMgr::DoDirectivePriority(char *val, XrdOucStream *cfg, bool)
{
   // Process 'priority' directive

   if (!val || !cfg)
      // undefined inputs
      return -1;

   // Priority change directive: get delta_priority
   int dp = strtol(val,0,10);
   XrdProofdPriority *p = new XrdProofdPriority("*", dp);
   // Check if an 'if' condition is present
   if ((val = cfg->GetToken()) && !strncmp(val,"if",2)) {
      if ((val = cfg->GetToken()) && val[0]) {
         p->fUser = val;
      }
   }
   // Add to the list
   fPriorities.Rep(p->fUser.c_str(), p);
   return 0;
}

//______________________________________________________________________________
int XrdProofdPriorityMgr::DoDirectiveSchedOpt(char *val, XrdOucStream *cfg, bool)
{
   // Process 'schedopt' directive

   if (!val || !cfg)
      // undefined inputs
      return -1;

   float of = -1.;
   int pmin = -1;
   int pmax = -1;
   int opt = -1;
   // Defines scheduling options
   while (val && val[0]) {
      XrdOucString o = val;
      if (o.beginswith("overall:")) {
         // The overall inflating factor
         o.replace("overall:","");
         sscanf(o.c_str(), "%f", &of);
      } else if (o.beginswith("min:")) {
         // The overall inflating factor
         o.replace("min:","");
         sscanf(o.c_str(), "%d", &pmin);
      } else if (o.beginswith("max:")) {
         // The overall inflating factor
         o.replace("max:","");
         sscanf(o.c_str(), "%d", &pmax);
      } else {
         if (o == "central")
            opt = kXPD_sched_central;
         else if (o == "local")
            opt = kXPD_sched_local;
      }
      // Check deprecated 'if' directive
      if (fMgr->Host() && cfg)
         if (XrdProofdAux::CheckIf(cfg, fMgr->Host()) == 0)
            return 0;
      // Next
      val = cfg->GetToken();
   }

   // Set the values (we need to do it here to avoid setting wrong values
   // when a non-matching 'if' condition is found)
   if (of > -1.)
      fOverallInflate = (of >= 1.) ? of : fOverallInflate;
   if (pmin > -1)
      fPriorityMin = (pmin >= 1 && pmin <= 40) ? pmin : fPriorityMin;
   if (pmax > -1)
      fPriorityMax = (pmax >= 1 && pmax <= 40) ? pmax : fPriorityMax;
   if (opt > -1)
      fSchedOpt = opt;

   // Make sure that min is <= max
   if (fPriorityMin > fPriorityMax) {
      TRACE(XERR, "DoDirectiveSchedOpt: inconsistent value for fPriorityMin (> fPriorityMax) ["<<
                  fPriorityMin << ", "<<fPriorityMax<<"] - correcting");
      fPriorityMin = fPriorityMax;
   }

   return 0;
}

//______________________________________________________________________________
int XrdProofdPriorityMgr::RemoveSession(int pid)
{
   // Remove from the active list the session with ID pid.
   // Return -ENOENT if not found, or 0.

   XrdOucString key; key += pid;
   return fSessions.Del(key.c_str());
}

//______________________________________________________________________________
int XrdProofdPriorityMgr::AddSession(const char *u, const char *g, int pid)
{
   // Add to the active list a session with ID pid. Check for duplications.
   // Returns 1 if the entry existed already and it has been replaced; or 0.

   int rc = 0;
   XrdOucString key; key += pid;
   XrdProofdSessionEntry *oldent = fSessions.Find(key.c_str());
   if (oldent) {
      rc = 1;
      fSessions.Rep(key.c_str(), new XrdProofdSessionEntry(u, g, pid));
   } else {
      fSessions.Add(key.c_str(), new XrdProofdSessionEntry(u, g, pid));
   }

   // Done
   return rc;
}

//__________________________________________________________________________
int XrdProofdPriorityMgr::SetProcessPriority(int pid, const char *user, int &dp)
{
   // Change priority of process pid belonging to user, if needed.
   // Return 0 on success, -errno in case of error

   // Change child process priority, if required
   if (fPriorities.Num() > 0) {
      XrdProofdPriority *pu = fPriorities.Find(user);
      if (pu) {
         dp = pu->fDeltaPriority;
         // Change the priority
         errno = 0;
         int priority = XPPM_NOPRIORITY;
         if ((priority = getpriority(PRIO_PROCESS, pid)) == -1 && errno != 0) {
            TRACE(XERR, "SetProcessPriority: getpriority: errno: " << errno);
            return -errno;
         }
         // Set the priority
         int newp = priority + dp;
         XrdProofUI ui;
         XrdProofdAux::GetUserInfo(geteuid(), ui);
         XrdSysPrivGuard pGuard((uid_t)0, (gid_t)0);
         if (XpdBadPGuard(pGuard, ui.fUid)) {
            TRACE(XERR, "SetProcessPriority: could not get privileges");
            return -1;
         }
         TRACE(SCHED, "SetProcessPriority: got privileges ");
         errno = 0;
         if (setpriority(PRIO_PROCESS, pid, newp) != 0) {
            TRACE(XERR, "SetProcessPriority: setpriority: errno: " << errno);
            return ((errno != 0) ? -errno : -1);
         }
         if ((getpriority(PRIO_PROCESS, pid)) != newp && errno != 0) {
            TRACE(XERR, "SetProcessPriority: did not succeed: errno: " << errno);
            return -errno;
         }
      }
   }

   // We are done
   return 0;
}

//
// Small class to describe an active session
//
//______________________________________________________________________________
XrdProofdSessionEntry::XrdProofdSessionEntry(const char *u, const char *g, int pid)
                     : fUser(u), fGroup(g), fPid(pid)
{
   // Constructor

   fPriority = XPPM_NOPRIORITY;
   fDefaultPriority = XPPM_NOPRIORITY;
   errno = 0;
   int prio = getpriority(PRIO_PROCESS, pid);
   if (errno != 0) {
      TRACE(XERR, "XrdProofdSessionEntry:"
                  " getpriority: errno: " << errno);
      return;
   }
   fDefaultPriority = prio;
}

//______________________________________________________________________________
XrdProofdSessionEntry::~XrdProofdSessionEntry()
{
   // Destructor

   SetPriority(fDefaultPriority);
}

//______________________________________________________________________________
int XrdProofdSessionEntry::SetPriority(int priority)
{
   // Change process priority

   if (priority != XPPM_NOPRIORITY)
      priority = fDefaultPriority;

   if (priority != fPriority) {
      // Set priority to the default value
      XrdProofUI ui;
      XrdProofdAux::GetUserInfo(geteuid(), ui);
      XrdSysPrivGuard pGuard((uid_t)0, (gid_t)0);
      if (XpdBadPGuard(pGuard, ui.fUid)) {
         TRACE(XERR, "SetPriority: could not get privileges");
         return -1;
      }
      errno = 0;
      if (setpriority(PRIO_PROCESS, fPid, priority) != 0) {
         TRACE(XERR, "SetPriority:"
                     " setpriority: errno: " << errno);
         return -1;
      }
      fPriority = priority;
   }

   // Done
   return 0;
}
