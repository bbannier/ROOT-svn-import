// @(#)root/proofx:$Id:$
// Author: Sangsu Ryu 22/06/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofBenchModeConstNFilesNode
#define ROOT_TProofBenchModeConstNFilesNode

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBenchModeConstNFilesNode                                       //
//                                                                      //
// A mode for PROOF benchmark test.                                     //
// In this mode, the same number of files are generated for all nodes   //
// in the cluster regardless of number of workers on the node.          //
// During the test, all of these files are processed no matter how many //
// workers are active.                                                  //
// Default number of files on each node is the maximum number of workers//
// on a node in the cluster.                                            //
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

class TProofBenchModeConstNFilesNode : public TProofBenchMode {

private:

   TProof* fProof;   //proof
   Int_t fNFiles;    //number of files per node
   TList* fNodes;    //list of nodes
   TString fName;    //name of this mode

protected:

   Int_t FillNodeInfo();

public:

   TProofBenchModeConstNFilesNode(Int_t nfiles=-1, TProof* proof=gProof);
   virtual ~TProofBenchModeConstNFilesNode();

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

   TProofBenchMode::EFileType GetFileType();

   void Print(Option_t* option=0)const;

   void SetProof(TProof* proof);
   void SetNFiles(Int_t nfiles);

   TProof* GetProof()const;
   Int_t GetNFiles()const;
   const char* GetName()const;

   ClassDef(TProofBenchModeConstNFilesNode,0)  //A mode for PROOF benchmark test
};

#endif
