// @(#)root/proofx:$Id:$
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

class TProofBenchRun : public TObject {

private:

public:

   enum EReadType {
      kReadNotSpecified=0,                    //read type not specified
      kReadFull=1,                            //read in a full event
      kReadOpt=2,                             //read in part of an event
      kReadNo=4,                              //do not read in event
      kReadAll=kReadFull | kReadOpt | kReadNo //read in a full event, part of a event and no event
   };

   enum EHistType {
      kHistNotSpecified=0,                  //histogram type not specified
      kHist1D=1,                            //1-D histogram
      kHist2D=2,                            //2-D histogram
      kHist3D=4,                            //3-D histogram
      kHistAll=kHist1D | kHist2D | kHist3D  //1-D, 2-D and 3-D histograms
   };

   enum ECleanupType {
      kCleanupNotSpecified=0,               //clean-up type not specified
      kCleanupFile=1,                       //clean-up by reading in dedicated files
      kCleanupKernel=2                      //clean-up by system call
   };

   virtual ~TProofBenchRun();

   virtual void Run(Long64_t nevents=-1,
                    Int_t ntries=-1,
                    Int_t start=-1,
                    Int_t stop=-1,
                    Int_t step=-1,
                    Int_t debug=-1,
                    Int_t draw=-1)=0;

   virtual void DrawPerfProfiles()=0;

   virtual void Print(Option_t* option="")const=0;

   ClassDef(TProofBenchRun, 0)   //Abstract base class for PROOF benchmark run
};

#endif
