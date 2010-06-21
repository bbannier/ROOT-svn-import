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
// TProofBenchRun                                                          //
//                                                                      //
// TProofBenchRun is a steering class for PROOF benchmark suite.           //
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

#ifndef ROOT_TProofBenchRun
#define ROOT_TProofBenchRun

#ifndef ROOT_TObject
#include "TObject.h"
#endif

class TProofBenchRun : public TObject {

public:

   enum EReadType {kReadNotSpecified=0, 
                   kReadFull=1, 
                   kReadOpt=2, 
                   kReadNo=4,
                   kReadAll=kReadFull | kReadOpt | kReadNo}; //Read type

   enum EHistType {kHistNotSpecified=0, 
                   kHist1D=1, 
                   kHist2D=2, 
                   kHist3D=4,
                   kHistAll=kHist1D | kHist2D | kHist3D}; //histogram types for CPU test

   enum ECleanupType {kCleanupNotSpecified=0,
                      kCleanupFile=1,        
                      kCleanupKernel=2};      //clean-up run type 

   virtual ~TProofBenchRun();           //Destructor

   virtual void Run(Long64_t nevents=-1,
                    Int_t ntries=-1,
                    Int_t start=-1,
                    Int_t stop=-1,
                    Int_t step=-1,
                    Int_t debug=-1,
                    Int_t draw=-1)=0;

   virtual void DrawPerfProfiles()=0;

   virtual void Print(Option_t* option="")const=0;   //Print status of an instance of this class 

private:

   ClassDef(TProofBenchRun, 0)   //PROOF benchmark suite steering
};

#endif
