// @(#)root/proof:$Id$
// Author: Long Tran-Thanh   14/09/07

/*************************************************************************
 * Copyright (C) 1995-2002, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofFile                                                           //
//                                                                      //
// Small class to steer the merging of files produced on the workers    //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofFile.h"
#include <TProofServ.h>
#include <TFileMerger.h>
#include <TFile.h>
#include <TList.h>
#include <TObjArray.h>
#include <TObject.h>
#include <TObjString.h>
#include <TSystem.h>
#include <TUUID.h>

ClassImp(TProofFile)

//______________________________________________________________________________
TProofFile::TProofFile():TNamed()
{
   // Default ctor

   fMerged = kFALSE;
   fLocation = "REMOTE";
   fMode = "CENTRAL";
}

//________________________________________________________________________________
TProofFile::TProofFile(const char* path, const char* location, const char* mode)
          : TNamed(path,"")
{
   // Main conctructor

   fMerged = kFALSE;

   TUrl u(path, kTRUE);
   // File name
   fFileName = u.GetFile();
   // Unique file name
   fFileName1 = GetTmpName(fFileName.Data());
   // Path
   fDir = u.GetUrl();
   Int_t pos = fDir.Index(fFileName);
   if (pos != kNPOS)
      fDir.Remove(pos);
   if (fDir == "file:")
      fDir = "";
   // Location
   fLocation = "REMOTE";
   if (location && strlen(location) > 0) {
      fLocation = location;
      if (fLocation.CompareTo("LOCAL", TString::kIgnoreCase) &&
          fLocation.CompareTo("REMOTE", TString::kIgnoreCase)) {
         Warning("TProofFile","unknown location %s: ignore (use: \"REMOTE\")", location);
         fLocation = "REMOTE";
      }
      fLocation.ToUpper();
   }
   // Mode
   fMode = "CENTRAL";
   if (mode && strlen(mode) > 0) {
      fMode = mode;
      if (fMode.CompareTo("CENTRAL", TString::kIgnoreCase) &&
          fMode.CompareTo("SEQUENTIAL", TString::kIgnoreCase)) {
         Warning("TProofFile","unknown mode %s: ignore (use: \"CENTRAL\")", mode);
         fMode = "CENTRAL";
      }
      fMode.ToUpper();
   }
   // Default 
   fOutputFileName = fFileName;
}

//______________________________________________________________________________
TString TProofFile::GetTmpName(const char* name)
{
   // Create a temporary unique name for this file

   TUUID uuid;

   TString tmpName(name);
   Ssiz_t pos = tmpName.Last('.');
   if (pos != kNPOS)
      tmpName.Insert(pos,Form("-%s",uuid.AsString()));
   else
      tmpName += Form("-%s",uuid.AsString());

   // Done
   return tmpName;
}

//______________________________________________________________________________
void TProofFile::SetFileName(const char* name)
{
   // Set the file name

   fFileName = name;
   fFileName1 = GetTmpName(name);
}

//______________________________________________________________________________
void TProofFile::SetOutputFileName(const char *name)
{
   // Set the name of the output file; in the form of an Url.

   if (name && strlen(name) > 0) {
      fOutputFileName = name;
   } else {
      fOutputFileName = fFileName;
   }
}


//______________________________________________________________________________
TFile* TProofFile::OpenFile(const char* opt)
{
   // Open the file using the unique temporary name

   if (fFileName1.IsNull())
      return 0;

   // Create the path
   TString fileLoc = (fDir.IsNull()) ? fFileName1
                                     : Form("%s/%s", fDir.Data(), fFileName1.Data());
   // Open the file
   TFile *retFile = TFile::Open(fileLoc, opt);

   return retFile;
}

//______________________________________________________________________________
Long64_t TProofFile::Merge(TCollection* list)
{
   // Merge objects from the list into this object

   if(!list || list->IsEmpty())
      return 0; 

   TString fileLoc;
   TString outputFileLoc = fOutputFileName;

   if (fMode == "SEQUENTIAL") {
      TFileMerger* merger = new TFileMerger;
      if (fLocation == "LOCAL") {
         merger->OutputFile(outputFileLoc);
         if (!fMerged) {
            fileLoc = Form("%s/%s",fDir.Data(), GetFileName());
            merger->AddFile(fileLoc);
            gSystem->Unlink(outputFileLoc);
         } else {
            merger->AddFile(outputFileLoc);
            gSystem->Unlink(outputFileLoc);
         }

         TList* elist = new TList;
         elist->AddAll(list); 
         TIter next(elist);
         TProofFile* pFile = 0;

         while ((pFile = (TProofFile*)next())) {
            fileLoc = Form("%s/%s", pFile->GetDir(), pFile->GetFileName());
            merger->AddFile(fileLoc);
         }

         Bool_t result = merger->Merge();
         if (!result) {
            Error("Merge", "Error during merge of your ROOT files");
            return -1;
         }

         if (!fMerged) {
            fileLoc = Form("%s/%s",fDir.Data(), GetFileName());
            gSystem->Unlink(fileLoc);
            fMerged = kTRUE;
         }

         next.Reset();
         while ((pFile = (TProofFile*)next())) {
            fileLoc = Form("%s/%s",pFile->GetDir(),pFile->GetFileName());
            gSystem->Unlink(fileLoc);
         }
      } else if (fLocation == "REMOTE") {

         TString outputFileLoc2 = GetTmpName(fOutputFileName);
         TString tmpOutputLoc = (outputFileLoc.BeginsWith("root://")) ? GetTmpName(fFileName) : "";
         TList* fileList = new TList;

         if (!fMerged) {
            fileLoc = Form("%s/%s",fDir.Data(), GetFileName());
            TFile* fCurrFile = TFile::Open(fileLoc,"READ");
            if (!fCurrFile) {
               Warning("Merge","Cannot open file: %s", fileLoc.Data());
            } else {
               fileList->Add(fCurrFile);
               Info("Merge", "now adding file :%s\n", fCurrFile->GetPath());
            }
            gSystem->Unlink(outputFileLoc);
         } else {
            if (tmpOutputLoc.IsNull()) {
               gSystem->Rename(outputFileLoc,outputFileLoc2);
            } else {
               TFile::Cp(outputFileLoc, outputFileLoc2);
               gSystem->Unlink(outputFileLoc);
            }

            TFile* fCurrOutputFile = TFile::Open(outputFileLoc2,"READ");
            if (!fCurrOutputFile) {
               Warning("Merge","Cannot open tmp output file: %s", outputFileLoc2.Data());
            } else {
               fileList->Add(fCurrOutputFile);
            }
         }

         TList* elist = new TList;
         elist->AddAll(list);
         TIter next(elist);
         TProofFile* pFile = 0;

         while ((pFile = (TProofFile*)next())) {
            fileLoc = Form("%s/%s",pFile->GetDir(),pFile->GetFileName());

            TFile* fCurrFile = TFile::Open(fileLoc.Data(),"READ");
            if (!fCurrFile) {
               Warning("Merge","Cannot open file: %s", fileLoc.Data());
               continue;
            } else {
               fileList->Add(fCurrFile);
            }
         }

         TFile* outputFile;
         if (tmpOutputLoc.IsNull()) {
            outputFile = TFile::Open(outputFileLoc, "RECREATE");
         } else {
            outputFile = TFile::Open(tmpOutputLoc,"RECREATE");
         }

         if (!outputFile) {
            Error("Merge","cannot open output file %s",outputFileLoc.Data());
            return -1;
         }
         Bool_t result =  merger->MergeRecursive(outputFile, fileList, 0);
         if (!result) {
            Error("Merge", "Error during merge of ROOT files");

            TIter fnext(fileList);
            TFile *fCurrFile = 0;
            while ((fCurrFile = (TFile*)fnext())) {
               fCurrFile->Close();
            }
            return -1;
         } else {
            outputFile->Write();
            outputFile->Close();

            TIter fnext(fileList);
            TFile *fCurrFile = 0;
            while ((fCurrFile = (TFile*)fnext())) {
               fCurrFile->Close();
            }

            if (!fMerged) {
               fileLoc = Form("%s/%s",fDir.Data(),GetFileName());
               gSystem->Unlink(fileLoc);
               fMerged = kTRUE;
            }

            next.Reset();
            while ((pFile = (TProofFile *)next())) {
               fileLoc = Form("%s/%s",pFile->GetDir(),pFile->GetFileName());
               gSystem->Unlink(fileLoc);
            }

            gSystem->Unlink(outputFileLoc2); 
            if (!tmpOutputLoc.IsNull()) {
               TFile::Cp(tmpOutputLoc,outputFileLoc);
               gSystem->Unlink(tmpOutputLoc);
            }
         } //end else
      } else {   // end fLocation = "Remote"
         // the given merging location is not valid
         Error("Merge", "invalid location value: %s", fLocation.Data());
         return -1;
      }
      SafeDelete(merger);

      // end fMode = "SEQUENTIAL"

   } else if (fMode == "CENTRAL") {

      // if we merge the outputfiles centrally

      if (fLocation != "REMOTE" && fLocation != "LOCAL") {
         Error("Merge", "invalid location value: %s", fLocation.Data());
         return -1;
      }

      // Get the file merger instance
      Bool_t isLocal = (fLocation == "REMOTE") ? kFALSE : kTRUE;
      TFileMerger *filemerger = gProofServ->GetProofFileMerger(isLocal);
      if (!filemerger) {
         Error("Merge", "could not instantiate the file merger");
         return -1;
      }

      if (!fMerged) {

         filemerger->OutputFile(outputFileLoc);
         gSystem->Unlink(outputFileLoc);

         fileLoc = Form("%s/%s",fDir.Data(),GetFileName());
         filemerger->AddFile(fileLoc);

         fMerged = kTRUE;
      }

      TList* elist = new TList;
      elist->AddAll(list); 
      TIter next(elist);
      TProofFile* pFile = 0;

      while((pFile = (TProofFile*)next())) {
         fileLoc = Form("%s/%s",pFile->GetDir(),pFile->GetFileName());
         filemerger->AddFile(fileLoc);
      }

      // end fMode = "CENTRAL"

   } else {
      Error("Merge", "invalid mode value: %s", fMode.Data());
      return -1;
   }

   // Done
   return 0;
}

//______________________________________________________________________________
void TProofFile::Print(Option_t *) const
{
   // Dump the class content

   Info("Print","-------------- %s : start ------------", GetName());
   Info("Print"," dir:              %s", fDir.Data());
   Info("Print"," file name:        %s", fFileName.Data());
   Info("Print"," location:         %s", fLocation.Data());
   Info("Print"," mode:             %s", fMode.Data());
   Info("Print"," output file name: %s", fOutputFileName.Data());
   Info("Print"," ordinal:          %s", fWorkerOrdinal.Data());
   Info("Print","-------------- %s : done -------------", GetName());

   return;
}
