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
// TProofBenchFileGenerator                                                          //
//                                                                      //
// TProofBenchFileGenerator is a steering class for PROOF benchmark suite.           //
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

#ifndef ROOT_TProofBenchFileGenerator
#define ROOT_TProofBenchFileGenerator

#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

class TProof;
class TProofBenchMode;
class TDSet;

R__EXTERN TProof *gProof;

class TProofBenchFileGenerator: public TObject{

public:

   TProofBenchFileGenerator(TProofBenchMode* mode=0,
                            Long64_t nevents=10000,
                            Int_t maxnworkers=-1,
                            Int_t start=1,
                            Int_t stop=-1,
                            Int_t step=1,
                            TString basedir="",
                            Int_t ntracks=100,
                            Bool_t regenerate=kFALSE,
                            TProof* proof=gProof);

   virtual ~TProofBenchFileGenerator();          //destructor

   virtual Int_t  GenerateFiles(Int_t nf=-1,
                                Long64_t nevents=-1,
                                TString basedir="",
                                Int_t regenerate=-1,
                                Int_t ntracks=-1);

   virtual Int_t MakeDataSets(Int_t nf=-1,   //when ==-1, use current set value
                              Int_t start=-1, //when ==-1, use current set value
                              Int_t stop=-1,  //when ==-1, use current set value
                              Int_t step=-1,  //when ==-1, use current set value
                              const TDSet* tdset=0,
                              const char* option="V");

   virtual Int_t MakeDataSets(const char* option);

   virtual Int_t MakeDataSets(Int_t nf,
                              Int_t np,
                              const Int_t *wp,
                              const TDSet* tdset=0,
                              const char *option="V");

   virtual void Print(Option_t* option="")const;

   void SetMode(TProofBenchMode* mode);
   void SetNEvents(Long64_t nevents);
   void SetMaxNWorkers(Int_t maxnworkers);
   void SetMaxNWorkers(TString sworkers);
   void SetStart(Int_t start);
   void SetStop(Int_t stop);
   void SetStep(Int_t step);
   void SetBaseDir(TString basedir);
   void SetNTracks(Int_t ntracks);
   void SetRegenerate(Int_t regenerate);

   TProofBenchMode* GetMode()const;
   Long64_t GetNEvents()const;
   Int_t GetMaxNWorkers()const;
   Int_t GetStart()const;
   Int_t GetStop()const;
   Int_t GetStep()const;
   TString GetBaseDir()const;
   Int_t GetNTracks()const;
   Int_t GetRegenerate()const;

protected:

   Int_t FillNodeInfo();

private:

   TProof* fProof;                 //pointer to proof
   TProofBenchMode* fMode;

   Long64_t fNEvents;              //number of events in a file
   Int_t fMaxNWorkers;

   Int_t fStart;                   //starting number of cpu cores
   Int_t fStop;
   Int_t fStep;                    //test to be performed every fStep workers
   TString fBaseDir;               //base directory for files
   Int_t fNTracks;                 //number of tracks to generate in an event
   Int_t fRegenerate;              //when true, regenerate files

   //Int_t fDebug;                   //debug switch, when on various debug plots will be saved to file

   TList* fNodes;               // List of worker nodes info
   TDSet* fDataSetGenerated;  //Data set generated at worker nodes for benchmarking
//   TDSet* fDataSetGeneratedCleanup;//Data set generated at worker nodes for cleaning up memory

   ClassDef(TProofBenchFileGenerator,0)         //PROOF benchmark suite steering
};

#endif
