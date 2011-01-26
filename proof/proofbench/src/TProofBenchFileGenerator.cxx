// @(#)root/proofx:$Id$
// Author: Sangsu Ryu 22/6/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBenchFileGenerator                                             //
//                                                                      //
// This class lets you generate files and register them as datasets     //
// to be used for Proof benchmark test.                                 //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofBenchFileGenerator.h"
#include "TProofBenchMode.h"
#include "TProofBenchTypes.h"
#include "TProofNodes.h"
#include "TFile.h"
#include "TFileCollection.h"
#include "TFileInfo.h"
#include "TProof.h"
#include "TString.h"
#include "Riostream.h"
#include "TMap.h"
#include "TProfile.h"
#include "TKey.h"
#include "TPerfStats.h"
#include "TParameter.h"
#include "TSortedList.h"
#include "TError.h"

ClassImp(TProofBenchFileGenerator)

//______________________________________________________________________________
TProofBenchFileGenerator::TProofBenchFileGenerator(TProofBenchMode* mode,
                               Long64_t nevents, Int_t start, Int_t stop,
                               Int_t step, TString basedir, Int_t ntracks,
                               Bool_t regenerate, TProof* proof,
                               TProofNodes* nodes)
:fProof(0), fMode(mode), fNEvents(nevents), fStart(start), fStop(stop),
fStep(step), fBaseDir(basedir), fNTracks(ntracks), fRegenerate(regenerate),
fNodes(nodes), fFilesGenerated(0)
{
   fProof=proof?proof:gProof;

   if (stop==-1){
      if (fNodes){
         fStop=fNodes->GetNWorkersCluster();
      }
      else{
         if (fProof){
            fStop=fProof->GetListOfSlaveInfos()->GetSize();
         }
      }
   }

   if (fBaseDir.IsNull() && fProof){
      TUrl url(fProof->GetDataPoolUrl());
      fBaseDir=url.GetFile();
   }
}

//______________________________________________________________________________
TProofBenchFileGenerator::~TProofBenchFileGenerator(){
   if (fFilesGenerated){
      delete fFilesGenerated;
   }
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::GenerateFiles(Int_t nfiles, Long64_t nevents,
                              TString basedir, Int_t regenerate, Int_t ntracks)
{

   // Generates files on worker nodes for I/O test or for cleanup run
   // Input parameters do not change corresponding data members
   // Member 'fFilesGenerated' gets filled up with generated fileinfos
   //
   // Input parameters
   //    nfiles: This parameter is passed to the mode in use.
   //        When -1, data member fNFiles of mode in use is used.
   //    nevents: Number of events in a file
   //             When clean-up files are generated, this parameter is ignored
   //             and files large enough to clean up node memory are generated.
   //    basedir: Base directory for the files to be generated on the worker
   //             nodes. When null, data member fBaseDir is used.
   //    regenerate: Regenerate files when >0,
   //                Reuse files if they exist when =0, 
   //                Use data member fRegenerate when ==-1
   //    ntracks: Number of traks in an event. Use data member fNTracks
   //             when ==-1
   // Return: 
   //    0 when ok
   //   <0 otherwise

   if (!fMode){
      Error("GenerateFiles", "mode not set, doing nothing");
      return -1;
   }

   Info("GenerateFiles", "Generating files for mode %s.", fMode->GetName());
   Int_t nactive_sav;

   if (fProof){
      nactive_sav=fProof->GetParallel();
      fProof->SetParallel(99999);
   } else{
      Error("GenerateFiles", "Proof not set, doing nothing");
      return -1;
   }


   // Check input parameters
   if (nfiles==-1){
      nfiles=fMode->GetNFiles();
   }
   if (nevents==-1){
      nevents=fNEvents;
   }
      
   if (basedir.IsNull()){
      basedir=fBaseDir;
   }

   if (regenerate==-1){
      regenerate=fRegenerate; 
   }
 
   if (ntracks==-1){
      ntracks=fNTracks;
   }
 
   if (fFilesGenerated) delete fFilesGenerated;
   fFilesGenerated = new TList();
   fFilesGenerated->SetOwner();

   TList* wl=fProof->GetListOfSlaveInfos();

   TSortedList* wlcopy=new TSortedList;
   wlcopy->SetName("PROOF_SlaveInfos");

   TIter nwl(wl);
   TSlaveInfo *si=0;
   while ((si = (TSlaveInfo *)nwl())) {
      wlcopy->Add(new TSlaveInfo(*si));
   }
   fProof->AddInput(wlcopy);

   // Build file map to generate on worker nodes
   TMap* filesmap=fMode->FilesToProcess(nfiles);

   if (!filesmap){
      Error("GenerateFiles", "Could not figure out files to be generated;"
                             " returning.");
      return -1;
   }

   // Add the file map in the input list
   fProof->AddInput(filesmap);

   EPBFileType filetype = fMode->GetFileType();
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

   // Check outputs
   TList* l = fProof->GetOutputList();

   if (!l){
      Error("GenerateFiles", "list of output not found");
      // Put it back to old configuration
      fProof->SetParallel(nactive_sav);
      return -1;
   }

   // Merge outputs
   TObject* obj;
   TList* lfilesgenerated;
   TString outputname, hostname, filename, newfilename;

   TIter nxt(l);
   while((obj=nxt())){
      outputname=obj->GetName();
      if (outputname.Contains("PROOF_FilesGenerated")){
         if ((lfilesgenerated=dynamic_cast<TList*>(obj))){
            TIter nxtobj(lfilesgenerated);
            TObject* xobj=0;
            while ((xobj = nxtobj())) {
               fFilesGenerated->Add(xobj);
            }
         }
         else{
            Error("GenerateFiles", "%s not type TList*; moving to the next"
                  " list", outputname.Data());
            continue;
         }
      }
   }

   // Sort output list
   fFilesGenerated->Sort();

   //replace protocol and port
   TUrl datapoolurl(fProof->GetDataPoolUrl());
   const char* datapoolprotocol=datapoolurl.GetProtocol();
   Int_t datapoolport=datapoolurl.GetPort();

   TIter nxtfi(fFilesGenerated);
   TFileInfo* fi=0;
   while ((fi = (TFileInfo*)nxtfi())) {
      fi->GetFirstUrl()->SetProtocol(datapoolprotocol);
      fi->GetFirstUrl()->SetPort(datapoolport);
   }
   
   // Print outputs
   Info("GenerateFiles", "List of files generarted or that exist already:");
   fFilesGenerated->Print("A");

   // Clear input parameters
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

   // Put it back to old configuration
   fProof->SetParallel(nactive_sav);

   return 0;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::MakeDataSets(Int_t nfiles, Int_t start,
                                Int_t stop, Int_t step, const TList* listfiles,
                                const char* option)
{
   // Make data sets out of list of files 'listfiles' and register them
   // for benchmark test
   //
   // Input parameters
   //    nfiles: This parameter is passed to the mode in use.
   //        When -1, data member fNFiles of mode in use is used.
   //    start: Start number of workers, when -1 (default) data member fStart
   //           is used
   //    stop: Ending number of workers, when -1 (default) data member fStop
   //          is used
   //    step: Incremental number of workers, when -1 (default) data member
   //          fStep is used
   //    listfiles: List of files (TFileInfo*) from which file collection will
   //               be built in this function. 
   //               When listfiles=0, the list filled up
   //               by TProofBenchFileGenerator::GenerateFiles is used.
   //    option: Option to TProof::RegisterDataSet
   // Return: 0 when ok
   //        <0 otherwise

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

   if (listfiles){
      fMode->MakeDataSets(nfiles, start, stop, step, listfiles, option, fProof);
      return 0;
   }
   else{
      if (fFilesGenerated){
         fMode->MakeDataSets(nfiles, start, stop, step, fFilesGenerated,
                             option, fProof);
      }
      else{
         Error("MakeDataSet", "Empty data set; Either generate files first"
                              " or supply one.");
         return -1;
      }
   }

   return 0;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::MakeDataSets(const char* option)
{
   // Make data sets and register them for benchmark test
   //
   // Input parameters
   //    option: Option to TProof::RegisterDataSet
   // Return: 0 when ok, 
   //        <0 otherwise

   if ( fFilesGenerated ) {
      fMode->MakeDataSets(-1, fStart, fStop, fStep, fFilesGenerated, option,
                         fProof);
   }
   else{
      Error("MakeDataSet", "Generate files first");
      return -1;
   }

   return 0;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::MakeDataSets(Int_t nfiles, Int_t np,
                                const Int_t *wp, const TList* listfiles,
                                const char* option)
{
   // Create the data sets out of list of files 'listfiles' and register them
   // for benchmark test
   //
   // Input parameters:
   //    nfiles: This parameter is passed to the mode in use.
   //            When -1, data member fNFiles of mode in use is used.
   //    np: Number of test points
   //    wp: Array of number of workers for test points (should contain np 
   //        points)
   //    listfiles: List of files (TFileInfo*) from which file collection will
   //               be built in this function. When listfiles=0, the list filled
   //               up by TProofBenchFileGenerator::GenerateFiles is used.
   //    option: Option to TProof::RegisterDataSet
   // Return:
   //    0 when ok
   //   <0 otherwise

   if (listfiles){
      fMode->MakeDataSets(nfiles, np, wp, listfiles, option, fProof);
   }
   else{
      if ( fFilesGenerated ) {
         fMode->MakeDataSets(nfiles, np, wp, fFilesGenerated, option, fProof);
      }
      else{
         Error("MakeDataSet", "Empty data set; Either generate files first or"
               " supply one.");
         return -1;
      }
   }
   return 0;
}

//______________________________________________________________________________
void TProofBenchFileGenerator::Print(Option_t* option) const
{

   if (fProof) fProof->Print(option);
   if (fMode) fMode->Print(option);
   Printf("fNEvents=%lld", fNEvents);
   Printf("fStart=%d", fStart);
   Printf("fStop=%d", fStop);
   Printf("fStep=%d", fStep);
   Printf("fBaseDir=\"%s\"", fBaseDir.Data()); 
   Printf("fNTracksBench=%d", fNTracks);
   Printf("fRegenerate=%d", fRegenerate);
   if (fNodes) fNodes->Print(option);
   if (fFilesGenerated) fFilesGenerated->Print(option);
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
TProofBenchMode* TProofBenchFileGenerator::GetMode() const
{
   return fMode;
}

//______________________________________________________________________________
Long64_t TProofBenchFileGenerator::GetNEvents() const
{
   return fNEvents;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::GetStart() const
{
   return fStart;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::GetStop() const
{
   return fStop;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::GetStep() const
{
   return fStep;
}

//______________________________________________________________________________
TString TProofBenchFileGenerator::GetBaseDir() const
{
   return fBaseDir;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::GetNTracks() const
{
   return fNTracks;
}

//______________________________________________________________________________
Int_t TProofBenchFileGenerator::GetRegenerate() const
{
   return fRegenerate;
}
