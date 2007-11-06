//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Oct  9 09:09:38 2007 by ROOT version 5.17/03
// from TTree T/test
// found on file: Memory Directory
//////////////////////////////////////////////////////////

#ifndef PythiaMC_h
#define PythiaMC_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>

class TH1F;
class TPythia6;
class TFile;
class TProofFile;
class TTree;
class TClonesArray;

class PythiaMC : public TSelector {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain

   // Declaration of leave types
   TH1F           *fH1;
   TPythia6       *fPythia;
   TFile          *fFile;
   TProofFile     *fProofFile;
   TTree          *fTree;
   Int_t           fNParticles;        // Number of particles
   TClonesArray   *fParticles;         // Array with all particles

   // List of branches

   PythiaMC(TTree * /*tree*/ =0) { fH1 = 0; fPythia = 0; fFile = 0;
                                   fProofFile = 0; fTree = 0; fParticles = 0;}
   virtual ~PythiaMC();
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

   ClassDef(PythiaMC,0);
};

#endif

#ifdef PythiaMC_cxx
void PythiaMC::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normaly not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fChain->SetMakeClass(1);

}

Bool_t PythiaMC::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normaly not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

#endif // #ifdef PythiaMC_cxx
