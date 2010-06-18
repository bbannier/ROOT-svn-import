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

TProofBenchFileGenerator::~TProofBenchFileGenerator(){
   if (fDataSetGenerated){
      delete fDataSetGenerated;
   }
}

/*
//_________________________________________________________________________________
Int_t TProofBench::GenerateFilesN(Int_t nf, Long64_t fileent)
{
   // Generate the files needed for the test using TPacketizerFile
   // *** This is only to show how it works ***

   // Find out the number of physically different machines
   if (FillNodeInfo()) {
      Error("GenerateFilesN", "could not get information about workers!");
      return -1;
   }

   // Number of events per file
   Long64_t oldNEvents = fNEvents;
   fNEvents = fileent;

   // Create the file names and the map {worker,files}
   // Naming:
   //         <basedir>/event_<file>.root
   TMap *filesmap = new TMap;
   filesmap->SetName("PROOF_FilesToProcess");
   Long64_t entries = 0;
   TIter nxni(fNodes);
   TProofNode *ni = 0;
   while ((ni = (TProofNode *) nxni())) {
      TList *files = new TList;
      files->SetName(ni->GetName());
      Int_t i = 0;
      for (i = 0; i < nf; i++) {
         files->Add(new TObjString(TString::Format("%s/event_%d.root", fBaseDir.Data(), i)));
         entries++;
      }
      filesmap->Add(new TObjString(ni->GetName()), files);
      files->Print();
   }

   // Set the relevant input parameters
   //SetInputParameters();
   if (fProof){
      fProof->SetParameter("PROOF_BenchmarkBaseDir", fBaseDir.Data());
      fProof->SetParameter("PROOF_BenchmarkRunType", fRunType);
      fProof->SetParameter("PROOF_BenchmarkNHists", fNHists);
      fProof->SetParameter("PROOF_BenchmarkHistType", fHistType);
      fProof->SetParameter("PROOF_BenchmarkNTries", fNTries);
      fProof->SetParameter("PROOF_BenchmarkNEvents", fNEvents);
      fProof->SetParameter("PROOF_BenchmarkDraw", Int_t(fDraw));
      fProof->SetParameter("PROOF_BenchmarkRegenerate", Int_t(fRegenerate));
      fProof->SetParameter("PROOF_BenchmarkMode", fBenchmarkMode);
      fProof->SetParameter("PROOF_BenchmarkNFilesAWorker", fNFilesAWorker);
      fProof->SetParameter("PROOF_BenchmarkNFilesANode", fNFilesANode);
   }
   else{
      Error("GenerateFilesN", "Proof not set, doing noting");
   }

   // Add the file map in the input list
   fProof->AddInput(filesmap);

   // Set the packetizer to be the one on test
   fProof->SetParameter("PROOF_Packetizer", "TPacketizerFile");

   // Run
   fProof->Process("TSelEventGenN", entries);

   // Clear the input parameters
   ClearInputParameters();
   fProof->DeleteParameters("PROOF_Packetizer");
   fProof->GetInputList()->Remove(filesmap);
   filesmap->SetOwner(kTRUE);
   SafeDelete(filesmap);

   // Restore previous value
   fNEvents = oldNEvents;

   // Done
   return 0;
}*/

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
/*
//_________________________________________________________________________________
Int_t TProofBench::CreateDataSetsN(const char *basedir, const char *lab,
                                   Int_t np, const Int_t *wp,
                                   Int_t nr, Int_t nfw, Int_t nfmx)
{
   // Create the datasets for tests wth PROOF-Lite
   // NB: Should be extended to the case the files are not local

   // Check inputs
   if (np <= 0 || !wp || !fProof) {
      Error("CreateDataSetsN", "wrong inputs (%d, %p, %p)", np, wp, fProof);
      return -1;
   }

   // There will be 'nr' datasets per point, rotating the files
   // Dataset naming:
   //                   ds_event_[lab]_[wrks_point]_[run]
   //
   TString dsname, slab("");
   if (lab && strlen(lab) > 0) slab.Form("_%s", lab);
   Int_t kp, kr, kf, kk = 0;
   for (kp = 0; kp < np; kp++) {
      for (kr = 0; kr < nr; kr++) {
         // Dataset name
         dsname.Form("ds_event%s_%d_%d", slab.Data(), wp[kp], kr);
         Info("CreateDataSetsN", "creating dataset '%s' ...", dsname.Data());
         // Create the TFileCollection
         TFileCollection *fc = new TFileCollection;
         Int_t nf = nfw * wp[kp];
         for (kf = 0; kf < nf; kf++) {
            Info("CreateDataSetsN", "adding file '%s/event_%d.root' ...", basedir, kk);
            fc->Add(TString::Format("%s/event_%d.root", basedir, kk++));
            if (kk >= nfmx) kk = 0;
         }
         fc->Update();
         // Register dataset with verification
         fProof->RegisterDataSet(dsname, fc, "OV");
         // Cleanup
         delete fc;
      }
   }
   // Done
   return 0;
}

Int_t TProofBench::GenerateDataSetN(const char *dset,
                                     Int_t nw, Int_t nfw, Long64_t fileent)
{
   // Generate the files needed for a test on PROOF-Lite with nw workers.
   // It will generate nfw files per worker for 'nr' different runs.
   // Each file will have 'fileent' entries.
   // A set of datasets 'dset_[nw]_[run]' are automatically registered and verified
   // Uses TPacketizerFile.
   // *** This is only to show how it works ***

   // Number of events per file
   Long64_t oldNEvents = fNEvents;
   fNEvents = fileent;

   // Create dataset containers
   Int_t nr = 4;
   TFileCollection fcs[4];
   Int_t ir = 0;
   for (ir = 0; ir < nr ; ir++) {
     fcs[ir].SetName(TString::Format("%s_%d_%d", dset, nw, ir));
   }

   // Number of files
   Int_t nf = nw * nfw;

   // Create the file names and the map {worker,files}
   // Naming:
   //         <basedir>/event_<file>.root
   TMap *filesmap = new TMap;
   filesmap->SetName("PROOF_FilesToProcess");
   TList *files = new TList;
   files->SetName(gSystem->HostName());
   Long64_t entries = 0;
   Int_t kk = 0;
   for (ir = 0; ir < nr ; ir++) {
      Int_t i = 0;
      for (i = 0; i < nf; i++) {
         fcs[ir].Add(new TFileInfo(TString::Format("%s/event_%d.root", fBaseDir.Data(), kk)));
         files->Add(new TObjString(TString::Format("%s/event_%d.root", fBaseDir.Data(), kk++)));
         entries++;
      }
   }
   filesmap->Add(new TObjString((gSystem->HostName())), files);
   files->Print();

   // Set the relevant input parameters
   //SetInputParameters();
   if (fProof){
      fProof->SetParameter("PROOF_BenchmarkRunType", fRunType);
      fProof->SetParameter("PROOF_BenchmarkNEvents", fNEvents);
      fProof->SetParameter("PROOF_BenchmarkNTracksBench", fNTracksBench);
      fProof->SetParameter("PROOF_BenchmarkNTracksCleanup", fNTracksCleanup);
   }
   else{
      Error("GenerateFilesN", "Proof not set, doing noting");
   }

   // Add the file map in the input list
   fProof->AddInput(filesmap);

   // Set the packetizer to be the one on test
   fProof->SetParameter("PROOF_Packetizer", "TPacketizerFile");

   // Run
   fProof->Process("TSelEventGenN", entries);

   // Clear the input parameters
   ClearInputParameters();
   fProof->DeleteParameters("PROOF_Packetizer");
   fProof->GetInputList()->Remove(filesmap);
   filesmap->SetOwner(kTRUE);
   SafeDelete(filesmap);

   // Restore previous value
   fNEvents = oldNEvents;

   // Register and verify the datasets
   for (ir = 0; ir < nr ; ir++) {
     fProof->RegisterDataSet(fcs[ir].GetName(), &fcs[ir], "OV");
   }

   // Done
   return 0;
}*/

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

//_________________________________________________________________________________
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
/*
   // Check inputs
   if (np <= 0 || !wp || !fProof) {
      Error("MakeDataSets", "wrong inputs (%d, %p, %p)", np, wp, fProof);
      return -1;
   }

   if (!tdset){
      if (fDataSetGeneratedBench){
         tdset=fDataSetGeneratedBench;
      }
      else{
         Error("MakeDataSets", "Empty data set; Files not generated");
         return -1;
      }
   }
   TString smode, stem;
   switch (mode){
      case kModeConstNFilesNode:
         smode="ConstNFilesNode";
         stem="_Benchmark_";
      break;
      case kModeConstNFilesWorker:
         if (!tdset){
            if (fDataSetGeneratedBench){
               tdset=fDataSetGeneratedBench;
            }
            else{
               Error("MakeDataSets", "Empty data set; Files not generated");
               return -1;
            }
         }
         smode="ConstNFilesWorker";
         stem="_Benchmark_";
      break;
      case kModeVaryingNFilesWorker:
         if (!tdset){
            if (fDataSetGeneratedBench){
               tdset=fDataSetGeneratedBench;
            }
            else{
               Error("MakeDataSets", "Empty data set; Files not generated");
               return -1;
            }
         }
         smode="VaryingNFilesWorker";
         stem="_Benchmark_";
      break;
      case kModeCleanup:
         Error("MakeDataSets", "Nothing to be done for this mode; %d", mode);
      break;
      default:
         Error("MakeDataSets", "Benchmark mode not recognized; %d", mode);
         return -1;
      break;
   }
   // There will be 'nr' datasets per point, rotating the files
   // Dataset naming:
   //                   ds_event_[lab]_[wrks_point]_[run]
   TString dsname;

   if (mode==kModeConstNFilesNode|| mode==kModeConstNFilesWorker){
      Int_t kp;
      for (kp = 0; kp < np; kp++) {
         // Dataset name
 	 dsname.Form("DataSetEvent%s_%d_%d", smode.Data(), wp[kp], nf);
         Info("MakeDataSets", "creating dataset '%s' ...", dsname.Data());
         // Create the TFileCollection
         TFileCollection *fc = new TFileCollection;

         TIter nxni(fNodes);
         TProofNode *ni = 0;
         while ((ni = (TProofNode *) nxni())) {
            TString nodename=ni->GetName();
            Int_t nworkers=ni->GetNWrks();

            //set number of files to add for each node
            Int_t nfiles=0;
            if (mode==kModeConstNFilesNode){
                  nfiles=nf;
            }
            else if (kModeConstNFilesWorker){
                  nfiles=nf*nworkers;
            }
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
         fProof->RegisterDataSet(dsname, fc, opt);
         SafeDelete(fc);
      }
   }
   else if (mode==kModeVaryingNFilesWorker){
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
         TList *wl = fProof->GetListOfSlaveInfos();
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
         fProof->RegisterDataSet(dsname, fc, opt);
         lecopy->SetOwner(kTRUE);
         SafeDelete(lecopy);
         SafeDelete(fc);
      }
   }
*/
   return 0;
}

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

void TProofBenchFileGenerator::SetMode(TProofBenchMode* mode)
{
   fMode=mode;
}

void TProofBenchFileGenerator::SetNEvents(Long64_t nevents)
{
   fNEvents=nevents;
}

void TProofBenchFileGenerator::SetMaxNWorkers(Int_t maxnworkers)
{
   fMaxNWorkers=maxnworkers;
}

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

void TProofBenchFileGenerator::SetStart(Int_t start)
{
   fStart=start;
}

void TProofBenchFileGenerator::SetStop(Int_t stop)
{
   fStop=stop;
}

void TProofBenchFileGenerator::SetStep(Int_t step)
{
   fStep=step;
}

void TProofBenchFileGenerator::SetBaseDir(TString basedir)
{
   fBaseDir=basedir;
}

void TProofBenchFileGenerator::SetNTracks(Int_t ntracks)
{
   fNTracks=ntracks;
}

void TProofBenchFileGenerator::SetRegenerate(Int_t regenerate)
{
   fRegenerate=regenerate;
}

TProofBenchMode* TProofBenchFileGenerator::GetMode()const
{
   return fMode;
}

Long64_t TProofBenchFileGenerator::GetNEvents()const
{
   return fNEvents;
}

Int_t TProofBenchFileGenerator::GetMaxNWorkers()const
{
   return fMaxNWorkers;
}

Int_t TProofBenchFileGenerator::GetStart()const
{
   return fStart;
}

Int_t TProofBenchFileGenerator::GetStop()const
{
   return fStop;
}

Int_t TProofBenchFileGenerator::GetStep()const
{
   return fStep;
}

TString TProofBenchFileGenerator::GetBaseDir()const
{
   return fBaseDir;
}

Int_t TProofBenchFileGenerator::GetNTracks()const
{
   return fNTracks;
}

Int_t TProofBenchFileGenerator::GetRegenerate()const
{
   return fRegenerate;
}

//_________________________________________________________________________________
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
