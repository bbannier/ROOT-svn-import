// @(#)root/proofx:$Id$
// Author: Sangsu Ryu 22/06/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofBench
#define ROOT_TProofBench

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBench                                                          //
//                                                                      //
// Steering class for PROOF benchmarks                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif
#ifndef ROOT_TProofBenchTypes
#include "TProofBenchTypes.h"
#endif
#ifndef ROOT_TString
#include "TString.h"
#endif


class TFile;
class TProof;
class TProofBenchRunCPU;
class TProofBenchRunDataRead;

class TProofBench : public TObject {

private:

protected:

   TProof* fProof;               // Proof
   TFile  *fOutFile;             // Output file
   TString fOutFileName;         // Name of the output file
   Int_t   fNtries;              // Number of times a measurement is repeated

   TProofBenchRunCPU      *fRunCPU; // Instance to run CPU scans
   TProofBenchRunDataRead *fRunDS;  // Instance to run data-read scans

public:

   TProofBench(const char *url, const char *outfile, const char *proofopt = 0);

   virtual ~TProofBench();

   Int_t RunCPU(Long64_t nevents=-1, Int_t start=-1, Int_t stop=-1, Int_t step=-1);
   Int_t RunCPUx(Long64_t nevents=-1, Int_t start=-1, Int_t stop=-1);
   Int_t RunDataSet(const char *dset, EPBReadType readtype = kPBReadOpt,
                    Int_t start = 1, Int_t stop = -1, Int_t step = 1, TProof *pclnup = 0);

   Int_t ReleaseCache(const char *dset);
                    
   void  CloseOutFile() { SetOutFile(0); }
   Int_t OpenOutFile(Bool_t wrt = kFALSE, Bool_t verbose = kTRUE);
   Int_t SetOutFile(const char *outfile, Bool_t verbose = kTRUE);
   void SetNTries(Int_t nt) { if (nt > 0) fNtries = nt; }

   static void DrawCPU(const char *outfile, const char *opt = "abs:");
   static void DrawDataSet(const char *outfile, const char *opt = "mbs:", Int_t kseq = 1);

   ClassDef(TProofBench, 0)   // Steering class for PROOF benchmarks
};

#endif
