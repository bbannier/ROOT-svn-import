// @(#)root/proofx:$Id$
// Author: Sangsu Ryu 22/06/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBenchVaryingNFilesWorker                                       //
//                                                                      //
// A mode for PROOF benchmark test.                                     //
// In this mode, a given number of files are generated for each worker. //
// During the test, the number of files to be processed in the cluster  //
// is fNFiles * number_of_active_workers_in_the_cluster.                //
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

//______________________________________________________________________________
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

//______________________________________________________________________________
TProofBenchModeVaryingNFilesWorker::~TProofBenchModeVaryingNFilesWorker()
{
}

//______________________________________________________________________________
void TProofBenchModeVaryingNFilesWorker::Print(Option_t* option)const
{
   if (fProof) fProof->Print(option);
   Printf("fNFiles=%d", fNFiles);
   if (fNodes) fNodes->Print(option);
   Printf("fName=%s", fName.Data());
}

//______________________________________________________________________________
TMap* TProofBenchModeVaryingNFilesWorker::FilesToProcess(Int_t nf)
{

   // Create a map of files to be generated on worker nodes.
   // File name format is "EventTree_Benchmark_nfile_serial.root".
   // Input parameters do not change corresponding data members.
   //
   // Input parameters
   //    nf: Number of files per worker.
   //        When nf=-1, use data member fNFiles.
   // Returns
   //    Map of files to be generated on the worker nodes.

   if (nf==-1){
      nf=fNFiles;
   }

   TMap *filesmap = new TMap;
   filesmap->SetName("PROOF_FilesToProcess");
   TIter nxni(fNodes);
   TProofNode *ni = 0;

   while ((ni = (TProofNode *) nxni())) {
      TList *files = new TList;
      files->SetName(ni->GetName());
      Int_t nwrks=ni->GetNWrks();
      Int_t nfilesthisnode=nwrks*nf;
      for (Int_t i = 0; i<nfilesthisnode; i++) {
         files->Add(new TObjString(TString::Format("EventTree_Benchmark_%d_0.root", i)));
      }
      filesmap->Add(new TObjString(ni->GetName()), files);
      //files->Print();
   }
   return filesmap;
}

//______________________________________________________________________________
Int_t TProofBenchModeVaryingNFilesWorker::MakeDataSets(Int_t nf,
                                                  Int_t start,
                                                  Int_t stop,
                                                  Int_t step,
                                                  const TDSet* tdset,
                                                  const char* option,
                                                  TProof* proof)
{
   // Make data sets out of data set 'tdset' and register them.
   // Input parameters
   //    nf: Number of files a node.
   //        When ==-1, data member fNFiles are used.
   //    start: Start scan at 'start' number of workers.
   //    stop: Stop scan at 'stop' number of workers.
   //          When ==-1, it is set to total number of workers in the cluster.
   //    step: Scan every 'step' workers.
   //    tdset: Data set ouf of which data sets are built and registered.
   //    option: Option to TProof::RegisterDataSet(...).
   //    proof: Proof
   // Return
   //   0 when ok
   //  <0 otherwise

   if (!fProof){
      Error("MakeDataSets", "proof not set, doing nothing");
      return -1;
   }

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
   return MakeDataSets(nf, np, wp, tdset, option, proof);

}

//______________________________________________________________________________
Int_t TProofBenchModeVaryingNFilesWorker::MakeDataSets(Int_t nf,
                                                  Int_t np,
                                                  const Int_t *wp,
                                                  const TDSet* tdset,
                                                  const char *option,
                                                  TProof* proof)
{

   // Make data sets out of data set 'tdset' and register them.
   // Data set name has the form : DataSetEventConstNFilesWorker_nactiveworkersincluster_nfilesanode
   // Input parameters
   //    nf: Number of files a node.
   //    np: Number of test points.
   //    wp: 'np'-sized array containing the number of active workers to process files.
   //    tdset: Data set ouf of which data sets are built and registered.
   //    option: Option to TProof::RegisterDataSet(...).
   //    proof: Proof
   // Return
   //   0 when ok
   //  <0 otherwise

   if (!fProof){
      Error("MakeDataSets", "proof not set, doing nothing");
      return -1;
   }

   if (!tdset){
      Error("MakeDataSets", "No generated files provided; returning");
      return -1;
   }

   if (nf==-1){
      nf=fNFiles;
      Info("MakeDataSets", "Number of files a node is %d for %s", nf, GetName());
   }

   // Dataset naming: DataSetEventConstNFilesNode_nworkersincluster_nfilesaworker

   TString dsname;
   Int_t kp;
   Int_t nfiles=0;
   for (kp = 0; kp < np; kp++) {
      TFileCollection *fc = new TFileCollection;
      dsname.Form("DataSetEvent%s_%d_%d", GetName(), wp[kp], nf);
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
            //filename=root://hostname/directory/EventTree_Benchmark_filenumber_serial.root
            //remove upto "Benchmark_"
            tmpstring=filename;
            TString stem="_Benchmark_";

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

//______________________________________________________________________________
void TProofBenchModeVaryingNFilesWorker::SetProof(TProof* proof)
{
   fProof=proof;
}

//______________________________________________________________________________
void TProofBenchModeVaryingNFilesWorker::SetNFiles(Int_t nfiles)
{
   fNFiles=nfiles;
}

//______________________________________________________________________________
TProof* TProofBenchModeVaryingNFilesWorker::GetProof()const
{
   return fProof;
}

//______________________________________________________________________________
Int_t TProofBenchModeVaryingNFilesWorker::GetNFiles()const
{
   return fNFiles;
}

//______________________________________________________________________________
const char* TProofBenchModeVaryingNFilesWorker::GetName()const
{
   return fName.Data();
}

//______________________________________________________________________________
Int_t TProofBenchModeVaryingNFilesWorker::FillNodeInfo()
{
   // Re-Generate the list of worker node info (fNodes)
   // (the existing info is always removed)
   // Return
   //    0 if ok
   //   <0 otherwise

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
