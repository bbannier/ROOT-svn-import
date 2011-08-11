//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Jul 21 14:20:19 2011 by ROOT version 5.27/06
// from TTree rt/ROOT Tree
// found on file: /tmp/ganis/event/event-lxplus301.cern.ch-999.root
//////////////////////////////////////////////////////////

#ifndef ReadSel_h
#define ReadSel_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>

#include "RTEvent.h"

class TH1;

class ReadSel : public TSelector {
public :
   // Histos
   TH1 *_jets;
   TH1 *_jet_flavor;
   TH1 *_jet_e;
   TH1 *_jet_px;
   TH1 *_jet_py;
   TH1 *_jet_pz;
   TH1 *_jet_x;
   TH1 *_jet_y;
   TH1 *_jet_z;

   TH1 *_muons;
   TH1 *_muon_e;
   TH1 *_muon_px;
   TH1 *_muon_py;
   TH1 *_muon_pz;
   TH1 *_muon_x;
   TH1 *_muon_y;
   TH1 *_muon_z;

   TTree          *fChain;   //!pointer to the analyzed TTree or TChain

   // Declaration of leaf types
   RTEvent         *event;

   // List of branches
   TBranch        *b_event;   //!

   ReadSel(TTree * /*tree*/ =0) : _jets(0), _jet_flavor(0), _jet_e(0), _jet_px(0),
                                _jet_py(0), _jet_pz(0), _jet_x(0), _jet_y(0), _jet_z(0),
                                _muons(0), _muon_e(0), _muon_px(0), _muon_py(0), _muon_pz(0),
                                _muon_x(0), _muon_y(0), _muon_z(0) { }
   virtual ~ReadSel() { }
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

   ClassDef(ReadSel,0);
};

#endif

#ifdef ReadSel_cxx
void ReadSel::Init(TTree *tree)
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
   fChain->SetMakeClass(0);

   fChain->SetBranchAddress("event", &event, &b_event);
}

Bool_t ReadSel::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.
   Info("Notify","processing file: %s", fChain->GetCurrentFile()->GetName());

   return kTRUE;
}

#endif // #ifdef ReadSel_cxx
