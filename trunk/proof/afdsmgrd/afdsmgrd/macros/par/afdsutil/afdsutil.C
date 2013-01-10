/**
 * afdsutil.C -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * This macro (better if run compiled) can be found mainly on AAFs as a PROOF
 * package named VO_ALICE@AFDSUtils::<version>. It contains a series of
 * functions that help in creating and managing datasets either "remotely" from
 * within a PROOF connection, or locally, if the directory containing the
 * datasets is mounted on your local filesystem and you have proper privileges.
 *
 * Each "public" function begins with "af": once you have loaded the macro, you
 * can easily obtain a list of available functions by using ROOT's
 * autocompletion feature. Instead, functions that are meant to be kept private
 * begin with "_af".
 *
 * Detailed descriptions are found in comments right before the function header.
 */

#if !defined(__CINT__) || defined (__MAKECINT__)

#include <Getline.h>
#include <Riostream.h>
#include <TDataSetManagerFile.h>
#include <TEnv.h>
#include <TError.h>
#include <TFileCollection.h>
#include <TFileInfo.h>
#include <TGrid.h>
#include <TGridResult.h>
#include <THashList.h>
#include <TList.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TPRegexp.h>
#include <TProof.h>
#include <TFile.h>
#include <TTree.h>
#include <TKey.h>
#include <TFileStager.h>
#include <TRandom.h>
#include <TROOT.h>

#endif

/* ========================================================================== *
 *                            "PRIVATE" FUNCTIONS                             *
 * ========================================================================== */

// A global variable
Int_t _afOldErrorIgnoreLevel = -1;

/** Switches on the quiet ROOT mode.
 */
void _afRootQuietOn() {
  _afOldErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = 10000;
}

/** Switches off the quiet ROOT mode.
 */
void _afRootQuietOff() {
  if (_afOldErrorIgnoreLevel != -1) {
    gErrorIgnoreLevel = _afOldErrorIgnoreLevel;
    _afOldErrorIgnoreLevel = -1;
  }
}

/** Gets an environment variable either locally or on the environment of the
 *  PROOF cluster. The assumption is that all PROOF workers share the same
 *  environment. The TString returned must be disposed properly by the user. If
 *  the variable is not set or can't be retrieved, NULL is returned.
 */
TString *_afGetEnvVar(TString name, Bool_t local = kTRUE) {
  TString *val = NULL;
  if (local) {
    const char *valChr = gSystem->Getenv(name.Data());
    if (valChr) val = new TString(valChr);
  }
  else {
    if (gProof) {
      TString tfn;
      while (kTRUE) {
        tfn = Form("/tmp/proof-remote-envvar-%05.0lf", gRandom->Rndm()*10000);
        if (gSystem->AccessPathName(tfn.Data())) break;  // bizarre retval
      }
      gROOT->ProcessLine(
        Form("gProof->Exec(\".!env|grep '^%s'|cut -b%d-\"); > %s",
        name.Data(), name.Length()+2, tfn.Data()) );
      ifstream fp(tfn.Data());
      if (fp) {
        val = new TString();
        fp >> *val;
        fp.close();
        if (val->IsNull()) {
          delete val;
          val = NULL;
        }
      }
      gSystem->Unlink(tfn.Data());
    }
  }
  return val;
}


/** Return kTRUE if PROOF mode is active, and connects to PROOF (masteronly) if
 *  no PROOF connection is active.
 */
Bool_t _afProofMode() {
  Bool_t proofMode = gEnv->GetValue("af.proofmode", 1);
  if ((proofMode) && ((!gProof) || (!gProof->IsValid()))) {
    TProof::Open(gEnv->GetValue("af.userhost", "alice-caf.cern.ch"),
      "masteronly");
  }
  return proofMode;
}

/** Connects to AliEn, returning kTRUE on success and kFALSE on failure.
 */
Bool_t _afAliEnConnect() {
  if (!gGrid) {
    if (!TGrid::Connect("alien:", 0, 0, "-debug 0")) {
      Printf("Error: can't connect to AliEn!");
      return kFALSE;
    }
  }
  return kTRUE;
}

/** A TDataSetManagerFile object is created using default parameters, if no
 *  parameters are given. If repoPath is not NULL (default), a temporary dataset
 *  repository is created in a temporary directory, and the path to repository
 *  is returned on the TString pointed by repoPath. After usage, that string can
 *  be safely removed by means of "rm -rf".
 */
TDataSetManagerFile *_afCreateDsMgr(TString *repoPath = NULL,
  Bool_t tempDsRepo = kFALSE) {

  TDataSetManagerFile *mgr = NULL;
  if (!tempDsRepo) {

    //
    // Ordinary repository from configuration
    //

    const char *dsPath = gEnv->GetValue("af.dspath", "/pool/datasets");
    mgr = new TDataSetManagerFile( NULL, NULL, Form("dir:%s", dsPath) );
    if (repoPath) *repoPath = dsPath;

  }
  else {

    //
    // Creates a temporary unique repository
    //

    TString tempDir;

    do {
      tempDir.Form("%s/afdsutil-dsrepo-%04x", gSystem->TempDirectory(),
        gRandom->Integer(0xffff));
      //Printf("Testing directory: %s", tempDir.Data());
    }
    while (!gSystem->AccessPathName(tempDir));

    if (gSystem->mkdir(tempDir.Data()) != 0) return NULL;  // mkdir failed
    mgr = new TDataSetManagerFile(NULL, NULL, Form("dir:%s", tempDir.Data()));

    if (repoPath) *repoPath = tempDir;

  }
  return mgr;
}

/** Creates a new TFileStager based on current redirector URL.
 */
TFileStager *_afCreateStager(TUrl **stagerUrl = NULL) {
  const char *url = gEnv->GetValue("af.redirurl", "root://localhost:1234/$1");
  TUrl redirUrl(url);
  redirUrl.SetFile("");
  redirUrl.SetOptions("");
  redirUrl.SetAnchor("");
  TFileStager *fileStager = TFileStager::Open(redirUrl.GetUrl());
  if (stagerUrl != NULL) {
    *stagerUrl = new TUrl(redirUrl);
  }
  return fileStager;
}

/** Gets a flattened TList of datasets. The list must be destroyed by the user,
 *  and it is owner of its content. The list is alphabetically ordered.
 *
 *  The dsMask parameter can both specify a single dataset name or a mask which
 *  must be understandable by the TDataSetManagerFile::GetDataSets() function.
 */
TList *_afGetListOfDs(const char *dsMask = "/*/*") {

  TDataSetManagerFile *mgr = NULL;
  TList *listOfDs = new TList();
  listOfDs->SetOwner();

  if (!_afProofMode()) {
    mgr = _afCreateDsMgr();
  }

  // First of all it tries to open a dataset named "mask"; if does not succeed,
  // it considers it a mask.
  Int_t oldErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = 10000;
  TFileCollection *fc = NULL;
  if (mgr) {
    fc = mgr->GetDataSet(dsMask);
  }
  else {
    fc = gProof->GetDataSet(dsMask);
  }
  gErrorIgnoreLevel = oldErrorIgnoreLevel;

  if (strchr(dsMask, '*') == NULL) {

    // What to do if it appears to be a single dataset name

    fc = NULL;

    _afRootQuietOn();

    if (mgr) {
      fc = mgr->GetDataSet(dsMask);
    }
    else {
      fc = gProof->GetDataSet(dsMask);
    }

    _afRootQuietOff();

    if (fc) {
      listOfDs->Add( new TObjString(dsMask) );
      delete fc;
      if (mgr) {
        delete mgr;
      }
    }

    return listOfDs;  // it contains a single name or nothing if empty

  }

  // Dataset name contains stars: let's consider it a mask

  TMap *groups = NULL;
  if (mgr) {
    groups = mgr->GetDataSets(dsMask, TDataSetManager::kReadShort);
  }
  else {
    groups = gProof->GetDataSets(dsMask, ":lite:");
  }

  if (!groups) {
    if (mgr) {
      delete mgr;
    }
    return listOfDs;  // will be empty
  }

  groups->SetOwnerKeyValue();  // important to avoid leaks!
  TIter gi(groups);
  TObjString *gn;

  if (mgr) {

    // TDataSetManagerFile mode

    while ((gn = dynamic_cast<TObjString *>(gi.Next()))) {

      TMap *users = dynamic_cast<TMap *>( groups->GetValue( gn->String() ) );
      users->SetOwnerKeyValue();
      TIter ui(users);
      TObjString *un;

      while ((un = dynamic_cast<TObjString *>(ui.Next()))) {

        TMap *dss = dynamic_cast<TMap *>( users->GetValue( un->String() ) );
        dss->SetOwnerKeyValue();
        TIter di(dss);
        TObjString *dn;

        while ((dn = dynamic_cast<TObjString *>(di.Next()))) {

          // No COMMON user/group mapping is done here...
          TString dsUri = TDataSetManager::CreateUri( gn->String(),
            un->String(), dn->String() );

          listOfDs->Add( new TObjString(dsUri.Data()) );

        }
      }
    }

    delete mgr;
  }
  else {
    // PROOF mode
    while ((gn = dynamic_cast<TObjString *>(gi.Next()))) {
      listOfDs->Add( new TObjString( gn->String().Data() ) );
    }
  }

  delete groups;

  listOfDs->Sort();  // sorted alphabetically

  return listOfDs;
}

/** Reads the list of URLs inside a TFileInfo and keeps only the last URL in the
 *  list.
 */
void _afKeepOnlyLastUrl(TFileInfo *fi) {

  TList *urlList = new TList();
  urlList->SetOwner();
  Int_t count = 0;
  Int_t nUrls = fi->GetNUrls();
  TUrl *url;

  fi->ResetUrl();

  while ((url = fi->NextUrl())) {
    if (++count < nUrls) {
      urlList->Add( new TObjString(url->GetUrl()) );
    }
  }

  TIter i(urlList);
  TObjString *s;
  while ((s = dynamic_cast<TObjString *>(i.Next()))) {
    fi->RemoveUrl( s->String().Data() );
  }

  fi->ResetUrl();

  delete urlList;
}

/** Appends an alien:// URL at the end of the URL list, if it is able to guess
 *  it from the current URL.
 *
 *  It returns kTRUE on success (i.e. URL appended) and kFALSE if it was
 *  impossible to determine the alien:// URL.
 */
Bool_t _afAppendAliEnUrlFile(TFileInfo *fi) {

  TString curUrl = fi->GetCurrentUrl()->GetUrl();
  TString newUrl;

  TPMERegexp reAliEn("/alice/.*");
  if (reAliEn.Match( curUrl ) == 1) {
    newUrl = "alien://" + reAliEn[0];
    fi->AddUrl( newUrl.Data() );  // by default it is appended to the end
    return kTRUE;
  }

  return kFALSE;
}

/** Prepends a root://redir URL at the beginning of the URL list, if it is
 *  possible to find at least one AliEn URL to match.
 *
 *  It returns kTRUE on success (i.e. URL appended) and kFALSE if it was
 *  impossible to find any alien:// URL, or if redirector URL already existed.
 */
Bool_t _afPrependRedirUrlFile(TFileInfo *fi) {

  TUrl *url;
  const char *redirUrl = gEnv->GetValue("af.redirurl",
    "root://localhost:1234/$1");
  TPMERegexp re("^alien:\\/\\/(.*)$");
  Bool_t success = kFALSE;

  fi->ResetUrl();
  while (( url = fi->NextUrl() )) {
    if ( strcmp( url->GetProtocol(), "alien" ) == 0 ) {
      TString buf = url->GetUrl();
      re.Substitute(buf, redirUrl);
      success = fi->AddUrl( buf.Data(), kTRUE );  // kTRUE = beginning of list
        // success will be true if URL was added correctly
      break;
    }
  }

  return success;
}

/** Launch xrd to unstage the file. Return value of xrd is ignored. By default,
 *  output is not suppressed. If launchRemotely is kTRUE (default) the command
 *  is launched on the remote PROOF master rather than invoking xrd locally.
 *  This feature is useful to allow AAF administrators to block remote access
 *  to xrootd port 1094, but still give rights to the users to delete files.
 *  Enabling users to delete files on the remote cluster implies that the AAF
 *  users are conscious of "fair use" policies.
 *
 *  Returns kTRUE on success, kFALSE otherwise.
 */
Bool_t _afUnstage(TUrl *url, Bool_t suppressOutput = kFALSE,
  Bool_t launchRemotely = kTRUE) {

  TString cmd = Form("xrd %s:%d rm %s", url->GetHost(), url->GetPort(),
    url->GetFile());

  if (suppressOutput) {
    cmd.Append(" > /dev/null 2>&1");
  }

  Printf(">> Unstaging %s: %s", (launchRemotely ? "remotely" : "locally"),
    cmd.Data());

  if (launchRemotely) {
    gProof->Exec( Form("if (gProofServ->IsMaster()) "
      "gSystem->Exec(\"/pool/PROOF-AAF/xrootd*/bin/%s\");",
      cmd.Data()), kTRUE);
  }
  else {
    gSystem->Exec( cmd );
  }

  return kTRUE;
}

/** Formats a file size returning the new size and the unit of measurement. Unit
 *  of measurement is always three characters long.
 */
void _afNiceSize(Long64_t bytes, TString &um, Double_t &size) {

  const char *ums[] = { "byt", "KiB", "MiB", "GiB", "TiB" };
  Int_t maxDiv = sizeof(ums)/sizeof(const char *);
  Int_t nDiv = 0;
  Double_t b = bytes;

  while ((b >= 1024.) && (nDiv+1 < maxDiv)) {
    b /= 1024.;
    nDiv++;
  }

  um = ums[nDiv];
  size = b;
}

/** Appends TFileInfos from fc to origFc, only if the current URL of new entries
 *  is not present in origFc. Returns number of entries added, or -1 if at least
 *  one collection is NULL.
 */
Long64_t _afUpdateDs(TFileCollection *origFc, TFileCollection *fc) {

  if ((!origFc) || (!fc)) return -1;

  TIter i(fc->GetList());
  TIter j(origFc->GetList());
  TFileInfo *fi;
  TFileInfo *fj;
  Long64_t added = 0;

  while ((fi = dynamic_cast<TFileInfo *>(i.Next()))) {

    Bool_t found = kFALSE;
    j.Reset();
    while ((fj = dynamic_cast<TFileInfo *>(j.Next()))) {
      if (fj->FindByUrl( fi->GetCurrentUrl()->GetUrl() )) {
        found = kTRUE;
        break;
      }
    }

    if (!found) {
      // Add new item at the end of the original file collection
      fj = new TFileInfo(*fi);  // copy because TFileColl... has ownership
      origFc->Add(fj);
      added++;
    }

    //Printf("[%s] %s", (found ? "NOP" : "ADD"), fi->GetCurrentUrl()->GetUrl());
  }

  return added;
}

/** Saves a dataset to the disk or on PROOF, depending on the opened connection.
 *  It returns kTRUE on success, kFALSE on failure.
 *
 *  If quiet is kTRUE, it does print messages on success/failure.
 *
 *  If overwrite is kTRUE and dsUri exists already, it is overwritten. If update
 *  is explicitly requested, in case of an already existing dsUri, new URLs are
 *  added.
 *
 *  Options overwrite and update may not be both kTRUE at the same time.
 */
Bool_t _afSaveDs(TString dsUri, TFileCollection *fc, Bool_t overwrite,
  Bool_t quiet = kFALSE, Bool_t update = kFALSE) {

  Bool_t regSuccess;

  TString opts = "T";
  if (overwrite || update) opts.Append("O");

  TFileCollection *origFc = NULL;  // the original one, if reqd
  TFileCollection *newFc = NULL;   // the one to write

  if (!_afProofMode()) {

    //
    // Local mode (overwrite manually implemented)
    //

    TString group, user, name;
    TString dsRepoPath;
    TDataSetManagerFile *mgr = _afCreateDsMgr(&dsRepoPath);

    // If dsUri is not full (i.e. only dsname without path), group and user are
    // filled accordingly
    mgr->ParseUri(dsUri, &group, &user, &name);
    dsUri.Form("/%s/%s/%s", group.Data(), user.Data(), name.Data());

    TString dsFullDir;
    TString dsFullPath;
    dsFullDir.Form("%s/%s/%s", dsRepoPath.Data(), group.Data(), user.Data());
    dsFullPath.Form("%s/%s.root", dsFullDir.Data(), name.Data());

    // Check if file exists (no ovw)
    if ((!overwrite) && (!gSystem->AccessPathName(dsFullPath.Data()))) {
      if (!quiet) Printf(">> I won't overwrite dataset %s", dsUri.Data());
      delete mgr;
      return kFALSE;
    }

    if ((update) && (origFc = mgr->GetDataSet(dsUri))) {
      _afUpdateDs(origFc, fc);
      newFc = origFc;
    }
    else newFc = fc;

    // Workaround: if path does not exist, attempt to create it
    if (gSystem->AccessPathName(dsFullDir.Data())) {
      gSystem->mkdir(dsFullDir.Data(), kTRUE);  // kTRUE == recursive ('-p')
    }

    if (mgr->WriteDataSet(group, user, name, newFc) == 0) regSuccess = kFALSE;
    else regSuccess = kTRUE;

    delete mgr;

  }
  else {

    //
    // PROOF mode
    //

    if ((update) && (origFc = gProof->GetDataSet(dsUri))) {
      _afUpdateDs(origFc, fc);
      newFc = origFc;
    }
    else newFc = fc;

    regSuccess = gProof->RegisterDataSet(dsUri.Data(), newFc, opts.Data());
  }

  if (origFc) delete origFc;

  if (regSuccess) {
    if (!quiet) {
      Printf(">> Dataset %s %s with success", dsUri.Data(),
        (update ? "updated" : "written"));
    }
    return kTRUE;
  }

  if (!quiet) {
    Printf(">> Can't write dataset %s, check permissions", dsUri.Data());
  }

  return kFALSE;
}

/** Makes a collection from AliEn find. This is equivalent to the AliEn command:
 *
 *    find <basePath> <fileName>
 *
 *  An "anchor" is added to every file, if specified. The default tree of the
 *  collection can also be set.
 */
TFileCollection *_afAliEnFind(TString basePath, TString fileName,
  TString anchor, TString defaultTree, TString regExp = "",
  Bool_t rootArchiveSubst = kFALSE, Bool_t printEntries = kFALSE) {

  if (!_afAliEnConnect()) return NULL;

  TFileCollection *fc = new TFileCollection();

  TGridResult *res = gGrid->Query(basePath.Data(), fileName.Data());
  Int_t nEntries = res->GetEntries();

  TPMERegexp *re = NULL;
  TPMERegexp *archSubst = NULL;
  TString substWith = "";

  if (rootArchiveSubst) {
    anchor = "";  // no anchor allowed if substituting

    TString temp = Form("/%s$", fileName.Data());
    archSubst = new TPMERegexp(temp.Data());
    substWith = Form("/root_archive.zip#%s", fileName.Data());
  }

  if ((nEntries > 0) && (!regExp.IsNull())) {
    re = new TPMERegexp(regExp);
  }

  for (Int_t i=0; i<nEntries; i++) {

    Long64_t size = TString(res->GetKey(i, "size")).Atoll();

    TString tUrl = res->GetKey(i, "turl");

    // Perform optional regexp match
    if (((re != NULL) && (re->Match(tUrl) > 0)) || (re == NULL)) {
      if (anchor != "") {
        tUrl.Append("#");
        tUrl.Append(anchor);
      }

      if (archSubst) {
        archSubst->Substitute(tUrl, substWith, kFALSE);
      }

      if (printEntries) {
        Printf(">> %s", tUrl.Data());
      }

      fc->Add( new TFileInfo( tUrl, size, res->GetKey(i, "guid"),
        res->GetKey(i, "md5") ) );
    }

  }

  if (re) delete re;
  if (archSubst) delete archSubst;

  if (defaultTree != "") {
    fc->SetDefaultTreeName(defaultTree.Data());
  }

  fc->Update();  // needed

  return fc;
}

/** Obtains user's input without the final '\n'.
 */
TString _afGetLine(const char *prompt) {
  cout << "\e[1m" << prompt << "\e[m" << endl;
  char *buf = (char *)Getline("");
  Int_t l = strlen(buf);
  while ((--l >= 0) && ((buf[l] == '\n') || (buf[l] == '\r'))) {
    buf[l] = '\0';
  }
  return TString(buf);
}

/** Fills the metadata of the given TFileInfo by reading information from the
 *  file pointed by the first URL in the list. Information about TTrees and
 *  classes that inherit thereof are read.
 *
 *  fastScan enables retrieval of number of events from Tag file, if available,
 *  and falls back on standard TFile::Open() upon failure.
 *
 *  In case of success it returns kTRUE. If any failure occurs it returns
 *  kFALSE.
 */
Bool_t _afFillMetaDataFile(TFileInfo *fi, Bool_t fastScan = kFALSE,
  TString defTree = "", Bool_t quiet = kFALSE) {

  if (quiet) {
    _afRootQuietOn();
  }

  // Can't do fastScan if no default tree is given
  if ((fastScan) && (defTree.IsNull())) {
    fastScan = kFALSE;
  }

  // Let's start by removing old metadata
  fi->RemoveMetaData();

  // Get the URL and eventually open an AliEn connection
  TUrl *url = fi->GetCurrentUrl();

  if ((!gGrid) && ((strcmp(url->GetProtocol(), "alien") == 0) || fastScan)) {
    if (!TGrid::Connect("alien:")) {
      _afRootQuietOff();
      return kFALSE;
    }
  }

  // Will be set to kFALSE only if fast scan succeeds
  Bool_t slowScan = kTRUE;

  // Fast scan
  if (fastScan) {

    // Find the AliEn URL
    Bool_t aliEnFound = kFALSE;
    fi->ResetUrl();
    while ((url=fi->NextUrl()) != NULL) {
      if (strcmp(url->GetProtocol(), "alien") == 0) {
        aliEnFound = kTRUE;
        break;
      }
    }

    // Reset the URLs, if we need to fall back on TFile::Open()
    fi->ResetUrl();
    url = fi->NextUrl();

    // Get the AliEn path
    if (aliEnFound) {
      TString basePath = gSystem->DirName(url->GetFile());
      TFileCollection *fc = _afAliEnFind(basePath, "Run*.ESD.tag.root", "", "");

      if (fc) {
        if (fc->GetNFiles() == 1LL) {

          TIter i(fc->GetList());
          TFileInfo *tagFi;

          while ( (tagFi = dynamic_cast<TFileInfo *>(i.Next())) != NULL ) {
            TUrl *tagUrl = tagFi->GetCurrentUrl();
            TPMERegexp re("Run[0-9]*\\.Event0_([0-9]+)\\.ESD\\.tag\\.root");
            Int_t match = re.Match(tagUrl->GetFile());

            if (match == 2) {
              Long64_t ent = re[1].Atoll();
              if (ent > 0) {
                slowScan = kFALSE;
                TFileInfoMeta *meta = new TFileInfoMeta(defTree.Data());
                meta->SetEntries(ent);
                fi->AddMetaData(meta);
              }
            }
          }
        }
        delete fc;
      }
    }

  } // end of fast scan

  if (slowScan) {

    // Slow scan: let us open the file
    TFile *f = TFile::Open(url->GetUrl());

    if (!f) {
      Printf("Can't open file %s!", url->GetUrl());
      _afRootQuietOff();
      return kFALSE;
    }

    // Get the ROOT file content
    TIter k( f->GetListOfKeys() );
    TKey *key;

    while (( key = dynamic_cast<TKey *>(k.Next()) )) {

      if ( TClass::GetClass(key->GetClassName())->InheritsFrom("TTree") ) {

        // Every TTree (or inherited thereof) will be scanned for entries
        TFileInfoMeta *meta = new TFileInfoMeta( Form("/%s", key->GetName()) );
        TTree *tree = dynamic_cast<TTree *>( key->ReadObj() );

        // Maybe the file is now unaccessible for some reason, and the tree is
        // unreadable!
        if (tree) {
          meta->SetEntries( tree->GetEntries() );
          fi->AddMetaData(meta);  // TFileInfo is owner of its metadata
          //delete tree;  // CHECK: should I delete it or not?
        }
        else {
          Printf("!! In file %s, can't read TTree %s!",
            url->GetUrl(), key->GetName());
          f->Close();
          delete f;
          _afRootQuietOff();
          return kFALSE;
        }

      }
    }

    f->Close();
    delete f;

    _afRootQuietOff();

  } // end of slow scan

  return kTRUE;
}

/** Returns a shorter version of the supplied string
 */
TString _afShortStr(const char *str, Int_t maxlen) {
  TString s = str;
  TString o;
  Int_t ns = 10;          // n. chars from the start
  Int_t ne = maxlen-3-3;  // n. chars from the end
  if ((ne >= 3) && (s.Length() > maxlen)) {
    o = s(0, ns);
    o += "...";
    o += s(s.Length()-ne, ne);
  }
  else {
    o = str;
  }
  return o;
}

/** Takes as input a string containing a list of run ranges separated either by
 *  colons or by commas, and returns an ordered vector of integers with the
 *  parsed run numbers. Duplicates are removed. The range separator is the dash.
 *
 *  Example: the string "1230-1235:1240:1241-1243" will be parsed to array as:
 *  { 1230, 1231, 1232, 1233, 1234, 1235, 1240, 1241, 1242, 1243 }
 */
vector<Int_t> *_afExpandRunList(TString runList) {

  vector<Int_t> *runNumsPtr = new vector<Int_t>();
  vector<Int_t> &runNums = *runNumsPtr;

  TObjArray *runs = runList.Tokenize(":,");
  runs->SetOwner();
  TIter run(runs);
  TObjString *runOs;

  while ( (runOs = dynamic_cast<TObjString *>(run.Next())) ) {

    TString runStr = runOs->String();

    TPMERegexp p("^([0-9]+)-([0-9]+)$");
    if (p.Match(runStr) == 3) {
      Int_t r1 = p[1].Atoi();
      Int_t r2 = p[2].Atoi();

      if (r1 > r2) {
        // Swap
        r1 = r1 ^ r2;
        r2 = r1 ^ r2;
        r1 = r1 ^ r2;
      }

      for (Int_t r=r1; r<=r2; r++) {
        runNums.push_back(r);
      }
    }
    else {
      runNums.push_back(runStr.Atoi());
    }
  }

  delete runs;

  // Bubble sort (slow)
  for (UInt_t i=0; i<runNums.size(); i++) {
    for (UInt_t j=i+1; j<runNums.size(); j++) {
      if (runNums[j] < runNums[i]) {
        runNums[i] = runNums[i] ^ runNums[j];
        runNums[j] = runNums[i] ^ runNums[j];
        runNums[i] = runNums[i] ^ runNums[j];
      }
    }
  }

  // Remove duplicates
  {
    vector<Int_t>::iterator itr, prev;
    for (itr=runNums.begin(); itr!=runNums.end(); itr++) {
      if (itr == runNums.begin()) continue;
      prev = itr;
      prev--;
      if (*prev == *itr) {
        runNums.erase(itr);
        itr--;
      }
    }
  }

  return runNumsPtr;
}

/** Tells whether the given TFileInfo matches at least one of the URLs stored
 *  in the given TList of files (which is a TList of TObjString). Function is
 *  protected against misuse of NULL pointers.
 */
Bool_t _afFileInfoMatchList(TFileInfo *fi, TList *listOfFiles) {
  if ((fi == NULL) || (listOfFiles == NULL)) return kFALSE;

  TIter i(listOfFiles);
  TObjString *os;
  TString *url;

  while ( (os = dynamic_cast<TObjString *>(i.Next())) != NULL ) {
    url = &(os->String());
    if ( fi->FindByUrl(url->Data()) ) return kTRUE;
  }

  return kFALSE;
} 

/** See function afMarkUrlAs() for synopsis. This function processes a single
 *  TFileCollection in memory, while afMarkUrlAs() processes a dataset or a list
 *  of datasets by invoking this function. The first parameter might be "*" for
 *  all files, or a single file name; if NULL, each TFileInfo is matched against
 *  each file in the listOfFiles TList.
 */
Int_t _afMarkUrlOfCollectionAs(const char *fileUrl, TString bits,
  TFileCollection *fc, TString filterBits = "SsCc",
  TString options = "", TList *listOfFiles = NULL) {

  if ((fileUrl == NULL) && (listOfFiles == NULL)) return 0;

  // Parse options
  Bool_t keepOnlyLastUrl = kFALSE;
  Bool_t appendRecoveredAliEnPath = kFALSE;
  Bool_t prependRedirectorPath = kFALSE;
  Bool_t fastScan = kFALSE;
  Bool_t quiet = kFALSE;

  options.ToLower();
  TObjArray *tokOpts = options.Tokenize(":");
  TIter opt(tokOpts);
  TObjString *oopt;

  while (( oopt = dynamic_cast<TObjString *>(opt.Next()) )) {
    TString &sopt = oopt->String(); 
    if (sopt == "keeplast") {
      keepOnlyLastUrl = kTRUE;
    }
    else if (sopt == "alien") {
      appendRecoveredAliEnPath = kTRUE;
    }
    else if (sopt == "redir") {
      prependRedirectorPath = kTRUE;
    }
    else if (sopt == "fast") {
      fastScan = kTRUE;
    }
    else if (sopt == "quiet") {
      quiet = kTRUE;
    }
    else {
      Printf("Warning: ignoring unknown option \"%s\"", sopt.Data());
    }
  }

  Bool_t allFiles = kFALSE;
  if ((fileUrl) && strcmp(fileUrl, "*") == 0) allFiles = kTRUE;

  // How to mark URLs: parse string and check for incongruences
  Bool_t bS = kFALSE;
  Bool_t bs = kFALSE;
  Bool_t bC = kFALSE;
  Bool_t bc = kFALSE;
  Bool_t bM = kFALSE;
  Bool_t bm = kFALSE;

  if (bits.Index('S') >= 0) bS = kTRUE;
  if (bits.Index('s') >= 0) bs = kTRUE;
  if (bits.Index('C') >= 0) bC = kTRUE;
  if (bits.Index('c') >= 0) bc = kTRUE;
  if (bits.Index('M') >= 0) bM = kTRUE;
  if (bits.Index('m') >= 0) bm = kTRUE;

  Bool_t err = kFALSE;

  if (bs && bS) {
    Printf("Cannot set STAGED and NOT STAGED at the same time!");
    err = kTRUE;
  }

  if (bc && bC) {
    Printf("Cannot set CORRUPTED and NOT CORRUPTED at the same time!");
    err = kTRUE;
  }

  if (bm && bM) {
    Printf("Cannot REMOVE and REFRESH metadata at the same time!");
    err = kTRUE;
  }

  if ( !(bs || bS || bc || bC || bm || bM) && !(keepOnlyLastUrl ||
    appendRecoveredAliEnPath || prependRedirectorPath) ) {
    Printf("Nothing to do, specify some bits to change or some options");
    err = kTRUE;
  }

  if (err) return -1;

  // Which files to consider (SsCc)
  Bool_t filterc = kFALSE;
  Bool_t filterC = kFALSE;
  Bool_t filters = kFALSE;
  Bool_t filterS = kFALSE;

  if (filterBits == "") filterS = filters = filterC = filterc = kTRUE;

  if (filterBits.Index('S') >= 0) filterS = kTRUE;
  if (filterBits.Index('s') >= 0) filters = kTRUE;
  if (filterBits.Index('C') >= 0) filterC = kTRUE;
  if (filterBits.Index('c') >= 0) filterc = kTRUE;

  if ((!filterS) && (!filters)) filterS = filters = kTRUE;
  if ((!filterC) && (!filterc)) filterC = filterc = kTRUE;

  Int_t nChanged = 0;
  TIter j(fc->GetList());
  TFileInfo *fi;

  while ( (fi = dynamic_cast<TFileInfo *>(j.Next())) ) {

    if ( allFiles ||
         ( fileUrl && fi->FindByUrl(fileUrl) ) ||
         ( listOfFiles && _afFileInfoMatchList(fi, listOfFiles) )
       ) {

      Bool_t isC = fi->TestBit(TFileInfo::kCorrupted);
      Bool_t isS = fi->TestBit(TFileInfo::kStaged);

      if (((isC && !filterC) || (!isC && !filterc) ||
           (isS && !filterS) || (!isS && !filters))) {
        //Printf(">> File skipped");
        continue;
      }

      //if ((!allFiles) && (!quiet)) {
      //  Printf(">> Found in dataset %s", dsUri.Data());
      //}

      if (bC)      fi->SetBit(TFileInfo::kCorrupted);
      else if (bc) fi->ResetBit(TFileInfo::kCorrupted);

      if (bS)      fi->SetBit(TFileInfo::kStaged);
      else if (bs) fi->ResetBit(TFileInfo::kStaged);

      if (bm) fi->RemoveMetaData();
      else if (bM) {
        if (!_afFillMetaDataFile( fi, fastScan, fc->GetDefaultTreeName(),
          kTRUE )) {

          // The following is non-fatal
          Printf("There was a problem updating metadata of file %s",
            fi->GetCurrentUrl()->GetUrl());
        }
      }

      if (appendRecoveredAliEnPath) _afAppendAliEnUrlFile(fi);
      if (keepOnlyLastUrl) _afKeepOnlyLastUrl(fi);
      if (prependRedirectorPath) _afPrependRedirUrlFile(fi);

      nChanged++;

    }

  } // end while over files

  if (nChanged > 0) fc->Update();

  return nChanged;
}

/** Replaces a <RUN> specifier in the given string with the a run number
 *  properly padded with zeroes. If plain "<RUN>" is present, default padding
 *  is performed (specify with defaultPad, defaults to 0 = no pad). To manually
 *  specify padding, use, e.g., "<RUN123>" for 123 digits.
 */
TString _afReplaceRunPadded(TString &fmt, UInt_t runNum,
  UInt_t defaultPad = 0) {

  TPMERegexp runRe("<RUN([1-9][0-9]*)?>");  // no leading zero allowed
  TString out = fmt;

  Int_t n_match = runRe.Match(out);

  switch (n_match) {

    case 0: // no pattern
      return out;
    break;

    case 1: // <RUN>, without pad number: use default pad

      if (defaultPad) {
        TString pad = Form("%%0%dd", defaultPad);
        runRe.Substitute(out, pad.Data(), kFALSE);
      }
      else {
        runRe.Substitute(out, "%d", kFALSE);
      }

    break;

    case 2: // <RUN12345>
      runRe.Substitute(out, "%0$1d", kTRUE);
    break;

  }

  return Form(out.Data(), runNum);

}

/* ========================================================================== *
 *                             "PUBLIC" FUNCTIONS                             *
 * ========================================================================== */

/** Print current dataset utilities settings.
 */
void afPrintSettings() {

  Printf("\033[34mDatasets path:\033[m "
    "\033[35m%s\033[m - change it with afSetDsPath()",
    gEnv->GetValue("af.dspath", "/tmp"));

  Printf("\033[34mPROOF connection string:\033[m "
    "\033[35m%s\033[m - change it with afSetProofUserHost()", 
    gEnv->GetValue("af.userhost", "alice-caf.cern.ch"));

  Printf("\033[34mUsing PROOF connection?\033[m "
    "\033[35m%s\033[m - toggle it with afSetProofMode()",
    (gEnv->GetValue("af.proofmode", 1) ? \
      "Yes, working on the remote PROOF cluster" : "No, working locally"));

  Printf("\033[34mFiles path with redirector ($1 is the file path):\033[m "
    "\033[35m%s\033[m - change it with afSetRedirUrl()",
    gEnv->GetValue("af.redirurl", "root://localhost:1234/$1"));

  Printf("\033[34mAliEn dataset repository:\033[m "
    "\033[35m%s\033[m - change it with afSetAliEnDsRepo()",
    gEnv->GetValue("af.aliendsrepo", "/alice/cern.ch/tmp"));

}

/** Opens a PROOF connection.
 */
void afOpen() {
  const char *connStr = gEnv->GetValue("af.userhost", "alice-caf.cern.ch");
  TProof::Open(connStr, "masteronly");
}

/** Resets (soft, then hard) PROOF.
 */
void afReset() {
  const char *connStr = gEnv->GetValue("af.userhost", "alice-caf.cern.ch");
  TProof::Reset(connStr);
  TProof::Reset(connStr, kTRUE);
}

/** Sets the URL of the redirector. $1 *must* be present in the string, or else
 *  it will complain about it without setting the value.
 */
void afSetRedirUrl(TString &redirUrl) {
  if (redirUrl.Index("$1") == kNPOS) {
    Printf("Error: path *must* contain $1 which will be substituted with the "
      "file path, as in root://redir/pool/alien/$1");
  }
  else {
    gEnv->SetValue("af.redirurl", redirUrl);
    gEnv->SaveLevel(kEnvUser);
  }
}

/** Sets the path of the dataset source, and saves it to the af.dspath variable
 *  inside the user's ~/.rootrc.
 */
void afSetDsPath(const char *dsPath) {
  gEnv->SetValue("af.dspath", dsPath);
  gEnv->SaveLevel(kEnvUser);
}

/** Sets the default PROOF connection parameters and saves them.
 */
void afSetProofUserHost(const char *userHost = "localhost") {
  gEnv->SetValue("af.userhost", userHost);
  gEnv->SaveLevel(kEnvUser);
}

/** Sets the PROOF mode: do not use the TDataSetManagerFile directly, but use a
 *  PROOF connection. Useful for users who are not administrators.
 */
void afSetProofMode(Bool_t proofMode = kTRUE) {
  gEnv->SetValue("af.proofmode", (Int_t)proofMode);
  gEnv->SaveLevel(kEnvUser);
}

/** Sets the AliEn base directory for datasets repository.
 */
void afSetAliEnDsRepo(const char *dsRepo) {
  gEnv->SetValue("af.aliendsrepo", dsRepo);
  gEnv->SaveLevel(kEnvUser);
}

/** A content of a dataset is shown. There is the possibility to show only files
 *  that are (un)staged or (un)corrupted by combining one or more of SsCc in the
 *  showOnly string parameter in any order.
 *
 *  Moreover, you can specify one or more of Rrn to see:
 *
 *   - which files are staged for real (R)
 *   - which files aren't staged for real (r)
 *   - which files have no redirector information (n)
 *
 *  Usage examples:
 *
 *   - To show every staged file:                  "S" or "SCc"
 *   - To show every corrupted file:               "C" or "CSs"
 *   - To show every unstaged and corrupted file:  "sC"
 *   - To show every file (default):               "Ss" or "Cc" or "SsCc"
 */
void afShowDsContent(const char *dsUri, TString showOnly = "SsCc") {

  TDataSetManagerFile *mgr = NULL;
  TFileCollection *fc;

  if (_afProofMode()) {
    fc = gProof->GetDataSet(dsUri);
  }
  else {
    mgr = _afCreateDsMgr();
    fc = mgr->GetDataSet(dsUri);
  }

  if (!fc) {
    Printf("Error opening dataset URI %s", dsUri);
    if (mgr) {
      delete mgr;
    }
    return;
  }

  Bool_t bS, bs, bC, bc, bR, br, bn;
  bS = bs = bC = bc = bR = br = bn = kFALSE;

  if (showOnly.Index('S') >= 0) bS = kTRUE;
  if (showOnly.Index('s') >= 0) bs = kTRUE;
  if (showOnly.Index('C') >= 0) bC = kTRUE;
  if (showOnly.Index('c') >= 0) bc = kTRUE;
  if (showOnly.Index('R') >= 0) bR = kTRUE;
  if (showOnly.Index('r') >= 0) br = kTRUE;
  if (showOnly.Index('n') >= 0) bn = kTRUE;

  // If Ss (or Cc) omitted, show both Ss (or Cc) -- does not apply to Rr
  if (!bc && !bC) bc = bC = kTRUE;
  if (!bs && !bS) bs = bS = kTRUE;

  // Check is the file is staged for real (it takes time!)
  Bool_t checkStaged = (br || bR || bn);
  TFileStager *fileStager = NULL;

  TUrl *redirUrl = NULL;

  if (checkStaged) {
    fileStager = _afCreateStager(&redirUrl);
    if (!fileStager) {
      Printf("Warning: can't open file stager, impossible to check real stage "
        "status");
      checkStaged = kFALSE;
    }
    else {
      Printf("Warning: real staging information may have been cached by xrootd "
      "and may not reflect the actual file status");
    }
  }

  TIter i(fc->GetList());
  TFileInfo *inf;
  UInt_t count = 0;
  UInt_t countMatching = 0;

  while ((inf = dynamic_cast<TFileInfo *>(i.Next()))) {
    count++;

    Bool_t s = inf->TestBit(TFileInfo::kStaged);
    Bool_t c = inf->TestBit(TFileInfo::kCorrupted);
    Char_t r = 'n';  // possible values: R=online, r=not staged, n=info n.a.

    TUrl *url;

    if ( ((s && bS) || (!s && bs)) && ((c && bC) || (!c && bc)) ) {

      // Check if the file is *really* staged, if possible
      if (checkStaged) {
        inf->ResetUrl();
        while ((url = inf->NextUrl())) {
          if ((strcmp(url->GetProtocol(), redirUrl->GetProtocol()) == 0) &&
            (strcmp(url->GetHost(), redirUrl->GetHost()) == 0)) {
            TString where;
          
            //gSystem->RedirectOutput("/dev/null");
            r = (fileStager->Locate(url->GetUrl(), where) == 0) ? 'R' : 'r';
            //gSystem->RedirectOutput(0x0);
            break;

          }
        }

        if (!(((r == 'R') && bR) || ((r == 'r') && br) || ((r == 'n') && bn))) {
          continue;
        }

        inf->ResetUrl();
      }
      else {
        r = ' ';  // real staging information not requested
      }

      TFileInfoMeta *meta = inf->GetMetaData();  // gets the first one
      Int_t entries = -1;
      const char *treeName = "";
      if (meta) {
        entries = meta->GetEntries();
        treeName = meta->GetName();
      }

      TString um;
      Double_t sz;
      _afNiceSize(inf->GetSize(), um, sz);

      Printf("%4u. %c%c%c | % 7d | %-10s | %6.1lf %s | %s",
        ++countMatching,
        (s ? 'S' : 's'), (c ? 'C' : 'c'), r,
        entries,
        treeName,
        sz, um.Data(),
        inf->GetCurrentUrl()->GetUrl());
      inf->NextUrl();

      // Every URL is shown, not only first
      while ((url = inf->NextUrl())) {
        Printf("          |         |            |            | %s",
          url->GetUrl());
      }
      inf->ResetUrl();
    }
  }

  delete fc;

  if (mgr) delete mgr;
  if (fileStager) {
    delete fileStager;
    delete redirUrl;
  }

  Printf(">> There are %u file(s) in the dataset, %u matched your criteria",
    count, countMatching);
}

/** Prints summarized information about a certain dataset. It can also check how
 *  many files are on disk for real.
 */
void afDataSetInfo(const char *dsUri, Bool_t checkStaged = kFALSE) {

  TDataSetManagerFile *mgr = NULL;
  TFileCollection *fc;

  if (_afProofMode()) {
    fc = gProof->GetDataSet(dsUri);
  }
  else {
    mgr = _afCreateDsMgr();
    fc = mgr->GetDataSet(dsUri);
  }

  if (!fc) {
    Printf("Error opening dataset URI %s", dsUri);
    if (mgr) {
      delete mgr;
    }
    return;
  }

  TFileStager *fileStager = NULL;
  TUrl *redirUrl;

  if (checkStaged) {
    fileStager = _afCreateStager(&redirUrl);
    if (!fileStager) {
      Printf("Warning: can't open file stager, impossible to check real stage "
        "status");
      checkStaged = kFALSE;
    }
  }

  TIter i(fc->GetList());
  TFileInfo *inf;

  // Counters
  Long64_t nS, ns, nC, nc, nR, nr, nn;
  nS = ns = nC = nc = nR = nr = nn = 0;

  while ((inf = dynamic_cast<TFileInfo *>(i.Next()))) {

    Bool_t s = inf->TestBit(TFileInfo::kStaged);
    Bool_t c = inf->TestBit(TFileInfo::kCorrupted);

    TUrl *url;

    // Check if the file is *really* staged, if possible
    if (checkStaged) {
      Char_t r = 'n';  // possible values: R=online, r=not staged, n=info n.a.
      inf->ResetUrl();
      while ((url = inf->NextUrl())) {
        if ((strcmp(url->GetProtocol(), redirUrl->GetProtocol()) == 0) &&
          (strcmp(url->GetHost(), redirUrl->GetHost()) == 0)) {
          TString where;
          
          //gSystem->RedirectOutput("/dev/null");
          r = (fileStager->Locate(url->GetUrl(), where) == 0) ? 'R' : 'r';
          //gSystem->RedirectOutput(0x0);
          break;
        }
      }

      if (r == 'R') nR++;
      else if (r == 'r') nr++;
      else nn++;
    }

    if (s) nS++;
    else ns++;

    if (c) nC++;
    else nc++;
  }

  if (mgr) delete mgr;
  if (fileStager) {
    delete fileStager;
    delete redirUrl;
  }

  Printf("Dataset %s contains %lld files:", dsUri, fc->GetNFiles());
  Printf(">> %lld marked as staged, %lld marked as not staged", nS, ns);
  Printf(">> %lld good, %lld corrupted", nc, nC);
  if (checkStaged) {
    Printf(">> %lld really staged, %lld not on disk, %lld unknown", nR, nr, nn);
  }

  delete fc;
}

/** Marks a file matching the URL as (un)staged or (un)corrupted: choose mode
 *  with one or more among "SsCc". Clearly, "S" is incompatible with "s" and "C"
 *  is incompatible with "c". The URL may match any URL of a TFileInfo, even not
 *  the first one.
 *
 *  A value of "M" refreshes current metadata (i.e. file is "verified"), while
 *  a value of "m" deletes the metadata from the object.
 *
 *  The filterBits option tells the function which files to consider: by
 *  default consider all files, but you can decide to modify only (un)staged or
 *  (un)corrupted files, with one or more of the following: "SsCc". For
 *  instance, to consider only staged files use "S".
 *
 *  EXAMPLE: to remove metadata and mark as corrupted, use "Cm"; instead, to
 *  uncorrupt, mark as staged and fill metadata, use "cSm".
 *
 *  The URL is searched in the dataset(s) specified by dsMask.
 *
 *  If "*" is used as URL, it applies to every entry in the dataset.
 *
 *  It is possible to pass a list of URLs written one by line in a text file, by
 *  passing as URL the string ">/path/to/list_of_urls.txt". The file name after
 *  the major (">") sign will be used as list of URLs.
 *
 *  Available options may be separated by colons:
 *
 *   - keeplast : every URL in each TFileInfo is removed, but the last one
 *
 *   - alien    : appends a guessed AliEn path *before* removing all the other
 *                URLs; useful if the dataset was created without keeping the
 *                originating URL in each TFileInfo (if using afdsmgrd <=
 *                v0.1.7).
 *
 *   - redir    : path of file on the redirector is prepended
 *
 *   - fast     : try to read number of events from the ESD tag file, if
 *                available, or fallback on standard, slower file opening; it
 *                applies only if using with option 'M'
 *
 */
void afMarkUrlAs(const char *fileUrl, TString bits = "",
  const char *dsMask = "/*/*", TString filterBits = "SsCc",
  TString options = "") {

  TDataSetManagerFile *mgr = NULL;

  if (!_afProofMode()) mgr = _afCreateDsMgr();

  // List of files in a text file
  TList *listOfFiles = NULL;
  if (fileUrl[0] == '>') {
    Printf("Reading list of files %s...", &fileUrl[1]);
    ifstream ifs(&fileUrl[1]);
    if (!ifs) {
      Printf("Can't read list of files, aborting");
      return;
    }

    TString buf;
    listOfFiles = new TList();
    listOfFiles->SetOwner(kTRUE);
    while (ifs >> buf) listOfFiles->Add( new TObjString(buf.Data()) );

    ifs.close();
  }

  TList *listOfDs = _afGetListOfDs(dsMask);
  Int_t regErrors = 0;
  TIter i(listOfDs);
  TObjString *dsUriObj;

  while ( (dsUriObj = dynamic_cast<TObjString *>(i.Next())) ) {

    TString dsUri = dsUriObj->String();
    Int_t nChanged = 0;

    TFileCollection *fc;
    if (mgr) fc = mgr->GetDataSet(dsUri.Data());
    else fc = gProof->GetDataSet(dsUri.Data());

    if (listOfFiles) {
      // Passing TList of files
      nChanged = _afMarkUrlOfCollectionAs(NULL, bits, fc, filterBits, options,
        listOfFiles);
    }
    else {
      // Passing fileUrl
      nChanged = _afMarkUrlOfCollectionAs(fileUrl, bits, fc, filterBits,
        options);
    }

    if (nChanged < 0) break;
    else if (nChanged > 0) {
      if (!_afSaveDs(dsUri, fc, kTRUE, kFALSE)) regErrors++;
    }

    delete fc;
  }

  if (mgr) delete mgr;
  delete listOfDs;

  if (listOfFiles) delete listOfFiles;  // owner of contents

  if (regErrors > 0) {
    Printf("%d error(s) writing back datasets encountered, check permissions",
      regErrors);
  }
}

/** Does TFileCollection::Update() on each dataset.
 */
void afUpdateDs(const char *dsMask) {

  TDataSetManagerFile *mgr = NULL;
  if (!_afProofMode()) mgr = _afCreateDsMgr();

  TList *listOfDs = _afGetListOfDs(dsMask);
  TIter i(listOfDs);
  TObjString *dsUriObj;

  while ( (dsUriObj = dynamic_cast<TObjString *>(i.Next())) ) {

    TString dsUri = dsUriObj->String();

    TFileCollection *fc;
    if (mgr) fc = mgr->GetDataSet(dsUri.Data());
    else fc = gProof->GetDataSet(dsUri.Data());

    Printf("Updating %s...", dsUri.Data());

    fc->Update();
    _afSaveDs(dsUri, fc, kTRUE);
    delete fc;

  }

  if (mgr) delete mgr;
  delete listOfDs;

}

/** Entries whose URL or tree name (toggle with "tree" option) matches the given
 *  regex are *removed* from the given dataset(s). Possible options, separated
 *  by colons, are:
 *
 *   - showkept : show kept entries
 *   - showdel  : show removed entries
 *   - tree     : match entry's default tree name instead of URL
 *   - commit   : actually save changes on dataset
 *
 *  No possible "harm" is done on datasets unless the "commit" option is given.
 *  Double-check the results of your operation before using this option and
 *  saving changes to the datasets, because there is no turning back to that.
 */
void afRemoveUrlsFromDs(const char *dsMask, const char *regex,
  TString options = "showkept:showdel") {

  // Parse options
  Bool_t commit = kFALSE;
  Bool_t showKept = kFALSE;
  Bool_t showDel = kFALSE;
  Bool_t matchTree = kFALSE;

  options.ToLower();

  TObjArray *tokOpts = options.Tokenize(":");
  TIter opt(tokOpts);
  TObjString *oopt;

  while (( oopt = dynamic_cast<TObjString *>(opt.Next()) )) {
    TString &sopt = oopt->String(); 
    if (sopt == "commit") commit = kTRUE;
    else if (sopt == "showkept") showKept = kTRUE;
    else if (sopt == "showdel") showDel = kTRUE;
    else if (sopt == "tree") matchTree = kTRUE;
    else {
      Printf("Warning: ignoring unknown option \"%s\"", sopt.Data());
    }
  }

  if ((!showKept) && (!showDel)) {
    showKept = kTRUE;
    showDel = kTRUE;
  }

  TDataSetManagerFile *mgr = NULL;
  if (!_afProofMode()) mgr = _afCreateDsMgr();

  TPMERegexp re(regex);

  TList *listOfDs = _afGetListOfDs(dsMask);
  Int_t saveErrs = 0;
  TIter i(listOfDs);
  TObjString *dsUriObj;

  while ( (dsUriObj = dynamic_cast<TObjString *>(i.Next())) ) {

    //
    // Inside a dataset
    //

    TString dsUri = dsUriObj->String();
    Int_t nExcluded = 0;

    Printf("*** Processing dataset %s ***", dsUri.Data());

    TFileCollection *fc;
    if (mgr) fc = mgr->GetDataSet(dsUri.Data());
    else fc = gProof->GetDataSet(dsUri.Data());

    TFileCollection *newFc = NULL;
    if (commit) {
      newFc = new TFileCollection();
      newFc->SetDefaultTreeName( fc->GetDefaultTreeName() );
    }

    TIter j(fc->GetList());
    TFileInfo *fi;

    while ( (fi = dynamic_cast<TFileInfo *>(j.Next())) ) {

      //
      // For each file
      //

      Bool_t includeUrl = kTRUE;
      const char *treeName = NULL;

      if (matchTree) {

        // Match tree name (toggle with "tree" option, default is to match URLs)
        TFileInfoMeta *meta = fi->GetMetaData();
        if (meta) {
          treeName = meta->GetName();
          if (re.Match(meta->GetName()) > 0) includeUrl = kFALSE;
        }

      }
      else {

        // Match at least one URL
        TUrl *curl;
        fi->ResetUrl();
        while (( curl = fi->NextUrl() )) {
          if (re.Match(curl->GetUrl()) > 0) {
            includeUrl = kFALSE;
            break;
          }
        }
        fi->ResetUrl();

      }

      if ((includeUrl && showKept) || (!includeUrl && showDel)) {
        const char *status =
          (includeUrl ? "\033[1;32mKEEP\033[m" : "\033[1;31mDELE\033[m");

        if (treeName) {
          Printf("[%s] %s (tree: %s)", status, fi->GetFirstUrl()->GetUrl(),
            treeName);
        }
        else {
          Printf("[%s] %s", status, fi->GetFirstUrl()->GetUrl());
        }
      }

      if (includeUrl) {
        if (commit) {
          TFileInfo *newFi = new TFileInfo(*fi);
          newFc->Add( newFi );  // newFc becomes owner of newFi
        }
      }
      else nExcluded++;

    }

    if (nExcluded > 0) {
      if (commit) {
        Printf("%d entries removed: saving...", nExcluded);
        newFc->Update();
        _afSaveDs(dsUri, newFc, kTRUE);
        delete newFc;
      }
      else {
        Printf("Warning: not saving dataset %s (%d entries removed) because "
          "no \"commit\" option was specified", dsUri.Data(), nExcluded);
      }
    }
    else {
      Printf("No modification made in dataset %s", dsUri.Data());
    }

    delete fc;
  }

  if (mgr) delete mgr;
  delete listOfDs;

  if (saveErrs > 0) {
    Printf("%d error(s) writing back datasets encountered, check permissions",
      saveErrs);
  }

}

/** Finds the exact match of a given URL within the mask of datasets given (by
 *  default in all datasets).
 */
void afFindUrl(const char *fileUrl, const char *dsMask = "/*/*") {

  TDataSetManagerFile *mgr = NULL;

  if (!_afProofMode()) {
    mgr = _afCreateDsMgr();
  }

  TList *listOfDs = _afGetListOfDs(dsMask);
  TIter i(listOfDs);
  TObjString *dsUriObj;
  Int_t nFoundDs = 0;
  Int_t nFoundTotal = 0;

  while ( (dsUriObj = dynamic_cast<TObjString *>(i.Next())) ) {

    TString dsUri = dsUriObj->String();

    TFileCollection *fc;
    if (mgr) {
      fc = mgr->GetDataSet(dsUri.Data());
    }
    else {
      fc = gProof->GetDataSet(dsUri.Data());
    }

    TIter j(fc->GetList());
    TFileInfo *fi;
    Int_t nFoundIntoDs = 0;

    while ( (fi = dynamic_cast<TFileInfo *>(j.Next())) ) {
      if (fi->FindByUrl(fileUrl)) {
        nFoundIntoDs++;
        nFoundTotal++;
      }
    }

    if (nFoundIntoDs) {
      if (nFoundIntoDs == 1) {
        Printf(">> Found in dataset %s once", dsUri.Data());
      }
      else {
        Printf(">> Found in dataset %s (%d times)", dsUri.Data(), nFoundIntoDs);
      }
      nFoundDs++;
    }

    delete fc;
  }

  if (mgr) {
    delete mgr;
  }
  delete listOfDs;

  Printf("Found %d time(s) in %d different dataset(s)", nFoundTotal, nFoundDs);
}

/** Repair datasets: this function gives the possibility to take actions on
 *  corrupted files. Possible actions are:
 *
 *   - unlist:      files are removed from dataset
 *   - unstage:     files are deleted from storage and marked as unstaged
 *   - condunstage: same as unstage, but storage deletion is triggered only if
 *                  file is marked as staged
 *   - uncorrupt:   files are marked as unstaged/uncorrupted
 *   - delendpurl:  removes endpoint URL from the corrupted file, leaving only
 *                  originating and redirector's ones
 *
 *  Actions can be combined, separe them with colon ':'. Actions "unlist" and
 *  "uncorrupt" can not be combined.
 *
 *  For instance, to remove files both from the list and from the disk, you can
 *  use "unlist:unstage".
 *
 *  If no valid action is given, corrupted files are only listed.
 *
 *  The dataset(s) to be repaired are limited by the dsMask parameter, which can
 *  be both a single dataset name and a mask.
 *
 *  If you specify a non-empty filename for listOutFile, the list of bad files
 *  is written there.
 */
void afRepairDs(const char *dsMask = "/*/*", const TString action = "",
  const TString listOutFile = "") {

  Bool_t aUncorrupt = kFALSE;
  Bool_t aUnstage = kFALSE;
  Bool_t aCondUnstage = kFALSE;
  Bool_t aUnlist = kFALSE;
  Bool_t aDelEndpUrl = kFALSE;

  TObjArray *tokens = action.Tokenize(":");

  for (Int_t i=0; i<tokens->GetEntries(); i++) {

   TObjString *tok = dynamic_cast<TObjString *>(tokens->At(i));

    if (tok->String() == "uncorrupt") {
      aUncorrupt = kTRUE;
    }
    else if (tok->String() == "unstage") {
      aUnstage = kTRUE;
    }
    else if (tok->String() == "condunstage") {
      aCondUnstage = kTRUE;
    }
    else if (tok->String() == "unlist") {
      aUnlist = kTRUE;
    }
    else if (tok->String() == "delendpurl") {
      aDelEndpUrl = kTRUE;
    }

  }

  delete tokens;

  // Check for incompatible options
  if (aUncorrupt && aUnlist) {
    Printf("Can't mark as uncorrupted and unlist at the same time.");
    return;
  }
  if (aUnstage && aCondUnstage) {
    Printf("Please specify only one amongst \"unstage\" and \"condunstage\".");
    return;
  }

  // Output a text file with the list of "bad" files
  ofstream outList;
  if (listOutFile != "") {
    outList.open(listOutFile.Data());
    if (!outList) {
      Printf("The desired output text file can not be opened, aborting.");
      return;
    }
  }

  Bool_t unstageRemotely;

  TDataSetManagerFile *mgr = NULL;
  if (!_afProofMode()) {
    mgr = _afCreateDsMgr();
    unstageRemotely = kFALSE;
  }
  else {
    unstageRemotely = kTRUE;
  }

  TList *listOfDs = _afGetListOfDs(dsMask);
  TObjString *dsUriObj;
  TIter iDs(listOfDs);

  // Loop over datasets
  while ( (dsUriObj = dynamic_cast<TObjString *>(iDs.Next())) ) {

    TString dsUri = dsUriObj->String();
    TFileCollection *fc;
    TFileCollection *newFc = NULL;

    if (mgr) {
      fc = mgr->GetDataSet(dsUri.Data());
    }
    else {
      fc = gProof->GetDataSet(dsUri.Data());
    }

    if (aUncorrupt || aUnstage || aCondUnstage || aDelEndpUrl || aUnlist) {
      newFc = new TFileCollection();
      newFc->SetDefaultTreeName( fc->GetDefaultTreeName() );
    }

    Printf("Scanning dataset %s for corrupted files...", dsUri.Data());

    TIter j(fc->GetList());
    TFileInfo *fi;
    Int_t nChanged = 0;

    while ( (fi = dynamic_cast<TFileInfo *>(j.Next())) ) {

      Bool_t c = fi->TestBit(TFileInfo::kCorrupted);

      if (c) {

        TUrl *url = fi->GetFirstUrl();

        Printf(">> CORRUPTED: %s", url->GetUrl());
        if (outList.is_open()) {
          outList << url->GetUrl() << endl;
        }

        if (aUncorrupt || aUnstage || aUnlist || aDelEndpUrl) {

          if (aUncorrupt) {
            fi->ResetBit(TFileInfo::kCorrupted);
            fi->ResetBit(TFileInfo::kStaged);
          }

          if ((aUnstage) && (_afUnstage(url, kTRUE, unstageRemotely))) {
            fi->ResetBit(TFileInfo::kStaged);
          }

          if (aCondUnstage) {
            if ((fi->TestBit(TFileInfo::kStaged)) &&
              (_afUnstage(url, kTRUE, unstageRemotely))) {
              fi->ResetBit(TFileInfo::kStaged);
            }
          }

          if (aDelEndpUrl) {

            Int_t nUrlsToDel = fi->GetNUrls() - 2;
            if (nUrlsToDel > 0) {

              TString **urlsToDel = new TString*[nUrlsToDel];
              Int_t i;

              // Collecting URLs to delete (all but last two)
              fi->ResetUrl();
              for (i=0; i<nUrlsToDel; i++) {
                urlsToDel[i] = new TString(fi->NextUrl()->GetUrl());
              }
              fi->ResetUrl();

              // Deleting URLs
              for (i=0; i<nUrlsToDel; i++) {
                fi->RemoveUrl(urlsToDel[i]->Data());
                delete urlsToDel[i];
              }

              delete[] urlsToDel;

           }
           else nChanged--;  // Trick to avoid saving if not needed

          }

          if (!aUnlist) {
            TFileInfo *newFi = new TFileInfo(*fi);
            newFc->Add(newFi);
          }

          nChanged++;
        }

      }
      else if (newFc) {
        TFileInfo *newFi = new TFileInfo(*fi);
        newFc->Add(newFi);
      }

    }

    delete fc;

    if (nChanged > 0) {
      newFc->Update();
      if ( _afSaveDs(dsUri, newFc, kTRUE, kTRUE) ) {
        Printf("Dataset %s has changed - # of files: %lld (%.2f%% staged)",
          dsUri.Data(), newFc->GetNFiles(), newFc->GetStagedPercentage());
      }
      else {
        Printf("Error while writing dataset %s", dsUri.Data());
      }
    }

    if (newFc) {
      delete newFc;
    }

  }

  if (mgr) {
    delete mgr;
  }
  delete listOfDs;

  if (outList.is_open()) {
    outList.close();
  }
}

/** Shows on the screen the list of datasets that match the search mask.
 */
void afShowListOfDs(const char *dsMask = "/*/*", Bool_t fast = kTRUE) {

  TList *dsList = _afGetListOfDs(dsMask);

  TObjString *nameObj;
  TIter i(dsList);
  Int_t count = 0;

  TDataSetManagerFile *mgr = NULL;
  if (!_afProofMode()) mgr = _afCreateDsMgr();

  if (fast) {

    Printf("   # |                    dataset name                     ");
    Printf("-----+-----------------------------------------------------");

    while ( (nameObj = dynamic_cast<TObjString *>(i.Next())) )
      Printf("%4d | %-50s", ++count, nameObj->String().Data());

  }
  else {

    TString um;
    Double_t sz;

    Printf("   # |                    dataset name                    | files |"
      "    tree    |    size    | staged |  cor  ");
    Printf("-----+----------------------------------------------------+-------+"
      "------------+------------+--------+-------");

    while ( (nameObj = dynamic_cast<TObjString *>(i.Next())) ) {
      TFileCollection *fc;
      if (mgr) fc = mgr->GetDataSet(nameObj->String().Data());
      else fc = gProof->GetDataSet(nameObj->String().Data());

      if (!fc) {
        Printf("%4d | %-50s | problems fetching dataset information!", ++count,
          nameObj->String().Data());
      }
      else {
        _afNiceSize(fc->GetTotalSize(), um, sz);

        Float_t stg = fc->GetStagedPercentage();
        Float_t cor = fc->GetCorruptedPercentage();

        TString stg_str;
        TString cor_str;

        if (stg == 0.) stg_str = "  --  ";
        else stg_str = Form("%5.1f%%", stg);

        if (cor == 0.) cor_str = "  --  ";
        else cor_str = Form("%5.1f%%", cor);

        const char *treeName = fc->GetDefaultTreeName();
        if (!treeName) treeName = "";

        Printf("%4d | %-50s | %5lld | %-10s | %6.1lf %s | %s | %s",
          ++count, nameObj->String().Data(), fc->GetNFiles(), treeName, sz,
          um.Data(), stg_str.Data(), cor_str.Data());
        delete fc;
      }
    }

  }

  Printf(">> There are %d dataset(s) matching your criteria",
    dsList->GetSize());

  if (mgr) delete mgr;
  delete dsList;
}

/** A shortcut to mark every file on a specified dataset mask as nonstaged and
 *  noncorrupted.
 *
 *  Also, only the last URL of each file is kept, which should be the
 *  originating one.
 *
 *  If the dataset was staged by afdsmgrd <= v0.1.7, then you can set
 *  recoverAliEnUrl = kTRUE to try to restore the originating alien:// URL.
 */
void afResetDs(const char *dsMask = "/*/*", Bool_t recoverAliEnUrl = kFALSE) {
  TString opts = "keeplast:";
  if (recoverAliEnUrl) opts.Append("alien");
  afMarkUrlAs("*", "scm", dsMask, "", opts);
}

/** A shortcut to prepend the redirector path of the file (read from the gEnv)
 *  to every file in the given dataset(s). No further processing is done.
 */
void afPrependRedirUrl(const char *dsMask = "/*/*") {
  afMarkUrlAs("*", "", dsMask, "", "redir");
}

/** Removes a dataset from the disk. Files associated to the dataset are not
 *  removed.
 */
void afRemoveDs(TString dsUri) {
  if (_afProofMode()) {
    gProof->RemoveDataSet(dsUri.Data());
  }
  else {
    TString path = Form("%s/%s.root", gEnv->GetValue("af.dspath", "/tmp"),
      dsUri.Data());
    if (gSystem->Unlink(path.Data()) < 0) {
      Printf("Can't remove %s from disk", path.Data());
    }
    else {
      Printf("Dataset removed: %s", dsUri.Data());
    }
  }
}

/** Fills metadata information inside the TFileInfo for the given dataset or
 *  list of datasets.
 *
 *  Beware that scanning implies a TFile::Open(). AliEn connection is opened
 *  automatically if needed.
 *
 *  Options may contain, separated by colons:
 *
 *   - rescan    : by default only files with no metadata are rescanned; if
 *                 rescan, every file is rescanned
 *
 *   - corrupt   : files which can't be scanned are marked as corrupted; by
 *                 default they are just ignored
 *
 *   - setstaged : updated files (or all files, if the "rescan" option is set)
 *                 are marked as staged
 *
 *   - fast      : try to read number of events from the ESD tag file, if
 *                 available, or fallback on standard, slower file opening
 *
 *  If corruptIfFail, files whose metadata can't be obtained are marked as
 *  corrupted.
 */
void afFillMetaData(TString dsMask, TString options = "") {

  // Parse options
  Bool_t rescanAll = kFALSE;
  Bool_t corruptIfFail = kFALSE;
  Bool_t setStaged = kFALSE;
  Bool_t fastScan = kFALSE;

  options.ToLower();
  TObjArray *tokOpts = options.Tokenize(":");
  TIter opt(tokOpts);
  TObjString *oopt;

  while (( oopt = dynamic_cast<TObjString *>(opt.Next()) )) {
    TString &sopt = oopt->String(); 
    if (sopt == "rescan") {
      rescanAll = kTRUE;
    }
    else if (sopt == "corrupt") {
      corruptIfFail = kTRUE;
    }
    else if (sopt == "setstaged") {
      setStaged = kTRUE;
    }
    else if (sopt == "fast") {
      fastScan = kTRUE;
    }
    else {
      Printf("Warning: ignoring unknown option \"%s\"", sopt.Data());
    }
  }

  Int_t saveEvery = 10;

  TList *dsList = _afGetListOfDs(dsMask);

  TObjString *dsUriObj;
  TIter i(dsList);

  TDataSetManagerFile *mgr = NULL;
  if (!_afProofMode()) {
    mgr = _afCreateDsMgr();
  }

  while ( (dsUriObj = dynamic_cast<TObjString *>(i.Next())) ) {

    const char *dsUri = dsUriObj->String().Data();
    TFileCollection *fc;

    // Gets the current dataset, both in PROOF and "offline" mode
    if (mgr) {
      fc = mgr->GetDataSet(dsUri);
    }
    else {
      fc = gProof->GetDataSet(dsUri);
    }

    // Dataset may not exist anymore!
    if (!fc) {
      Printf("*** Error opening dataset URI %s, skipping ***", dsUri);
      continue;
    }

    Printf("*** Processing dataset %s ***", dsUri);

    // Loop over all files in dataset
    Int_t nChanged = 0;
    Int_t nTotal = fc->GetNFiles();
    Int_t nCount = 0;
    const char *defTree = fc->GetDefaultTreeName();

    TIter j(fc->GetList());
    TFileInfo *fi;

    while (( fi = dynamic_cast<TFileInfo *>(j.Next()) )) {

      nCount++;

      printf("[% 4d/% 4d] [....] %s", nCount, nTotal,
        fi->GetCurrentUrl()->GetUrl());
      cout << flush;

      TString status;
      Int_t nEvts = -1;
      TFileInfoMeta *meta;

      // Metadata already present and told not to rescan all?
      if ((fi->GetMetaData()) && (!rescanAll)) {
        status = "\033[34mSKIP\033[m";  // blue
        nEvts = 0;
      }
      else {

        if (!_afFillMetaDataFile( fi, fastScan, defTree, kTRUE )) {
          if (corruptIfFail) {
            fi->SetBit(TFileInfo::kCorrupted);
            status = "\033[31mCORR\033[m";  // red
            nChanged++;
          }
          else {
            status = "\033[31mFAIL\033[m";  // red
          }
        }
        else {
          if (setStaged) {
            fi->SetBit(TFileInfo::kStaged);
          }
          status = "\033[32m OK \033[m";  // green
          nEvts = 0;
          nChanged++;
        }      
      }

      // Prints out the status (skipped, marked as corrupted, failed, ok) and
      // some other info
      printf("\r[% 4d/% 4d] [%s] %s", nCount, nTotal, status.Data(),
        fi->GetCurrentUrl()->GetUrl());

      // Print event numbers in default tree, if possible
      if ((nEvts != -1) && (defTree) && (meta = fi->GetMetaData(defTree))) {
        nEvts = meta->GetEntries();
        Printf(" (%d evts)", nEvts);
      }
      else {
        cout << endl;
      }

      // Saves sometimes
      if ((nChanged) && ((nChanged % saveEvery) == 0)) {
        fc->Update();
        _afSaveDs(dsUri, fc, kTRUE);
        nChanged = 0;
      }

    }

    // Save, if necessary
    if (nChanged > 0) {
      fc->Update();
      _afSaveDs(dsUri, fc, kTRUE);
    }

    delete fc;
  }

  delete dsList;

  // Deletes the manager, only if created
  if (mgr) { delete mgr; }

}

/** Creates a dataset from AliEn find. See http://aaf.cern.ch/node/160 for
 *  instructions.
 */
void afDataSetFromAliEn(TString basePath, TString fileName,
  TString postFindFilter, TString anchor, TString treeName,
  TString runList, TString dsPattern, TString options = "") {

  // Parse options
  Bool_t dryRun    = kTRUE;
  Bool_t setStaged = kFALSE;
  Bool_t addRedir  = kFALSE;
  Bool_t verify    = kFALSE;
  Bool_t aliEnCmd  = kFALSE;
  Bool_t updateDs  = kFALSE;
  Bool_t autoArch  = kTRUE;
  Bool_t printEnts = kFALSE;

  options.ToLower();
  TObjArray *tokOpts = options.Tokenize(":");
  TIter opt(tokOpts);
  TObjString *oopt;

  while (( oopt = dynamic_cast<TObjString *>(opt.Next()) )) {
    TString &sopt = oopt->String(); 
    if (sopt == "commit") {
      dryRun = kFALSE;
    }
    else if (sopt == "setstaged") {
      setStaged = kTRUE;
    }
    else if (sopt == "cache") {
      addRedir = kTRUE;
    }
    else if (sopt == "verify") {
      verify = kTRUE;
    }
    else if (sopt == "aliencmd") {
      aliEnCmd = kTRUE;
    }
    else if (sopt == "update") {
      updateDs = kTRUE;
    }
    else if (sopt == "noautoarch") {
      autoArch = kFALSE;
    }
    else if (sopt == "print") {
      printEnts = kTRUE; // for debug
    }
    else {
      Printf("Warning: ignoring unknown option \"%s\"", sopt.Data());
    }
  }

  if (dryRun) {

    Printf("\nWarning: dry run is enabled by default. Use \"commit\" to save "
      "changes if you are sure of what you are doing\n");

    if (verify) {
      Printf("Warning: can't verify dataset on dry run");
      verify = kFALSE;
    }
    if (addRedir) {
      Printf("Warning: can't add redirector URL on dry run");
      addRedir = kFALSE;
    }
  }

  // Warning on changed behaviour
  if (fileName.EndsWith(".zip") && autoArch) {
    Printf("\nFatal: you specified an archive as fileName, but the option " \
      "\"noautoarch\" is not set: from AFDSUtils v0.9.7 archive names are " \
      "automatically added, and you are probably expecting the old behavior. " \
      "Please refer to http://aaf.cern.ch/node/160 for up-to-date howtos.\n");
    return;
  }

  Ssiz_t idx;

  if (_afProofMode()) {

    // In PROOF mode, complete dataset name with group and user, if not given
    idx = dsPattern.Index("/");
    if (idx == -1) {
      TString temp;
      temp.Form("/%s/%s/%s", gProof->GetGroup(), gProof->GetUser(),
        dsPattern.Data());
      dsPattern = temp;
    }

  }

  // Parse run list
  vector<Int_t> *runNumsPtr = _afExpandRunList(runList);
  vector<Int_t> &runNums = *runNumsPtr;
  UInt_t nRuns = runNums.size();

  // For each run (or only once if no runlist was given)
  for (UInt_t i=0; (i < nRuns) || (nRuns == 0) ; i++) {

    TString dsName, basePathRun, postFindFilterRun;

    if (nRuns == 0) {
      dsName = dsPattern;
      basePathRun = basePath;
      postFindFilterRun = postFindFilter;
    }
    else {
      dsName = _afReplaceRunPadded(dsPattern, runNums[i], 9);
      basePathRun = _afReplaceRunPadded(basePath, runNums[i]);
      postFindFilterRun = _afReplaceRunPadded(postFindFilter, runNums[i]);
    }

    // Echoes the corresponding AliEn find command. Please note that in our
    // TGrid::Query() we use '*' as wildcards, while in aliensh we should
    // replace them with '%'
    if (aliEnCmd) {

      TString cmdFind;
      cmdFind.Form("find %s %s", basePathRun.Data(), fileName.Data());
      cmdFind.ReplaceAll("*", "%");

      if (!postFindFilter.IsNull()) {
        Printf("\033[33maliensh>\033[m %s | grep -E '%s'",
          cmdFind.Data(), postFindFilterRun.Data());
      }
      else {
        Printf("\033[33maliensh>\033[m %s", cmdFind.Data());
      }
    }

    // Run AliEn find: output on a collection
    TFileCollection *fc = _afAliEnFind(basePathRun, fileName, anchor, treeName,
      postFindFilterRun, autoArch, printEnts);
    if (fc == NULL) {
      delete runNumsPtr;
      Printf("Creation of datasets from AliEn aborted.");
      return;
    }

    // Set staged bit if requested (to avoid real staging)
    if (setStaged) {
      fc->SetBitAll(TFileInfo::kStaged);
      fc->Update();
    }

    TString opStatus;
    Bool_t wasSaved = kFALSE;

    // Zero files?
    if (fc->GetNFiles() == 0LL) {
      opStatus = "\033[33mno results, skipped\033[m";
    }
    else if (!dryRun) {

      wasSaved = _afSaveDs(dsName, fc, kFALSE, kTRUE, updateDs);
      //                               ^^^^^^  ^^^^^  ^^^^^^^^
      //                               overwr  quiet  _update_

      if (wasSaved) {
        if (updateDs) opStatus = "\033[32mupdated\033[m";
        else opStatus = "\033[32msaved\033[m";
      }
      else {
        opStatus = "\033[31merror saving\033[m";
      }
    }
    else {
      opStatus = "\033[34mdry run\033[m";
    }

    // Print: dataset name, num. files, total size, success|failure
    TString um;
    Double_t fmtSize;
    _afNiceSize(fc->GetTotalSize(), um, fmtSize);
    Printf("%-50s : %6llu files, size: %6.1lf %s [%s]", dsName.Data(),
      (ULong64_t)fc->GetNFiles(), fmtSize, um.Data(), opStatus.Data());

    // If requested, "verify" the dataset (fast mode) and "cache"
    if (wasSaved) {
      if (verify) afFillMetaData(dsName, "fast");
      if (addRedir) afPrependRedirUrl(dsName);
    }

    if (nRuns == 0) break;

  }

  Printf("\nPay attention: revise the settings you are using:\n");
  afPrintSettings();
  cout << endl;

  // Delete list of runs
  delete runNumsPtr;
}

/** Function to quickly create datasets from official data stored on AliEn in
 *  a "standard" path. Examples of what is considered "standard" follows.
 *
 *  ESDs from real data:
 *    /alice/data/2010/LHC10h/000139104/ESDs/pass1/...
 *
 *  AODs from real data:
 *    /alice/data/2010/LHC10h/000139104/ESDs/pass1/AOD040/...
 *
 *  ESDs from simulation:
 *    /alice/sim/LHC10h1/137045/...
 *
 *  AODs from simulation (with or without year, autodetected):
 *    /alice/sim/LHC11d5/118506/AOD063/...
 *    /alice/sim/2011/LHC11f5/139517/AOD081/...
 *
 * For the cases uncovered by this function, use afDataSetFromAliEn().
 */
void afDataSetQuick(Bool_t sim = kFALSE, TString period = "LHC10h",
  TString runRange = "139104-139107,139306", Bool_t esd = kTRUE,
  TString pass = "pass1", Int_t aodNum = 40, TString options = "") {

  TString basePath;
  TString temp;
  Int_t year = 0;

  // Get year from LHC period
  if (period.Length() >= 5) {
    temp = period[3];
    temp.Append(period[4]);
    year = temp.Atoi();
    if (year > 90) year += 1900;
    else year += 2000;
  }

  // First part of the path
  if (sim) {
    // Check whether this period is in /alice/sim/<period> or in
    // /alice/sim/<year>/<period> and act properly, since naming convention is
    // unclear!
    if (!_afAliEnConnect()) return;

    basePath = Form("/alice/sim/%s", period.Data());  // no year
    if (!gGrid->Cd(basePath.Data())) {
      basePath = Form("/alice/sim/%d/%s", year, period.Data());
    }
    basePath.Append("/<RUN6>");

    // Sim
    if (!esd) {
      temp = Form("/AOD%03d", aodNum);
      basePath.Append(temp);
    }
  }
  else {
    // Parse the pass string: if it starts with a number, prepend "pass"
    if ((pass[0] >= '0') && (pass[0] <= '9')) pass.Prepend("pass");

    // Data
    basePath = Form("/alice/data/%d/%s/<RUN9>/ESDs/%s", year, period.Data(),
      pass.Data());
    if (esd) {
      basePath.Append("/*.*");
    }
    else {
      temp = Form("/AOD%03d", aodNum);
      basePath.Append(temp);
    }
  }

  // File name and tree name
  TString fileName;
  TString treeName;

  if (esd) {
    fileName = "AliESDs.root";
    treeName = "/esdTree";
  }
  else {
    fileName = "AliAOD.root";
    treeName = "/aodTree";
  }

  // Dataset name
  TString dsPattern;
  TString filter;

  if (_afProofMode()) {
    // PROOF: normal user
    dsPattern = Form("%s_%s_<RUN>_",
      (sim) ? "sim" : "data",
      period.Data());
  }
  else {
    // Direct access: full dataset path
    dsPattern = Form("/alice/%s/%s_<RUN>_",
      (sim) ? "sim" : "data",
      period.Data());
  }

  if (esd) {
    dsPattern.Append("ESDs");
  }
  else {
    temp = Form("AOD%03d", aodNum);
    dsPattern.Append(temp);
    filter = "AOD[0-9]{3}/[0-9]{3,4}";  // only AODnnn/(m)mmm: skip StageX dirs
  }

  if (!sim) {

    // Data has passes: append pass number to the end of dataset name
    TPMERegexp passRe("^([a-zA-Z]*)pass([0-9]+.*)$");
    if (passRe.Match(pass) == 3) {
      TString passAppended;
      passAppended.Form("_%sp%s", passRe[1].Data(), passRe[2].Data());
      dsPattern.Append(passAppended);
    }
    else {
      dsPattern.Append("_");
      dsPattern.Append(pass);
    }

  }

  // Invoke the complete function
  afDataSetFromAliEn(basePath, fileName, filter, "", treeName, runRange,
    dsPattern, options);

}

/** Interactive interface to afDataSetQuick().
 */
void afDataSetWizard() {

  Bool_t sim;
  Bool_t esd;
  TString period;
  TString runRange;
  TString options;
  TString pass;
  TString temp;
  Int_t aodNum = -1;

  // Sim/data?
  while (kTRUE) {
    temp = _afGetLine("Simulation or data [s/d]?");
    temp.ToLower();
    if (temp == "s") {
      sim = kTRUE;
      break;
    }
    else if (temp == "d") {
      sim = kFALSE;
      break;
    }
  }

  // LHC Period?
  period = _afGetLine("LHC period (e.g.: LHC10h)?");

  // Run range?
  runRange = _afGetLine("Run range (e.g. 139104-139107,139306)?");

  // ESDs/AOD?
  while (kTRUE) {
    temp = _afGetLine("ESDs or AODs [e/a]?");
    temp.ToLower();
    if (temp == "e") {
      esd = kTRUE;
      break;
    }
    else if (temp == "a") {
      esd = kFALSE;

      // AOD number?
      temp = _afGetLine("AOD number (e.g, 040)?");
      aodNum = temp.Atoi();
      break;
    }
  }

  // Pass?
  if (!sim) {
    pass = _afGetLine("Pass (e.g., 1, pass1, 1_HLT, pass1_HLT, cpass1...)?");
  }

  // Options?
  options = _afGetLine("Colon-separated options (e.g., commit:aliencmd)?");

  //
  // End of user input
  //

  afDataSetQuick(sim, period, runRange, esd, pass, aodNum, options);

}

/** Writes the contents of the specified dataset (or pattern of datasets) on
 *  text files placed in the specified outDir. outDir is created if nonexistent,
 *  and only the nUrl'th URL of each entry is written to the text file. If nUrl
 *  is a "big" number, it represents the last URL, while 1 represents the first
 *  one. If aliEnToRedirector is kTRUE and the selected URL is of alien:// type,
 *  it is converted to the corresponding redirector URL before being written on
 *  the list. If onlyGood is kTRUE, only staged and noncorrupted files are
 *  considered.
 */
void afDsToPlainText(TString dsMask, TString outDir = "/tmp", UInt_t nUrl = 999,
  Bool_t aliEnToRedirector = kTRUE, Bool_t onlyGood = kTRUE) {

  if (nUrl == 0) {
    Printf("Valid values for nUrl start from 1 for the first URL.");
    return;
  }

  TDataSetManagerFile *mgr = NULL;
  if (!_afProofMode()) mgr = _afCreateDsMgr();

  TList *listOfDs = _afGetListOfDs(dsMask);

  TString redirPtn = gEnv->GetValue("af.redirurl", "root://localhost:1234/$1");
  TPMERegexp alienRe("^alien:\\/\\/(.*)$");

  // Objects to eliminate unwanted characters in dataset name
  TString substPtn = "$2$3_";
  TPMERegexp re("(^/|(.))([^/]*)/");

  // Create output directory
  gSystem->mkdir(outDir.Data(), kTRUE);  // kTRUE means "recursive" (i.e. "-p")

  TIter i(listOfDs);
  TObjString *dsUriObj;
  while ( (dsUriObj = dynamic_cast<TObjString *>(i.Next())) ) {

    TString ofName = dsUriObj->String();
    while (re.Substitute(ofName, substPtn)) {}
    ofName = Form("%s/%s.txt", outDir.Data(), ofName.Data());

    ofstream of( ofName.Data() );
    if (!of) {
      Printf("Can't write dataset %s on %s, skipping",
        dsUriObj->String().Data(), ofName.Data());
      continue;
    }
    else Printf("Writing dataset %s on %s...", dsUriObj->String().Data(),
      ofName.Data());

    TFileCollection *fc;
    if (mgr) fc = mgr->GetDataSet(dsUriObj->String().Data());
    else fc = gProof->GetDataSet(dsUriObj->String().Data());

    TIter j(fc->GetList());
    TFileInfo *fi;
    UInt_t thisNUrl;
    while ( (fi = dynamic_cast<TFileInfo *>(j.Next())) ) {

      if (onlyGood) {
        if ((fi->TestBit(TFileInfo::kCorrupted)) ||
          (!fi->TestBit(TFileInfo::kStaged))) continue;
      }

      TUrl *url = NULL;

      thisNUrl = fi->GetNUrls();
      if (nUrl <= thisNUrl) thisNUrl = nUrl;

      fi->ResetUrl();
      for (UInt_t k=1; k<=thisNUrl; k++) url = fi->NextUrl();

      if (!url) continue;

      TString buf = url->GetUrl();
      if (aliEnToRedirector && (strcmp(url->GetProtocol(), "alien") == 0)) {
        alienRe.Substitute(buf, redirPtn);
      }

      of << buf.Data() << endl;
    }

    of.close();

  }

  if (mgr) delete mgr;
  delete listOfDs;
}

/** Performs auto-detection of AFDSUtils settings, also by using environment
 *  variables of AAFs.
 */
void afAutoSettings() {
  Bool_t local = kTRUE;
  if (gProof) {
    local = kFALSE;

    // PROOF connection already opened
    afSetProofMode(kTRUE);
    TString temp = Form("%s:%s@%s", gProof->GetUser(), gProof->GetGroup(),
      gProof->GetMaster());
    afSetProofUserHost(temp.Data());

    // Remote datasets repository
    const char *user = gProof->GetUser();
    temp.Form("/alice/cern.ch/user/%c/%s/aaf_datasets", *user, user);
    afSetAliEnDsRepo(temp.Data());

  }
  else {
    // Assume local processing
    afSetProofMode(kFALSE);
  }

  // Path of datasets
  TString *dsPath = _afGetEnvVar("ALICE_PROOF_AAF_PROOF_DIR", local);
  if (dsPath) {
    dsPath->Append("/dataset");
    afSetDsPath(dsPath->Data());
    delete dsPath;
  }

  // Hostname of the redirector
  TString *redir = _afGetEnvVar("ALICE_PROOF_AAF_XROOTD_REDIRECTOR", local);
  if (redir) {
    redir->Prepend("root://");
    redir->Append("/$1");
    afSetRedirUrl(*redir);
    delete redir;
  }
}

/** This function clones a list of datasets, specified with the usual mask, to
 *  the user's datasets directory on AliEn, defined by a gEnv variable. By
 *  default, files on AliEn won't be overwritten.
 *
 *  All datasets pushed to AliEn are cleaned up first: every URL is removed
 *  (except the originating one) and files are marked as Staged (Corrupted bit
 *  is not touched).
 */
void afCloneDataSetsToAliEn(TString dsMask, Bool_t overwrite = kFALSE) {

  // Open connection to AliEn
  if (!_afAliEnConnect()) return;

  TDataSetManagerFile *mgr = NULL;
  if (!_afProofMode()) mgr = _afCreateDsMgr();

  TList *listOfDs = _afGetListOfDs(dsMask);

  // Warning: from this point on, we back up current settings and restore them
  // at the end. New settings will be on a temporary local repository.
  Bool_t  oldProofMode = gEnv->GetValue("af.proofmode", 1);
  TString oldDsPath    = gEnv->GetValue("af.dspath", "/pool/dataset");

  // A new, temporary repository is created
  TString tmpDsRepoPath;
  TDataSetManagerFile *tmpMgr = _afCreateDsMgr(&tmpDsRepoPath, kTRUE);

  // Point default settings to the temporary repository
  afSetDsPath(tmpDsRepoPath.Data());
  afSetProofMode(kFALSE);
  //afPrintSettings();

  // This list was obtained with "old" settings
  Int_t nLocalProc = 0;
  Int_t nLocalSucc = 0;
  TIter i(listOfDs);
  TObjString *dsUriObj;
  while ( (dsUriObj = dynamic_cast<TObjString *>(i.Next())) ) {

    TString dsUri = dsUriObj->String();
    TFileCollection *fc = NULL;
    nLocalProc++;

    if (mgr) fc = mgr->GetDataSet(dsUri.Data());
    else fc = gProof->GetDataSet(dsUri.Data());

    if (!fc) {
      Printf("Warning: can't obtain dataset %s: skipped", dsUri.Data());
      continue;
    }

    // Dataset is modified in memory
    Int_t nChanged = _afMarkUrlOfCollectionAs("*", "S", fc, "", "keeplast");
    if (nChanged < 0) {
      delete fc;
      continue;
    }

    // Save file to the temporary repository (quiet: false)
    if (!_afSaveDs(dsUri, fc, kTRUE, kFALSE)) {
      delete fc;
      continue;
    }

    // Arrived at this point, we have a success
    nLocalSucc++;
    delete fc;

  }

  delete listOfDs;
  if (mgr) delete mgr;

  // AliEn repository
  TString aliEnBase = gEnv->GetValue("af.aliendsrepo", "/alice/cern.ch/tmp");

  // Get list of datasets to upload on AliEn: all datasets in temporary dir
  Int_t nAliEnProc = 0;
  Int_t nAliEnSucc = 0;
  listOfDs = _afGetListOfDs("/*/*");
  TIter j(listOfDs);

  while ( (dsUriObj = dynamic_cast<TObjString *>(j.Next())) ) {

    nAliEnProc++;

    TString dsUri = dsUriObj->String();
    TString aliEnFullPath;
    TString aliEnFullDir;
    TString localFullPath;
    aliEnFullPath.Form("alien://%s%s.root", aliEnBase.Data(), dsUri.Data());
    aliEnFullDir = gSystem->DirName(aliEnFullPath.Data());
    localFullPath.Form("%s/%s.root", tmpDsRepoPath.Data(), dsUri.Data());

    // Safety check: don't overwrite accidentaly
    if ((!overwrite) && (!gSystem->AccessPathName(aliEnFullPath.Data()))) {
      Printf("Warning: I won't overwrite %s: skipped", aliEnFullPath.Data());
      continue;
    }

    // Time to copy: check if the directory is there, create it in case
    if (gSystem->AccessPathName(aliEnFullDir.Data())) {
      gGrid->Mkdir(&(aliEnFullDir.Data())[8], "-p");  // skip alien://
    }

    // Copy the file
    Printf("Transferring dataset %s...", dsUri.Data());
    Bool_t ok = TFile::Cp(localFullPath.Data(), aliEnFullPath.Data());
    if (ok) nAliEnSucc++;

  }

  delete listOfDs;

  // Restore old settings
  afSetDsPath(oldDsPath.Data());
  afSetProofMode(oldProofMode);

  // Print information on screen
  Printf("Transfers to local repo: %d OK / %d total", nLocalSucc, nLocalProc);
  Printf("Transfers to AliEn repo: %d OK / %d total", nAliEnSucc, nAliEnProc);

  // Get rid of the local temporary repository
  delete tmpMgr;
  gSystem->Exec(Form("rm -rf -- '%s'", tmpDsRepoPath.Data()));

}

/** This function clones some datasets specified with the usual mask from the
 *  default dataset repository for the current AliEn user (a gEnv variable) to
 *  the currently active dataset settings.
 *
 *  Datasets are reset before being written to the local destination. In
 *  particular, corrupted bit will not touched, the redirector is appended and
 *  they are marked as Staged in order to prevent staging without manual
 *  "unlocking".
 *
 *  Options might be, as a string: "overwrite", to overwrite existing datasets
 *  in the destination repository, and "update" to update, if needed, existing
 *  datasets with new information from AliEn datasets.
 *
 *  By default, destination datasets are neither updated not overwritten.
 */
void afCloneDataSetsFromAliEn(TString dsMask, TString options = "") {

  // Check parameters
  Bool_t overwrite = kFALSE;
  Bool_t update = kFALSE;

  if (options == "update") {
    overwrite = kTRUE;
    update = kTRUE;
  }
  else if (options == "overwrite") {
    overwrite = kTRUE;
  }

  // Open connection to AliEn
  if (!_afAliEnConnect()) return;

  TDataSetManagerFile *mgr = NULL;
  if (!_afProofMode()) mgr = _afCreateDsMgr();

  // Warning: from this point on, we back up current settings and restore them
  // at the end. New settings will be on a temporary local repository.
  Bool_t  oldProofMode = gEnv->GetValue("af.proofmode", 1);
  TString oldDsPath    = gEnv->GetValue("af.dspath", "/pool/dataset");

  // A new, temporary repository is created
  TString tmpDsRepoPath;
  TDataSetManagerFile *tmpMgr = _afCreateDsMgr(&tmpDsRepoPath, kTRUE);

  // Point default settings to the temporary repository
  afSetDsPath(tmpDsRepoPath.Data());
  afSetProofMode(kFALSE);
  //afPrintSettings();

  // Emulate match pattern of TDataSetManager
  TString dsMatchPtn = dsMask(0, dsMask.Index("*"));

  // Obtain a list of datasets from AliEn, in a TFileCollection
  TString aliEnBase = gEnv->GetValue("af.aliendsrepo", "/alice/cern.ch/tmp");
  TFileCollection *aliEnDsFc = _afAliEnFind(aliEnBase, "*.root", "", "", "");

  TIter dsi(aliEnDsFc->GetList());
  TFileInfo *fi;

  Int_t nAliEnProc = 0;
  Int_t nAliEnSucc = 0;

  while ( (fi = dynamic_cast<TFileInfo *>(dsi.Next())) ) {

    TString aliEnFullPath = fi->GetCurrentUrl()->GetUrl();
    Int_t beginIdx = aliEnFullPath.Index(aliEnBase.Data())+aliEnBase.Length();
    TString dsUri = aliEnFullPath(beginIdx, aliEnFullPath.Length()-beginIdx-5);
    TString localFullPath;
    TString localFullDir;
    localFullPath.Form("%s/%s.root", tmpDsRepoPath.Data(), dsUri.Data());
    localFullDir = gSystem->DirName(localFullPath.Data());

    if (dsUri.BeginsWith(dsMatchPtn)) {
      nAliEnProc++;
      Printf("Fetching dataset %s from AliEn...", dsUri.Data());
      gSystem->mkdir(localFullDir.Data(), kTRUE);
      Bool_t ok = TFile::Cp(aliEnFullPath.Data(), localFullPath.Data());
      if (ok) nAliEnSucc++;
    }
    else {
      //Printf(">> %s: skip", dsUri.Data());
      continue;
    }

  }

  delete aliEnDsFc;
  aliEnDsFc = NULL;

  // Get list of datasets matching the mask in the local temporary repository
  Int_t nLocalProc = 0;
  Int_t nLocalSucc = 0;
  TList *listOfDs = _afGetListOfDs(dsMask);
  TIter i(listOfDs);
  TObjString *dsUriObj;

  // Restore old settings
  afSetDsPath(oldDsPath.Data());
  afSetProofMode(oldProofMode);
  //afPrintSettings();

  while ( (dsUriObj = dynamic_cast<TObjString *>(i.Next())) ) {

    nLocalProc++;

    TString dsUri = dsUriObj->String();
    TFileCollection *fc = tmpMgr->GetDataSet(dsUri.Data());

    if (!fc) {
      Printf("Warning: can't read dataset %s from temp repository: skipped",
        dsUri.Data());
      continue;
    }

    // Mark all files as Staged, do not touch Corrupted bit, keep only last URL
    // and add redirector's URL as per current settings
    Int_t nChanged = _afMarkUrlOfCollectionAs("*", "S", fc, "",
      "keeplast:redir");
    if (nChanged < 0) {
      delete fc;
      continue;
    }

    // Save file to the final repository (quiet: false)
    if (!_afSaveDs(dsUri, fc, overwrite, kFALSE, update)) {
      delete fc;
      continue;
    }

    // Arrived at this point, we have a success
    nLocalSucc++;
    delete fc;

  }

  delete listOfDs;

  // Cleanup of all dataset repositories
  if (mgr) delete mgr;
  delete tmpMgr;
  gSystem->Exec(Form("rm -rf -- '%s'", tmpDsRepoPath.Data()));

  // Print information on screen
  Printf("Transfers from AliEn repo: %d OK / %d total", nAliEnSucc, nAliEnProc);
  Printf("Transfers from local repo: %d OK / %d total", nLocalSucc, nLocalProc);

}

/* ========================================================================== *
 *                                ENTRY POINT                                 *
 * ========================================================================== */

/** Init function: it just prints a welcome message with the default parameters.
 */
void afdsutil() {
  cout << endl;
  Printf("Dataset management utilities -- "
    "by Dario Berzano <dario.berzano@cern.ch>");
  Printf("Bugs: https://savannah.cern.ch/projects/aaf/");
  Printf("Available functions start with \"af\", use autocompletion to list.");
  cout << endl;

  Bool_t autoSettings = gEnv->GetValue("af.autosettings", 1);
  if (autoSettings) afAutoSettings();
  else Printf("Warning: auto-detection of settings disabled by user\n");

  afPrintSettings();
  cout << endl;
}
