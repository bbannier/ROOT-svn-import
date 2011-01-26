// @(#)root/proofx:$Id$
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
class TProofNodes;

class TProofBenchModeConstNFilesNode : public TProofBenchMode {

private:

   Int_t fNFiles;    //number of files per node
   TProofNodes * fNodes;    //list of nodes

protected:

public:

   TProofBenchModeConstNFilesNode(Int_t nfiles=-1, TProof* proof=0,
                                  TProofNodes* nodes=0);
   virtual ~TProofBenchModeConstNFilesNode();

   TMap* FilesToProcess(Int_t nfiles);

   Int_t MakeDataSets(Int_t nfiles, Int_t start, Int_t stop, Int_t step,
                      const TList* listfiles, const char* option,
                      TProof* proof);
   
   Int_t MakeDataSets(Int_t nfiles, Int_t np, const Int_t *wp,
                      const TList* listfiles, const char *option,
                      TProof* proof);

   EPBFileType GetFileType();

   void Print(Option_t* option=0) const;

   void SetProof(TProof* proof);
   void SetNFiles(Int_t nfiles);

   TProof* GetProof() const;
   Int_t GetNFiles() const;
   const char* GetName() const;

   ClassDef(TProofBenchModeConstNFilesNode,0)  //A mode for PROOF benchmark
                                               //test
};

#endif
