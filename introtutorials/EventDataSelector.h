//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Jul  8 16:08:00 2010 by ROOT version 5.27/05
// from TTree EventData/Tutorial tree
// found on file: eventdata.root
//////////////////////////////////////////////////////////

#ifndef EventDataSelector_h
#define EventDataSelector_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
// ============ <EDITED HERE ================
#include <TProofServ.h>
// ============ /EDITED HERE> ================
   const Int_t kMaxfParticles = 1293;

class EventDataSelector : public TSelector {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain

   // ============ <EDITED HERE ================
   TH1            *fPosX; // X position of the particles
   ULong64_t       fTotalDataSize; // sum of data size (in bytes) of all events
   // ============ /EDITED HERE> ================

   // Declaration of leaf types
 //EventData       *event;
   Int_t           fParticles_;
   Double_t        fParticles_fPosX[kMaxfParticles];   //[fParticles_]
   Double_t        fParticles_fPosY[kMaxfParticles];   //[fParticles_]
   Double_t        fParticles_fPosZ[kMaxfParticles];   //[fParticles_]
   Double_t        fParticles_fMomentum[kMaxfParticles];   //[fParticles_]
   Double_t        fParticles_fMomentumPhi[kMaxfParticles];   //[fParticles_]
   Double_t        fParticles_fMomentumEta[kMaxfParticles];   //[fParticles_]
   Long64_t        fParticles_fTags[kMaxfParticles][128];   //[fParticles_]
   Int_t           fEventSize;

   // List of branches
   TBranch        *b_event_fParticles_;   //!
   TBranch        *b_fParticles_fPosX;   //!
   TBranch        *b_fParticles_fPosY;   //!
   TBranch        *b_fParticles_fPosZ;   //!
   TBranch        *b_fParticles_fMomentum;   //!
   TBranch        *b_fParticles_fMomentumPhi;   //!
   TBranch        *b_fParticles_fMomentumEta;   //!
   TBranch        *b_fParticles_fTags;   //!
   TBranch        *b_event_fEventSize;   //!

   // ============ <EDITED HERE ================
   EventDataSelector(TTree * /*tree*/ =0):
      fPosX(0), fTotalDataSize(0) { }
   // ============ /EDITED HERE> ================

   virtual ~EventDataSelector() { }
   virtual Int_t   Version() const { return 2; }
   virtual void    Begin(TTree *tree);
   virtual void    SlaveBegin(TTree *tree);
   virtual void    Init(TTree *tree);
   virtual Bool_t  Notify();
   virtual Bool_t  Process(Long64_t entry);
   virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
   virtual void    SetOption(const char *option) { fOption = option; }
   virtual void    SetObject(TObject *obj) { fObject = obj; }
   virtual void    SetInputList(TList *input) { fInput = input; }
   virtual TList  *GetOutputList() const { return fOutput; }
   virtual void    SlaveTerminate();
   virtual void    Terminate();

   ClassDef(EventDataSelector,0);
};

#endif

#ifdef EventDataSelector_cxx
void EventDataSelector::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fChain->SetMakeClass(1);

   // ============ <EDITED HERE ================
   if (!gProofServ) {
      // Enable TTree read-ahead:
      fChain->SetCacheSize(30000000); // 30MB
      fChain->AddBranchToCache("fParticles");
      fChain->AddBranchToCache("fParticles.fPosX");
      fChain->AddBranchToCache("fParticles.fMomentum");
      fChain->AddBranchToCache("fEventSize");
   }
   // ============ /EDITED HERE> ================

   fChain->SetBranchAddress("fParticles", &fParticles_, &b_event_fParticles_);
   fChain->SetBranchAddress("fParticles.fPosX", fParticles_fPosX, &b_fParticles_fPosX);
   fChain->SetBranchAddress("fParticles.fPosY", fParticles_fPosY, &b_fParticles_fPosY);
   fChain->SetBranchAddress("fParticles.fPosZ", fParticles_fPosZ, &b_fParticles_fPosZ);
   fChain->SetBranchAddress("fParticles.fMomentum", fParticles_fMomentum, &b_fParticles_fMomentum);
   fChain->SetBranchAddress("fParticles.fMomentumPhi", fParticles_fMomentumPhi, &b_fParticles_fMomentumPhi);
   fChain->SetBranchAddress("fParticles.fMomentumEta", fParticles_fMomentumEta, &b_fParticles_fMomentumEta);
   fChain->SetBranchAddress("fParticles.fTags[128]", fParticles_fTags, &b_fParticles_fTags);
   fChain->SetBranchAddress("fEventSize", &fEventSize, &b_event_fEventSize);
}

Bool_t EventDataSelector::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

#endif // #ifdef EventDataSelector_cxx
