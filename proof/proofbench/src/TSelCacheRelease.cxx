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
// TSelCacheRelease                                                     //
//                                                                      //
// PROOF selector for file cache release.                               //
// List of files to be cleaned for each node is provided by client.     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define TSelCacheRelease_cxx

#include "TSelCacheRelease.h"
#if 0
#include "TParameter.h"
#include "TProofNodeInfo.h"
#include "TProofBenchTypes.h"
#include "TProof.h"
#include "TMap.h" 
#include "TDSet.h"
#include "TFileInfo.h"
#include "TFile.h"
#include "TSortedList.h"
#include "TRandom.h"
#include "Event.h"
#include "TProofServ.h"
#else
#include "TDSet.h"
#include "TMap.h"

#include <unistd.h>
#include <fcntl.h>
#endif

ClassImp(TSelCacheRelease)

//______________________________________________________________________________
Bool_t TSelCacheRelease::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either TTree::GetEntry() or TBranch::GetEntry()
   // to read either all or the required parts of the data. When processing
   // keyed objects with PROOF, the object is already loaded and is available
   // via the fObject pointer.
   //
   // This function should contain the "body" of the analysis. It can contain
   // simple or elaborate selection criteria, run algorithms on the data
   // of the event and typically fill histograms.

   // WARNING when a selector is used with a TChain, you must use
   //  the pointer to the current TTree to call GetEntry(entry).
   //  The entry is always the local entry number in the current tree.
   //  Assuming that fChain is the pointer to the TChain being processed,
   //  use fChain->GetTree()->GetEntry(entry).

   TDSetElement *fCurrent = 0;
   TPair *elemPair = 0;
   if (fInput && (elemPair = dynamic_cast<TPair *>
                      (fInput->FindObject("PROOF_CurrentElement")))) {
      if ((fCurrent = dynamic_cast<TDSetElement *>(elemPair->Value()))) {
         Info("Process", "entry %lld: file: '%s'", entry, fCurrent->GetName());
      } else {
         Error("Process", "entry %lld: no file specified!", entry);
         return kFALSE;
      }
   }


   TString filename = fCurrent->GetName();
   Info("Process", "cleaning up file cache of file '%s'", filename.Data());
   Int_t fd;
   fd = open(filename.Data(), O_RDONLY);
   if (fd) {
      fdatasync(fd);
      posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
      close(fd);
   } else {
      Error("Process", "cannot open file '%s' for cache clean up", filename.Data());
   }

   return kTRUE;
}

