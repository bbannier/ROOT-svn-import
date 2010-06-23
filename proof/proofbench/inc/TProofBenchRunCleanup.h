// @(#)root/proofx:$Id:$
// Author: Sangsu Ryu 22/06/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofBenchRunCleanup
#define ROOT_TProofBenchRunCleanup

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBenchRunCleanup                                                //
//                                                                      //
// Represents a memory cleaning-up run for PROOF benchmark.             //
// During I/O benchmark, files are repeatedly read.                     //
// Every new run should read file from disk, not from memory.           //
// 2 ways of clean up method are provided. One is brute force way       //
// in which dedicated files large enough to clean up memory             //
// on the machine are read in before every run. The other way is clean  //
// up files cache by calling posix_fadvise. It works only on Linux      //
// for now.                                                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TString
#include "TString.h"
#endif

#ifndef ROOT_TProofBenchRun
#include "TProofBenchRun.h"
#endif

class TProof;
class TFile;
class TCanvas;
class TProfile;

R__EXTERN TProof *gProof;

class TProofBenchRunCleanup : public TProofBenchRun{

private:

   TProof* fProof;               //proof

   TProofBenchRun::ECleanupType fCleanupType; //cleanup type
   Long64_t fNEvents;            //number of events per file for CPU test and/or I/O test
   Int_t fMaxNWorkers;           //number of maximum processes, 
                                 //this can be more than the number of total workers in the cluster
   TString fDataSetCleanup;      //data set to be cleaned up at nodes using fadvice
   Int_t fDraw;                  //draw switch
   Int_t fDebug;                 //debug switch

   TFile* fFile;                 //output file to write performance histograms and trees on
   TDirectory* fDirProofBench;   //directory for proof outputs
   Bool_t fWritable;             //file writable

   TString fName;                //name of this run

protected:

   Int_t SetParameters();
   Int_t DeleteParameters();

   const char* BuildPatternName(const char* objname, const char* delimiter="_");

public:

   TProofBenchRunCleanup(TProofBenchRun::ECleanupType cleanuptype=TProofBenchRun::kCleanupNotSpecified,
                         TString filename="",
                         Option_t* foption="",
                         TProof* proof=gProof,
                         Int_t maxnworkers=-1,
                         Long64_t nevents=-1,
                         Int_t draw=0,
                         Int_t debug=0);

   virtual ~TProofBenchRunCleanup();

   void Run(Long64_t nevents,
            Int_t,
            Int_t,
            Int_t,
            Int_t,
            Int_t debug,
            Int_t draw);

   void BuildPerfProfiles(Int_t,
                          Int_t,
                          Int_t,
                          Int_t);

   void DrawPerfProfiles();

   void Print(Option_t* option="")const;

   void SetCleanupType(TProofBenchRun::ECleanupType cleanuptype);
   void SetNEvents(Long64_t nevents);
   void SetMaxNWorkers(Int_t maxnworkers);
   void SetMaxNWorkers(TString sworkers);
   void SetDataSetCleanup(const TString& dataset);
   void SetDraw(Int_t draw);
   void SetDebug(Int_t debug);
   TFile* OpenFile(const char* filename="",
                           Option_t* option="",
                           const char* ftitle = "",
                           Int_t compress = 1);
   void SetDirProofBench(TDirectory* dir);
   void SetName(TString name);

   TProofBenchRun::ECleanupType GetCleanupType()const;
   Long64_t GetNEvents()const;
   Int_t GetMaxNWorkers()const;
   TString GetDataSetCleanup()const;
   Int_t GetDraw()const;
   Int_t GetDebug()const;
   TFile* GetFile()const;
   TDirectory* GetDirProofBench() const;
   const char* GetName()const;

   TString GetNameStem()const;

   ClassDef(TProofBenchRunCleanup,0)   //Represents a memory cleaning-up run for PROOF benchmark
};

#endif
