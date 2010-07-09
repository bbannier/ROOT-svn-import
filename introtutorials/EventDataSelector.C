#define EventDataSelector_cxx
// The class definition in EventDataSelector.h has been generated automatically
// by the ROOT utility TTree::MakeSelector(). This class is derived
// from the ROOT class TSelector. For more information on the TSelector
// framework see $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.

// ============ <EDITED HERE ================
// The file for this selector can be found at
// http://lcg-heppkg.web.cern.ch/lcg-heppkg/ROOT/eventdata.root
// i.e run
//   root [0] f = TFile::Open("http://lcg-heppkg.web.cern.ch/lcg-heppkg/ROOT/eventdata.root");
//   root [1] EventTree->Process("EventDataSelector.C+")
// ============ /EDITED HERE> ================

// The following methods are defined in this file:
//    Begin():        called every time a loop on the tree starts,
//                    a convenient place to create your histograms.
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//                    called only on the slave servers.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//
// To use this file, try the following session on your Tree T:
//
// Root > T->Process("EventDataSelector.C")
// Root > T->Process("EventDataSelector.C","some options")
// Root > T->Process("EventDataSelector.C+")
//

#include "EventDataSelector.h"
#include <TH2.h>
#include <TStyle.h>


void EventDataSelector::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();
   // ============ <EDITED HERE ================
   fPosX = new TH1F("hPosX", "Position in X", 20, -5, 5);
   // enable bin errors:
   fPosX->Sumw2();
   // ============ /EDITED HERE> ================
}

void EventDataSelector::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

}

Bool_t EventDataSelector::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either EventDataSelector::GetEntry() or TBranch::GetEntry()
   // to read either all or the required parts of the data. When processing
   // keyed objects with PROOF, the object is already loaded and is available
   // via the fObject pointer.
   //
   // This function should contain the "body" of the analysis. It can contain
   // simple or elaborate selection criteria, run algorithms on the data
   // of the event and typically fill histograms.
   //
   // The processing can be stopped by calling Abort().
   //
   // Use fStatus to set the return value of TTree::Process().
   //
   // The return value is currently not used.

   // ============ <EDITED HERE ================
   // *** 1. *** Tell the tree to load the data for this entry:
   // We only need the number of particles...
   b_event_fParticles_->GetEntry(entry);
   // ... and their position in X...
   b_fParticles_fPosX->GetEntry(entry);
   // ... their momentum...
   b_fParticles_fMomentum->GetEntry(entry);
   // ... and the size of the event:
   b_event_fEventSize->GetEntry(entry);

   // *** 2. *** Do the actual analysis
   for (int iParticle = 0; iParticle < fParticles_; ++iParticle) {
      if (fParticles_fMomentum[iParticle] > 40.)
         fPosX->Fill(fParticles_fPosX[iParticle]);
   }

   fTotalDataSize += fEventSize;
   // ============ /EDITED HERE> ================

   return kTRUE;
}

void EventDataSelector::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

void EventDataSelector::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

   // ============ <EDITED HERE ================
   int sizeInMB = fTotalDataSize/1024/1024;
   printf("Total size of all events: %d MB\n", sizeInMB);

   // Fit the histogram:
   fPosX->Fit("pol2");

   // and draw it:
   fPosX->Draw();
   // ============ /EDITED HERE> ================
}
