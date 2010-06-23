// @(#)root/proofx:$Id:$
// Author: Sangsu Ryu 22/6/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofBenchFileGenerator
#define ROOT_TProofBenchFileGenerator

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBenchFileGenerator                                             //
//                                                                      //
// This class lets you generate files and register them as datasets     //
// to be used for Proof benchmark test.                                 //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

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

private:

   TProof* fProof;                 //pointer to proof
   TProofBenchMode* fMode;         //benchmark mode 

   Long64_t fNEvents;              //number of events in a file
   Int_t fMaxNWorkers;             //reserved

   Int_t fStart;                   //start number of workers to scan
   Int_t fStop;                    //stop number of workers to scan
   Int_t fStep;                    //test to be performed every fStep workers
   TString fBaseDir;               //base directory for files
   Int_t fNTracks;                 //number of tracks to generate in an event
   Int_t fRegenerate;              //when true, regenerate files

   TList* fNodes;                  //list of nodes information
   TDSet* fDataSetGenerated;       //data set generated at worker nodes for benchmarking

protected:

   Int_t FillNodeInfo();

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

   virtual ~TProofBenchFileGenerator();

   virtual Int_t  GenerateFiles(Int_t nf=-1,
                                Long64_t nevents=-1,
                                TString basedir="",
                                Int_t regenerate=-1,
                                Int_t ntracks=-1);

   virtual Int_t MakeDataSets(Int_t nf=-1,
                              Int_t start=-1,
                              Int_t stop=-1,
                              Int_t step=-1,
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

   ClassDef(TProofBenchFileGenerator,0)  //Generate files and register them as datasets to be used for the Proof benchmark test
};

#endif
