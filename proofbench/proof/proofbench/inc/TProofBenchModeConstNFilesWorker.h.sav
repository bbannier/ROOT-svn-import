// @(#)root/proofx:$Id$
// Author: Sangsu Ryu 22/06/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofBenchModeConstNFilesWorker
#define ROOT_TProofBenchModeConstNFilesWorker

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBenchModeConstNFilesWorker                                     //
//                                                                      //
// A mode for PROOF benchmark test.                                     //
// In this mode, a given number of files are generated for each worker. //
// During the test, all of these files are processed no matter how many //
// workers are active.                                                  //
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

class TProofBenchModeConstNFilesWorker : public TProofBenchMode {

private:

   TProof* fProof;            //proof
   Int_t fNFiles;             //number of files a node for I/O test
   TProofNodes* fNodes;       //list of nodes
   TString fName;             //name of this mode

protected:

public:
   
   TProofBenchModeConstNFilesWorker(Int_t nfiles=1, TProof* proof=0, TProofNodes* nodes=0);
    virtual ~TProofBenchModeConstNFilesWorker();

   TMap* FilesToProcess(Int_t nfiles);

   Int_t MakeDataSets(Int_t nfiles, Int_t start, Int_t stop, Int_t step,
                      const TList* listfiles, const char* option,
                      TProof* proof, Int_t flag_nx);

   Int_t MakeDataSets(Int_t nfiles, Int_t np, const Int_t *wp,
                      const TList* listfiles, const char *option,
                      TProof* proof, Int_t flag_nx);

   TProofBenchMode::EFileType GetFileType()
                        {return TProofBenchMode::kFileBenchmark;}

   void Print(Option_t* option=0) const;
   
   void SetProof(TProof* proof);
   void SetNFiles(Int_t nfiles);

   TProof* GetProof() const;
   Int_t GetNFiles() const;
   const char* GetName() const; 

   ClassDef(TProofBenchModeConstNFilesWorker,0)  //A mode for PROOF benchmark
                                                 // test
};

#endif
