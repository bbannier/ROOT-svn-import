// @(#)root/proofx:$Id:$
// Author: Sangsu Ryu 29/09/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofBench
#define ROOT_TProofBench

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBench                                                          //
//                                                                      //
// PROOF benchmark steering class.                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif
#ifndef ROOT_TString
#include "TString.h"
#endif
#ifndef ROOT_TProofBenchRun
#include "TProofBenchRun.h"
#endif

class TFile;
class TProofBenchRunCleanup;
class TProofBenchRunCPU;
class TProofBenchRunDataRead;
class TProofNodes;
class TProof;
class TProofBenchFileGenerator;

class TProofBench : public TObject
{

public:

   enum ERunType {kRunNotSpecified=0, kRunCPU=1, kRunDataRead=2,
                  kRunAll=kRunDataRead|kRunCPU};

   enum EModeType {kModeNotSpecified=0, kModeVaryingNFilesWorker=1,
                   kModeConstNFilesNode=2, kModeConstNFilesWorker=4,
                   kModeAll=kModeConstNFilesNode | kModeConstNFilesWorker
                          | kModeVaryingNFilesWorker};

private:

   TProof* fProof;               //proof

   EModeType fModeType;
   TProofBenchRun::ECleanupType fCleanupType;
   ERunType fRunType;

   TString fBaseDir;             //base directory for files
   Int_t fNTracks;               //number of tracks to generate in an event
   Int_t fRegenerate;            //when true, regenerate files

   TProofBenchRun::EHistType fHistType;  //histogram type
   Int_t fNHists;                //number of histograms

   TProofBenchRun::EReadType fReadType;

   Int_t fNFiles;

   Long64_t fNEventsCPU;         //number of events for CPU-bound test
   Long64_t fNEventsDataRead;    //number of events for IO-bound test
   Int_t fNTries;                //number of tries 
   Int_t fStart;                 //start number of workers to scan
   Int_t fStop;                  //stop number of workers to scan
   Int_t fStep;                  //test to be performed every fStep workers
   Int_t fDraw;                  //draw switch
   Int_t fDebug;                 //debug switch
   Int_t fNx;                    //When 1, the same number of workers on all
                                 //nodes in the cluster are activated
                                 //at the same time

   TFile* fFile;
   TDirectory* fDirProofBench;

   TProofNodes* fNodes;          //node information

   TList* fListMode;
   TProofBenchRunCleanup* fRunCleanup;
   TProofBenchRunCPU* fRunCPU;
   TProofBenchRunDataRead* fRunDataRead;

   TProofBenchFileGenerator* fFileGenerator;

public:

   TProofBench(TString filename="", Option_t* foption="",
               const char* diroutput="ProofBench", TProof* proof=0,
               EModeType modetype=kModeVaryingNFilesWorker,
               TProofBenchRun::ECleanupType cleanuptype=TProofBenchRun::kCleanupFileAdvise,
               ERunType runtype=kRunAll, TString basedir="");

   virtual ~TProofBench();

   //Int_t  GenerateFiles(Int_t nfiles=-1, Long64_t nevents=-1,
   //                     TString basedir="", Int_t regenerate=-1,
   //                     Int_t ntracks=-1);
   //Int_t MakeDataSets(Int_t nfiles=-1, Int_t start=-1, Int_t stop=-1,
   //                   Int_t step=-1, const char* option="V");
   Int_t MakeDataSets(Int_t nfiles=-1, Long64_t nevents=-1, TString basedir="",
                      Int_t regenerate=-1, Int_t ntracks=-1, Int_t start=-1,
                      Int_t stop=-1, Int_t step=-1, const char* option="V",
                      Int_t nx=-1);
   Int_t MakeDataSets(const char* option);

   void Run(const char* diroutput="", ERunType runtype=kRunNotSpecified,
            EModeType modetype=kModeNotSpecified, Int_t start=-1, Int_t stop=-1,
            Int_t step=-1, Int_t ntries=-1, Int_t nx=-1, Int_t debug=-1,
            Int_t draw=-1);

   TFile* OpenFile(const char* filename="", Option_t* option="",
                   const char* ftitle = "", Int_t compress = 1,
                   const char* diroutput="ProofBench");

   void GetModes(TList* modelist, EModeType modetype=kModeNotSpecified);

   void Print(Option_t* option="") const;

   //setters
   void SetModeType(EModeType modetype);
   void SetCleanupType(TProofBenchRun::ECleanupType cleanuptype);
   void SetRunType(ERunType runtype);
   void SetHistType(TProofBenchRun::EHistType histtype);
   void SetNHists(Int_t nhists);
   void SetReadType(TProofBenchRun::EReadType readtype);
   void SetNFiles(Int_t nfiles);
   void SetNEventsCPU(Long64_t nevents);
   void SetNEventsDataRead(Long64_t nevents);
   void SetNTries(Int_t ntries);
   void SetStart(Int_t start);
   void SetStop(Int_t stop);
   void SetStep(Int_t step);
   void SetDraw(Int_t draw);
   void SetDebug(Int_t debug);
   void SetNx(Int_t nx);
   void SetBaseDir(TString basedir);
   void SetNTracks(Int_t ntracks);
   void SetRegenerate(Int_t regenerate);

   //getters
   EModeType GetModeType() const;
   TProofBenchRun::ECleanupType GetCleanupType() const;
   ERunType GetRunType() const;
   TProofBenchRun::EHistType GetHistType() const;
   Int_t GetNHists() const;
   TProofBenchRun::EReadType GetReadType() const;
   Int_t GetNFiles() const;
   Long64_t GetNEventsCPU() const;
   Long64_t GetNEventsDataRead() const;
   Int_t GetNTries() const;
   Int_t GetStart() const;
   Int_t GetStop() const;
   Int_t GetStep() const;
   Int_t GetDraw() const;
   Int_t GetDebug() const;
   Int_t GetNx() const;
   TString GetBaseDir() const;
   Int_t GetNTracks() const;
   Int_t GetRegenerate() const;

   ClassDef(TProofBench, 0) //Proof benchmark steering class
};
#endif
