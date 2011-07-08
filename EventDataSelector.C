// This class is derived from the ROOT class TSelector. 
// For more information on the TSelector framework see 
// $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.

// The file for this selector can be found at
// http://lcg-heppkg.web.cern.ch/lcg-heppkg/ROOT/eventdata.root
// i.e run
//   root [0] f = TFile::Open("http://lcg-heppkg.web.cern.ch/lcg-heppkg/ROOT/eventdata.root");
//   root [1] EventTree->Process("EventDataSelector.C+")

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

#include "TROOT.h"
#include "TChain.h"
#include "TFile.h"
#include "TSelector.h"
#include "TProofServ.h"
#include "TH1.h"
#include "TStyle.h"

const Int_t kMaxfParticles = 1293;

class EventDataSelector : public TSelector {
public :

   ULong64_t    fTotalDataSize; // sum of data size (in bytes) of all events

   // Declaration of leaf types
   Int_t        fParticles;
   Double_t     fParticlesPosX[kMaxfParticles];       //[fParticles_]
   Double_t     fParticlesMomentum[kMaxfParticles];   //[fParticles_]
   Int_t        fCurrentEventSize;

   // List of branches
   TBranch     *fParticlesBranch;
   TBranch     *fParticlesPosXBranch;
   TBranch     *fParticlesMomentumBranch;
   TBranch     *fEventSizeBranch;

   EventDataSelector(TTree * = 0): fTotalDataSize(0) { }

   virtual ~EventDataSelector() { }
   virtual void    Init(TTree *tree);
   virtual void    SlaveBegin(TTree *tree);
   virtual Bool_t  Process(Long64_t entry);
   virtual void    Terminate();
   virtual Int_t   Version() const { return 2; }

   ClassDef(EventDataSelector,0);
};

void EventDataSelector::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // To use SetBranchAddress() with simple types (e.g. double, int)
   // instead of objects (e.g. std::vector&lt;Particle&gt;).
   tree->SetMakeClass(1);

   // Connect the branches with their member variables.
   tree->SetBranchAddress("fParticles", &fParticles, &fParticlesBranch);
   tree->SetBranchAddress("fParticles.fPosX", fParticlesPosX, &fParticlesPosXBranch);
   tree->SetBranchAddress("fParticles.fMomentum", fParticlesMomentum, &fParticlesMomentumBranch);
   tree->SetBranchAddress("fEventSize", &fCurrentEventSize, &fEventSizeBranch);
}

void EventDataSelector::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

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

   // *** 1. *** Tell the tree to load the data for this entry:
   // We only need the number of particles...
   fParticlesBranch->GetEntry(entry);
   // ... and their position in X...
   fParticlesPosXBranch->GetEntry(entry);
   // ... their momentum...
   fParticlesMomentumBranch->GetEntry(entry);
   // ... and the size of the event:
   fEventSizeBranch->GetEntry(entry);

   // *** 2. *** Do the actual analysis

   fTotalDataSize += fEventSizeBranch;

   return kTRUE;
}

void EventDataSelector::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

   int sizeInMB = fTotalDataSize/1024/1024;
   printf("Total size of all events: %d MB\n", sizeInMB);
}
