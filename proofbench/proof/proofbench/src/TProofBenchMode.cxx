// @(#)root/proofx:$Id$
// Author: Sangsu Ryu 22/06/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBenchMode                                                      //
//                                                                      //
// An abstract class for modes for PROOF benchmark test.                //
// A mode determines how files are generated in the cluster and used    //
// during the PROOF benchmark test.                                     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofBenchMode.h"
#include "TList.h"
#include "TProof.h"

ClassImp(TProofBenchMode)

const char* const kPROOF_BenchSelPar    = "ProofBenchSel";  // PAR with bench selectors
const char* const kPROOF_BenchSelParDir = "proof/proofbench/src/";  // Location of ProofBenchSel

//______________________________________________________________________________
TProofBenchMode::TProofBenchMode(TProof *proof, const char *name) : TNamed(name, "")
{
   // Constructor: check PROOF and load selectors PAR
   
   fProof = proof;
   if (!fProof){
      fProof = gProof;
   }
   ResetBit(kInvalidObject);
   if (!fProof || (fProof && !fProof->IsValid())) {
      SetBit(kInvalidObject);
      return;
   }
   // Load selectors if needed
   TList *enp = fProof->GetListOfEnabledPackages();
   if (enp && enp->FindObject(kPROOF_BenchSelPar)) {
      if (gDebug > 0) Info("","package '%s' already enabled", kPROOF_BenchSelPar);
      return;
   }
   TString par = TString::Format("%s%s.par", kPROOF_BenchSelParDir, kPROOF_BenchSelPar);
   Printf("Uploading '%s' ...", par.Data());
   if (fProof->UploadPackage(par) != 0) {
      SetBit(kInvalidObject);
      return;
   }
   Printf("Enabling '%s' ...", kPROOF_BenchSelPar);
   if (fProof->EnablePackage(kPROOF_BenchSelPar) != 0) {
      SetBit(kInvalidObject);
      return;
   }
} 

//______________________________________________________________________________
TProofBenchMode::~TProofBenchMode()
{
   //destructor
}
