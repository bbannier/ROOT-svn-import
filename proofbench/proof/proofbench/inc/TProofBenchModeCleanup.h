// @(#)root/proofx:$Id:$
// Author:

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofBenchModeCleanup
#define ROOT_TProofBenchModeCleanup

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBenchModeCleanup                                                          //
//                                                                      //
// TProofBenchModeCleanup is a steering class for PROOF benchmark suite.           //
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

#ifndef ROOT_TProofBenchMode
#include "TProofBenchMode.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

class TProof;

R__EXTERN TProof *gProof;

class TProofBenchModeCleanup : public TProofBenchMode {

private:

   TProof* fProof;
   TList* fNodes;
   TString fName;

protected:

   Int_t FillNodeInfo();

public:

   TProofBenchModeCleanup(TProof* proof=gProof);

   virtual ~TProofBenchModeCleanup();          //destructor

   TMap* FilesToProcess(Int_t);//not used
 
   Int_t MakeDataSets(Int_t,   //not used
                      Int_t,   //not used
                      Int_t,   //not used
                      Int_t,   //not used
                      const TDSet* tdset,
                      const char* option,
                      TProof* proof);
 
   Int_t MakeDataSets(Int_t,
                      Int_t,
                      const Int_t*,
                      const TDSet*,
                      const char*,
                      TProof*); //this function does nothing

   TProofBenchMode::EFileType GetFileType();

   void Print(Option_t* option=0)const;
 
   void SetProof(TProof* proof);
   void SetNFiles(Int_t);

   TProof* GetProof()const;
   Int_t GetNFiles()const;
   const char* GetName()const;

   ClassDef(TProofBenchModeCleanup,0)         //PROOF benchmark suite steering
};

#endif
