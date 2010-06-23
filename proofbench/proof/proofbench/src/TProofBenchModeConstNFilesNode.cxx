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
#include "TProofNode.h"
#include "TFileCollection.h"
#include "TFileInfo.h"
#include "TProof.h"
#include "TString.h"
#include "TDSet.h"
#include "Riostream.h"
#include "TEnv.h"
#include "TMap.h"
#include "TObjArray.h"

ClassImp(TProofBenchModeConstNFilesNode)

//______________________________________________________________________________
TProofBenchModeConstNFilesNode::TProofBenchModeConstNFilesNode(Int_t nfiles, TProof* proof)
:fProof(proof), fNFiles(nfiles), fNodes(0), fName("ConstNFilesNode")
{
   FillNodeInfo();

   if (fNFiles==-1){
      //Assign maximum number of workers on nodes in the cluster
      TIter nxni(fNodes);
      TProofNode *ni = 0;
      Int_t maxnworkers=0;
      Int_t nworkers;
      while ((ni = (TProofNode *) nxni())) {
         nworkers=ni->GetNWrks();
         maxnworkers=nworkers>maxnworkers?nworkers:maxnworkers;
      }
      fNFiles=maxnworkers;
   }
}

//______________________________________________________________________________
TProofBenchModeConstNFilesNode::~TProofBenchModeConstNFilesNode()
{
}

//______________________________________________________________________________
void TProofBenchModeConstNFilesNode::Print(Option_t* option)const
{
   if (fProof) fProof->Print(option);
   Printf("fNFiles=%d", fNFiles);
   if (fNodes) fNodes->Print(option);
   Printf("fName=%s", fName.Data());
}

//______________________________________________________________________________
TMap* TProofBenchModeConstNFilesNode::FilesToProcess(Int_t nf)
{

//Generates files on worker nodes for I/O test or for cleanup run
//Input parameters do not change corresponding data members
//
//Input parameters
//   nf: Number of files per node. When nf=-1, use data member fNFiles.
//Returns: 
//  map with files to be generated on the worker nodes

   // Create the file names and the map {worker,files}
   // Naming:        <basedir>/EventTree_Benchmark_nfile_serial.root

   TMap *filesmap = new TMap;
   filesmap->SetName("PROOF_FilesToProcess");
   TIter nxni(fNodes);
   TProofNode *ni = 0;

   if (nf==-1){
      nf=fNFiles;
   }

   while ((ni = (TProofNode *) nxni())) {
      TList *files = new TList;
      files->SetName(ni->GetName());
      for (Int_t i = 0; i<nf; i++) {
         files->Add(new TObjString(TString::Format("EventTree_Benchmark_%d_0.root", i)));
      }
      filesmap->Add(new TObjString(ni->GetName()), files);
      //files->Print();
   }
   return filesmap;
}

//______________________________________________________________________________
Int_t TProofBenchModeConstNFilesNode::MakeDataSets(Int_t nf,
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
      Info("MakeDataSets", "Number of files a node is %d for %s", nf, GetName());
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

//______________________________________________________________________________
Int_t TProofBenchModeConstNFilesNode::MakeDataSets(Int_t nf,
                                              Int_t np,
                                              const Int_t *wp,
                                              const TDSet* tdset,
                                              const char *option,
                                              TProof* proof)
{

   // There will be 'nr' datasets per point, rotating the files
   // Dataset naming: DataSetEventConstNFilesNode_nworkersincluster_nfilesanode
   TString smode, stem;
   smode="ConstNFilesNode";
   stem="_Benchmark_";

   TString dsname;
   Int_t kp;
   for (kp=0; kp<np; kp++) {
      // Dataset name
      dsname.Form("DataSetEvent%s_%d_%d", smode.Data(), wp[kp], nf);
      Info("MakeDataSets", "creating dataset '%s' ...", dsname.Data());
      // Create the TFileCollection
      TFileCollection *fc = new TFileCollection;

      TIter nxni(fNodes);
      TProofNode *ni = 0;
      while ((ni = (TProofNode *) nxni())) {
         TString nodename=ni->GetName();

         //set number of files to add for each node
         Int_t nfiles=nf;
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

            //filename=root://hostname//directory/EventTree_Benchmark_filenumber_serial.root
            //remove upto "Benchmark_"
            tmpstring=filename;
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
   fProof=proof;
}

//______________________________________________________________________________
void TProofBenchModeConstNFilesNode::SetNFiles(Int_t nfiles)
{
   fNFiles=nfiles;
}

//______________________________________________________________________________
TProof* TProofBenchModeConstNFilesNode::GetProof()const
{
   return fProof;
}

//______________________________________________________________________________
Int_t TProofBenchModeConstNFilesNode::GetNFiles()const
{
   return fNFiles;
}

//______________________________________________________________________________
const char* TProofBenchModeConstNFilesNode::GetName()const
{
   return fName.Data();
}

//______________________________________________________________________________
Int_t TProofBenchModeConstNFilesNode::FillNodeInfo()
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
