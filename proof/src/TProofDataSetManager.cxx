// @(#)root/base:$Id$
// Author: Jan Fiete Grosse-Oetringhaus, 04.06.07

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofDataSetManager                                                 //
//                                                                      //
// This class contains functions to handle datasets in PROOF            //
// It is the layer between TProofServ and the file system that stores   //
// the datasets.                                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TProofDataSetManager.h"

#include "Riostream.h"
#include "TEnv.h"
#include "TFileCollection.h"
#include "TFileInfo.h"
#include "TFile.h"
#include "TFileStager.h"
#include "TLockFile.h"
#include "TMap.h"
#include "TRegexp.h"
#include "TMD5.h"
#include "TMessage.h"
#include "TSystem.h"
#include "TError.h"
#include "TPRegexp.h"
#include "TVirtualMonitoring.h"
#include "TObjArray.h"
#include "THashList.h"
#include "TKey.h"
#include "TTree.h"
#include "TParameter.h"
#include "TSocket.h"
#include "TProof.h"
#include "TProofServ.h"
#include "TUri.h"


// Dataset lock file
const char *TProofDataSetManager::fgDataSetLockFile   = "/tmp/dataset-lock";
// Limit in seconds after a lock automatically expires
Int_t        TProofDataSetManager::fgLockFileTimeLimit = 120;
// Name for common datasets
const char*  TProofDataSetManager::fgCommonDataSetTag   = "COMMON";

ClassImp(TProofDataSetManager)

//_____________________________________________________________________________
TProofDataSetManager::TProofDataSetManager(const char *dataSetDir,
                                           const char *group,
                                           const char *user, Bool_t silent)
                     : fSilent(silent), fDataSetDir(dataSetDir), fGroup(group),
                       fUser(user), fMSSUrl(), fCommonUser(), fCommonGroup(),
                       fCheckQuota(kFALSE), fGroupQuota(), fGroupUsed(),
                       fUserUsed(), fNTouchedFiles(0), fNOpenedFiles(0),
                       fNDisappearedFiles(0), fMTimeGroupConfig(-1)
{
   //
   // Main constructor

   fGroupQuota.SetOwner();
   fGroupUsed.SetOwner();
   fUserUsed.SetOwner();

   // Read config file
   ReadGroupConfig(gEnv->GetValue("ProofDataSetManager.GroupFile",""));

   // Entry-point to the mass storage system
   fMSSUrl = gEnv->GetValue("ProofDataSetManager.MSSUrl","");
}

//______________________________________________________________________________
Bool_t TProofDataSetManager::ReadGroupConfig(const char *cf)
{
   // Read group config file 'cf'.
   // If cf == 0 re-read, if changed, the file pointed by fGroupConfigFile .
   //
   // expects the following directives:
   // Group definition:
   //   group <groupname> <user>+
   // disk quota
   //   property <groupname> diskquota <quota in GB>

   // Validate input
   FileStat_t st;
   if (!cf || (strlen(cf) <= 0) || !strcmp(cf, fGroupConfigFile.Data())) {
      // If this is the first time we cannot do anything
      if (fGroupConfigFile.IsNull()) {
         Error("ReadGroupConfig", "path to config file indefined - exit");
         return kFALSE;
      }
      // Check if fGroupConfigFile has changed
      if (gSystem->GetPathInfo(fGroupConfigFile, st)) {
         Error("ReadGroupConfig", "could not stat %s", fGroupConfigFile.Data());
         return kFALSE;
      }
      if (st.fMtime <= fMTimeGroupConfig) {
         if (!fSilent)
            Info("ReadGroupConfig","file has not changed - do nothing");
         return kTRUE;
      }
   }

   // Either new file or the file has changed
   if (cf && (strlen(cf) > 0)) {
      // The file must exist and be readable
      if (gSystem->GetPathInfo(cf, st)) {
         Error("ReadGroupConfig", "could not stat %s", cf);
         return kFALSE;
      }
      if (!gSystem->TestPermissions(st, kReadPermission)) {
         Error("ReadGroupConfig", "cannot read %s", cf);
         return kFALSE;
      }
      // Ok
      fGroupConfigFile = cf;
      fMTimeGroupConfig = st.fMtime;
   }

   if (!fSilent)
      Info("ReadGroupConfig","reading group config from %s", cf);

   // Open the config file
   ifstream in;
   in.open(cf);
   if (!in.is_open()) {
      Error("ReadGroupConfig", "could not open config file %s", cf);
      return kFALSE;
   }

   // Container for the global common user
   TString tmpCommonUser;

   // Go through
   TString line;
   while (in.good()) {
      // Read new line
      line.ReadLine(in);
      // Parse it
      Ssiz_t from = 0;
      TString key;
      if (!line.Tokenize(key, from, " ")) // No token
         continue;
      // Parsing depends on the key
      if (key == "property") {
         // Read group
         TString grp;
         if (!line.Tokenize(grp, from, " ")) {// No token
            if (!fSilent)
               Info("ReadGroupConfig","incomplete line: '%s'", line.Data());
            continue;
         }
         // Read type of property
         TString type;
         if (!line.Tokenize(type, from, " ")) // No token
            continue;
         if (type == "diskquota") {
            // Read diskquota
            TString sdq;
            if (!line.Tokenize(sdq, from, " ")) // No token
               continue;
            if (sdq.IsDigit()) {
               Long64_t quota = (Long64_t) 1024 * 1024 * 1024 * sdq.Atoi();
               fGroupQuota.Add(new TObjString(grp),
                               new TParameter<Long64_t> ("group quota", quota));
            }
         } else if (type == "commonuser") {
            // Read common user for this group
            TString comusr;
            if (!line.Tokenize(comusr, from, " ")) // No token
               continue;

         }

      } else if (key == "dataset") {
         // Read type
         TString type;
         if (!line.Tokenize(type, from, " ")) {// No token
            if (!fSilent)
               Info("ReadGroupConfig","incomplete line: '%s'", line.Data());
            continue;
         }
         if (type == "diskquota") {
            // Read diskquota switch ("on"/"off")
            TString swt;
            if (!line.Tokenize(swt, from, " ")) // No token
               continue;
            if (!swt.CompareTo("on", TString::kIgnoreCase))
               fCheckQuota = kTRUE;
         } else if (type == "commonuser") {
            // Read global common user
            TString comusr;
            if (!line.Tokenize(comusr, from, " ")) // No token
               continue;
            fCommonUser = comusr;
         } else if (type == "commongroup") {
            // Read global common group
            TString comgrp;
            if (!line.Tokenize(comgrp, from, " ")) // No token
               continue;
            fCommonGroup = comgrp;
         }
      }
   }
   in.close();

   return kTRUE;
}

//______________________________________________________________________________
const char *TProofDataSetManager::GetDataSetPath(const char *group,
                                                 const char *user,
                                                 const char *dsName)
{
   //
   // Returns path of the indicated dataset
   // Contains a static TString for result. Copy result before using twice.

   if (strcmp(group, fgCommonDataSetTag) == 0)
     group = fCommonGroup;

   if (strcmp(user, fgCommonDataSetTag) == 0)
     user = fCommonUser;

   static TString result;
   result.Form("%s/%s/%s/%s.root", fDataSetDir.Data(), group, user, dsName);
   return result;
}

//______________________________________________________________________________
TFileCollection *TProofDataSetManager::GetDataSet(const char *uri)
{
   // Utility function used in various methods for user dataset upload.

   TString dsUser, dsGroup, dsName;
   if (ParseDataSetUri(uri, &dsGroup, &dsUser, &dsName) == kFALSE)
      return 0;

   return GetDataSet(dsGroup, dsUser, dsName);
}

//______________________________________________________________________________
TFileCollection *TProofDataSetManager::GetDataSet(const char *group,
                                                  const char *user,
                                                  const char *dsName,
                                                  const char *option,
                                                  TMD5 **checksum)
{
   //
   // Returns the dataset <dsName> of user <user> in group <group> .
   // If checksum is non-zero, it will contain the pointer to a TMD5 sum object
   // with the checksum of the file, has to be deleted by the user.
   // If option contains "S" the shortobject is read, that does not contain the
   // list of files. This is much faster.

   TLockFile lock(fgDataSetLockFile, fgLockFileTimeLimit);

   TString path(GetDataSetPath(group, user, dsName));

   if (gSystem->AccessPathName(path) != kFALSE) {
      if (!fSilent)
         Error("GetDataSet", "File %s does not exist", path.Data());
      return 0;
   }

   TMD5 *retrievedChecksum = 0;
   if (checksum) {
      // save md5 sum
      retrievedChecksum = TMD5::FileChecksum(path);
      if (!retrievedChecksum) {
         if (!fSilent)
            Error("GetDataSet", "Could not get checksum of %s", path.Data());
         return 0;
      }
   }

   TFile *f = TFile::Open(path.Data());
   if (!f) {
      if (!fSilent)
         Error("GetDataSet", "Could not open file %s", path.Data());
      if (retrievedChecksum)
         delete retrievedChecksum;
      return 0;
   }

   TFileCollection *fileList = 0;
   if (TString(option).Contains("S", TString::kIgnoreCase))
     fileList = dynamic_cast<TFileCollection*> (f->Get("dataset_short"));

   if (!fileList)
     fileList = dynamic_cast<TFileCollection*> (f->Get("dataset"));

   f->Close();
   delete f;

   if (checksum)
      *checksum = retrievedChecksum;
   return fileList;
}

//______________________________________________________________________________
Int_t TProofDataSetManager::WriteDataSet(const char *group, const char *user,
                                         const char *dsName, TFileCollection *dataset,
                                         const char *option, TMD5 *checksum)
{
   //
   // Writes indicated dataset. If option contains "E", the file must still exist,
   // otherwise the new dataset is not written (returns 3 in this case).
   // If checksum is non-zero the files current checksum is checked against it,
   // if it does not match the file is not written (the function returns 2 in this
   // case, if the file has disappeared it is also not written (i.e. checksum
   // implies option "E").
   // Returns != 0 for success, 0 for error

   TLockFile lock(fgDataSetLockFile, fgLockFileTimeLimit);

   Bool_t checkIfExists = kFALSE;
   if (TString(option).Contains("E", TString::kIgnoreCase) || checksum)
      checkIfExists = kTRUE;

   TString path(GetDataSetPath(group, user, dsName));

   if (checkIfExists) {
      // check if file still exists, otherwise it was deleted in the meanwhile and is not written here
      Long_t tmp;
      if (gSystem->GetPathInfo(path, 0, (Long_t*) 0, 0, &tmp) != 0) {
         if (!fSilent)
            Info("WriteDataSet", "Dataset disappeared. Discarding update.");
         return 3;
      }
   }

   if (checksum) {
      // verify md5 sum, otherwise the file was changed in the meanwhile and is not overwritten here
      TMD5 *checksum2 = TMD5::FileChecksum(path);
      if (!checksum2) {
         if (!fSilent)
            Error("WriteDataSet", "Could not get checksum of %s", path.Data());
         return 0;
      }

      Bool_t checksumAgrees = (*checksum == *checksum2);
      delete checksum2;

      if (!checksumAgrees) {
         if (!fSilent)
            Info("WriteDataSet", "Dataset changed. Discarding update.");
         return 2;
      }
   }

   // write first in ".<file>" then rename to recover from crash during writing
   TString tempFile(path);
   Int_t index = -1;
   while (tempFile.Index("/", index+1) >= 0)
      index = tempFile.Index("/", index+1);

   tempFile.Insert(index+1, ".");

   TFile *f = TFile::Open(tempFile, "RECREATE");
   if (!f) {
      if (!fSilent)
         Error("WriteDataSet",
               "Could not open dataset for writing %s", tempFile.Data());
      return 0;
   }

   // write full TFileCollection
   dataset->Write("dataset", TObject::kSingleKey);

   // write only metadata
   THashList *list = dataset->GetList();
   dataset->SetList(0);
   dataset->Write("dataset_short", TObject::kSingleKey);

   f->Close();
   delete f;

   dataset->SetList(list);

   // file is written, rename to real filename
   if (gSystem->Rename(tempFile, path) != 0) {
      if (!fSilent)
         Error("WriteDataSet",
               "Renaming %s to %s failed. Dataset might be corrupted.",
               tempFile.Data(), path.Data());
      return 0;
   }

   return 1;
}

//______________________________________________________________________________
Bool_t TProofDataSetManager::RemoveDataSet(const char *group, const char *user,
                                           const char *dsName)
{
   // Removes the indicated dataset

   TLockFile lock(fgDataSetLockFile, fgLockFileTimeLimit);

   TString path(GetDataSetPath(group, user, dsName));

   return (gSystem->Unlink(path) == 0);
}

//______________________________________________________________________________
Bool_t TProofDataSetManager::ExistsDataSet(const char *group, const char *user,
                                           const char *dsName)
{
   // Checks if the indicated dataset exits

   TLockFile lock(fgDataSetLockFile, fgLockFileTimeLimit);

   TString path(GetDataSetPath(group, user, dsName));

   return (gSystem->AccessPathName(path) == kFALSE);
}

//______________________________________________________________________________
TMap *TProofDataSetManager::GetDataSets(const char *group, const char *user,
                                        const char *option)
{
   //
   // Returns all datasets for the given <group> and <user>.
   // If <user> is 0, it returns all datasets for the given <group>.
   // If <group> is 0, it returns all datasets.
   // The returned TMap contains:
   //    <group> --> <map of users> --> <map of datasets> --> <dataset> (TFileCollection)
   //
   // 'option' is forwarded to GetDataSet .

   if (group && strcmp(group, fgCommonDataSetTag) == 0)
     group = fCommonGroup;

   if (user && strcmp(user, fgCommonDataSetTag) == 0)
     user = fCommonUser;

   TMap *result = new TMap;
   result->SetOwner();

   if (!fSilent)
      Info("GetDataSets", "opening dir %s", fDataSetDir.Data());

   void *dataSetDir = 0;
   if ((dataSetDir = gSystem->OpenDirectory(fDataSetDir))) {
      // loop over groups (or just process <group>)
      Bool_t continueGroup = kTRUE;
      while (continueGroup) {
         const char *currentGroup = 0;

         if (group && strlen(group) > 0 && strcmp(group, "*")) {
            currentGroup = group;
            continueGroup = kFALSE;
         } else {
            currentGroup = gSystem->GetDirEntry(dataSetDir);
            if (!currentGroup)
               break;
         }

         if (strcmp(currentGroup, ".") == 0 || strcmp(currentGroup, "..") == 0)
            continue;

         TString groupDirPath;
         groupDirPath.Form("%s/%s", fDataSetDir.Data(), currentGroup);

         void *groupDir = gSystem->OpenDirectory(groupDirPath);
         if (!groupDir)
            continue;

         // loop over users (or just process <user>)
         Bool_t continueUser = kTRUE;
         while (continueUser) {
            const char *currentUser = 0;
            if (user && strlen(user) > 0 && strcmp(user, "*")) {
               currentUser = user;
               continueUser = kFALSE;
            } else {
               currentUser = gSystem->GetDirEntry(groupDir);
               if (!currentUser)
                  break;
            }

            if (strcmp(currentUser, ".") == 0 || strcmp(currentUser, "..") == 0)
               continue;

            TString userDirPath;
            userDirPath.Form("%s/%s", groupDirPath.Data(), currentUser);
            void *userDir = gSystem->OpenDirectory(userDirPath);
            if (!userDir)
               continue;

            TRegexp rg("^[^./][^/]*.root$");  //check that it is a root file, not starting with "."

            // loop over datasets
            const char *dsEnt = 0;
            while ((dsEnt = gSystem->GetDirEntry(userDir))) {
               TString datasetFile(dsEnt);
               if (datasetFile.Index(rg) != kNPOS) {
                  TString datasetName(datasetFile(0, datasetFile.Length()-5));

                  if (!fSilent)
                     Info("GetDataSets", "found dataset %s of user %s in group %s",
                                        datasetName.Data(), currentUser, currentGroup);

                  TFileCollection *fileList = GetDataSet(currentGroup, currentUser, datasetName, option);
                  if (!fileList) {
                     if (!fSilent)
                        Error("GetDataSets", "dataset %s (user %s, group %s) could not be opened",
                                            datasetName.Data(), currentUser, currentGroup);
                     continue;
                  }

                  // we found a dataset, now add it to the map

                  // COMMON dataset transition
                  const char *mapGroup = currentGroup;
                  if (strcmp(mapGroup, fCommonGroup) == 0)
                     mapGroup = fgCommonDataSetTag;
                  const char *mapUser = currentUser;
                  if (strcmp(mapUser, fCommonUser) == 0)
                     mapUser = fgCommonDataSetTag;

                  TMap *userMap = dynamic_cast<TMap*> (result->GetValue(mapGroup));
                  if (!userMap) {
                     userMap = new TMap;
                     userMap->SetOwner();
                     result->Add(new TObjString(mapGroup), userMap);
                  }

                  TMap *datasetMap = dynamic_cast<TMap*> (userMap->GetValue(mapUser));
                  if (!datasetMap) {
                     datasetMap = new TMap;
                     datasetMap->SetOwner();
                     userMap->Add(new TObjString(mapUser), datasetMap);
                  }
                  datasetMap->Add(new TObjString(datasetName), fileList);
               }
            }
            gSystem->FreeDirectory(userDir);
         }
         gSystem->FreeDirectory(groupDir);
      }
      gSystem->FreeDirectory(dataSetDir);
   }

   return result;
}

//______________________________________________________________________________
Int_t TProofDataSetManager::ScanDataSet(const char *group, const char *user,
                                        const char *dsName, const char *option)
{
   // See documentation of ScanDataSet(TFileCollection *dataset, const char *option)

   TFileCollection *dataset = GetDataSet(group, user, dsName);
   if (!dataset)
      return -1;

   Int_t result = ScanDataSet(dataset, option);

   if (result == 2)
      if (WriteDataSet(group, user, dsName, dataset) == 0)
         return -2;

   delete dataset;

   return result;
}

//______________________________________________________________________________
Int_t  TProofDataSetManager::ScanDataSet(TFileCollection *dataset, const char *option)
{
   // Updates the information in a dataset by opening all files that are not yet
   // marked staged creates metadata for each tree in the opened file
   //
   // Options:
   //   max=NN (process a maximum of NN files)
   //   reopen (opens also files that are marked staged)
   //   touch  (opens and toches files that are marked staged; implies reopen)
   //
   // Return code
   //   negative in case of error
   //     -1 dataset not found
   //     -2 could not write dataset after verification
   //
   //   positive in case of success
   //     1 dataset was not changed
   //     2 dataset was changed
   //
   // The number of opened, touched, disappeared files can be retrieved by
   // GetNTouchedFiles(), GetNOpenedFiles(), GetNDisapparedFiles()

   // parse options
   Int_t maxFiles = -1;
   TString optionStr(option);

   TPRegexp regExp("max=[0-9]+");
   TObjArray *match = regExp.MatchS(option);
   if (match && match->GetEntries() > 0) {
      TString& str = (dynamic_cast<TObjString*> (match->First()))->String();
      TString number = str(str.Index("=")+1, str.Length());
      maxFiles = number.Atoi();
      if (!fSilent)
         Info("ScanDataSet", "processing a maximum of %d files", maxFiles);
      delete match;
      match = 0;
   }

   Bool_t reopen = kFALSE;
   if (optionStr.Contains("reopen", TString::kIgnoreCase))
      reopen = kTRUE;

   Bool_t touch = kFALSE;
   if (optionStr.Contains("touch", TString::kIgnoreCase)) {
      reopen = kTRUE;
      touch = kTRUE;
   }

   fNTouchedFiles = 0;
   fNOpenedFiles = 0;
   fNDisappearedFiles = 0;

   Bool_t changed = kFALSE;

   TFileStager *stager = (!fMSSUrl.IsNull()) ? TFileStager::Open(fMSSUrl) : 0;
   Bool_t createStager = (stager) ? kFALSE : kTRUE;

   // TODO preserve old value, set back
   gEnv->SetValue("XNet.ReadAheadSize", 0);
   gEnv->SetValue("XNet.ReadCacheSize", 0);
   gEnv->SetValue("XNet.MaxRedirectCount", 7);

   // Check which files have been staged, this can be replaced by a bulk command,
   // once it exists in the xrdclient
   TList newStagedFiles;
   TIter iter2(dataset->GetList());
   TFileInfo *fileInfo = 0;
   while ((fileInfo = dynamic_cast<TFileInfo*> (iter2.Next()))) {

      fileInfo->ResetUrl();

      if (!fileInfo->GetCurrentUrl()) {
         if (!fSilent)
            Error("ScanDataSet",
                  "GetCurrentUrl() is 0 for %s", fileInfo->GetFirstUrl()->GetUrl());
         continue;
      }

      TUrl url(*(fileInfo->GetCurrentUrl()));
      // Remove anchor (e.g. #AliESDs.root) because IsStaged() and TFile::Open
      // with filetype=raw do not accept anchors
      url.SetAnchor("");

      if (fileInfo->TestBit(TFileInfo::kStaged)) {
         if (fileInfo->TestBit(TFileInfo::kCorrupted))
            continue;

         if (!reopen)
            continue;

         // check if file is still available, if touch is set actually read from the file

         if (!fSilent && (fNTouchedFiles+fNDisappearedFiles) % 100 == 0)
            Info("ScanDataSet",
                 "opening %d. file: %s", fNTouchedFiles+fNDisappearedFiles,
                 fileInfo->GetCurrentUrl()->GetUrl());

         TFile *file = TFile::Open(Form("%s?filetype=raw", url.GetUrl()));

         if (file) {
            if (touch) {
               // actually access the file
               char tmpChar = 0;
               file->ReadBuffer(&tmpChar, 1);
            }
            file->Close();
            delete file;
            fNTouchedFiles++;
         } else { // file could not be opened, reset staged bit
            if (!fSilent)
               Info("ScanDataSet", "file %s disappeared", url.GetUrl());
            fileInfo->ResetBit(TFileInfo::kStaged);
            fNDisappearedFiles++;
            changed = kTRUE;

            // remove invalid URL, if other one left...
            if (fileInfo->GetNUrls() > 1)
               fileInfo->RemoveUrl(fileInfo->GetCurrentUrl()->GetUrl());
         }

         continue;
      }

      // only open maximum number of 'new' files
      if (maxFiles > 0 && newStagedFiles.GetEntries() >= maxFiles)
         continue;

      stager = createStager ? TFileStager::Open(url.GetUrl()) : stager;

      Bool_t result = kFALSE;
      if (stager) {
         result = stager->IsStaged(url.GetUrl());
         if (gDebug > 0 && !fSilent)
            Info("ScanDataSet", "IsStaged: %s: %d", url.GetUrl(), result);
         if (createStager)
            SafeDelete(stager);
      } else {
         Warning("ScanDataSet",
                 "could not get stager instance for '%s'", url.GetUrl());
      }

      if (result == kFALSE)
         continue;

      newStagedFiles.Add(fileInfo);
   }
   SafeDelete(stager);

   // loop over now staged files
   if (!fSilent && newStagedFiles.GetEntries() > 0)
      Info("ScanDataSet",
           "opening %d files that appear to be newly staged.",
           newStagedFiles.GetEntries());

   // Prevent blocking of TFile::Open, if the file disappeared in the last nanoseconds
   Bool_t oldStatus = TFile::GetOnlyStaged();
   TFile::SetOnlyStaged(kTRUE);

   Int_t count = 0;
   TIter iter3(&newStagedFiles);
   while ((fileInfo = dynamic_cast<TFileInfo*> (iter3.Next()))) {

      if (!fSilent && count++ % 100 == 0)
         Info("ScanDataSet", "processing %d.'new' file: %s",
                             count, fileInfo->GetCurrentUrl()->GetUrl());

      TUrl *url = fileInfo->GetCurrentUrl();

      TFile *file = 0;

      // To determine the size we have to open the file without the anchor
      // (otherwise we get the size of the contained file - in case of a zip archive)
      Bool_t zipFile = kFALSE;
      if (strlen(url->GetAnchor()) > 0) {
         zipFile = kTRUE;
         TUrl urlNoAnchor(*url);
         urlNoAnchor.SetAnchor("");
         urlNoAnchor.SetOptions("filetype=raw");

         file = TFile::Open(urlNoAnchor.GetUrl());
      } else
         file = TFile::Open(url->GetUrl());

      if (!file)
         continue;

      changed = kTRUE;

      fileInfo->SetBit(TFileInfo::kStaged);

      // add url of the disk server in front of the list
      TUrl urlDiskServer(*url);
      urlDiskServer.SetHost(file->GetEndpointUrl()->GetHost());
      fileInfo->AddUrl(urlDiskServer.GetUrl(), kTRUE);
      if (!fSilent)
        Info("ScanDataSet", "added URL %s", urlDiskServer.GetUrl());

      if (file->GetSize() > 0)
          fileInfo->SetSize(file->GetSize());

      if (zipFile) {
         file->Close();
         delete file;

         file = TFile::Open(url->GetUrl());
         if (!file) {
            // if the file could be opened before, but fails now it is corrupt...
            if (!fSilent)
               Info("ScanDataSet", "marking %s as corrupt", url->GetUrl());
            fileInfo->SetBit(TFileInfo::kCorrupted);
            continue;
         }
      }

      // disable warnings when reading a tree without loading the corresponding library
      Int_t oldLevel = gErrorIgnoreLevel;
      gErrorIgnoreLevel = kError+1;

      // loop over all entries and create/update corresponding metadata
      // this code only used the objects in the basedir, should be extended to cover directories also
      // It only processes TTrees
      if (file->GetListOfKeys()) {
         TIter keyIter(file->GetListOfKeys());
         TKey *key = 0;
         while ((key = dynamic_cast<TKey*> (keyIter.Next()))) {
            if (strcmp(key->GetClassName(), "TTree"))
               continue;

            TString keyStr;
            keyStr.Form("/%s", key->GetName());

            TFileInfoMeta *metaData = fileInfo->GetMetaData(keyStr);
            if (!metaData) {
               // create it
               metaData = new TFileInfoMeta(keyStr, key->GetClassName());
               fileInfo->AddMetaData(metaData);

               if (!fSilent && gDebug > 0)
                  Info("ScanDataSet", "created meta data for tree %s", keyStr.Data());
            }

            // fill values
            // TODO if we cannot read the tree, is the file corrupt also?
            TTree *tree = dynamic_cast<TTree*> (file->Get(key->GetName()));
            if (tree) {
               if (tree->GetEntries() > 0)
                  metaData->SetEntries(tree->GetEntries());
               if (tree->GetTotBytes() > 0)
                  metaData->SetTotBytes(tree->GetTotBytes());
               if (tree->GetZipBytes() > 0)
                  metaData->SetZipBytes(tree->GetZipBytes());
            }
         }
      }

      // set back old warning level
      gErrorIgnoreLevel = oldLevel;

      file->Close();
      delete file;

      fNOpenedFiles++;
   }

   TFile::SetOnlyStaged(oldStatus);

   dataset->Update();

   return (changed) ? 2 : 1;
}

//______________________________________________________________________________
void TProofDataSetManager::GetQuota(const char *group, const char *user,
                                    const char *dsName, TFileCollection *dataset)
{
   //
   // Gets quota information from this dataset

   if (!fSilent)
      Info("GetQuota", "processing dataset %s %s %s", group, user, dsName);

   if (dataset->GetTotalSize() > 0) {
      TParameter<Long64_t> *size =
         dynamic_cast<TParameter<Long64_t>*> (fGroupUsed.GetValue(group));
      if (!size) {
         size = new TParameter<Long64_t> ("group used", 0);
         fGroupUsed.Add(new TObjString(group), size);
      }

      size->SetVal(size->GetVal() + dataset->GetTotalSize());

      TMap *userMap = dynamic_cast<TMap*> (fUserUsed.GetValue(group));
      if (!userMap) {
         userMap = new TMap;
         fUserUsed.Add(new TObjString(group), userMap);
      }

      size = dynamic_cast<TParameter<Long64_t>*> (userMap->GetValue(user));
      if (!size) {
         size = new TParameter<Long64_t> ("user used", 0);
         userMap->Add(new TObjString(user), size);
      }

      size->SetVal(size->GetVal() + dataset->GetTotalSize());
   }
}

//______________________________________________________________________________
void TProofDataSetManager::PrintUsedSpace()
{
   //
   // Prints the quota

   Info("PrintUsedSpace", "listing used space");

   TIter iter(&fUserUsed);
   TObjString *group = 0;
   while ((group = dynamic_cast<TObjString*> (iter.Next()))) {
      TMap *userMap = dynamic_cast<TMap*> (fUserUsed.GetValue(group->String()));

      TParameter<Long64_t> *size =
         dynamic_cast<TParameter<Long64_t>*> (fGroupUsed.GetValue(group->String()));

      if (userMap && size) {
         Printf("Group %s: %lld B = %.2f GB", group->String().Data(), size->GetVal(),
                                      (Float_t) size->GetVal() / 1024 / 1024 / 1024);

         TIter iter2(userMap);
         TObjString *user = 0;
         while ((user = dynamic_cast<TObjString*> (iter2.Next()))) {
            TParameter<Long64_t> *size2 =
               dynamic_cast<TParameter<Long64_t>*> (userMap->GetValue(user->String().Data()));
            if (size2)
               Printf("  User %s: %lld B = %.2f GB", user->String().Data(), size2->GetVal(),
                                            (Float_t) size2->GetVal() / 1024 / 1024 / 1024);
         }

         Printf("------------------------------------------------------");
      }
   }
}

//______________________________________________________________________________
void TProofDataSetManager::MonitorUsedSpace(TVirtualMonitoringWriter *monitoring)
{
   //
   // Log info to the monitoring server

   Info("MonitorUsedSpace", "sending used space to monitoring server");

   TIter iter(&fUserUsed);
   TObjString *group = 0;
   while ((group = dynamic_cast<TObjString*> (iter.Next()))) {
      TMap *userMap = dynamic_cast<TMap*> (fUserUsed.GetValue(group->String()));
      TParameter<Long64_t> *size =
         dynamic_cast<TParameter<Long64_t>*> (fGroupUsed.GetValue(group->String()));

      if (!userMap || !size)
         continue;

      TList *list = new TList;
      list->SetOwner();
      list->Add(new TParameter<Long64_t>("_TOTAL_", size->GetVal()));
      Long64_t groupQuota = GetGroupQuota(group->String());
      if (groupQuota != -1)
         list->Add(new TParameter<Long64_t>("_QUOTA_", groupQuota));

      TIter iter2(userMap);
      TObjString *user = 0;
      while ((user = dynamic_cast<TObjString*> (iter2.Next()))) {
         TParameter<Long64_t> *size2 =
            dynamic_cast<TParameter<Long64_t>*> (userMap->GetValue(user->String().Data()));
         if (!size2)
            continue;
         list->Add(new TParameter<Long64_t>(user->String().Data(), size2->GetVal()));
      }

      monitoring->SendParameters(list, group->String());
      delete list;
   }
}

//______________________________________________________________________________
Long64_t TProofDataSetManager::GetGroupUsed(const char *group)
{
   //
   // Returns the used space of that group

   if (strcmp(group, fgCommonDataSetTag) == 0)
      group = fCommonGroup;

   TParameter<Long64_t> *size =
      dynamic_cast<TParameter<Long64_t>*> (fGroupUsed.GetValue(group));
   if (!size) {
      if (!fSilent)
         Error("GetGroupUsed", "group %s not found", group);
      return 0;
   }

   return size->GetVal();
}

//______________________________________________________________________________
Long64_t TProofDataSetManager::GetGroupQuota(const char *group)
{
   //
   // returns the quota a group is allowed to have

   if (strcmp(group, fgCommonDataSetTag) == 0)
      group = fCommonGroup;

   TParameter<Long64_t> *value =
      dynamic_cast<TParameter<Long64_t>*> (fGroupQuota.GetValue(group));
   if (!value) {
      if (!fSilent)
         Error("GetGroupQuota", "group %s not found", group);
      return 0;
   }
   return value->GetVal();
}

//______________________________________________________________________________
void TProofDataSetManager::UpdateUsedSpace()
{
   // updates the used space maps

   TMap *datasets = GetDataSets(0, 0, "S");
   if (!datasets) {
      if (!fSilent)
         Error("UpdateUsedSpace", "did not retrieve datasets");
      return;
   }

   // clear used space entries
   fGroupUsed.DeleteAll();
   fUserUsed.DeleteAll();

   TIter iter(datasets);
   TObjString *group = 0;
   while ((group = dynamic_cast<TObjString*> (iter.Next()))) {
      TMap *userMap = dynamic_cast<TMap*> (datasets->GetValue(group->String()));
      if (userMap) {
         TIter iter2(userMap);
         TObjString *user = 0;
         while ((user = dynamic_cast<TObjString*> (iter2.Next()))) {
            TMap *datasetMap = dynamic_cast<TMap*> (userMap->GetValue(user->String()));
            if (datasetMap) {
               TIter iter3(datasetMap);
               TObjString *dsName = 0;
               while ((dsName = dynamic_cast<TObjString*> (iter3.Next()))) {
                  TFileCollection *dataset =
                     dynamic_cast<TFileCollection*> (datasetMap->GetValue(dsName->String()));

                  GetQuota(group->String(), user->String(), dsName->String(), dataset);
               }
               datasetMap->DeleteAll();
            }
         }
         userMap->DeleteAll();
      }
   }

   datasets->DeleteAll();
   delete datasets;
   datasets = 0;
}

//______________________________________________________________________________
Int_t TProofDataSetManager::HandleRequest(TMessage *mess, TSocket *sock, FILE *flog)
{
   // Handle here all requests about datasets.
   // The socket 'sock' is used, when the case requires, to reply to teh client.
   // Return 0 if OK, -1 if OK.

   if (!fSilent)
      Info("HandleRequest", "enter");

   if (!sock || !sock->IsValid()) {
      Error("HandleRequest", "input socket invalid - protocol error?");
      return -1;
   }

   Int_t rc = 0;
   TString dsUser, dsGroup, dsName;
   TString uri; // used in most cases
   TString opt;
   Int_t type = 0;
   (*mess) >> type;
   switch (type) {
      case TProof::kCheckDataSetName:
         //
         // Check whether this dataset exist
         // Communication Summary
         //   Client                              Master
         //     |------------>DataSetName----------->|
         //     |<-------kMESS_OK/kMESS_NOTOK<-------| (Name OK/file exists)
         {  (*mess) >> uri;

            if (ParseDataSetUri(uri, &dsGroup, &dsUser, &dsName) == kFALSE) {
               rc = -1;
               break;
            }

            if (ExistsDataSet(dsGroup, dsUser, dsName))
               //Dataset name does exist
               rc = -1;
         }
         break;
      case TProof::kRegisterDataSet:
         // list size must be above 0
         {  (*mess) >> uri;
            (*mess) >> opt;

            if (ParseDataSetUri(uri, 0, 0, &dsName, 0, kTRUE) == kFALSE) {
               rc = -1;
               break;
            }

            // check if list is empty
            TFileCollection *dataSet =
               dynamic_cast<TFileCollection*> ((mess->ReadObject(TFileCollection::Class())));
            if (!dataSet || dataSet->GetList()->GetSize() == 0) {
               rc = -1;
               Error("HandleRequest", "can not save an empty list.");
               break;
            }

            // Check option
            if (!opt.Contains("O", TString::kIgnoreCase)) {
               // Fail if it exists already
               if (ExistsDataSet(fGroup, fUser, dsName)) {
                  //Dataset name does exist
                  rc = -1;
                  delete dataSet;
                  break;
               }
            }

            // We will save a sorted list
            dataSet->Sort();

            // a temporary list to hold the unique members (i.e. the very set)
            TList *uniqueFileList = new TList();
            TIter nextFile(dataSet->GetList());
            TFileInfo *prevFile = (TFileInfo*)nextFile();
            uniqueFileList->Add(prevFile);
            while (TFileInfo *obj = (TFileInfo*)nextFile()) {
               if (prevFile->Compare(obj)) {  // add entities only once to the temporary list
                  uniqueFileList->Add(obj);
                  prevFile = obj;
               }
            }

            // clear dataSet and add contents of uniqueFileList
            // needed otherwise THashList deletes the objects even when nodelete is set
            dataSet->GetList()->SetOwner(0);
            dataSet->GetList()->Clear("nodelete");
            dataSet->GetList()->SetOwner(1);
            dataSet->GetList()->AddAll(uniqueFileList);
            uniqueFileList->SetOwner(kFALSE);
            delete uniqueFileList;

            // enforce certain settings
            dataSet->SetName(dsName);
            dataSet->ResetBitAll(TFileInfo::kStaged);
            dataSet->ResetBitAll(TFileInfo::kCorrupted);
            dataSet->RemoveMetaData();

            // update accumulated information
            dataSet->Update();

            if (fCheckQuota) {
               if (dataSet->GetTotalSize() <= 0) {
                  Error("HandleRequest", "Datasets without size information are not accepted");
                  rc = -1;
                  break;
               }
               // now check the quota
               UpdateUsedSpace();
               Long64_t used = GetGroupUsed(fGroup) + dataSet->GetTotalSize();

               Info("HandleRequest", "Your group %s uses %.1f GB + %.1f GB for the new dataset. "
                                    "The available quota is %.1f GB", fGroup.Data(),
                                    (Float_t) GetGroupUsed(fGroup)    / 1073741824,
                                    (Float_t) dataSet->GetTotalSize() / 1073741824,
                                    (Float_t) GetGroupQuota(fGroup)   / 1073741824);
               if (used > GetGroupQuota(fGroup)) {
                  rc = -1;
                  Error("HandleRequest", "quota exceeded");
                  break;
               }
            }

            Bool_t success = WriteDataSet(fGroup, fUser, dsName, dataSet);

            delete dataSet;

            if (!success) {
               rc = -1;
               Error("HandleRequest", "could not write dataset");
            }
         }
         break;
      case TProof::kShowDataSets:
         {  (*mess) >> uri;
            (*mess) >> opt;

            if (ParseDataSetUri(uri, &dsGroup, &dsUser, 0, 0, kFALSE, kTRUE) == kFALSE) {
               rc = -1;
               break;
            }

            TMap *datasets = GetDataSets(dsGroup, dsUser, "S");
            if (!datasets) {
               Error("HandleRequest", "could not read datasets");
               rc = -1;
               break;
            }

            Printf("Dataset URI                               |# Files|Default tree|# Events|  Disk  |Staged");

            TIter iter(datasets);
            TObjString *group = 0;
            while ((group = dynamic_cast<TObjString*> (iter.Next()))) {
               TMap *userMap = dynamic_cast<TMap*> (datasets->GetValue(group->String()));
               if (userMap) {
                  TIter iter2(userMap);
                  TObjString *user = 0;
                  while ((user = dynamic_cast<TObjString*> (iter2.Next()))) {

                     TMap *datasetMap = dynamic_cast<TMap*> (userMap->GetValue(user->String()));
                     if (datasetMap) {
                        TIter iter3(datasetMap);
                        TObjString *dsName = 0;
                        while ((dsName = dynamic_cast<TObjString*> (iter3.Next()))) {
                           TFileCollection *dataset =
                              dynamic_cast<TFileCollection*> (datasetMap->GetValue(dsName->String()));
                           if (!dataset)
                              continue;

                           TString dsNameFormatted = Form("/%s/%s/%s", group->String().Data(),
                                                                       user->String().Data(),
                                                                       dsName->String().Data());
                           // Magic number?
                           if (dsNameFormatted.Length() < 42)
                              dsNameFormatted.Resize(42);

                           TString treeInfo(dataset->GetDefaultTreeName());
                           if (treeInfo.Length() > 0) {
                              if (treeInfo.Length() < 12)
                                 treeInfo.Resize(12);
                              TFileInfoMeta* meta = dataset->GetMetaData(dataset->GetDefaultTreeName());
                              if (meta)
                                 treeInfo += Form("|%8lld", meta->GetEntries());
                           } else
                              treeInfo = "        N/A";

                           treeInfo.Resize(21);
                           // Renormalize the size to kB, MB or GB
                           const char *unit[4] = {"kB", "MB", "GB", "TB"};
                           Int_t k = 0, refsz = 1024;
                           Int_t xsz = (Int_t) (dataset->GetTotalSize() / refsz);
                           while (xsz > 1024 && k < 3) {
                              k++;
                              refsz *= 1024;
                              xsz = (Int_t) (dataset->GetTotalSize() / refsz);
                           }
                           Printf("%s|%7lld|%s|%5d %s| %3d %%", dsNameFormatted.Data(),
                                  dataset->GetNFiles(), treeInfo.Data(),
                                  xsz, unit[k], (Int_t) dataset->GetStagedPercentage());
                        }
                        datasetMap->DeleteAll();
                     }
                  }
                  userMap->DeleteAll();
               }
            }
            datasets->DeleteAll();
            delete datasets;
            datasets = 0;
         }
         break;
      case TProof::kGetDataSets:
         {  (*mess) >> uri;
            (*mess) >> opt;

            if (ParseDataSetUri(uri, &dsGroup, &dsUser, 0, 0, kFALSE, kTRUE) == kFALSE) {
               sock->Send(kMESS_NOTOK);
               break;
            }

            TMap *datasets = GetDataSets(dsGroup, dsUser);
            TMap *returnMap = new TMap;

            TIter iter(datasets);
            TObjString* group = 0;
            while ((group = dynamic_cast<TObjString*> (iter.Next()))) {
               TMap* userMap = dynamic_cast<TMap*> (datasets->GetValue(group->String()));
               if (userMap) {
                  TIter iter2(userMap);
                  TObjString* user = 0;
                  while ((user = dynamic_cast<TObjString*> (iter2.Next()))) {
                     TMap* datasetMap = dynamic_cast<TMap*> (userMap->GetValue(user->String()));
                     if (datasetMap) {
                        TIter iter3(datasetMap);
                        TObjString* dsName = 0;
                        while ((dsName = dynamic_cast<TObjString*> (iter3.Next()))) {
                           TFileCollection* dataset =
                              dynamic_cast<TFileCollection*> (datasetMap->GetValue(dsName->String()));
                           if (!dataset)
                              continue;

                           TString dsNameFormatted;
                           dsNameFormatted.Form("/%s/%s/%s", group->String().Data(),
                                                user->String().Data(), dsName->String().Data());
                           returnMap->Add(new TObjString(dsNameFormatted), dataset);
                        }
                        datasetMap->Delete(); // only Delete not DeleteAll, TFileCollection* is
                                              // deleted later in returnMap->DeleteAll()
                     }
                  }
                  userMap->DeleteAll();
               }
            }
            datasets->DeleteAll();
            delete datasets;
            datasets = 0;

            sock->SendObject(returnMap, kMESS_OK);
            returnMap->DeleteAll();
         }
         break;
      case TProof::kGetDataSet:
         {  (*mess) >> uri;
            (*mess) >> opt;
            if (ParseDataSetUri(uri, &dsGroup, &dsUser, &dsName) == kFALSE) {
               sock->Send(kMESS_NOTOK);
               break;
            }
            if (TFileCollection *fileList = GetDataSet(dsGroup, dsUser, dsName)) {
               sock->SendObject(fileList, kMESS_OK);
               delete fileList;
            } else                   // no such dataset
               sock->Send(kMESS_NOTOK);
         }
         break;
      case TProof::kRemoveDataSet:
         {  (*mess) >> uri;
            (*mess) >> opt;

            if (ParseDataSetUri(uri, 0, 0, &dsName, 0, kTRUE) == kFALSE) {
               // error
               sock->Send(kMESS_NOTOK);
               break;
            }

            Bool_t success = RemoveDataSet(fGroup, fUser, dsName);
            if (!success) {
               Error("HandleRequest", "error removing dataset %s", dsName.Data());
               rc = -1;
            }
         }
         break;
      case TProof::kVerifyDataSet:
         {  (*mess) >> uri;
            (*mess) >> opt;

            if (ParseDataSetUri(uri, 0, 0, &dsName, 0, kTRUE) == kFALSE) {
               // error
               rc = -1;
               break;
            }

            Bool_t fOldSilent = fSilent;
            fSilent = kFALSE;
            Int_t result = 0;
            {  TProofServLogHandlerGuard hg(flog, sock);
               result = ScanDataSet(fGroup, fUser, dsName, "reopen");
            }
            fSilent = fOldSilent;

            if (result > 0)
               Info("HandleRequest",
                    "VerifyDataSet: %d files 'new'; %d files touched; %d files disappeared",
                      GetNOpenedFiles(), GetNTouchedFiles(), GetNDisapparedFiles());

            rc = (result > 0) ? GetNDisapparedFiles() : -1;
         }
         break;
      case TProof::kGetQuota:
         {  (*mess) >> opt;
            TMap *groupQuotaMap = GetGroupQuotaMap();
            if (!groupQuotaMap)
               sock->Send(kMESS_NOTOK);

            sock->SendObject(groupQuotaMap, kMESS_OK);
         }
         break;
      case TProof::kShowQuota:
         {  (*mess) >> opt;

            UpdateUsedSpace();

            TMap *groupQuotaMap = GetGroupQuotaMap();
            TMap *userUsedMap = GetUserUsedMap();
            if (!groupQuotaMap || !userUsedMap) {
               rc = -1;
               break;
            }

            TIter iter(groupQuotaMap);
            TObjString *group = 0;
            while ((group = dynamic_cast<TObjString*> (iter.Next()))) {
               Long64_t groupQuota = GetGroupQuota(group->String());
               Long64_t groupUsed = GetGroupUsed(group->String());

               Printf("Group %s uses %.1f GB out of %.1f GB", group->String().Data(),
                                            (Float_t) groupUsed / 1024 / 1024 / 1024,
                                            (Float_t) groupQuota / 1024 / 1024 / 1024);

               // display also user information
               if (!opt.Contains("U", TString::kIgnoreCase))
                  continue;

               TMap *userMap = dynamic_cast<TMap*> (userUsedMap->GetValue(group->String()));
               if (!userMap)
                  continue;

               TIter iter2(userMap);
               TObjString *user = 0;
               while ((user = dynamic_cast<TObjString*> (iter2.Next()))) {
                  TParameter<Long64_t> *size2 =
                     dynamic_cast<TParameter<Long64_t>*> (userMap->GetValue(user->String().Data()));
                  if (!size2)
                     continue;

                  Printf("  User %s uses %.1f GB", user->String().Data(),
                                       (Float_t) size2->GetVal() / 1024 / 1024 / 1024);
               }

               Printf("------------------------------------------------------");
            }
         }
         break;
      default:
         rc = -1;
         Error("HandleRequest", "unknown type %d", type);
         break;
   }

   // We are done
   return 0;
}

//______________________________________________________________________________
Bool_t TProofDataSetManager::ParseDataSetUri(const char *uri,
                                             TString *dsGroup, TString *dsUser,
                                             TString *dsName, TString *dsTree,
                                             Bool_t onlyCurrent, Bool_t wildcards)
{
   // Parses a (relative) URI that describes a DataSet on the cluster.
   // After resolving against a base URI consisting of proof://masterhost/group/user/
   // - meaning masterhost, group and user of the current session -
   // the path is checked to contain exactly three elements separated by '/':
   // group/user/dsname
   // If wildcards, '*' is allowed in group and user and dsname is allowed to be empty.
   // If onlyCurrent, only group and user of current session are allowed.
   // Only non-null parameters are filled by this function.
   // Returns kTRUE in case of success.
 
   // construct our base URI using session defaults (host, group, user)
   TUri base(fMSSUrl + "/" + fGroup + "/" + fUser + "/");
   base.Print();

   // append trailing slash if missing when wildcards enables
   TString uristr(uri);
   if (wildcards and !uristr.EndsWith("/") && uristr.Length() > 0)
      uristr += '/';

   // resolve given URI agains the base
   TUri resolved = TUri::Transform(uristr, base);
   resolved.Print();

   // some URI restrictions
   if (resolved.GetScheme() != base.GetScheme()) {
      Error ("ParseDataSetUri", "URI Scheme <%s> unsupported", resolved.GetScheme().Data());
      return kFALSE;
   }
   // notabene: hostname is treated unnormalised
   if (resolved.GetHost() != base.GetHost()) {
      Error ("ParseDataSetUri", "Hosts other than the master are not supported");
      return kFALSE;
   }
   if (resolved.HasQuery()) {
      Info ("ParseDataSetUri", "URI query part <%s> ignored", resolved.GetQuery().Data());
   }
      
   // we have to tokenize an absolute path
   TObjArray *tokens = resolved.GetPath().Tokenize("/");
   // (0)/group(1)/user(2)/dsname(3)
   if (tokens->GetEntries() != 4) {
      Error ("ParseDataSetUri", "Illegal dataset path");
      return kFALSE;
   }

   // get individual values from tokens
   TString group = ((TObjString*) tokens->At(1))->GetString();
   TString user =  ((TObjString*) tokens->At(2))->GetString();
   TString name =  ((TObjString*) tokens->At(3))->GetString();
   delete tokens;
   TString tree =  resolved.GetFragment();

   // check for unwanted use of wildcards
   if ( (user=="*" or group=="*") && !wildcards) {
      Error ("ParseDataSetUri", "No wildcards allowed for user/group in this context");
      return kFALSE;
   }

   // dsname may only be empty if wildcards expected
   if (!wildcards && name.IsNull()) {
      Error ("ParseDataSetUri", "DataSet name is empty");
      return kFALSE;
   }

   // construct regexp whitelist for checking illegal characters in user/group
   TPRegexp wcExp (wildcards ? "^(?:[A-Za-z0-9-]*|[*])$" : "^[A-Za-z0-9-]*$");

   // check for illegal characters in all components
   if (!wcExp.Match(group)) {
      Error("ParseDataSetUri", "Illegal characters in group");
      return kFALSE;
   }

   if (!wcExp.Match(user)) {
      Error("ParseDataSetUri", "Illegal characters in user");
      return kFALSE;
   }

   if (name.Contains(TRegexp("[^A-Za-z0-9-._]"))) {
      Error("ParseDataSetUri", "Illegal characters in dataset name");
      return kFALSE;
   }

   if (tree.Contains(TRegexp("[^A-Za-z0-9-/_]"))) {
      Error("ParseDataSetUri", "Illegal characters in tree");
      return kFALSE;
   }

   // Check user & group
   if (onlyCurrent && (group.CompareTo(fGroup) || user.CompareTo(fUser))) {
      Error("ParseDataSetUri", "only datasets from your group/user allowed");
      return kFALSE;
   }

   // fill parameters passed by reference, if defined
   if (dsGroup)
      *dsGroup = group;
   if (dsUser)
      *dsUser = user;
   if (dsName)
      *dsName = name;
   if (dsTree)
      *dsTree = tree;

   return kTRUE;
}
