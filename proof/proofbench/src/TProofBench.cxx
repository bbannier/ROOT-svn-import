// @(#)root/proofx:$Id$
// Author: Sangsu Ryu 29/9/2010

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
// Proof benchmark steering class.                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofBench.h"
#include "TProofBenchFileGenerator.h"
#include "TProofBenchModeConstNFilesNode.h"
#include "TProofBenchModeConstNFilesWorker.h"
#include "TProofBenchModeVaryingNFilesWorker.h"
#include "TProofBenchModeCleanup.h"
#include "TProofBenchRunCPU.h"
#include "TProofBenchRunDataRead.h"
#include "TProofBenchRunCleanup.h"
#include "TProofNodes.h"
#include "TString.h"
#include "TProof.h"
#include "TFile.h"
#include "TROOT.h"

ClassImp(TProofBench)

//______________________________________________________________________________
TProofBench::TProofBench(TString filename, Option_t* foption, 
                         const char* diroutput, TProof* proof,
                         EModeType modetype,
                         TProofBenchRun::ECleanupType cleanuptype,
                         ERunType runtype, TString basedir):
fProof(proof), fModeType(modetype), fCleanupType(cleanuptype),
fRunType(runtype), fBaseDir(basedir), fNTracks(100),
fRegenerate(kFALSE), fHistType(TProofBenchRun::kHist1D), fNHists(16),
fReadType(TProofBenchRun::kReadFull), fNFiles(-1), fNEventsCPU(1000000),
fNEventsDataRead(100000), fNTries(2), fStart(1), fStop(-1), fStep(1),
fDraw(0), fDebug(0), fNx(0), fFile(0), fDirProofBench(0), fNodes(0),
fListMode(0), fRunCleanup(0), fRunCPU(0), fRunDataRead(0)
{
   //Default constructor.

   if(!OpenFile(filename.Data(), foption, diroutput)){
      gROOT->cd();
      if (gDirectory->GetDirectory(diroutput) || gDirectory->mkdir(diroutput)){
         gDirectory->cd(diroutput);
         fDirProofBench=gDirectory;
      }
      else{
         fDirProofBench=gROOT;
         Warning("TProofBench", "cannot make directory:%s%s;"
                 " Output is going to %s",
              gROOT->GetPath(), diroutput, gROOT->GetPath());
      }
   }

   if (!fProof && gProof){
      fProof=gProof;
      Info("ProofBench", "fProof is set to gProof");
   }

   if (!fProof){
      return;
   }

   //fill up node information
   fNodes=new TProofNodes(fProof);

   if (fBaseDir.IsNull() && fProof){
      TUrl url(fProof->GetDataPoolUrl());
      fBaseDir=url.GetFile();
   }

   fStop=fNodes->GetNWorkersCluster();

   //create modes
   fListMode=new TList;
   fListMode->SetOwner(kTRUE);
   fListMode->Add(new TProofBenchModeConstNFilesNode(fNFiles, fProof,
                                                     fNodes));
   fListMode->Add(new TProofBenchModeConstNFilesWorker(fNFiles, fProof,
                                                       fNodes));
   fListMode->Add(new TProofBenchModeVaryingNFilesWorker(fNFiles, fProof,
                                                         fNodes));

   fFileGenerator=new TProofBenchFileGenerator(0, fNEventsDataRead, fStart,
                           fStop, fStep, fBaseDir, fNTracks, fRegenerate, fNx,
                           fProof, fNodes);

   //create runs
   fRunCleanup=new TProofBenchRunCleanup(fCleanupType, fDirProofBench, fProof,
                                         fDebug);

   fRunCPU=new TProofBenchRunCPU(fHistType, fNHists, fDirProofBench, fProof,
                           fNodes, fNEventsCPU, fNTries, fStart, fStop, fStep,
                           fNx, fDraw, fDebug);

   fRunDataRead=new TProofBenchRunDataRead(0, fRunCleanup, fReadType,
                           fDirProofBench, fProof, fNodes, fNEventsDataRead,
                           fNTries, fStart, fStop, fStep, fNx, fDebug);
}

//______________________________________________________________________________
TProofBench::~TProofBench()
{
   //Destructor.

   if (fNodes){
      delete fNodes;
   }
   if (fListMode){
      delete fListMode;
   }
   if (fFileGenerator){
      delete fFileGenerator;
   }
   if (fRunCleanup){
      delete fRunCleanup;
   }
   if (fRunCPU){
     delete fRunCPU;
   } 
   if (fRunDataRead){
      delete fRunDataRead;
   }
   if (fFile){
      fFile->Close();
      delete fFile;
   }
}

//______________________________________________________________________________
Int_t TProofBench::MakeDataSets(Int_t nfiles, Long64_t nevents,
                        TString basedir, Int_t regenerate, Int_t ntracks,
                        Int_t start, Int_t stop, Int_t step, const char* option,
                        Int_t nx)
{

   // Generates files on worker nodes for I/O test or for cleanup run
   // Input parameters do not change corresponding data members
   // Input parameters
   //    nf: This parameter is passed to the mode in use.
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
   //    nf: This parameter is passed to the mode in use.
   //        When -1, data member fNFiles of mode in use is used.
   //    start: Start number of workers, when -1 (default) data member fStart
   //           is used
   //    stop: Ending number of workers, when -1 (default) data member fStop
   //          is used
   //    step: Incremental number of workers, when -1 (default) data member
   //          fStep is used
   //    listfiles: List of files (TFileInfo*) from which file collection will
   //               be built in this function. 
   //               When listfiles=0, the list filled up by
   //               TProofBenchFileGenerator::GenerateFiles is used.
   //    option: Option to TProof::RegisterDataSet
   //    nx: When it is 1, the same number of workers on all nodes in the
   //        cluster are activated at the same time. Scan starts with 'start'
   //        active workers on all nodes, 'step' more workers are activated
   //        until scan is done with 'stop' active worker on all nodes.
   // Return: 
   //    0 when ok
   //   <0 otherwise

   if (!fProof){
      Error("MakeDataSets", "Proof not set, doing nothing");
      return -1;
   }
   
   // Check input parameters
   nfiles=(nfiles==-1)?fNFiles:nfiles;

   nevents=(nevents==-1)?fNEventsDataRead:nevents;
    
   if (basedir.IsNull()){
      basedir=fBaseDir;
   }
   
   regenerate=(regenerate==-1)?fRegenerate:regenerate;

   ntracks=(ntracks==-1)?fNTracks:ntracks;
 
   nfiles=(nfiles==-1)?fNFiles:nfiles;
   start=(start==-1)?fStart:start;
   stop=(stop==-1)?fStop:stop;
   step=(step==-1)?fStep:step;
   nx=(nx==-1)?fNx:nx;

   if (start>stop){
       Error("MakeDataSets", "Wrong inputs; start(%d)>stop(%d)", start, stop);
       return -1;
   }

   Int_t ret=0;

   //Generate cleanup-files only for reading-in method
   if (fCleanupType & TProofBenchRun::kCleanupReadInFiles){
      TProofBenchMode* mode=new TProofBenchModeCleanup(fProof, fNodes);
      fFileGenerator->SetMode(mode);
      ret+=fFileGenerator->GenerateFiles(nfiles, nevents, basedir, regenerate,
                                    ntracks);
      ret+=fFileGenerator->MakeDataSets(nfiles, start, stop, step, 0, option,
                                        0);
      delete mode;
   }

   //Generate data files and make data set
   TList* modes=new TList();
   modes->SetOwner(kFALSE);
   GetModes(modes, fModeType);
   TIter nxtmode(modes);
   TProofBenchMode* mode=0;
   while (mode=dynamic_cast<TProofBenchMode*>(nxtmode())){
      fFileGenerator->SetMode(mode);
      ret+=fFileGenerator->GenerateFiles(nfiles, nevents, basedir, regenerate,
                                    ntracks);
      ret+=fFileGenerator->MakeDataSets(nfiles, start, stop, step, 0, option,
                                        nx);
   }
   delete modes;

   return ret;
}

/*
//______________________________________________________________________________
Int_t TProofBench::MakeDataSets(Int_t nfiles, Int_t start, Int_t stop,
                                Int_t step, const char* option)
{
   // Make data sets out of list of files 'listfiles' and register them
   // for benchmark test
   //
   // Input parameters
   //    nf: This parameter is passed to the mode in use.
   //        When -1, data member fNFiles of mode in use is used.
   //    start: Start number of workers, when -1 (default) data member fStart
   //           is used
   //    stop: Ending number of workers, when -1 (default) data member fStop
   //          is used
   //    step: Incremental number of workers, when -1 (default) data member
   //          fStep is used
   //    listfiles: List of files (TFileInfo*) from which file collection will
   //               be built in this function. 
   //               When listfiles=0, the list filled up by
   //               TProofBenchFileGenerator::GenerateFiles is used.
   //    option: Option to TProof::RegisterDataSet
   // Return: 0 when ok
   //        <0 otherwise

   nfiles=(nfiles==-1)?fNFiles:nfiles;
   start=(start==-1)?fStart:start;
   stop=(stop==-1)?fStop:stop;
   step=(step==-1)?fStep:step;

   if (start>stop){
       Error("MakeDataSets", "Wrong inputs; start(%d)>stop(%d)", start, stop);
       return -1;
   }

   //make sure files are there
   GenerateFiles(nfiles, fNEventsDataRead, "", fRegenerate, fNTracks);

   Int_t ret=0;

   //make data set for clean-up
   if (fCleanupType & TProofBenchRun::kCleanupReadInFiles){
      TProofBenchMode* mode=new TProofBenchModeCleanup(fProof, fNodes);
      fFileGenerator->SetMode(mode);
      ret+=fFileGenerator->MakeDataSets(nfiles, start, stop, step, 0, option);
      delete mode;
   }

   //make data set for data read
   TList* modes=new TList();
   modes->SetOwner(kFALSE);
   GetModes(modes, fModeType);
   TIter nxtmode(modes);
   TProofBenchMode* mode=0;
   while (mode=dynamic_cast<TProofBenchMode*>(nxtmode())){
      fFileGenerator->SetMode(mode);
      ret+=fFileGenerator->MakeDataSets(nfiles, start, stop, step, 0, option);
   }
   delete modes;

   return ret;
}
*/

//______________________________________________________________________________
Int_t TProofBench::MakeDataSets(const char* option)
{
   // Make data sets and register them for benchmark test
   //
   // Input parameters
   //    option: Option to TProof::RegisterDataSet
   // Return: 0 when ok, 
   //        <0 otherwise

   MakeDataSets(fNFiles, fNEventsDataRead, fBaseDir, fRegenerate, fNTracks,
                fStart, fStop, fStep, option);
   return 0;
}

//______________________________________________________________________________
void TProofBench::Run(const char* diroutput, ERunType runtype,
                      EModeType modetype, Int_t start, Int_t stop, Int_t step,
                      Int_t ntries, Int_t nx, Int_t debug, Int_t draw)
{
   //Run benchmark tests.
   //Input parameters:
   //   diroutput: Name of directory to put outputs.
   //   runtype: Run type, CPU-bound test or IO-bound test. When not given data
   //           member fRunType is used.
   //   modetype: Mode type. When not given data member fModeType is used.
   //   start: Test start at 'start' active workers. When not given, data
   //          member fStart is used.
   //   stop: Test top at 'stop' active workers. When not given, data member
   //         fStop is used.
   //   step: Test is done every 'step' active workers. When not given, data
   //         member fStep is used.
   //   ntries: Number of tries for each active-workers point. When not given,
   //           data member fNTries is used.
   //   debug: When 'debug' is set, various histograms are saved for debuging
   //          purpose. When not given, data member fDebug is used.
   //   draw: When 'draw' is set, various histograms are displayed while test
   //         is processed.
   //Return: Nothing.

   if (!fProof){
      Printf("Proof not set; returning");
      return;
   }
   runtype=(runtype==kRunNotSpecified)?fRunType:runtype;
   modetype=(modetype==kModeNotSpecified)?fModeType:modetype;
   start=(start==-1)?fStart:start;
   stop=(stop==-1)?fStop:stop;
   step=(step==-1)?fStep:step;
   ntries=(ntries==-1)?fNTries:ntries;
   nx=(nx==-1)?fNx:nx;
   debug=(debug==-1)?fDebug:debug;
   draw=(draw==-1)?fDraw:draw;

   if (!TString(diroutput).IsNull()){//user has specified a directory name
      if (fFile){ //file
         TString path=TString(fFile->GetPath())+TString(diroutput);
         if (!fFile->GetDirectory(path.Data())){
            fFile->mkdir(diroutput);
         }
         fFile->cd(diroutput);
         if (gDirectory->GetList()->GetSize()>0){
            Error("Run", "The directory '%s' has outputs. Specifiy new"
                  " directory. Doing nothing", gDirectory->GetPath());
            return;
         }
      }
      else{ //memory
         TString path=TString(gROOT->GetPath())+TString(diroutput);
         if (!gROOT->GetDirectory(path.Data())){
            gROOT->mkdir(diroutput);
         }
         gROOT->cd(diroutput);
         if (gDirectory->GetList()->GetSize()>0){
            Error("Run", "The directory '%s' has outputs. Specifiy new"
                  " directory. Doing nothing", gDirectory->GetPath());
            return;
         }
      }
   }
   else{
      fDirProofBench->cd();
      //if something is in there, do nothing and return.
      if (fDirProofBench->GetList()->GetSize()>0){
         Error("Run", "The directory '%s' has outputs. Specifiy new directory."
               " Doing nothing", fDirProofBench->GetPath());
         return;
      }
   }

   if (runtype & kRunCPU){
      fRunCPU->SetDirProofBench(gDirectory);
      fRunCPU->Run(fNEventsCPU, start, stop, step, ntries, nx, debug, draw);
   }
   if (runtype & kRunDataRead){
      //set output directory for cleanup run
      fRunCleanup->SetDirProofBench(gDirectory);

      //set up clean-up type
      fRunCleanup->SetCleanupType(fCleanupType);

      TList* modes=new TList();
      modes->SetOwner(kFALSE);
      GetModes(modes, modetype);
      TIter nxtmode(modes);
      TProofBenchMode* mode=0;
      while (mode=(TProofBenchMode*)nxtmode()){
         if (fNFiles>0){
            mode->SetNFiles(fNFiles);
         }

         fRunDataRead->SetMode(mode);
         fRunDataRead->SetDirProofBench(gDirectory);
         fRunDataRead->Run(fNEventsDataRead, start, stop, step, ntries, nx,
                           debug, draw);
      }
      delete modes;
   }
}

//______________________________________________________________________________
void TProofBench::Print(Option_t* option)const
{
   //Print status of this object.

   if (fProof){
      fProof->Print(option);
   }
   else{
      Printf("No Proof session open");
   }

   Printf("fModeType=%d", fModeType);
   Printf("fCleanupType=%d", fCleanupType);
   Printf("fRunType=%d", fRunType);
   Printf("fBaseDir=%s", fBaseDir.Data());
   Printf("fNTracks=%d", fNTracks);
   Printf("fRegenerate=%d", fRegenerate);
   Printf("fHistType=%d", fHistType);
   Printf("fNHists=%d", fNHists);
   Printf("fReadType=%d", fReadType);
   Printf("fNFiles=%d", fNFiles);
   Printf("fNEventsCPU=%lld", fNEventsCPU);
   Printf("fNEventsDataRead=%lld", fNEventsDataRead);
   Printf("fNTries=%d", fNTries);
   Printf("fStart=%d", fStart);
   Printf("fStop=%d", fStop);
   Printf("fStep=%d", fStep);
   Printf("fDraw=%d", fDraw);
   Printf("fDebug=%d", fDebug);
   Printf("fNx=%d", fNx);
 
   //if (fFile) fFile->Print(option);
   if (fDirProofBench){
      Printf("Output directory=%s", fDirProofBench->GetPath());
      fDirProofBench->Print(option);
   }
   if (fNodes) fNodes->Print(option);
}

//______________________________________________________________________________
TFile* TProofBench::OpenFile(const char* filename, Option_t* option,
                             const char* ftitle, Int_t compress,
                             const char* diroutput)
{
   // Opens a file which output profiles and/or intermediate files
   // (trees, histograms when debug is set)
   // are to be written to. Makes a directory named "ProofBench" and changes
   // to the directory.
   // Input parameters:
   //    filename: Name of the file to open
   //    option: Option for TFile::Open(...) function
   //    ftitle: Title parameter for TFile::Open(...) function
   //    compress: Compression parameter for TFile::Open(...) function
   //    diroutput: output directory. If it does not exist it is created.
   // Returns:
   //    Open file if a file is already open
   //    New file just opened
   //    0 when open fails;

   TString sfilename(filename);
   sfilename.Remove(TString::kBoth, ' '); //remove leading and trailing white
                                          // space(s)
   sfilename.Remove(TString::kBoth, '\t'); //remove leading and trailing tab 
                                           //character(s)

   if (sfilename.IsNull()){
      return fFile;
   }

   TString soption(option);
   soption.ToLower();

   if (fFile){
      Error("OpenFile", "File alaredy open; %s;"
            "Close it before open another file", fFile->GetName());
      return fFile;
   }

   TDirectory* dirsav=gDirectory;
   fFile=new TFile(sfilename, option, ftitle, compress);

   if (fFile->IsZombie()){//open failed
      Error("FileOpen", "Cannot open file: %s", sfilename.Data());
      fFile->Close();
      fFile=0;
      dirsav->cd();
      return 0;
   }
   else{//open succeeded
      if (fFile->GetDirectory(diroutput) || fFile->mkdir(diroutput)){
         fFile->cd(diroutput);
         fDirProofBench=gDirectory;
         return fFile;
      }
      else{//cannot make output directory
         Error("FileOpen", "Cannot create directory: %s", diroutput);
         fFile->Close();
         fFile=0;
         dirsav->cd();
         return 0; 
      }
   }
}

//______________________________________________________________________________
void TProofBench::GetModes(TList* modelist, EModeType modetype)
{
   //Build a list of modes that match modetype. 
   //modelist should be provided
   //Input parameters:
   //   modelist: list to which modes to be added
   //   modetype: mode type
   //Return:
   //   Nothing

   if (!modelist){
      Warning("GetModeList", "modelist should be provided");
      return;
   }
   modetype=(modetype==kModeNotSpecified)?fModeType:modetype;
   TIter nxtmode(fListMode);
   TObject* mode=0;
   while (mode=nxtmode()){
      if (modetype & kModeConstNFilesNode
         && dynamic_cast<TProofBenchModeConstNFilesNode*>(mode) ){
         modelist->Add(mode);
         continue;
      }
      if (modetype & kModeConstNFilesWorker
         && dynamic_cast<TProofBenchModeConstNFilesWorker*>(mode)){
         modelist->Add(mode);
         continue;
      }
      if (modetype & kModeVaryingNFilesWorker
         && dynamic_cast<TProofBenchModeVaryingNFilesWorker*>(mode)){
         modelist->Add(mode);
         continue;
      }
   }
}

//______________________________________________________________________________
void TProofBench::SetModeType(EModeType modetype)
{
   //Set mode to 'modetype'.
   fModeType=modetype;
}

//______________________________________________________________________________
void TProofBench::SetCleanupType(TProofBenchRun::ECleanupType cleanuptype)
{
   //Set clean-up type to 'cleanup'.
   fCleanupType=cleanuptype;
}

//______________________________________________________________________________
void TProofBench::SetRunType(ERunType runtype)
{
   //Set run type to 'runtype'.
   fRunType=runtype;
}

//______________________________________________________________________________
void TProofBench::SetBaseDir(TString basedir)
{
   fBaseDir=basedir;
}

//______________________________________________________________________________
void TProofBench::SetNTracks(Int_t ntracks)
{  
   fNTracks=ntracks;
}

//______________________________________________________________________________
void TProofBench::SetRegenerate(Int_t regenerate)
{  
   fRegenerate=regenerate;
}

//______________________________________________________________________________
void TProofBench::SetHistType(TProofBenchRun::EHistType histtype)
{
   //Set hist type to 'histtype'.
   fHistType=histtype;
}

//______________________________________________________________________________
void TProofBench::SetNHists(Int_t nhists)
{
   //Set number of histograms for CPU-bound test to 'nhists'.
   fNHists=nhists;
}

//______________________________________________________________________________
void TProofBench::SetReadType(TProofBenchRun::EReadType readtype)
{
   //Set read type for IO-bound test to 'readtype'.
   fReadType=readtype;
}

//______________________________________________________________________________
void TProofBench::SetNFiles(Int_t nfiles)
{
   //Set number of files per node/worker to 'nfiles'. Exact meaningn of this
   //parameter depends on the mode(s) in use.
   fNFiles=nfiles;
}

//______________________________________________________________________________
void TProofBench::SetNEventsCPU(Long64_t nevents)
{
   //Set number of events for CPU-bound test to 'nevents'.
   fNEventsCPU=nevents;
}

//______________________________________________________________________________
void TProofBench::SetNEventsDataRead(Long64_t nevents)
{
   //Set number of events for IO-bound test to 'nevents'.
   fNEventsDataRead=nevents;
}

//______________________________________________________________________________
void TProofBench::SetNTries(Int_t ntries)
{
   //Set number of tries for each active-workers point to 'ntries'.
   fNTries=ntries;
}

//______________________________________________________________________________
void TProofBench::SetStart(Int_t start)
{
   //Set starting number of active workers to 'start'.
   fStart=start;
}

//______________________________________________________________________________
void TProofBench::SetStop(Int_t stop)
{
   //Set stop number of active workers to 'stop'.
   fStop=stop;
}

//______________________________________________________________________________
void TProofBench::SetStep(Int_t step)
{
   //Set step number of active workers to 'step'.
   fStep=step;
}

//______________________________________________________________________________
void TProofBench::SetDraw(Int_t draw)
{
   //Set draw switch to 'draw'.
   fDraw=draw;
}

//______________________________________________________________________________
void TProofBench::SetDebug(Int_t debug)
{
   //Set debug switch to 'debug'.
   fDebug=debug;
}

//______________________________________________________________________________
void TProofBench::SetNx(Int_t nx)
{
   //Set nx flag to 'nx'.
   fNx=nx;
}

//______________________________________________________________________________
TProofBench::EModeType TProofBench::GetModeType() const
{
   //Get mode type.
   return fModeType;
}

//______________________________________________________________________________
TProofBenchRun::ECleanupType TProofBench::GetCleanupType() const
{
   //Get clean-up type for IO-bound test.
   return fCleanupType;
}

//______________________________________________________________________________
TProofBench::ERunType TProofBench::GetRunType() const
{
   //Get run type.
   return fRunType;
}

//______________________________________________________________________________
TString TProofBench::GetBaseDir() const
{
   return fBaseDir;
}

//______________________________________________________________________________
Int_t TProofBench::GetNTracks() const
{
   return fNTracks;
}

//______________________________________________________________________________
Int_t TProofBench::GetRegenerate() const
{
   return fRegenerate;
}

//______________________________________________________________________________
TProofBenchRun::EHistType TProofBench::GetHistType() const
{
   //Get hist type for CPU-bound test.
   return fHistType;
}

//______________________________________________________________________________
Int_t TProofBench::GetNHists() const
{
   //Get number of histograms to generate for CPU-bound test.
   return fNHists;
}

//______________________________________________________________________________
TProofBenchRun::EReadType TProofBench::GetReadType() const
{
   //Get read type for IO-bound test.
   return fReadType;
}

//______________________________________________________________________________
Int_t TProofBench::GetNFiles() const
{
   //Get number of files per node/worker for IO-bound test.
   return fNFiles;
}

//______________________________________________________________________________
Long64_t TProofBench::GetNEventsCPU() const
{
   //Get number of events for CPU-bound test.
   return fNEventsCPU;
}

//______________________________________________________________________________
Long64_t TProofBench::GetNEventsDataRead() const
{
   //Get number of events for IO-bound test.
   return fNEventsDataRead;
}

//______________________________________________________________________________
Int_t TProofBench::GetNTries() const
{
   //Get number of tries for each active-workers point.
   return fNTries;
}

//______________________________________________________________________________
Int_t TProofBench::GetStart() const
{
   //Get start number of active workers.
   return fStart;
}

//______________________________________________________________________________
Int_t TProofBench::GetStop() const
{
   //Get stop number of active workers.
   return fStop;
}

//______________________________________________________________________________
Int_t TProofBench::GetStep() const
{
   //Get step number of active workers.
   return fStep;
}

//______________________________________________________________________________
Int_t TProofBench::GetDraw() const
{
   //Get draw switch.
   return fDraw;
}

//______________________________________________________________________________
Int_t TProofBench::GetDebug() const
   //Get debug switch.
{
   return fDebug;
}

//______________________________________________________________________________
Int_t TProofBench::GetNx() const
   //Get nx flag.
{
   return fNx;
}
