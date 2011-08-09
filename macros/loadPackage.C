//
// Macro to load a PAR package in the local session; the macro checks the package
// against the version available in packdir and eventually reloads / rebuilds it.
// This macro performs the equivalent operations of:
//     1. TProof::UploadPackage
//     2. TProof::EnablePackage
//
// Advised usage:
// 0. load the macro
//    root[] .L loadPackage.C;
// 1. load package "mypack" from the reference package directory (~/proof/packages)
//    root[] loadPackage("mypack");
// 2. load package "thepack" from another package directory, e.g. /opt/sw/packdir
//    root[] loadPackage("thepack", "/opt/sw/packdir");
// 3. load package "mypack" from the reference directory clearing any existing build before
//    root[] loadPackage("mypack", "", kTRUE);

#include "TInterpreter.h"
#include "TMD5.h"
#include "TROOT.h"
#include "TString.h"
#include "TSystem.h"

void loadPackage(const char *pack, const char *packdir = "$PWD/packages", Bool_t clear = kFALSE, Bool_t debug = kFALSE)
{
   // Check arguments
   if (!pack || strlen(pack) <= 0) {
      Printf("loadPackage: package name undefined: cannot proceed!");
      return;
   }
   TString pdir(packdir);
   if (pdir.IsNull()) pdir = "$PWD/packages";
   gSystem->ExpandPathName(pdir);
   if (gSystem->AccessPathName(pdir, kFileExists)) {
      // Create directory
      if (gSystem->mkdir(pdir, kTRUE) != 0) {
         Printf("loadPackage: could not create reference directory %s", pdir.Data());
         return;
      }
      Printf("loadPackage: new reference directory %s created", pdir.Data());
   } else if (gSystem->AccessPathName(pdir, kWritePermission)) {
      Printf("loadPackage: requested reference directory is not writable"
             " (%s): using default (~/proof/packages)!", pdir.Data());
      pdir = "~/proof/packages";
      gSystem->ExpandPathName(pdir);
   }
   if (debug)
      Printf("loadPackage: package reference dir: %s", pdir.Data());

   // Save current directory
   TString ocwd = gSystem->WorkingDirectory();

   // Find out the package name and location wanted by the caller (pack can be a path)
   TString pn = gSystem->BaseName(pack);
   TString pd = gSystem->DirName(pack);
   if (pn.EndsWith(".par")) pn.ReplaceAll(".par", "");
   if (debug)
      Printf("loadPackage: package name: %s, local package dir: %s", pn.Data(), pd.Data());

   // Locate the package in the reference directory
   TString dpath(Form("%s/%s", pdir.Data(), pn.Data()));
   TString path(Form("%s/%s.par", pdir.Data(), pn.Data()));
   TString lpath(Form("%s/%s.par", pd.Data(), pn.Data()));
   if (debug) {
      Printf("loadPackage: package dir:        %s", dpath.Data());
      Printf("loadPackage: package path:       %s", path.Data());
      Printf("loadPackage: local package path: %s", lpath.Data());
   }

   // If the package exists already in the reference dir and we have a local copy, check if we
   // need to update it
   TMD5 *md5local = 0, *md5 = 0;
   if (!gSystem->AccessPathName(path)) {
      // A version of the package exists in the reference directory:
      // if we have a local one we check if we need to update it
      if (!gSystem->AccessPathName(lpath)) {
         // Calculate the MD5 sums
         md5local = TMD5::FileChecksum(lpath);
         md5 = TMD5::FileChecksum(path);
         if (!md5 || !md5local) {
            Printf("loadPackage: cannot evaluate MD5 sums (%p, %p): cannot proceed", md5, md5local);
            return;
         }
         if (*md5 != *md5local) {
            Printf("loadPackage: MD5 sums differ: uploading local version");
            // Copy new version
            if (gSystem->Exec(Form("cp -rp %s %s", lpath.Data(), path.Data())) != 0) {
               Printf("loadPackage: failure removing: %s", dpath.Data());
               return;
            }
         }
      }
   } else {
      // We upload the local copy, if we have it
      if (gSystem->AccessPathName(lpath)) {
         Printf("loadPackage: PAR file for '%s' not found - cannot continue", pn.Data());
         return;
      }
      // Copy new version
      Printf("loadPackage: PAR file for '%s' not found in the reference are: uploading local version", pn.Data());
      if (gSystem->Exec(Form("cp -rp %s %s", lpath.Data(), path.Data())) != 0) {
         Printf("loadPackage: failure removing: %s", dpath.Data());
         return;
      }
   }

   // Check if we are asked to clear the previous build
   if (clear) {
      // Version update needed: remove existing stuff
      Printf("loadPackage: clearing package '%s'", pn.Data());
      if (gSystem->Exec(Form("rm -fr %s", dpath.Data())) != 0) {
         Printf("loadPackage: failure removing: %s", dpath.Data());
         return;
      }
   }

   // Untar the package
   gSystem->ChangeDirectory(pdir);
   if (gSystem->Exec(Form("tar xzf %s", path.Data())) != 0) {
      Printf("loadPackage: failure untarring: %s", path.Data());
      return;
   }

   // Check if we need to build the package
   gSystem->ChangeDirectory(dpath);

   // check for BUILD.sh and execute
   if (!gSystem->AccessPathName("PROOF-INF/BUILD.sh")) {
      // read version from file proofvers.txt, and if current version is
      // not the same do a "BUILD.sh clean"
      Bool_t savever = kFALSE;
      TString v;
      Int_t rev = -1;
      FILE *f = fopen("PROOF-INF/proofvers.txt", "r");
      if (f) {
         TString r;
         v.Gets(f);
         r.Gets(f);
         rev = (!r.IsNull() && r.IsDigit()) ? r.Atoi() : -1;
         fclose(f);
      }
      if (!f || v != gROOT->GetVersion() ||
         (gROOT->GetSvnRevision() > 0 && rev != gROOT->GetSvnRevision())) {
         savever = kTRUE;
         Printf("loadPackage: %s:version change (current: %s:%d,"
                " build: %s:%d): cleaning ... ", pn.Data(),
               gROOT->GetVersion(), gROOT->GetSvnRevision(), v.Data(), rev);
         // Hard cleanup: go up the dir tree
         gSystem->ChangeDirectory(pdir);
         // remove package directory
         if (gSystem->Exec(Form("rm -fr %s", dpath.Data())) != 0) {
            Printf("loadPackage: failure removing: %s", dpath.Data());
            return;
         }
         // Untar the package
         if (gSystem->Exec(Form("tar xzf %s", path.Data())) != 0) {
            Printf("loadPackage: failure untarring: %s", path.Data());
            return;
         }
         // Store md5 in package/PROOF-INF/md5.txt
         if (md5) md5 = TMD5::FileChecksum(path);
         if (md5) {
            TString md5f = dpath + "/PROOF-INF/md5.txt";
            TMD5::WriteChecksum(md5f, md5);
         }
         // Go down to the package directory
         gSystem->ChangeDirectory(dpath);
      }
      // Build the package now
      TString ipath(gSystem->GetIncludePath());
      ipath.ReplaceAll("\"","");
      TString cmd = Form("export ROOTINCLUDEPATH=\"%s\" ; PROOF-INF/BUILD.sh",
                           ipath.Data());
      if (gSystem->Exec(cmd.Data()) != 0) {
         Printf("loadPackage: failure building: %s", path.Data());
         return;
      }
      f = fopen("PROOF-INF/proofvers.txt", "w");
      if (f) {
         fputs(gROOT->GetVersion(), f);
         fputs(Form("\n%d",gROOT->GetSvnRevision()), f);
         fclose(f);
      }
   }

   // Load the package now: check for SETUP.C and execute
   Int_t status = 0;
   if (!gSystem->AccessPathName("PROOF-INF/SETUP.C")) {
      Int_t err = 0;
      Int_t errm = gROOT->Macro("PROOF-INF/SETUP.C", &err);
      if (errm < 0)
         status = -1;
      if (err > TInterpreter::kNoError && err <= TInterpreter::kFatal)
         status = -1;
   }
   gSystem->ChangeDirectory(ocwd);

   if (status != 0) {
      // Notify the upper level
      Printf("loadPackage: failure loading %s ...", pn.Data());
      return;
   }

   // create link to package in working directory if not existing already
   FileStat_t st;
   Bool_t createlnk = kTRUE;
   if (gSystem->GetPathInfo(pn, st) == 0) {
      createlnk = kFALSE;
      if (st.fIsLink) {
         TString tmpcwd = gSystem->WorkingDirectory();
         gSystem->ChangeDirectory(pn);
         if (dpath != gSystem->WorkingDirectory()) {
            Printf("loadPackage: a symlink '%s' already exists pointing to a different path:"
                   " cannot create the correct symlink!", pn.Data());
         }
      } else {
         Printf("loadPackage: a file or directory '%s' already exists:"
                " cannot create the correct symlink!", pn.Data());
      }
   }
   if (createlnk) gSystem->Symlink(dpath, pn);

   // Add package to list of include directories to be searched
   // by ACliC
   gSystem->AddIncludePath(TString("-I") + pn);
   gROOT->ProcessLine(TString(".include ") + pn);

   // if successful add to list and propagate to slaves
   if (createlnk) {
      Printf("loadPackage: package %s successfully loaded", pn.Data());
   } else {
      Printf("loadPackage: package %s loaded but local symlink is not usable by macros or selectors", pn.Data());
   }
   // Reposition in the current directory
   gSystem->ChangeDirectory(ocwd);
}
