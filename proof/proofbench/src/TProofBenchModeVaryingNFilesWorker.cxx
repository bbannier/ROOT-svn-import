// @(#)root/proofx:$Id$
// Author:

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBench                                                          //
//                                                                      //
// TProofBench is a steering class for PROOF benchmark suite.           //
// The primary goal of benchmark suite is to determine the optimal      //
// configuration parameters for a set of machines to be used as PROOF   //
// cluster. The suite measures the performance of the cluster for a set //
// of standard tasks as a function of the number of effective processes.//
// From these results, indications about the optimal number of          //
// concurrent processes could be derived. For large facilities,         //
// the suite should also give indictions about the optimal number of    //
// of sub-masters into which the cluster should be partitioned.         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofBenchModeVaryingNFilesWorker.h"
#include "TProofNode.h"
#include "TFileCollection.h"
#include "TFileInfo.h"
#include "TProof.h"
#include "TString.h"
#include "TDSet.h"
#include "Riostream.h"
#include "TMap.h"
#include "TObjArray.h"

ClassImp(TProofBenchModeVaryingNFilesWorker)

TProofBenchModeVaryingNFilesWorker::TProofBenchModeVaryingNFilesWorker(Int_t nfiles, TProof* proof)
:fProof(proof),
fNFiles(nfiles),
fNodes(0),
fName("VaryingNFilesWorker")
{
   FillNodeInfo();

   //default number of files a worker is 1
   if (fNFiles==-1){
      fNFiles=1;
   }
}

TProofBenchModeVaryingNFilesWorker::~TProofBenchModeVaryingNFilesWorker()
{
}

void TProofBenchModeVaryingNFilesWorker::Print(Option_t* option)const
{
   if (fProof) fProof->Print(option);
   Printf("fNFiles=%d", fNFiles);
   if (fNodes) fNodes->Print(option);
   Printf("fName=%s", fName.Data());
}

//Default behaviour for generating files on the worker nodes.
TMap* TProofBenchModeVaryingNFilesWorker::FilesToProcess(Int_t nf,
                                                    const char* basedir)
{

//Generates files on worker nodes for I/O test or for cleanup run
//Input parameters do not change corresponding data members
//Data set member (fDataSetGeneratedBench or fDataSetGeneratedCleanup) gets filled up
//with generated data set elements
//
//Input parameters
//   filetype: kFileBenchmark : Generate files for benchmark run
//             kFileCleanup: Generate files for cleanup run
//   nf: Number of files per node when filetype==kFileBenchmark.
//       Ignored when filetype==kFileCleanup
//   nevents: Number of events in a file when filetype==kFileBenchmark.
//            Igonired when filetype==kFileCleanup 
//   basedir: Base directory for the files to be generated on the worker nodes. 
//   regenerate: Regenerate files when kTRUE,
//               Reuse files if they exist, when kFALSE (default)
//Returns: 
//   0 when ok
//  <0 when anything is wrong

   // Create the file names and the map {worker,files}
   // Naming:        <basedir>/EventTree_Benchmark_nfile_serial.root
   TMap *filesmap = new TMap;
   filesmap->SetName("PROOF_FilesToProcess");
   Long64_t entries = 0;
   TIter nxni(fNodes);
   TProofNode *ni = 0;

   while ((ni = (TProofNode *) nxni())) {
      TList *files = new TList;
      files->SetName(ni->GetName());
      Int_t nwrks=ni->GetNWrks();
      Int_t nfilesthisnode=nwrks*nf;
      for (Int_t i = 0; i<nfilesthisnode; i++) {
         files->Add(new TObjString(TString::Format("%s/EventTree_Benchmark_%d_0.root",
                                                   basedir, i)));
         entries++;
      }
      filesmap->Add(new TObjString(ni->GetName()), files);
      //files->Print();
   }
   return filesmap;
}

Int_t TProofBenchModeVaryingNFilesWorker::MakeDataSets(Int_t nf,
                                                  Int_t start,
                                                  Int_t stop,
                                                  Int_t step,
                                                  const TDSet* tdset,
                                                  const char* option,
                                                  TProof* proof)
{
   //check input parameters
   if (!tdset){
       Error("MakeDataSets", "No generated files provided; returning");
       return -1;
   }

   if (nf==-1){
      nf=fNFiles;
      Info("MakeDataSets", "Number of files a worker is %d for %s", nf, GetName());
   }

   //default max worker number is the number of all workers in the cluster
   if (stop==-1){
      TIter nxni(fNodes);
      TProofNode *ni = 0;
      Int_t nallworkers=0;
      while ((ni = (TProofNode *) nxni())) {
         nallworkers+=ni->GetNWrks();
      }
      stop=nallworkers;
   }

   const Int_t np=(stop-start)/step+1;
   Int_t wp[np];
   Int_t i=0;
   for (Int_t nactive=start; nactive<=stop; nactive+=step) {
      wp[i]=nactive;
      i++;
   }
   MakeDataSets(nf, np, wp, tdset, option, proof);

   return 0;
}

Int_t TProofBenchModeVaryingNFilesWorker::MakeDataSets(Int_t nf,
                                                  Int_t np,
                                                  const Int_t *wp,
                                                  const TDSet* tdset,
                                                  const char *option,
                                                  TProof* proof)
{
   //check input parameters
   if (!tdset){
      Error("MakeDataSets", "No generated files provided; returning");
      return -1;
   }

   // Dataset naming: DataSetEventConstNFilesNode_nworkersincluster_nfilesaworker
   TString smode, stem;
   smode="VaryingNFilesWorker";
   stem="_Benchmark_";

   TString dsname;
   Int_t kp;
   Int_t nfiles=0;
   for (kp = 0; kp < np; kp++) {
      TFileCollection *fc = new TFileCollection;
      dsname.Form("DataSetEvent%s_%d_%d", smode.Data(), wp[kp], nf);
      Info("MakeDataSets", "creating dataset '%s' ...", dsname.Data());

      nfiles=nf*wp[kp]; //number of files on all nodes for wp[kp] workers

      //Check if we have enough number of files
      TList* le=tdset->GetListOfElements();
      Int_t nfilesavailable=le->GetSize();
      if (nfilesavailable<nfiles){
         Warning("MakeDataSets", "Number of available files (%d) is smaller than needed (%d)"
                ,nfilesavailable, nfiles);
      }
      //Check if number of requested workers are not greater than number of available workers
      TList *wl = proof->GetListOfSlaveInfos();
      if (!wl) {
         Error("MakeDataSets", "Could not get information about workers!");
         return -2;
      }

      Int_t nworkersavailable=wl->GetSize();
      if (nworkersavailable<wp[kp]){ 
         Warning("MakeDataSets", "Number of available workers (%d) is smaller than needed (%d); "
                                 "Only upto %d(=%d files/worker * %d workers) out of %d(=%d files/worker* %d workers) files will be added to the data set"
                ,nworkersavailable, wp[kp], nf*nworkersavailable, nf, nworkersavailable, nfiles, nf, wp[kp]);
      }

      //copy tdset
      TList* lecopy=new TList;
      TIter nxte(le);
      TDSetElement *tde;
      while (tde=(TDSetElement*)nxte()){
         lecopy->Add(new TDSetElement(*tde));
      }
      //lecopy->Print("A");

      TIter nxwi(wl);
      TSlaveInfo *si = 0;
      TString nodename;
      Int_t nfilesadded=0;
      Int_t nfile; 
      TFileInfo* fileinfo;
      TUrl* url;
      TString hostname, filename, tmpstring; 
      while ((si = (TSlaveInfo *) nxwi())) {
         nodename=si->GetName(); 

         //start over for this worker
         TIter nxtelement(lecopy);
         Int_t nfilesadded_worker=0;
         while ((tde=(TDSetElement*)nxtelement())){

            fileinfo=tde->GetFileInfo();
            url=fileinfo->GetCurrentUrl();
            hostname=url->GetHost(); 
            filename=url->GetFile();

            if (hostname!=nodename) continue; 

            //Info("MakeDataSets", "filename=%s", fileinfo->GetName());
            //filename=root://hostname//directory/EventTree_Benchmark_filenumber_serial.root
            //remove upto "Benchmark_"
            tmpstring=filename;
            tmpstring.Remove(0, filename.Index(stem)+stem.Length());

            TObjArray* token=tmpstring.Tokenize("_");

            //Info ("CreateDataSetsN", "file %s", url->GetUrl());
            if (token){
               nfile=TString((*token)[0]->GetName()).Atoi();
               token=TString((*token)[1]->GetName()).Tokenize(".");
               Int_t serial=TString((*token)[0]->GetName()).Atoi();

               //ok found, add it to the dataset
               //count only once for set of split files
               if (serial==0){
                  if (nfilesadded_worker>=nf){
                     break;
                  }
                  nfilesadded_worker++;
               }
               fc->Add(fileinfo);
               lecopy->Remove(tde);
            }
            else{
               Error("MakeDataSets", "File name not recognized: %s", fileinfo->GetName());
               return -1;
            }
         }
         nfilesadded+=nfilesadded_worker;
         if (nfilesadded>=nfiles){
            break;
         }
      }
      if (nfilesadded<nfiles){
         Warning("MakeDataSets", "Only %d files out of %d files requested "
                                 "are added to data set %s",
                  nfilesadded, nfiles, dsname.Data());
      }
      fc->Update();
      // Register dataset with verification
      proof->RegisterDataSet(dsname, fc, option);
      lecopy->SetOwner(kTRUE);
      SafeDelete(lecopy);
      SafeDelete(fc);
   }
   return 0;
}

void TProofBenchModeVaryingNFilesWorker::SetProof(TProof* proof)
{
   fProof=proof;
}

void TProofBenchModeVaryingNFilesWorker::SetNFiles(Int_t nfiles)
{
   fNFiles=nfiles;
}

TProof* TProofBenchModeVaryingNFilesWorker::GetProof()const
{
   return fProof;
}

Int_t TProofBenchModeVaryingNFilesWorker::GetNFiles()const
{
   return fNFiles;
}

const char* TProofBenchModeVaryingNFilesWorker::GetName()const
{
   return fName.Data();
}

Int_t TProofBenchModeVaryingNFilesWorker::FillNodeInfo()
{
   // Re-Generate the list of worker node info (fNodes)
   // Return 0 if OK, -1 if proof not set, -2 if info could not be retrieved
   // (the existing info is always removed)

   if (!fProof){
      Error("FillNodeInfo", "proof not set, doing nothing");
      return -1;
   }

   if (fNodes) {
      fNodes->SetOwner(kTRUE);
      SafeDelete(fNodes);
   }
   fNodes = new TList;
   fNodes->SetOwner();//fNodes is the owner of the members

   // Get info
   TList *wl = fProof->GetListOfSlaveInfos();
   if (!wl) {
      Error("FillNodeInfo", "could not get information about workers!");
      return -2;
   }

   TIter nxwi(wl);
   TSlaveInfo *si = 0;
   TProofNode *ni = 0;
   while ((si = (TSlaveInfo *) nxwi())) {
      if (!(ni = (TProofNode *) fNodes->FindObject(si->GetName()))) {
         ni = new TProofNode(si->GetName(), si->GetSysInfo().fPhysRam);
         fNodes->Add(ni);
      } else {
         ni->AddWrks(1);
      }
   }
   // Notify
   Info("FillNodeInfo","%d physically different mahcines found", fNodes->GetSize());
   // Done
   return 0;
}
