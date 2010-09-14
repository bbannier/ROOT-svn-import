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
// TProofBenchModeConstNFilesWorker                                     //
//                                                                      //
// A mode for PROOF benchmark test.                                     //
// In this mode, a given number of files are generated for each worker. //
// During the test, the total number of files to be processed           //
// in the cluster is fNFiles * number_of_active_workers_in_the_cluster. //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofBenchModeConstNFilesWorker.h"
#include "TProofNode.h"
#include "TFileCollection.h"
#include "TFileInfo.h"
#include "TProof.h"
#include "TString.h"
#include "TDSet.h"
#include "Riostream.h"
#include "TMap.h"
#include "TObjArray.h"

ClassImp(TProofBenchModeConstNFilesWorker)

//______________________________________________________________________________
TProofBenchModeConstNFilesWorker::TProofBenchModeConstNFilesWorker(Int_t nfiles, TProof* proof)
:fProof(proof),
fNFiles(nfiles),
fNodes(0),
fName("ConstNFilesWorker")
{
   FillNodeInfo();

   //default number of files a worker is 1
   if (fNFiles==-1){
      fNFiles=1;
   }
}

//______________________________________________________________________________
TProofBenchModeConstNFilesWorker::~TProofBenchModeConstNFilesWorker()
{
}

//______________________________________________________________________________
void TProofBenchModeConstNFilesWorker::Print(Option_t* option)const
{
   if (fProof) fProof->Print(option);
   Printf("fNFiles=%d", fNFiles);
   if (fNodes) fNodes->Print(option);
   Printf("fName=%s", fName.Data());
}

//______________________________________________________________________________
TMap* TProofBenchModeConstNFilesWorker::FilesToProcess(Int_t nf)
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
         files->Add(new TObjString(TString::Format("%s_EventTree_Benchmark_%d_0.root", ni->GetName(), i)));
      }
      filesmap->Add(new TObjString(ni->GetName()), files);
      //files->Print();
   }
   return filesmap;
}

//______________________________________________________________________________
Int_t TProofBenchModeConstNFilesWorker::MakeDataSets(Int_t nf,
                                                Int_t start,
                                                Int_t stop,
                                                Int_t step,
                                                const TDSet* tdset,
                                                const char* option,
                                                const TUrl* poolurl,
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
   return MakeDataSets(nf, np, wp, tdset, option, poolurl, proof);
}

//______________________________________________________________________________
Int_t TProofBenchModeConstNFilesWorker::MakeDataSets(Int_t nf,
                                                Int_t np,
                                                const Int_t *wp,
                                                const TDSet* tdset,
                                                const char *option,
                                                const TUrl* poolurl,
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

   Int_t newport=0;
   const char* newprotocol=0;

   if (poolurl){
      newport=poolurl->GetPort();
      newprotocol=poolurl->GetProtocol();
   }

   // Dataset naming: DataSetEventConstNFilesWorker_nworkersincluster_nfilesaworker

   TString dsname;
   Int_t kp; 
   for (kp=0; kp<np; kp++) {
      // Dataset name
      dsname.Form("DataSetEvent%s_%d_%d", GetName(), wp[kp], nf);
      Info("MakeDataSets", "creating dataset '%s' ...", dsname.Data());
      // Create the TFileCollection 
      TFileCollection *fc = new TFileCollection;

      TIter nxni(fNodes);
      TProofNode *ni = 0;
      while ((ni = (TProofNode *) nxni())) {
         TString nodename=ni->GetName(); 
         Int_t nworkers=ni->GetNWrks();

         //set number of files to add for each node
         Int_t nfiles=nf*nworkers;
         Int_t nfilesadded=0;
         Int_t nfile;
         TList* lelement=tdset->GetListOfElements();
         TIter nxtelement(lelement); 
         TDSetElement *tdelement;
         TFileInfo* fileinfo;
         TUrl* url;
         TString hostname, filename, tmpstring;

         while ((tdelement=(TDSetElement*)nxtelement())){

            fileinfo=tdelement->GetFileInfo();
            url=fileinfo->GetCurrentUrl();
            hostname=url->GetHost();
            filename=url->GetFile();

            if (hostname!=nodename) continue;

            //filename=root://hostname/directory/EventTree_Benchmark_filenumber_serial.root
            //remove upto "Benchmark_"
            tmpstring=filename;
            TString stem="_Benchmark_";

            tmpstring.Remove(0, filename.Index(stem)+stem.Length());

            TObjArray* token=tmpstring.Tokenize("_");

            if (token){
               nfile=TString((*token)[0]->GetName()).Atoi();
               token=TString((*token)[1]->GetName()).Tokenize(".");
               Int_t serial=TString((*token)[0]->GetName()).Atoi();

               //ok found, add it to the dataset
               //count only once for set of split files
               if (serial==0){
                  if (nfilesadded>=nfiles){
                     break;
                  }
                  nfilesadded++;
               }

               if (poolurl){
                  url->SetProtocol(newprotocol);
                  url->SetPort(newport);
               }

               fc->Add(fileinfo);
               //Info ("CreateDataSetsN", "added");
            }
            else{
               Error("MakeDataSets", "File name not recognized: %s", fileinfo->GetName());
               return -1;
            }
         }
         if (nfilesadded<nfiles){
            Warning("MakeDataSets", "Not enough number of files; "
                                    "%d files out of %d files requested on node %s "
                                    "are added to data set %s",
                     nfilesadded, nfiles, nodename.Data(), dsname.Data());
         }
      }

      fc->Update();
      // Register dataset
      proof->RegisterDataSet(dsname, fc, option);
      SafeDelete(fc);
   }
   return 0;
}

//______________________________________________________________________________
void TProofBenchModeConstNFilesWorker::SetProof(TProof* proof)
{
   fProof=proof;
}

//______________________________________________________________________________
void TProofBenchModeConstNFilesWorker::SetNFiles(Int_t nfiles)
{
   fNFiles=nfiles;
}

//______________________________________________________________________________
TProof* TProofBenchModeConstNFilesWorker::GetProof()const
{
   return fProof;
}

//______________________________________________________________________________
Int_t TProofBenchModeConstNFilesWorker::GetNFiles()const
{
   return fNFiles;
}

//______________________________________________________________________________
const char* TProofBenchModeConstNFilesWorker::GetName()const
{
   return fName.Data();
}

//______________________________________________________________________________
Int_t TProofBenchModeConstNFilesWorker::FillNodeInfo()
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
