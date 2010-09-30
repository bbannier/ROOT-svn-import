// @(#)root/proofx:$Id$
// Author: Sangsu Ryu 22/06/2010

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
// TProofBenchModeCleanup                                               //
//                                                                      //
// Cleaning-up mode for Proof benchmark test.                           //
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

class TProofBenchModeCleanup : public TProofBenchMode {

private:

   TProof* fProof;    //proof
   TProofNodes* fNodes;     //list of nodes
   TString fName;     //name of this mode

public:

   TProofBenchModeCleanup(TProof* proof=0, TProofNodes* node=0);

   virtual ~TProofBenchModeCleanup();

   TMap* FilesToProcess(Int_t);
 
   Int_t MakeDataSets(Int_t, Int_t, Int_t, Int_t, const TList* listfiles,
                      const char* option, TProof* proof);
 
   Int_t MakeDataSets(Int_t, Int_t, const Int_t*, const TList*, const char*,
                      TProof*);

   TProofBenchMode::EFileType GetFileType();

   void Print(Option_t* option=0) const;
 
   void SetProof(TProof* proof);
   void SetNFiles(Int_t);

   TProof* GetProof() const;
   Int_t GetNFiles() const;
   const char* GetName() const;

   ClassDef(TProofBenchModeCleanup,0)    //Cleaning-up mode for PROOF
                                         // benchmark test
};

#endif
