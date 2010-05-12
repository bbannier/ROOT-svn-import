// @(#)root/proofx:$Id:$
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
// TProofBenchRunCPU                                                //
//                                                                      //
// TProofBenchRunCPU is ...                                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TProofBenchRunCPU
#define ROOT_TProofBenchRunCPU

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
class TCanvas;
class TProof;
class TProfile;

class TProofBenchMode;

R__EXTERN TProof *gProof;

class TProofBenchRunCPU : public TProofBenchRun{

public:

   TProofBenchRunCPU(TProofBenchRun::EHistType histtype=TProofBenchRun::kHistNotSpecified,
                     Int_t nhists=16,
                     TString filename="",//output file where benchmark performance plot will be written to, 
                                         //user has to provide one
                     Option_t* foption="", //option to TFile() 
                     TProof* proof=gProof,
                     Int_t maxnworkers=-1,//maximum number of workers to be tested. 
                                          //If not set (default), 2 times the number of total workers in the cluster available
                     Long64_t nevents=10000,
                     Int_t ntries=2,
                     Int_t start=1,
                     Int_t stop=-1,
                     Int_t step=1,
                     Int_t draw=0,
                     Int_t debug=0); //default constructor

   virtual ~TProofBenchRunCPU();          //Destructor

   void Run(Long64_t nevents,
            Int_t ntries,
            Int_t start,
            Int_t stop,
            Int_t step,
            Int_t debug,
            Int_t draw);

   void BuildPerfProfiles(Int_t start,
                          Int_t stop,
                          Int_t step,
                          Int_t debug);

   void DrawPerfProfiles();

   void Print(Option_t* option="")const;   //Print status of an instance of this class 

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
                           Int_t compress = 1);    //open a file for outputs 
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
   TCanvas* GetCPerfProfiles() const;
   const char* GetName()const;

   TString GetNameStem()const;

protected:

   Int_t FillNodeInfo();

   Int_t SetParameters();
   Int_t DeleteParameters();

   const char* BuildPatternName(const char* objname, const char* delimiter="_");
   const char* BuildNewPatternName(const char* objname, Int_t nactive, Int_t tries, const char* delimiter="_");
   const char* BuildProfileName(const char* objname, const char* type, const char* delimiter="_");
   const char* BuildProfileTitle(const char* objname, const char* type, const char* delimiter=" ");

private:

   TProof* fProof;                 //pointer to proof

   TProofBenchRun::EHistType fHistType;
   Int_t fNHists;

   Long64_t fNEvents;            //number of events per file for CPU test and/or I/O test
   Int_t fNTries;                //number of files for I/O test
   Int_t fMaxNWorkers;           //number of maximum processes, 
                                 //this can be more than the number of total workers in the cluster
   Int_t fStart;
   Int_t fStop;
   Int_t fStep;

   Int_t fDraw;
   Int_t fDebug;

   TFile* fFile;                 //output file to write performance histograms and trees on
   TDirectory* fDirProofBench;   //directory for proof outputs
   Bool_t fWritable;

   TList* fNodes;                // List of worker nodes info, fNodes is the owner of its members

   TList* fPerfStats;            //List of PROOF_PerfStats
   TProfile* fProfEvent;
   TCanvas* fCPerfProfiles;      //canvas for performance profile histograms

   TString fName;

   ClassDef(TProofBenchRunCPU,0)         //PROOF benchmark run for CPU test
};

#endif
