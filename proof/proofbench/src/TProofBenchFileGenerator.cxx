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

#include "TProofBenchFileGenerator.h"
#include "TProofBenchMode.h"
#include "TProofNode.h"
#include "TFile.h"
#include "TFileCollection.h"
#include "TFileInfo.h"
#include "TProof.h"
#include "TString.h"
#include "TDSet.h"
#include "Riostream.h"
#include "THashList.h"
#include "TMap.h"
#include "TEnv.h"
#include "TTree.h"
#include "TH1.h"
#include "TLeaf.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPaveText.h"
#include "TProfile.h"
#include "TLegend.h"
#include "TKey.h"
#include "TMap.h"
#include "TRegexp.h"
#include "TPerfStats.h"
#include "TParameter.h"
#include "TSelectorList.h"
#include "TDrawFeedback.h"
#include "TSortedList.h"
#include "TError.h"

#include <stdlib.h>

ClassImp(TProofBenchFileGenerator)

//______________________________________________________________________________
TProofBenchFileGenerator::TProofBenchFileGenerator(TProofBenchMode* mode,
                                         Long64_t nevents,
                                         Int_t maxnworkers,
                                         Int_t start,
                                         Int_t stop,
                                         Int_t step,
                                         TString basedir,
                                         Int_t ntracks,
                                         Bool_t regenerate,
                                         TProof* proof)
:fProof(proof),
fMode(mode),
fNEvents(nevents),
fMaxNWorkers(maxnworkers),
fStart(start),
fStop(stop),
fStep(step),
fBaseDir(basedir),
fNTracks(ntracks),
fRegenerate(regenerate),
fNodes(0),
fDataSetGenerated(0)
{
   FillNodeInfo();
   if (maxnworkers==-1){
      SetMaxNWorkers("1x");
   }
   else{
      SetMaxNWorkers(maxnworkers);
   }
   if (stop==-1){
      fStop=fMaxNWorkers;
   }
}

//______________________________________________________________________________
TProofBenchFileGenerator::~TProofBenchFileGenerator(){
   if (fDataSetGenerated){
      delete fDataSetGenerated;
   }
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::GenerateFiles(Int_t nf,
                                              Long64_t nevents,
                                              TString basedir,
                                              Int_t regenerate,
                                              Int_t ntracks)
{

//Generates files on worker nodes for I/O test or for cleanup run
//Input parameters do not change corresponding data members
//Data set member (fDataSetGeneratedBench or fDataSetGeneratedCleanup) gets filled up
//with generated data set elements
//
//Input parameters
//   nf: meaning of this parameter is the same as data member NFiles of mode in use
//   nevents: Number of events in a file when filetype==kFileBenchmark.
//            Igonired when filetype==kFileCleanup 
//   basedir: Base directory for the files to be generated on the worker nodes. 
//   regenerate: Regenerate files when kTRUE,
//               Reuse files if they exist, when kFALSE
//               Use data member fRegenerate when <0
//Returns: 
//   0 when ok
//  <0 when anything is wrong

   //Generate files on all nodes, active or inactive
   Int_t nactive_sav;

   if (fProof){
      nactive_sav=fProof->GetParallel();
      fProof->SetParallel(99999);
   }
   else{
      Error("GenerateFiles", "Proof not set, doing nothing");
      return -1;
   }

   //check input parameters
   if (nf<=0){
      nf=fMode->GetNFiles();
   }
   if (nevents<=0){
      nevents=fNEvents;
   }
      
   if (!basedir.Length()){
      basedir=fBaseDir;
   }

   if (regenerate<0){
      regenerate=fRegenerate; 
   }
 
   if (ntracks<0){
      ntracks=fNTracks;
   }
 
   if (fDataSetGenerated) delete fDataSetGenerated;
   fDataSetGenerated = new TDSet("TTree","EventTree");

   TList* wl=fProof->GetListOfSlaveInfos();

   TSortedList* wlcopy=new TSortedList;
   wlcopy->SetName("PROOF_SlaveInfos");

   TIter nwl(wl);
   TSlaveInfo *si=0;
   while (si=(TSlaveInfo*)nwl()){
      wlcopy->Add(new TSlaveInfo(*si));
   }
   fProof->AddInput(wlcopy);

   //build file map to generate on worker nodes
   TMap* filesmap=fMode->FilesToProcess(nf);
   // Add the file map in the input list
   fProof->AddInput(filesmap);

   TProofBenchMode::EFileType filetype=fMode->GetFileType();
   fProof->SetParameter("PROOF_BenchmarkFileType", filetype);
   fProof->SetParameter("PROOF_BenchmarkBaseDir", basedir.Data());
   fProof->SetParameter("PROOF_BenchmarkNEvents", nevents);
   fProof->SetParameter("PROOF_BenchmarkNTracks", ntracks);
   fProof->SetParameter("PROOF_BenchmarkRegenerate", regenerate);

   // Set the packetizer to be the one on test
   fProof->SetParameter("PROOF_Packetizer", "TPacketizerFile");

   // Run
   Int_t entries=filesmap->GetSize();
   fProof->Process("TSelEventGen", entries);

   //Check outputs
   TList* l = fProof->GetOutputList();

   if (!l){
      Error("GenerateFiles", "list of output not found");
      //Put it back to old configuration
      fProof->SetParallel(nactive_sav);
      return -1;
   }

   //l->Print("A");

   //merge outputs

   TObject* obj;
   TList* lfilesgenerated;
   TList* ltdelement;
   TDSet* tdset;
   TString outputname, hostname, filename, newfilename;
   TDSetElement* tdelement;

   TIter nxt(l);
   
   while((obj=nxt())){
      outputname=obj->GetName();
      if (outputname.Contains("PROOF_FilesGenerated")){
         lfilesgenerated=dynamic_cast<TList*>(obj);
         if (lfilesgenerated){
            TObjArray* token=outputname.Tokenize("_"); //filename=PROOF_FilesGenerated_hostname_ordinal
            hostname=((*token)[2])->GetName();

            tdset=dynamic_cast<TDSet*>(lfilesgenerated->At(0)); //lfilesgenerated is 1-element list
            ltdelement=tdset->GetListOfElements();
            TIter nxtelement(ltdelement);

            while((tdelement=(TDSetElement*)nxtelement())){
               filename=tdelement->GetName();
               newfilename="root://"+hostname+"/"+filename;
               tdelement->SetName(newfilename.Data());
            }
            fDataSetGenerated->Add(tdset);
         }
         else{
            Error("GenerateFiles", "%s not type TList*; moving to the next list", outputname.Data());
            continue;
         }
      }
   }

   // Clear the input parameters
   //ClearInputParameters();
   fProof->DeleteParameters("PROOF_BenchmarkFileType");
   fProof->DeleteParameters("PROOF_BenchmarkBaseDir");
   fProof->DeleteParameters("PROOF_BenchmarkNEvents");
   fProof->DeleteParameters("PROOF_BenchmarkRegenerate");
   fProof->DeleteParameters("PROOF_BenchmarkNTracks");

   fProof->DeleteParameters("PROOF_Packetizer");
   fProof->GetInputList()->Remove(filesmap);
   filesmap->SetOwner(kTRUE);
   SafeDelete(filesmap);
   fProof->GetInputList()->Remove(wlcopy);
   wlcopy->SetOwner(kTRUE);
   SafeDelete(wlcopy);

   //Put it back to old configuration
   fProof->SetParallel(nactive_sav);

   //sort output list
   TList* lds=0;
   lds=fDataSetGenerated->GetListOfElements();
   lds->Sort();

   //print outputs
   TIter nds(lds);
   TDSetElement* dse=0;
   Info("GenerateFiles", "List of files generarted:");
   while (dse=(TDSetElement*)nds()){
      dse->Print("A");
   }
   return 0;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::MakeDataSets(Int_t nf,
                                             Int_t start,
                                             Int_t stop,
                                             Int_t step,
                                             const TDSet* tdset,
                                             const char* option)
{
//Make data sets (file collection) and register them for benchmark test
//
//Input parameters
//   nf: The first arguement to TProofBenchMode::MakeDataSets(...). The exact meaning of this parameter depends on the mode being used.
//   start: Starting number of workers, when -1 (default) data member fStart is used
//   stop: Ending number of workers, when -1 (default) data member fStop is used
//   step: Incremental number of workers, when -1 (default) data member fStep is used
//   tdset: data set (TDSet*) from which file collection will be built within this function. 
//          When tdset=0, the data set built by TProofBenchFileGenerator::GenerateFiles is used. User can also supply one.
//   option: is used as option to TProof::RegisterDataSet
//Returns: 0 when ok, <0 when anything is wrong

   //check input parameters
   if (start==-1){
      start=fStart;
      Info("MakeDataSets", "Start number of workers is %d", start);
   }
   if (stop==-1){
       stop=fStop;
       Info("MakeDataSets", "End number of workers is set to %d", stop);
   }
   if (step==-1){
       step=fStep;
       Info("MakeDataSets", "Step number of workers is set to %d", step);
   }
   if (start>stop){
       Error("MakeDataSets", "Wrong inputs; start(%d)>stop(%d)", start, stop);
       return -1;
   }

   if (tdset){
      fMode->MakeDataSets(nf, start, stop, step, tdset, option, fProof);
      return 0;
   }
   else{
      if (fDataSetGenerated){
         fMode->MakeDataSets(nf, start, stop, step, fDataSetGenerated, option, fProof);
      }
      else{
         Error("MakeDataSet", "Generate files first");
         return -1;
      }
   }

   return 0;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::MakeDataSets(const char* option)
{
//Make data sets (file collection) and register them for benchmark test
//
//Input parameters
//   option: is used as option to TProof::RegisterDataSet
//Returns: 0 when ok, 
//        -1 when files are not generated

   if ( fDataSetGenerated ) {
      fMode->MakeDataSets(-1, fStart, fStop, fStep, fDataSetGenerated, option, fProof);
   }
   else{
      Error("MakeDataSet", "Generate files first");
      return -1;
   }

   return 0;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::MakeDataSets(Int_t nf,
                                             Int_t np,
                                             const Int_t *wp,
                                             const TDSet* tdset,
                                             const char* option)
{
   
  
//Create the data sets (file collection) and register them to Proof
//Input parameters:
//   mode: benchmark mode
//   np: Number of test points
//   wp: Array of number of workers for test points (should contain np points)
//   nf: Number of files 
//       It corresponds to fNFilesANode when mode is kModeConstNFilesNode,
//       fNFilesAWorker when mode=kModeConstNFilesWorker,
//       fNFilesAWorker when mode=kModeVaryingNFilesWorker
//   opt: Option to TProof::RegisterDataSet
//   tdset: Data set (TDSet*) from which file collections are to be built from. 
//          This is usually the data set built when GenerateFiles function is called but user can supply one.
//Returns:
//   0 when ok
//  <0 when anything is wrong

   fMode->MakeDataSets(nf, np, wp, tdset, option, fProof);
   return 0;
}

//______________________________________________________________________________
void TProofBenchFileGenerator::Print(Option_t* option)const{

   if (fProof) fProof->Print(option);
   Printf("fBaseDir=\"%s\"", fBaseDir.Data()); 

   //Printf("fMaxNWorkers=%d", fMaxNWorkers);
   Printf("fNEvents=%lld", fNEvents);
   Printf("fStart=%d", fStart);
   Printf("fStop=%d", fStop);
   Printf("fStep=%d", fStep);

   //Printf("fNTracksBench=%d", fNTracksBench);
   //Printf("fNTracksCleanup=%d", fNTracksCleanup);
   //Printf("fDebug=%d", fDebug);
   //Printf("fRegenerate=%d", fRegenerate);

   if (fMode) fMode->Print("A");

   if (fDataSetGenerated) fDataSetGenerated->Print(option);
   //if (fCPerfProfiles){
   //   Printf("Performance Profiles Canvas: Name=%s Title=%s", 
   //           fCPerfProfiles->GetName(), fCPerfProfiles->GetTitle());
   //}
   //if (fDirProofBench){
   //   Printf("fDirProofBench=%s", fDirProofBench->GetPath());
   //}

   if (fNodes) fNodes->Print(option);
}

//______________________________________________________________________________
void TProofBenchFileGenerator::SetMode(TProofBenchMode* mode)
{
   fMode=mode;
}

//______________________________________________________________________________
void TProofBenchFileGenerator::SetNEvents(Long64_t nevents)
{
   fNEvents=nevents;
}

//______________________________________________________________________________
void TProofBenchFileGenerator::SetMaxNWorkers(Int_t maxnworkers)
{
   fMaxNWorkers=maxnworkers;
}

//______________________________________________________________________________
void TProofBenchFileGenerator::SetMaxNWorkers(TString sworkers)
{
   sworkers.ToLower();
   sworkers.Remove(TString::kTrailing, ' ');
   if (fProof){
      if (sworkers.Contains("x")){//nx
         TList* lslave=fProof->GetListOfSlaveInfos();
         Int_t nslaves=lslave->GetSize();  //number of slave workers regardless of its status, active or inactive
         sworkers.Remove(TString::kTrailing, 'x');
         Int_t mult=sworkers.Atoi();
         fMaxNWorkers=mult*nslaves; //this number to be parameterized in the future
      }
   }
   else{
      Error("SetMaxNWorkers", "Proof not set, doing nothing");
   }
   return;
}

//______________________________________________________________________________
void TProofBenchFileGenerator::SetStart(Int_t start)
{
   fStart=start;
}

//______________________________________________________________________________
void TProofBenchFileGenerator::SetStop(Int_t stop)
{
   fStop=stop;
}

//______________________________________________________________________________
void TProofBenchFileGenerator::SetStep(Int_t step)
{
   fStep=step;
}

//______________________________________________________________________________
void TProofBenchFileGenerator::SetBaseDir(TString basedir)
{
   fBaseDir=basedir;
}

//______________________________________________________________________________
void TProofBenchFileGenerator::SetNTracks(Int_t ntracks)
{
   fNTracks=ntracks;
}

//______________________________________________________________________________
void TProofBenchFileGenerator::SetRegenerate(Int_t regenerate)
{
   fRegenerate=regenerate;
}

//______________________________________________________________________________
TProofBenchMode* TProofBenchFileGenerator::GetMode()const
{
   return fMode;
}

//______________________________________________________________________________
Long64_t TProofBenchFileGenerator::GetNEvents()const
{
   return fNEvents;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::GetMaxNWorkers()const
{
   return fMaxNWorkers;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::GetStart()const
{
   return fStart;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::GetStop()const
{
   return fStop;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::GetStep()const
{
   return fStep;
}

//______________________________________________________________________________
TString TProofBenchFileGenerator::GetBaseDir()const
{
   return fBaseDir;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::GetNTracks()const
{
   return fNTracks;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::GetRegenerate()const
{
   return fRegenerate;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::FillNodeInfo()
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
