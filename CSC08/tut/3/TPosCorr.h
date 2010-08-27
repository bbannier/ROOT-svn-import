//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Wed Aug 19 16:32:00 2009 by ROOT version 5.25/01
// from TTree MyTree/MyEvent Tree
// found on file: exercise3.root
//////////////////////////////////////////////////////////

#ifndef TPosCorr_h
#define TPosCorr_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TH2.h>

   const Int_t kMaxelectrons = 3000;

class TPosCorr : public TSelector {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain

   // Declaration of leaf types
   Int_t           electrons_;
   Float_t         electrons_fVertex_fX[kMaxelectrons];   //[electrons_]
   Float_t         electrons_fVertex_fY[kMaxelectrons];   //[electrons_]

   // List of branches
   TBranch        *b_electrons_;   //!
   TBranch        *b_electrons_fVertex_fX;   //!
   TBranch        *b_electrons_fVertex_fY;   //!

   TH2* fHistCorrPos;

   TPosCorr(TTree * /*tree*/ =0): fChain(0), fHistCorrPos(0) { }
   virtual ~TPosCorr() { }
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

   ClassDef(TPosCorr,0);
};

#endif

#ifdef TPosCorr_cxx
void TPosCorr::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("electrons", &electrons_, &b_electrons_);
   fChain->SetBranchAddress("electrons.fVertex.fX", electrons_fVertex_fX, &b_electrons_fVertex_fX);
   fChain->SetBranchAddress("electrons.fVertex.fY", electrons_fVertex_fY, &b_electrons_fVertex_fY);
}

Bool_t TPosCorr::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

#endif // #ifdef TPosCorr_cxx
