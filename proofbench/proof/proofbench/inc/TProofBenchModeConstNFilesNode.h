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
// TProofBenchModeConstNFilesNode                                                          //
//                                                                      //
// TProofBenchModeConstNFilesNode is a steering class for PROOF benchmark suite.           //
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

#ifndef ROOT_TProofBenchModeConstNFilesNode
#define ROOT_TProofBenchModeConstNFilesNode

#ifndef ROOT_TProofBenchMode
#include "TProofBenchMode.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

class TProof;

R__EXTERN TProof *gProof;

class TProofBenchModeConstNFilesNode : public TProofBenchMode {

public:

   TProofBenchModeConstNFilesNode(Int_t nfiles=-1, TProof* proof=gProof); //when nfiles=-1, as many files as maximum number 
                                               //of workers on node in the cluster will be generated
   virtual ~TProofBenchModeConstNFilesNode();          //destructor

   TMap* FilesToProcess(Int_t nf);

   Int_t MakeDataSets(Int_t nf,   //when ==-1, use data member fNFiles
                      Int_t start,  //
                      Int_t stop,  //when ==-1, end number of workers are set to max workers
                      Int_t step,
                      const TDSet* tdset,
                      const char* option,
                      TProof* proof);  //
   
   Int_t MakeDataSets(Int_t nf,
                      Int_t np,
                      const Int_t *wp,
                      const TDSet* tdset,
                      const char *option,
                      TProof* proof);

   TProofBenchMode::EFileType GetFileType();

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
   Int_t fNFiles;

   TList* fNodes;
   TString fName;

   ClassDef(TProofBenchModeConstNFilesNode,0)         //PROOF benchmark suite steering
};

#endif
