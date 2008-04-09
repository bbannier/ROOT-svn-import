// @(#)root/proofplayer:$Id:$
// Author: G. Ganis Mar 2008

/*************************************************************************
 * Copyright (C) 1995-2001, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofPlayerLite                                                     //
//                                                                      //
// This version of TProofPlayerRemote merges the functionality needed   //
// by clients and masters. It is used in optmized local sessions.       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofPlayerLite.h"

#include "MessageTypes.h"
#include "TDSet.h"
#include "TDSetProxy.h"
#include "TEntryList.h"
#include "TEventList.h"
#include "TMessage.h"
#include "TObjString.h"
#include "TPerfStats.h"
#include "TProof.h"
#include "TProofDebug.h"
#include "TSelector.h"
#include "TVirtualPacketizer.h"


//______________________________________________________________________________
Long64_t TProofPlayerLite::Process(TDSet *dset, const char *selector_file,
                                   Option_t *option, Long64_t nentries,
                                   Long64_t first)
{
   // Process specified TDSet on PROOF.
   // This method is called on client and on the PROOF master.
   // The return value is -1 in case of error and TSelector::GetStatus() in
   // in case of success.

   PDB(kGlobal,1) Info("Process","Enter");
   fDSet = dset;
   fExitStatus = kFinished;
   fEventsProcessed = 0;

   //   delete fOutput;
   if (!fOutput)
      fOutput = new TList;
   else
      fOutput->Clear();

   TPerfStats::Setup(fInput);
   TPerfStats::Start(fInput, fOutput);

   if(!SendSelector(selector_file)) return -1;

   TMessage mesg(kPROOF_PROCESS);
   TString fn(gSystem->BaseName(selector_file));

   // Parse option
   Bool_t sync = (fProof->GetQueryMode(option) == TProof::kSync);

   // Make sure that the temporary output list is empty
   if (fOutputLists) {
      fOutputLists->Delete();
      delete fOutputLists;
      fOutputLists = 0;
   }

   if (!sync) {
      gSystem->RedirectOutput(fProof->fLogFileName);
      Printf(" ");
      Info("Process","starting new query");
   }

   SafeDelete(fSelector);
   fSelectorClass = 0;
   if (!(fSelector = TSelector::GetSelector(selector_file))) {
      if (!sync)
         gSystem->RedirectOutput(0);
      return -1;
   }
   fSelectorClass = fSelector->IsA();
   fSelector->SetInputList(fInput);
   fSelector->SetOption(option);

   PDB(kLoop,1) Info("Process","Call Begin(0)");
   fSelector->Begin(0);

   PDB(kPacketizer,1) Info("Process","Create Proxy TDSet");
   TDSet *set = new TDSetProxy(dset->GetType(), dset->GetObjName(),
                               dset->GetDirectory());
   if (dset->TestBit(TDSet::kEmpty))
      set->SetBit(TDSet::kEmpty);
   if (InitPacketizer(dset, nentries, first, "TPacketizerUnit", "TPacketizer") != 0) {
      Error("Process", "cannot init the packetizer");
      fExitStatus = kAborted;
      return -1;
   }
   // reset start, this is now managed by the packetizer
   first = 0;

   if (!sync)
      gSystem->RedirectOutput(0);

   TCleanup clean(this);
   SetupFeedback();

   TString opt = option;

   // Workers will get the entry ranges from the packetizer
   Long64_t num = (fProof->IsParallel()) ? -1 : nentries;
   Long64_t fst = (fProof->IsParallel()) ? -1 : first;

   // Entry- or Event- list ?
   TEntryList *enl = (!fProof->IsMaster()) ? dynamic_cast<TEntryList *>(set->GetEntryList())
                                           : (TEntryList *)0;
   TEventList *evl = (!fProof->IsMaster() && !enl) ? dynamic_cast<TEventList *>(set->GetEntryList())
                                           : (TEventList *)0;
   // Broadcast main message
   PDB(kGlobal,1) Info("Process","Calling Broadcast");
   mesg << set << fn << fInput << opt << num << fst << evl << sync << enl;
   fProof->Broadcast(mesg);

   // Redirect logs from master to special log frame
   fProof->fRedirLog = kTRUE;

   if (!sync) {

      // Asynchronous query: just make sure that asynchronous input
      // is enabled and return the prompt
      PDB(kGlobal,1) Info("Process","Asynchronous processing:"
                                    " activating CollectInputFrom");
      fProof->Activate();

      // Return the query sequential number
      return fProof->fSeqNum;

   } else {

      // Wait for processing
      PDB(kGlobal,1) Info("Process","Synchronous processing: calling Collect");
      fProof->Collect();

      // Restore prompt logging (Collect leaves things as they were
      // at the time it was called)
      fProof->fRedirLog = kFALSE;

      HandleTimer(0); // force an update of final result
      // Store process info
      if (fPacketizer && fQuery)
         fQuery->SetProcessInfo(0, 0., fPacketizer->GetBytesRead(),
                                       fPacketizer->GetInitTime(),
                                       fPacketizer->GetProcTime());
      StopFeedback();

      if (GetExitStatus() != TProofPlayer::kAborted)
         return Finalize(kFALSE, sync);
      else
         return -1;
   }
}

//______________________________________________________________________________
Long64_t TProofPlayerLite::Finalize(Bool_t force, Bool_t sync)
{
   // Finalize a query.
   // Returns -1 in case error, 0 otherwise.

   if (fOutputLists == 0) {
      if (force && fQuery)
         return fProof->Finalize(Form("%s:%s", fQuery->GetTitle(),
                                               fQuery->GetName()), force);
   }

   Long64_t rv = 0;

   TPerfStats::Stop();

   // Merge the output files created on workers, if any
   MergeOutputFiles();

   if (fExitStatus != kAborted) {

      if (!sync) {
         // Reinit selector (with multi-sessioning we must do this until
         // TSelector::GetSelector() is optimized to i) avoid reloading of an
         // unchanged selector and ii) invalidate existing instances of
         // reloaded selector)
         if (ReinitSelector(fQuery) == -1) {
            Info("Finalize", "problems reinitializing selector \"%s\"",
                  fQuery->GetSelecImp()->GetName());
            return -1;
         }
      }

      // Some input parameters may be needed in Terminate
      fSelector->SetInputList(fInput);

      TIter next(fOutput);
      TList *output = fSelector->GetOutputList();
      while(TObject* obj = next()) {
         if (fProof->IsParallel() || DrawCanvas(obj) == 1)
            // Either parallel or not a canvas or not able to display it:
            // just add to the list
            output->Add(obj);
      }

      PDB(kLoop,1) Info("Finalize","Call Terminate()");
      fOutput->Clear("nodelete");
      fSelector->Terminate();

      rv = fSelector->GetStatus();

      // copy the output list back and clean the selector's list
      TIter it(output);
      while(TObject* o = it()) {
         fOutput->Add(o);
      }

      // Save the output list in the current query, if any
      if (fQuery) {
         fQuery->SetOutputList(fOutput);
         // Set in finalized state (cannot be done twice)
         fQuery->SetFinalized();
      } else {
         Warning("Finalize","current TQueryResult object is undefined!");
      }

      // We have transferred copy of the output objects in TQueryResult,
      // so now we can cleanup the selector, making sure that we do not
      // touch the output objects
      output->SetOwner(kFALSE);
      SafeDelete(fSelector);

      // Delete fOutput (not needed anymore, cannot be finalized twice),
      // making sure that the objects saved in TQueryResult are not deleted
      fOutput->SetOwner(kFALSE);
      SafeDelete(fOutput);
   } else {

      // Cleanup
      fOutput->SetOwner();
      SafeDelete(fSelector);
   }

   PDB(kGlobal,1) Info("Process","exit");
   return rv;
}

//______________________________________________________________________________
Bool_t TProofPlayerLite::HandleTimer(TTimer *)
{
   // Send feedback objects to client.

   PDB(kFeedback,2) Info("HandleTimer","Entry");

   if (fFeedbackTimer == 0) return kFALSE; // timer already switched off

   // process local feedback objects

   TList *fb = new TList;
   fb->SetOwner();

   TIter next(fFeedback);
   while( TObjString *name = (TObjString*) next() ) {
      TObject *o = fOutput->FindObject(name->GetName());
      if (o != 0) fb->Add(o->Clone());
   }

   if (fb->GetSize() > 0)
      StoreFeedback(this, fb); // adopts fb
   else
      delete fb;

   if (fFeedbackLists == 0) {
      fFeedbackTimer->Start(fFeedbackPeriod, kTRUE);   // maybe next time
      return kFALSE;
   }

   fb = MergeFeedback();

   fFeedbackTimer->Start(fFeedbackPeriod, kTRUE);

   return kFALSE; // ignored?
}
