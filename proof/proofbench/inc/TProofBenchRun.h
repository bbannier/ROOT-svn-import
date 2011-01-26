// @(#)root/proofx:$Id$
// Author: Sangsu Ryu 22/06/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofBenchRun
#define ROOT_TProofBenchRun

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBenchRun                                                       //
//                                                                      //
// Abstract base class for PROOF benchmark run.                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif
#ifndef ROOT_TProofBenchTypes
#include "TProofBenchTypes.h"
#endif

//class TFile;
class TProof;

class TProofBenchRun : public TObject {

private:

protected:

   TProof* fProof;               //proof

public:

   TProofBenchRun(TProof *proof = 0);

   virtual ~TProofBenchRun();

   virtual void Run(Long64_t nevents, Int_t start=-1, Int_t stop=-1,
                    Int_t step=-1, Int_t ntries=-1, Int_t debug=-1,
                    Int_t draw=-1)=0;
   virtual void Run(const char *dset, Int_t start=-1, Int_t stop=-1,
                    Int_t step=-1, Int_t ntries=-1, Int_t debug=-1,
                    Int_t draw=-1)=0;

   virtual void Print(Option_t* option="")const=0;

   ClassDef(TProofBenchRun, 0)   //Abstract base class for PROOF benchmark run
};

#endif
