// @(#)root/proofx:$Id:$
// Author: Sangsu Ryu 22/06/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofBenchRunCPU
#define ROOT_TProofBenchRunCPU

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBenchRunCPU                                                    //
//                                                                      //
// CPU-intensive PROOF benchmark test generates events and fill 1, 2,   //
//or 3-D histograms. No I/O activity is involved.                       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TString
#include "TString.h"
#endif

#ifndef ROOT_TProofBenchRun
#include "TProofBenchRun.h"
#endif

class TFile;
class TCanvas;
class TProof;
class TProfile;
class TTree;

class TProofBenchMode;

R__EXTERN TProof *gProof;

class TProofBenchRunCPU : public TProofBenchRun{

private:

   TProof* fProof;               //proof

   TProofBenchRun::EHistType fHistType;  //histogram type
   Int_t fNHists;                //number of histograms

   Long64_t fNEvents;            //number of events to generate
   Int_t fNTries;                //number of tries 
   Int_t fMaxNWorkers;           //number of maximum workers

   Int_t fStart;                 //start number of workers to scan
   Int_t fStop;                  //stop number of workers to scan
   Int_t fStep;                  //test to be performed every fStep workers

   Int_t fDraw;                  //draw switch
   Int_t fDebug;                 //debug switch

   TFile* fFile;                 //output file to write performance histograms and trees on
   TDirectory* fDirProofBench;   //directory for proof outputs
   Bool_t fWritable;             //file writable

   TList* fNodes;                //list of node information

   TList* fPerfStats;            //list of PROOF_PerfStats
   TList* fListPerfProfiles;     //list of performance profiles
   TCanvas* fCPerfProfiles;      //canvas for performance profile histograms

   TString fName;                //name of CPU run

protected:

   void FillPerfStatProfiles(TTree* t, TProfile* profile, Int_t nactive);

   Int_t FillNodeInfo();

   Int_t SetParameters();
   Int_t DeleteParameters();

   TString BuildPatternName(const TString& objname, const TString& delimiter="_");
   TString BuildNewPatternName(const TString& objname, Int_t nactive, Int_t tries, const TString& delimiter="_");
   TString BuildProfileName(const TString& objname, const TString& type, const TString& delimiter="_");
   TString BuildProfileTitle(const TString& objname, const TString& type, const TString& delimiter=" ");

public:

   TProofBenchRunCPU(TProofBenchRun::EHistType histtype=TProofBenchRun::kHistNotSpecified,
                     Int_t nhists=16,
                     TString filename="",
                     Option_t* foption="",
                     TProof* proof=gProof,
                     Int_t maxnworkers=-1,
                     Long64_t nevents=1000000,
                     Int_t ntries=2,
                     Int_t start=1,
                     Int_t stop=-1,
                     Int_t step=1,
                     Int_t draw=0,
                     Int_t debug=0);

   virtual ~TProofBenchRunCPU();

   void Run(Long64_t nevents,
            Int_t ntries,
            Int_t start,
            Int_t stop,
            Int_t step,
            Int_t debug,
            Int_t draw);

   void DrawPerfProfiles();

   void Print(Option_t* option="")const;

   void SetHistType(TProofBenchRun::EHistType histtype);
   void SetNHists(Int_t nhists);
   void SetNEvents(Long64_t nevents);
   void SetNTries(Int_t ntries);
   void SetMaxNWorkers(Int_t maxnworkers);
   void SetMaxNWorkers(TString sworkers);
   void SetStart(Int_t start);
   void SetStop(Int_t stop);
   void SetStep(Int_t step);
   void SetDraw(Int_t draw);
   void SetDebug(Int_t debug);
   TFile* OpenFile(const char* filename="",
                           Option_t* option="",
                           const char* ftitle = "",
                           Int_t compress = 1);
   void SetDirProofBench(TDirectory* dir);

   TProofBenchRun::EHistType GetHistType()const;
   Int_t GetNHists()const;
   Long64_t GetNEvents()const;
   Int_t GetNTries()const;
   Int_t GetMaxNWorkers()const;
   Int_t GetStart()const;
   Int_t GetStop()const;
   Int_t GetStep()const;
   Int_t GetDraw()const;
   Int_t GetDebug()const;
   TFile* GetFile()const;
   TDirectory* GetDirProofBench() const;
   TList* GetListOfPerfStats()const;
   TList* GetListPerfProfiles()const;
   TCanvas* GetCPerfProfiles() const;
   const char* GetName()const;

   TString GetNameStem()const;

   ClassDef(TProofBenchRunCPU,0)     //CPU-intensive PROOF benchmark
};

#endif
