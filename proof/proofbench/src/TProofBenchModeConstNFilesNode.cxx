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
// TProofBenchModeConstNFilesNode                                       //
//                                                                      //
// A mode for PROOF benchmark test.                                     //
// In this mode, the same number of files are generated for all nodes   //
// in the cluster regardless of number of workers on the node.          //
// During the test, all of these files are processed no matter how many //
// workers are active.                                                  //
// Default number of files on each node is the maximum number of workers//
// on a node in the cluster.                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofBenchModeConstNFilesNode.h"
#include "TProofNodes.h"
#include "TFileCollection.h"
#include "TFileInfo.h"
#include "TProof.h"
#include "TString.h"
#include "Riostream.h"
#include "TEnv.h"
#include "TMap.h"
#include "TObjArray.h"

ClassImp(TProofBenchModeConstNFilesNode)

//______________________________________________________________________________
TProofBenchModeConstNFilesNode::TProofBenchModeConstNFilesNode(Int_t nfiles,
                                          TProof* proof, TProofNodes* nodes)
:fProof(proof), fNFiles(nfiles), fNodes(nodes), fName("ConstNFilesNode")
{

   if (!fProof){
      fProof=gProof;
   }

   if (fNFiles==-1){
      //Assign maximum number of workers on nodes in the cluster
      Int_t maxnworkers=fNodes->GetMaxNWorkers();
      fNFiles=maxnworkers;
   }
}

//______________________________________________________________________________
TProofBenchModeConstNFilesNode::~TProofBenchModeConstNFilesNode()
{
}

//______________________________________________________________________________
void TProofBenchModeConstNFilesNode::Print(Option_t* option) const
{
   if (fProof) fProof->Print(option);
   Printf("fNFiles=%d", fNFiles);
   if (fNodes) fNodes->Print(option);
   Printf("fName=%s", fName.Data());
}

//______________________________________________________________________________
TMap* TProofBenchModeConstNFilesNode::FilesToProcess(Int_t nf)
{

   // Create a map of files to be generated on worker nodes.
   // File name format is "EventTree_Benchmark_nfile_serial.root".
   // Input parameters do not change corresponding data members.
   //
   // Input parameters
   //    nf: Number of files per node. 
   //        When nf=-1, use data member fNFiles.
   // Returns
   //    Map of files to be generated on the worker nodes.
   
   if (nf==-1){
      nf=fNFiles;
   }

   TMap *filesmap = new TMap;
   filesmap->SetName("PROOF_FilesToProcess");
   TList* nodes=fNodes->GetListOfNodes();
   TIter nxtnode(nodes);
   TList* node=0;
   while ((node = (TList*) nxtnode())) {
      TList *files = new TList;
      files->SetName(node->GetName());
      for (Int_t i = 0; i<nf; i++) {
         files->Add(new TObjString(TString::Format("%s_EventTree_Benchmark_%d_0.root", node->GetName(), i)));
      }
      filesmap->Add(new TObjString(node->GetName()), files);
   }

   Info("FilesToProcess", "Map of files to be generated:");
   filesmap->Print("A", -1);

   return filesmap;
}

//______________________________________________________________________________
Int_t TProofBenchModeConstNFilesNode::MakeDataSets(Int_t nf, Int_t start,
                                 Int_t stop, Int_t step, const TList* listfiles,
                                 const char* option, TProof* proof)
{
   // Make data sets out of list of files 'listfiles' and register them.
   // Input parameters
   //    nf: Number of files a node.
   //        When ==-1, data member fNFiles are used.
   //    start: Start scan at 'start' number of workers.
   //    stop: Stop scan at 'stop' number of workers.
   //    step: Scan every 'step' workers.
   //    listfiles: List of files (TFileInfo*) ouf of which data sets are built and registered.
   //    option: Option to TProof::RegisterDataSet(...).
   //    proof: Proof
   // Return
   //   0 when ok
   //  <0 otherwise

   if (!fProof){
      Error("MakeDataSets", "proof not set, doing nothing");
      return -1;
   }

   if (!listfiles){
       Error("MakeDataSets", "No generated files provided; returning");
       return -1;
   }

   if (nf==-1){
      nf=fNFiles;
      //Info("MakeDataSets", "Number of files a node is %d for %s", nf,
      //                                                            GetName());
   }

   // Default max worker number is the number of all workers in the cluster
   if (stop==-1){
      stop=fNodes->GetNWorkersCluster();
   }

   Info("MakeDataSets", "Making data sets for mode %s, number of files a node"
        "=%d, %d ~ %d active worker(s), every %d worker(s).", 
         GetName(), nf, start, stop, step);

   const Int_t np=(stop-start)/step+1;
   Int_t wp[np];
   Int_t i=0;
   for (Int_t nactive=start; nactive<=stop; nactive+=step) {
      wp[i]=nactive;
      i++;
   }

   return MakeDataSets(nf, np, wp, listfiles, option, proof);
}

//______________________________________________________________________________
Int_t TProofBenchModeConstNFilesNode::MakeDataSets(Int_t nf, Int_t np,
                                       const Int_t *wp, const TList* listfiles,
                                       const char *option, TProof* proof)
{
   // Make data sets out of list of files 'listfiles' and register them.
   // Data set name has the form : DataSetEventConstNFilesNode_nactiveworkersincluster_nfilesanode
   // Input parameters
   //    nf: Number of files a node.
   //    np: Number of test points.
   //    wp: 'np'-sized array containing the number of active workers to process files.
   //    listfiles: List of files (TFileInfo*) ouf of which data sets are built and registered.
   //    option: Option to TProof::RegisterDataSet(...).
   //    proof: Proof
   // Return
   //   0 when ok
   //  <0 otherwise

   if (!fProof){
      Error("MakeDataSets", "proof not set, doing nothing");
      return -1;
   }

   if (!listfiles){
      Error("MakeDataSets", "No generated files provided; returning");
      return -1;
   }

   if (nf==-1){
      nf=fNFiles;
      Info("MakeDataSets", "Number of files a node is %d for %s", nf,
                                                                  GetName());
   }

   TString dsname;
   Int_t kp;
   for (kp=0; kp<np; kp++) {
      // Dataset name
      dsname.Form("DataSetEvent%s_%d_%d", GetName(), wp[kp], nf);
      Info("MakeDataSets", "Creating dataset '%s' for %d active worker(s).",
            dsname.Data(), wp[kp]);
      // Create the TFileCollection
      TFileCollection *fc = new TFileCollection;

      TList* nodes=fNodes->GetListOfNodes();
      TIter nxtnode(nodes);
      TList *node = 0;
      while ((node = (TList*) nxtnode())) {
         TString nodename=node->GetName();

         //set number of files to add for each node
         Int_t nfiles=nf;
         Int_t nfilesadded=0;
         Int_t nfile;

         TIter nxtfileinfo(listfiles);
         TFileInfo* fileinfo;
         TUrl* url;
         TString hostname, filename, tmpstring;

         while ((fileinfo=(TFileInfo*)nxtfileinfo())){

            url=fileinfo->GetCurrentUrl();
            hostname=url->GetHost();
            filename=url->GetFile();

            if (hostname!=nodename) continue;

            //filename=root://hostname//directory/
            //EventTree_Benchmark_filenumber_serial.root
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
               fc->Add((TFileInfo*)(fileinfo->Clone()));
               //Info ("CreateDataSetsN", "added");
            }
            else{
               Error("MakeDataSets", "File name not recognized: %s",
                                      fileinfo->GetName());
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
      // Register dataset with verification
      proof->RegisterDataSet(dsname, fc, option);
      SafeDelete(fc);
   }
   return 0;
}

//______________________________________________________________________________
TProofBenchMode::EFileType TProofBenchModeConstNFilesNode::GetFileType()
{
   return TProofBenchMode::kFileBenchmark;
}

//______________________________________________________________________________
void TProofBenchModeConstNFilesNode::SetProof(TProof* proof)
{
   if (!proof){
      Warning("SetProof", "proof is null; doing nothing");
   }
   fProof=proof;
}

//______________________________________________________________________________
void TProofBenchModeConstNFilesNode::SetNFiles(Int_t nfiles)
{
   if (nfiles<=0){
      Warning("SetNFiles", "Number of files per worker does not make sense;"
                           " Doing nothing");
      return;
   }
   fNFiles=nfiles;
}

//______________________________________________________________________________
TProof* TProofBenchModeConstNFilesNode::GetProof() const
{
   return fProof;
}

//______________________________________________________________________________
Int_t TProofBenchModeConstNFilesNode::GetNFiles() const
{
   return fNFiles;
}

//______________________________________________________________________________
const char* TProofBenchModeConstNFilesNode::GetName() const
{
   return fName.Data();
}
