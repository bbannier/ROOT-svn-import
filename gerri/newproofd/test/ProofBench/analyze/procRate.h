//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Jan 15 14:01:39 2008 by ROOT version 5.17/05
// from TTree PROOF_PerfStats/PROOF Statistics
//////////////////////////////////////////////////////////

#ifndef procRate_h
#define procRate_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include "TPerfStats.h"

class procRate : public TSelector {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain

   // Declaration of leave types
   TPerfEvent      *PerfEvents;

   // List of branches
   TBranch        *b_PerfEvents;   //!

   procRate(TTree * /*tree*/ =0) { }
   virtual ~procRate() { }
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
private :
   Float_t *x1;
   Float_t *y1;
   Int_t fMaxTime;
   ClassDef(procRate,0);
};

#endif

#ifdef procRate_cxx
void procRate::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normaly not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   PerfEvents = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fMaxTime = 0;
   fChain = tree;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("PerfEvents", &PerfEvents, &b_PerfEvents);

   TString option = GetOption();
   
   x1 = (Float_t*) malloc(tree->GetEntries() * sizeof(Float_t));
   y1 = (Float_t*) malloc(tree->GetEntries() * sizeof(Float_t));
   for(int i = 0; i < tree->GetEntries(); i++) {
      x1[i] = i;
      y1[i] = 0;
   }
}

Bool_t procRate::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normaly not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

#endif // #ifdef procRate_cxx
