// @(#)root/proofx:$Id$
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
class TCanvas;
class TProfile;

class TProofBenchRunCleanup : public TProofBenchRun
{

private:

   EPBCleanupType fCleanupType; //cleanup type
   TString fDataSetCleanup;      //data set to be cleaned up at nodes using
                                 // fadvice
   Int_t fDebug;                 //debug switch
   TDirectory* fDirProofBench;   //directory for proof outputs
   TString fName;                //name of this run

protected:

   Int_t SetParameters();
   Int_t DeleteParameters();

   const char* BuildPatternName(const char* objname, const char* delimiter="_");

public:

   TProofBenchRunCleanup(EPBCleanupType cleanuptype = kPBCleanupNotSpecified,
                         TDirectory* dirproofbench=0, TProof* proof=0,
                         Int_t debug=0);

   virtual ~TProofBenchRunCleanup();

   void Run(Long64_t, Int_t, Int_t, Int_t, Int_t, Int_t debug, Int_t);
   void Run(const char *, Int_t, Int_t, Int_t, Int_t, Int_t, Int_t) { }

   void DrawPerfProfiles();

   void Print(Option_t* option="") const;

   void SetCleanupType(EPBCleanupType cleanuptype);
   void SetDataSetCleanup(const TString& dataset);
   void SetDebug(Int_t debug);
   void SetDirProofBench(TDirectory* dir);
   void SetName(TString name);

   EPBCleanupType GetCleanupType() const;
   TString GetDataSetCleanup() const;
   Int_t GetDebug() const;
   TDirectory* GetDirProofBench() const;
   const char* GetName() const;

   TString GetNameStem() const;

   ClassDef(TProofBenchRunCleanup,0)   //Represents a memory cleaning-up run
                                       // for PROOF benchmark
};

#endif
