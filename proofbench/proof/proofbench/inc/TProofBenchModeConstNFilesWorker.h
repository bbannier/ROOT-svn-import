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
// TProofBenchModeConstNFilesWorker                                                          //
//                                                                      //
// TProofBenchModeConstNFilesWorker is a steering class for PROOF benchmark suite.           //
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

#ifndef ROOT_TProofBenchModeConstNFilesWorker
#define ROOT_TProofBenchModeConstNFilesWorker

#ifndef ROOT_TProofBenchMode
#include "TProofBenchMode.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

class TProof;

R__EXTERN TProof *gProof;

class TProofBenchModeConstNFilesWorker : public TProofBenchMode {

public:

   TProofBenchModeConstNFilesWorker(Int_t nfiles=1, TProof* proof=gProof);  //default is to generate as many files as number 
                                                 //of workers on each node
    virtual ~TProofBenchModeConstNFilesWorker();          //destructor

    TMap* FilesToProcess(Int_t nf);

    Int_t MakeDataSets(Int_t nf,
                       Int_t start,
                       Int_t stop,
                       Int_t step,
                       const TDSet* tdset,
                       const char* option,
                       TProof* proof);

     Int_t MakeDataSets(Int_t nf,
                        Int_t np,
                        const Int_t *wp,
                        const TDSet* tdset,
                        const char *option,
                        TProof* proof);

   TProofBenchMode::EFileType GetFileType(){return TProofBenchMode::kFileBenchmark;}

   void Print(Option_t* option=0)const;

   void SetProof(TProof* proof);
   void SetNFiles(Int_t nfiles);

   TProof* GetProof()const;
   Int_t GetNFiles()const;
   const char* GetName()const; 

protected:

   Int_t FillNodeInfo();

private:

   TProof* fProof;
   Int_t fNFiles;             //number of files a node for I/O test
   TList* fNodes;
   TString fName;

   ClassDef(TProofBenchModeConstNFilesWorker,0)         //PROOF benchmark suite steering
};

#endif
