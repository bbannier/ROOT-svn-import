// @(#)root/proofx:$Id$
// Author: Gerardo Ganis Apr 2008

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofMgrLite                                                        //
//                                                                      //
// Basic functionality implementtaion in the case of Lite sessions      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <errno.h>
#ifdef WIN32
#include <io.h>
#endif

#include "TProofMgrLite.h"

#include "Getline.h"
#include "Riostream.h"
#include "TEnv.h"
#include "TError.h"
#include "TFile.h"
#include "TObjString.h"
#include "TProofLite.h"
#include "TProofLog.h"
#include "TROOT.h"
#include "TRegexp.h"
#include "TSortedList.h"

#ifdef WIN32
// ShowFile options
const UInt_t kShowLineNum = 0x1; //   show the line numbers
const UInt_t kGrep        = 0x2; //   show only lines with 'pattern'
const UInt_t kUnGrep      = 0x4; //   show only lines without 'pattern'
#endif

ClassImp(TProofMgrLite)

//______________________________________________________________________________
TProofMgrLite::TProofMgrLite(const char *url, Int_t dbg, const char *alias)
              : TProofMgr(url, dbg, alias)
{
   // Create a PROOF manager for the Lite environment.

   // Set the correct servert type
   fServType = kProofLite;

   // Fill in the sessiondir
   TString sandbox = gEnv->GetValue("Proof.Sandbox", "");
   if (sandbox.IsNull()) {
      sandbox.Form("~/%s", kPROOF_WorkDir);
   }
   gSystem->ExpandPathName(sandbox);
   sandbox = gSystem->UnixPathName(sandbox);
   // Subpath for this session in the sandbox (<sandbox>/path-to-working-dir)
   fSessionDir = gSystem->UnixPathName(gSystem->WorkingDirectory());
   fSessionDir.ReplaceAll(gSystem->UnixPathName(gSystem->HomeDirectory()),"");
   fSessionDir.ReplaceAll(":","-");
   fSessionDir.ReplaceAll("/","-");
   fSessionDir.Replace(0,1,"/",1);
   fSessionDir.Insert(0, sandbox.Data());
}

//______________________________________________________________________________
TProof *TProofMgrLite::CreateSession(const char *,
                                     const char *, Int_t loglevel)
{
   // Create a new session

   Int_t nwrk = TProofLite::GetNumberOfWorkers(fUrl.GetOptions());

   // Check if we have already a running session
   if (gProof && gProof->IsValid() && gProof->IsLite()) {
      if (nwrk > 0 && gProof->GetParallel() != nwrk) {
         delete gProof;
         gProof = 0;
      } else {
         // We have already a running session
         return gProof;
      }
   }

   // Create the instance
   TString u = (strlen(fUrl.GetOptions()) > 0) ? Form("lite/?%s", fUrl.GetOptions())
                                               : "lite";
   TProof *p = new TProofLite(u, 0, 0, loglevel, 0, this);

   if (p && p->IsValid()) {

      // Save record about this session
      Int_t ns = 1;
      if (fSessions) {
         // To avoid ambiguities in case of removal of some elements
         if (fSessions->Last())
            ns = ((TProofDesc *)(fSessions->Last()))->GetLocalId() + 1;
      } else {
         // Create the list
         fSessions = new TList;
      }

      // Create the description class
      Int_t st = (p->IsIdle()) ? TProofDesc::kIdle : TProofDesc::kRunning ;
      TProofDesc *d =
         new TProofDesc(p->GetName(), p->GetTitle(), p->GetUrl(),
                               ns, p->GetSessionID(), st, p);
      fSessions->Add(d);

   } else {
      // Session creation failed
      Error("CreateSession", "creating PROOF session");
      SafeDelete(p);
   }

   // We are done
   return p;
}

//_____________________________________________________________________________
TProofLog *TProofMgrLite::GetSessionLogs(Int_t isess,
                                         const char *stag, const char *pattern)
{
   // Get logs or log tails from last session associated with this manager
   // instance.
   // The arguments allow to specify a session different from the last one:
   //      isess   specifies a position relative to the last one, i.e. 1
   //              for the next to last session; the absolute value is taken
   //              so -1 and 1 are equivalent.
   //      stag    specifies the unique tag of the wanted session
   // The special value stag = "NR" allows to just initialize the TProofLog
   // object w/o retrieving the files; this may be useful when the number
   // of workers is large and only a subset of logs is required.
   // If 'stag' is specified 'isess' is ignored (unless stag = "NR").
   // If 'pattern' is specified only the lines containing it are retrieved
   // (remote grep functionality); to filter out a pattern 'pat' use
   // pattern = "-v pat".
   // Returns a TProofLog object (to be deleted by the caller) on success,
   // 0 if something wrong happened.

   TProofLog *pl = 0;

   // The absolute value of isess counts
   isess = (isess < 0) ? -isess : isess;

   // Special option in stag
   bool retrieve = 1;
   TString tag(stag);
   if (tag == "NR") {
      retrieve = 0;
      tag = "";
   }

   // The working dir
   TString sandbox(gSystem->UnixPathName(gSystem->WorkingDirectory()));
   sandbox.ReplaceAll(gSystem->UnixPathName(gSystem->HomeDirectory()),"");
   sandbox.ReplaceAll("/","-");
   sandbox.Replace(0,1,"/",1);
   if (strlen(gEnv->GetValue("Proof.Sandbox", "")) > 0) {
      sandbox.Insert(0, gSystem->UnixPathName(gEnv->GetValue("Proof.Sandbox", "")));
   } else {
      TString sb;
      sb.Form("~/%s", kPROOF_WorkDir);
      sandbox.Insert(0, sb.Data());
   }
   gSystem->ExpandPathName(sandbox);
   sandbox = gSystem->UnixPathName(sandbox);

   TString sessiondir;
   if (tag.Length() > 0) {
      sessiondir.Form("%s/session-%s", sandbox.Data(), tag.Data());
      if (gSystem->AccessPathName(sessiondir, kReadPermission)) {
         Error("GetSessionLogs", "information for session '%s' not available", tag.Data());
         return (TProofLog *)0;
      }
   } else {
      // Get the list of available dirs
      TSortedList *olddirs = new TSortedList(kFALSE);
      void *dirp = gSystem->OpenDirectory(sandbox);
      if (dirp) {
         const char *e = 0;
         while ((e = gSystem->GetDirEntry(dirp))) {
            if (!strncmp(e, "session-", 8)) {
               TString d(e);
               Int_t i = d.Last('-');
               if (i != kNPOS) d.Remove(i);
               i = d.Last('-');
               if (i != kNPOS) d.Remove(0,i+1);
               TString path = Form("%s/%s", sandbox.Data(), e);
               olddirs->Add(new TNamed(d, path));
            }
         }
         gSystem->FreeDirectory(dirp);
      }

      // Check isess
      if (isess > olddirs->GetSize() - 1) {
         Warning("GetSessionLogs",
                 "session index out of range (%d): take oldest available session", isess);
         isess = olddirs->GetSize() - 1;
      }

      // Locate the session dir
      TNamed *n = (TNamed *) olddirs->First();
      while (isess-- > 0) {
         olddirs->Remove(n);
         delete n;
         n = (TNamed *) olddirs->First();
      }
      sessiondir = n->GetTitle();
      tag = gSystem->BaseName(sessiondir);
      tag.ReplaceAll("session-", "");

      // Cleanup
      olddirs->SetOwner();
      delete olddirs;
   }
   Info("GetSessionLogs", "analysing session dir %s", sessiondir.Data());

   // Create the instance now
   pl = new TProofLog(tag, "", this);

   void *dirp = gSystem->OpenDirectory(sessiondir);
   if (dirp) {
      TSortedList *logs = new TSortedList;
      const char *e = 0;
      while ((e = gSystem->GetDirEntry(dirp))) {
         TString fn(e);
#ifndef WIN32
         if (fn.EndsWith(".log") && fn.CountChar('-') > 2) {
#else
         if (fn.EndsWith(".log") && fn.CountChar('-') > 0) {
#endif
            TString ord, url;
            if (fn.BeginsWith("session-")) {
               ord = "-1";
            } else if (fn.BeginsWith("worker-")) {
               ord = fn;
               ord.ReplaceAll("worker-", "");
               Int_t id = ord.First('-');
               if (id != kNPOS) ord.Remove(id);
               ord.ReplaceAll(".log", "");
               ord.ReplaceAll("0.", "");
            }
            if (!ord.IsNull()) {
               url = Form("%s/%s", sessiondir.Data(), e);
               // Add to the list
               logs->Add(new TNamed(ord, url));
               // Notify
               if (gDebug > 1)
                  Info("GetSessionLogs", "ord: %s, url: %s", ord.Data(), url.Data());
            }
         }
      }
      gSystem->FreeDirectory(dirp);

      TIter nxl(logs);
      TNamed *n = 0;
      while ((n = (TNamed *) nxl())) {
         TString ord = Form("0.%s", n->GetName());
         if (ord == "0.-1") ord = "0";
         // Add to the list
         pl->Add(ord, n->GetTitle());
      }

      // Cleanup
      logs->SetOwner();
      delete logs;
   }

   // Retrieve the default part
   if (pl && retrieve) {
      if (pattern && strlen(pattern) > 0)
         pl->Retrieve("*", TProofLog::kGrep, 0, pattern);
      else
         pl->Retrieve();
   }

   // Done
   return pl;
}

//______________________________________________________________________________
TObjString *TProofMgrLite::ReadBuffer(const char *fin, Long64_t ofs, Int_t len)
{
   // Read 'len' bytes from offset 'ofs' of the local file 'fin'.
   // Returns a TObjString with the content or 0, in case of failure

   if (!fin || strlen(fin) <= 0) {
      Error("ReadBuffer", "undefined path!");
      return (TObjString *)0;
   }

   // Open the file
#ifdef WIN32
   TString fn = fin;
#else
   TString fn = TUrl(fin).GetFile();
#endif
   Int_t fd = open(fn.Data(), O_RDONLY);
   if (fd < 0) {
      Error("ReadBuffer", "problems opening file %s", fn.Data());
      return (TObjString *)0;
   }

   // Total size
   off_t start = 0, end = lseek(fd, (off_t) 0, SEEK_END);

   // Set the offset
   if (ofs > 0 && ofs < end) {
      start = lseek(fd, (off_t) ofs, SEEK_SET);
   } else {
      start = lseek(fd, (off_t) 0, SEEK_SET);
   }
   if (len > (end - start + 1) || len <= 0)
      len = end - start + 1;

   TString outbuf;
   const Int_t kMAXBUF = 32768;
   char buf[kMAXBUF];
   Int_t left = len;
   Int_t wanted = (left > kMAXBUF - 1) ? kMAXBUF - 1 : left;
   do {
      while ((len = read(fd, buf, wanted)) < 0 && TSystem::GetErrno() == EINTR)
         TSystem::ResetErrno();

      if (len < 0) {
         Error("ReadBuffer", "error reading file %s", fn.Data());
         close(fd);
         return (TObjString *)0;
      } else if (len > 0) {
         if (len == wanted)
            buf[len-1] = '\n';
         buf[len] = '\0';
         outbuf += buf;
      }

      // Update counters
      left -= len;
      wanted = (left > kMAXBUF - 1) ? kMAXBUF - 1 : left;

   } while (len > 0 && left > 0);

   // Done
   return new TObjString(outbuf.Data());
}

//______________________________________________________________________________
TObjString *TProofMgrLite::ReadBuffer(const char *fin, const char *pattern)
{
   // Read lines containing 'pattern' in 'file'.
   // Returns a TObjString with the content or 0, in case of failure

   // If no pattern, read everything
   if (!pattern || strlen(pattern) <= 0)
      return (TObjString *)0;

   if (!fin || strlen(fin) <= 0) {
      Error("ReadBuffer", "undefined path!");
      return (TObjString *)0;
   }
#ifdef WIN32
   TString fn = fin;
#else
   TString fn = TUrl(fin).GetFile();
#endif
   TString pat(pattern);
   // Check if "-v"
   Bool_t excl = kFALSE;
   if (pat.Contains("-v ")) {
      pat.ReplaceAll("-v ", "");
      excl = kTRUE;
   }
   pat = pat.Strip(TString::kLeading, ' ');
   pat = pat.Strip(TString::kTrailing, ' ');
   pat = pat.Strip(TString::kLeading, '\"');
   pat = pat.Strip(TString::kTrailing, '\"');

   // Use a regular expression
   TRegexp re(pat);

   // Open file with file info
   ifstream in;
   in.open(fn.Data());

   TString outbuf;

   // Read the input list of files and add them to the chain
   TString line;
   while(in.good()) {

      // Read next line
      line.ReadLine(in);

      // Keep only lines with pattern
      if ((excl && line.Index(re) != kNPOS) ||
          (!excl && line.Index(re) == kNPOS)) continue;

      // Remove trailing '\n', if any
      if (!line.EndsWith("\n")) line.Append('\n');

      // Add to output
      outbuf += line;
   }
   in.close();

   // Done
   return new TObjString(outbuf.Data());
}

//______________________________________________________________________________
void TProofMgrLite::ExpandPath(TString &path, Bool_t &sandbox)
{
   // Expand path wrt the sandbox

   sandbox = kFALSE;
#ifdef WIN32
   if (!path.BeginsWith("/") && path.First(':') != 1) {
#else
   if (!path.BeginsWith("/")) {
#endif
      if (path.BeginsWith("../")) {
         path.Remove(0,2);
         path.Insert(0, gSystem->DirName(fSessionDir));
      } else if (path.BeginsWith("./") || path.BeginsWith("~/")) {
         path.Remove(0,1);
         if (!path.BeginsWith("/")) path.Insert(0, "/");
         path.Insert(0, fSessionDir);
      } else {
         path.Insert(0, "/");
         path.Insert(0, fSessionDir);
      }
   }
   path.ReplaceAll("//","/");
   // Check if in the sandbox
#ifdef WIN32
   if (path.Contains(gSystem->DirName(fSessionDir))) sandbox = kTRUE;
#else
   if (path.BeginsWith(gSystem->DirName(fSessionDir))) sandbox = kTRUE;
#endif
   // Done
   return;
}

#ifdef WIN32
//______________________________________________________________________________
void TProofMgrLite::ShowFile(const char *what,
                             Int_t lines, UInt_t opt, const char *pattern)
{
   // Show the content of file "what".
   // If 'lines' is 0, show the whole file.
   // If 'lines' is > 0, show starting from line 'lines' inclusive.
   // If 'lines' is < 0, show the last 'lines' lines.
   // 'Ored' options EShowFileOpt:
   //           kShowLineNum           show the line numbers
   //           kGrep                  show only lines with 'pattern'
   //           kUnGrep                show only lines without 'pattern'

   // Needs a file and read access to it
   if (!what || strlen(what) <= 0 ||
       gSystem->AccessPathName(what, kReadPermission)) {
      Error("ShowFile", "file undefined or unreadable (%s)", what);
      return;
   }

   FileStat_t st;
   if (gSystem->GetPathInfo(what, st) != 0) {
      Error("ShowFile", "could not stat file: %s", what);
      return;
   }
   if (!R_ISREG(st.fMode) != 0) {
      Error("ShowFile", "file '%s' is not regular", what);
      return;
   }
   if (st.fSize <= 0) {
      Error("ShowFile", "file '%s' is empty", what);
      return;
   }

   // Options
   Bool_t showlinenum = (opt & kShowLineNum) ? kTRUE : kFALSE;
   Bool_t grep        = (opt & kGrep) ? kTRUE : kFALSE;
   Bool_t ungrep      = (!grep && (opt & kUnGrep)) ? kTRUE : kFALSE;

   TList *lns = 0;
   Int_t lmin = -1;
   if (lines < 0) {
      if (st.fSize < 100000) {
         // We put in memory the file and then we display exactly the lines
         lns = new TList;
      } else {
         // We will be approximate: we first estimate the first line assuming
         // 30 chars in average per line; we will refine the estimation while reading
         lmin = st.fSize / 30 + lines;
      }
   } else if (lines >= 0) {
      lmin = lines;
   }

   // Open the file
   ifstream f;
   f.open(what);
   Int_t nn = 0, nctot;
   if (f.is_open()) {
      while (f.good()) {
         TString line;
         line.ReadToDelim(f);
         if (!f.good()) break;
         line.Strip(TString::kTrailing, '\n');
         // Count
         nn++;
         if (lns) {
            // Store the line
            lns->Add(new TObjString(line));
         } else if (lines < 0) {
            // Some sizes for average
            nctot += line.Length();
            if (nn > 10) {
               // Recalculate 'lmin'
               lmin = st.fSize / ( nctot / nn ) + lines;
            }
         }
         // Do we have to show it ?
         Bool_t show = (lns) ? kFALSE: kTRUE;
         // Do we have a line min ?
         if (lmin >= 0 && nn <= lmin) show = kFALSE;
         // Are we grepping positive ?
         if (show && grep) {
            show = (line.Contains(pattern)) ? kTRUE : kFALSE;
         }
         // Are we grepping negative ?
         if (show && ungrep) {
            show = (line.Contains(pattern)) ? kFALSE : kTRUE;
         }
         // Print
         if (show) {
            if (showlinenum) {
               Printf("%d: %s", nn, line.Data());
            } else {
               Printf("%s", line.Data());
            }
         }
      }
      f.close();
   } else {
      Error("ShowFile", "unable to open file %s", what);
   }

   // If in tail, show the required lines
   Int_t nltot = 0;
   if (lns && (nltot = lns->GetSize()) > 0) {
      lmin = nltot + lines;
      TIter nxl(lns);
      TObjString *os = 0;
      nn = 0;
      while ((os = (TObjString *) nxl())) {
         nn++;
         if (lmin < 0 || nn > lmin) Printf("%s", os->GetName());
      }
   }
   // Cleanup
   if (lns) {
      lns->SetOwner();
      delete lns;
   }

   return;
}
#endif

//______________________________________________________________________________
void TProofMgrLite::Grep(const char *what, const char *how, const char *)
{
   // Run 'grep'

   // The full path
   TString path(what);
   Bool_t sb;
   ExpandPath(path, sb);
   if (gSystem->AccessPathName(path, kReadPermission)) {
      Printf("more: cannot access '%s'", what);
      return;
   }

#ifndef WIN32
   TString opt(how);
   TString cmd = TString::Format("grep %s %s", opt.Data(), path.Data());
   if (gSystem->Exec(cmd) != 0)
      Printf("grep: problems executing '%s'", cmd.Data());
#else
   UInt_t sfopt = kGrep;
   TString opt(how), tkn, pat;
   Int_t from = 0;
   while (opt.Tokenize(tkn, from, " ")) {
      if (tkn == "-n") {
         sfopt |= kShowLineNum;
      } else if (tkn == "-v") {
         sfopt |= kUnGrep;
         sfopt &= ~kGrep;
      } else {
         if (pat.Length() > 0) pat += " ";
         pat += tkn;
      }
   }
   ShowFile(path, -1, sfopt, pat);
#endif
   return;
}

//______________________________________________________________________________
void TProofMgrLite::Ls(const char *what, const char *, const char *)
{
   // Run 'ls'

   // The full path
   TString path(what);
   Bool_t sb;
   ExpandPath(path, sb);

   // Run the command
   TString cmd = TString::Format("%s %s", kLS, path.Data());
   if (gSystem->Exec(cmd) != 0)
      Printf("ls: problems executing '%s'", cmd.Data());

   return;
}

//______________________________________________________________________________
void TProofMgrLite::More(const char *what, const char *how, const char *)
{
   // Run 'more'

   // The full path
   TString path(what);
   Bool_t sb;
   ExpandPath(path, sb);
   if (gSystem->AccessPathName(path, kReadPermission)) {
      Printf("more: cannot access '%s'", what);
      return;
   }

#ifndef WIN32
   TString opt(how);
   TString cmd = TString::Format("more %s %s", opt.Data(), path.Data());
   if (gSystem->Exec(cmd) != 0)
      Printf("more: problems executing '%s'", cmd.Data());
#else
   TString opt(how), tkn, pat;
   Int_t from = 0, lines = 0;
   while (opt.Tokenize(tkn, from, " ")) {
      if (tkn.BeginsWith("+")) {
         tkn = tkn.Strip(TString::kLeading, '+');
         if (tkn.IsDigit()) lines = tkn.Atoi();
      }
   }
   ShowFile(path, lines);
#endif
   return;
}

//______________________________________________________________________________
Int_t TProofMgrLite::Rm(const char *what, const char *how, const char *)
{
   // Run 'rm'

   // The full path
   TString path(what);
   Bool_t sb;
   ExpandPath(path, sb);

   Bool_t recursive = (how && !strcmp(how, "-r")) ? kTRUE : kFALSE;
   if (TProof::Unlink(path, recursive) != 0) {
      if (gDebug > 0) Printf("rm: problems unlinking '%s'", path.Data());
      return -1;
   }

   return 0;
}

//______________________________________________________________________________
void TProofMgrLite::Tail(const char *what, const char *how, const char *)
{
   // Run 'tail'

   // The full path
   TString path(what);
   Bool_t sb;
   ExpandPath(path, sb);
   if (gSystem->AccessPathName(path, kReadPermission)) {
      Printf("more: cannot access '%s'", what);
      return;
   }

#ifndef WIN32
   TString opt(how);
   TString cmd = TString::Format("tail %s %s", opt.Data(), path.Data());
   if (gSystem->Exec(cmd) != 0)
      Printf("tail: problems executing '%s'", cmd.Data());
#else
   TString opt(how), tkn, pat;
   Int_t from = 0, lines = -10;
   while (opt.Tokenize(tkn, from, " ")) {
      if (tkn.BeginsWith("-")) {
         tkn = tkn.Strip(TString::kLeading, '-');
         if (tkn.IsDigit()) lines = -tkn.Atoi();
      }
   }
   ShowFile(path, lines);
#endif
   return;
}

//______________________________________________________________________________
Int_t TProofMgrLite::Md5sum(const char *what, TString &sum, const char *)
{
   // Run 'md5sum'

   // The full path
   TString path(what);
   Bool_t sb;
   ExpandPath(path, sb);

   TMD5 *md5 = TMD5::FileChecksum(path);
   if (!md5) {
      Printf("md5sum: problems calculating check sum for '%s'", path.Data());
      return -1;
   }

   sum = md5->AsString();
   delete md5;

   // Done
   return 0;
}

//______________________________________________________________________________
Int_t TProofMgrLite::Stat(const char *what, FileStat_t &st, const char *)
{
   // Run 'stat'

   // The full path
   TString path(what);
   Bool_t sb;
   ExpandPath(path, sb);

   if (gSystem->GetPathInfo(path, st)) {
      Printf("stat: problems running stat for '%s'", path.Data());
      return -1;
   }

   // Done
   return 0;
}

//______________________________________________________________________________
Int_t TProofMgrLite::GetFile(const char *remote, const char *local, const char *opt)
{
   // Get file 'remote' from the sandbox into 'local'.
   // If opt is "force", the file, if it exists remotely, is copied in all cases.
   // If opt is "check" and the local file exists, a check for changes is done
   // is done using the md5 check sum.
   // Return 0 on success, -1 on error.

   Int_t rc = -1;

   // Check remote path name
   TString filerem(remote);
   if (filerem.IsNull()) {
      Error("GetFile", "source file path undefined");
      return rc;
   }
   Bool_t sb;
   ExpandPath(filerem, sb);

   // Parse option
   TString oo(opt);
   oo.ToUpper();
   Bool_t force = (oo == "FORCE") ? kTRUE : kFALSE;

   // Check local path name
   TString fileloc(local);
   if (fileloc.IsNull()) {
      // Set the same as the remote one, in the working dir
      fileloc = gSystem->BaseName(filerem);
   }
   gSystem->ExpandPathName(fileloc);

   // Get information about the local file
   UserGroup_t *ugloc = 0;
   Int_t rcloc = 0;
   FileStat_t stloc;
   if ((rcloc = gSystem->GetPathInfo(fileloc, stloc)) == 0) {
      if (R_ISDIR(stloc.fMode)) {
         // Add the filename of the remote file and re-check
         if (!fileloc.EndsWith("/")) fileloc += "/";
         fileloc += gSystem->BaseName(filerem);
         // Get again the status of the path
         rcloc = gSystem->GetPathInfo(fileloc, stloc);
      }
      if (rcloc == 0) {
         // It exists already. If it is not a regular file we cannot continue
         if (!R_ISREG(stloc.fMode)) {
            Printf("[GetFile] local file '%s' exists and is not regular: cannot continue",
                               fileloc.Data());
            return rc;
         }
         // Get our info
         if (!(ugloc = gSystem->GetUserInfo(gSystem->GetUid()))) {
            Error("GetFile", "cannot get user info for additional checks");
            return rc;
         }
         // Can we delete or overwrite it ?
         Bool_t owner = (ugloc->fUid == stloc.fUid && ugloc->fGid == stloc.fGid) ? kTRUE : kFALSE;
         Bool_t group = (!owner && ugloc->fGid == stloc.fGid) ? kTRUE : kFALSE;
         Bool_t other = (!owner && !group) ? kTRUE : kFALSE;
         delete ugloc;
         if ((owner && !(stloc.fMode & kS_IWUSR)) ||
             (group && !(stloc.fMode & kS_IWGRP)) || (other && !(stloc.fMode & kS_IWOTH))) {
            Printf("[GetFile] file '%s' exists: no permission to delete or overwrite the file", fileloc.Data());
            Printf("[GetFile] ownership: owner: %d, group: %d, other: %d", owner, group, other);
            Printf("[GetFile] mode: %x", stloc.fMode);
            return rc;
         }
      }
   }

   // Check the source file exists and get it check sum
   TString remsum;
   if (Md5sum(filerem, remsum) != 0) {
      Printf("[GetFile] source file '%s' does not exists or cannot be read", filerem.Data());
      return rc;
   }

   // If the file exists already locally, check if it is different
   bool same = 0;
   if (rcloc == 0 && !force) {
      TMD5 *md5loc = TMD5::FileChecksum(fileloc);
      if (md5loc) {
         if (remsum == md5loc->AsString()) {
            Printf("[GetFile] local file '%s' and remote file '%s' have the same MD5 check sum",
                            fileloc.Data(), filerem.Data());
            Printf("[GetFile] use option 'force' to override");
            same = 1;
         }
         delete md5loc;
      }

      // If a different file with the same name exists already, ask what to do
      if (!same) {
         char *a = Getline("Local file exists already: would you like to overwrite it? [N/y]");
         if (a[0] == 'n' || a[0] == 'N' || a[0] == '\0') return 0;
      } else {
         return 0;
      }
   }

   // Ok, we are going to copy: remove first the existing file
   if (rcloc == 0) {
      if (gSystem->Unlink(fileloc) != 0) {
         Error("GetFile", "cannot remove existing local file '%s'", fileloc.Data());
         return rc;
      }
   }

   // Copy the file
   if (!TFile::Cp(filerem, fileloc)) {
      Error("GetFile", "TFile::Cp: problems getting the file '%s'", fileloc.Data());
      return rc;
   }

   rc = 0;
   // Check if everything went fine
   TMD5 *md5loc = TMD5::FileChecksum(fileloc);
   if (!md5loc) {
      Printf("[GetFile] cannot get MD5 checksum of the new local file '%s'", fileloc.Data());
      rc = -1;
   } else if (remsum != md5loc->AsString()) {
      Printf("[GetFile] checksums for the local copy and the remote file differ: {rem:%s,loc:%s}",
                        remsum.Data(), md5loc->AsString());
      rc = -1;
      delete md5loc;
   }

   // Done
   return rc;
}

//______________________________________________________________________________
Int_t TProofMgrLite::PutFile(const char *local, const char *remote, const char *opt)
{
   // Put file 'local'to 'remote' to the master
   // If opt is "force", the file, if it exists remotely, is copied in all cases,
   // otherwise a check is done on the MD5sum.
   // Return 0 on success, -1 on error

   Int_t rc = -1;

   // Check local path name
   TString fileloc(local);
   if (fileloc.IsNull()) {
      Error("PutFile", "local file path undefined");
      return rc;
   }
   gSystem->ExpandPathName(fileloc);

   // Parse option
   TString oo(opt);
   oo.ToUpper();
   Bool_t force = (oo == "FORCE") ? kTRUE : kFALSE;

   // Check remote path name
   TString filerem(remote);
   if (filerem.IsNull()) {
      // Set the same as the local one, in the working dir
      filerem.Form("~/%s", gSystem->BaseName(fileloc));
   } else if (filerem.EndsWith("/")) {
      // Remote path is a directory: add the file name as in the local one
      filerem += gSystem->BaseName(fileloc);
   }

   // Get information about the local file
   Int_t rcloc = 0;
   FileStat_t stloc;
   if ((rcloc = gSystem->GetPathInfo(fileloc, stloc)) != 0 || !R_ISREG(stloc.fMode)) {
      // It dies not exists or it is not a regular file: we cannot continue
      const char *why = (rcloc == 0) ? "is not regular" : "does not exists";
      Printf("[PutFile] local file '%s' %s: cannot continue", fileloc.Data(), why);
      return rc;
   }
   // Get our info
   UserGroup_t *ugloc = 0;
   if (!(ugloc = gSystem->GetUserInfo(gSystem->GetUid()))) {
      Error("PutFile", "cannot get user info for additional checks");
      return rc;
   }
   // Can we read it ?
   Bool_t owner = (ugloc->fUid == stloc.fUid && ugloc->fGid == stloc.fGid) ? kTRUE : kFALSE;
   Bool_t group = (!owner && ugloc->fGid == stloc.fGid) ? kTRUE : kFALSE;
   Bool_t other = (!owner && !group) ? kTRUE : kFALSE;
   delete ugloc;
   if ((owner && !(stloc.fMode & kS_IRUSR)) ||
       (group && !(stloc.fMode & kS_IRGRP)) || (other && !(stloc.fMode & kS_IROTH))) {
      Printf("[PutFile] file '%s': no permission to read the file", fileloc.Data());
      Printf("[PutFile] ownership: owner: %d, group: %d, other: %d", owner, group, other);
      Printf("[PutFile] mode: %x", stloc.fMode);
      return rc;
   }

   // Local MD5 sum
   TString locsum;
   TMD5 *md5loc = TMD5::FileChecksum(fileloc);
   if (!md5loc) {
      Error("PutFile", "cannot calculate the check sum for '%s'", fileloc.Data());
      return rc;
   } else {
      locsum = md5loc->AsString();
      delete md5loc;
   }

   // Check the remote file exists and get it check sum
   Bool_t same = kFALSE;
   FileStat_t strem;
   TString remsum;
   if (Stat(filerem, strem) == 0) {
      if (Md5sum(filerem, remsum) != 0) {
         Printf("[PutFile] remote file exists but the check sum calculation failed");
         return rc;
      }
      // Check sums
      if (remsum == locsum) {
         if (!force) {
            Printf("[PutFile] local file '%s' and remote file '%s' have the same MD5 check sum",
                              fileloc.Data(), filerem.Data());
            Printf("[PutFile] use option 'force' to override");
         }
         same = kTRUE;
      }
      if (!force) {
         // If a different file with the same name exists already, ask what to do
         if (!same) {
            char *a = Getline("Remote file exists already: would you like to overwrite it? [N/y]");
            if (a[0] == 'n' || a[0] == 'N' || a[0] == '\0') return 0;
            force = kTRUE;
         } else {
            return 0;
         }
      }
      // Remove destination file
      if (Rm(filerem) != 0) {
         Error("PutFile", "cannot remove existing remote file '%s'", filerem.Data());
         return rc;
      }
   }

   // Copy the file
   TString temprem(filerem);
   Bool_t sb;
   ExpandPath(temprem, sb);
   if (!TFile::Cp(fileloc, temprem)) {
      Error("PutFile", "TFile::Cp: problems putting the file '%s'", fileloc.Data());
      return rc;
   }

   // Check if everything went fine
   rc = 0;
   if (Md5sum(filerem, remsum) != 0) {
      Printf("[PutFile] cannot get MD5 checksum of the new remote file '%s'", filerem.Data());
      rc = -1;
   } else if (remsum != locsum) {
      Printf("[PutFile] checksums for the local copy and the remote file differ: {rem:%s, loc:%s}",
                        remsum.Data(), locsum.Data());
      rc = -1;
   }

   // Done
   return rc;
}

//______________________________________________________________________________
Int_t TProofMgrLite::Cp(const char *src, const char *dst, const char *)
{
   // Copy files in/out of the sandbox. Either 'src' or 'dst' must be in the
   // sandbox.
   // Return 0 on success, -1 on error

   Int_t rc = -1;

   // Check source path name
   TString filesrc(src);
   if (filesrc.IsNull()) {
      Error("Cp", "source file path undefined");
      return rc;
   }
   // Check destination path name
   TString filedst(dst);
   if (filedst.IsNull()) {
      filedst = gSystem->BaseName(TUrl(filesrc.Data()).GetFile());
   } else if (filedst.EndsWith("/")) {
      // Remote path is a directory: add the file name as in the local one
      filedst += gSystem->BaseName(filesrc);
   }

   Info("Cp","src: %s, dst: %s", filesrc.Data(), filedst.Data());

   // Make sure that local files are in the format file://<file>
   TUrl usrc(filesrc.Data(), kTRUE);
   TUrl udst(filedst.Data(), kTRUE);

   // At least one in the sandbox (no third party copy)
   Bool_t sbsrc = kFALSE, sbdst = kFALSE;
   if (!strcmp(usrc.GetProtocol(), "file")) ExpandPath(filesrc, sbsrc);
   if (!strcmp(udst.GetProtocol(), "file")) ExpandPath(filedst, sbdst);
   if (!sbsrc && !sbdst) {
      Error("Cp", "at least source or detination must be in the sandbox");
      return rc;
   }

   // Copy the file
   rc = 0;
   if (!TFile::Cp(filesrc, filedst)) {
      Error("PutFile", "TFile::Cp: problems cp the file '%s' to '%s'", filesrc.Data(), filedst.Data());
      rc = -1;
   }
   // Done
   return rc;
}
