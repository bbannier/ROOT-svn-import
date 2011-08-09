//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri Jul  8 16:57:51 2011 by ROOT version 5.31/01
// from TTree rt/ROOT Tree
// found on file: testRT.root
//////////////////////////////////////////////////////////

#ifndef ReadSelSplit_h
#define ReadSelSplit_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
   const Int_t kMax_jets = 10;
   const Int_t kMax_muons = 5;

class TH1;

class ReadSelSplit : public TSelector {
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
   
   Bool_t          fReadAll;             // Switch for reading all or targetted

   TObject        *Find(const char *n);  // Find object in fInput or gDirectory

   TTree          *fChain;   //!pointer to the analyzed TTree or TChain

   // Declaration of leaf types
 //RTEvent         *event;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   Int_t           _jets_;
   UInt_t          _jets_fUniqueID[kMax_jets];   //[_jets_]
   UInt_t          _jets_fBits[kMax_jets];   //[_jets_]
   UInt_t          _jets__p4_fUniqueID[kMax_jets];   //[_jets_]
   UInt_t          _jets__p4_fBits[kMax_jets];   //[_jets_]
   UInt_t          _jets__p4_fP_fUniqueID[kMax_jets];   //[_jets_]
   UInt_t          _jets__p4_fP_fBits[kMax_jets];   //[_jets_]
   Double_t        _jets__p4_fP_fX[kMax_jets];   //[_jets_]
   Double_t        _jets__p4_fP_fY[kMax_jets];   //[_jets_]
   Double_t        _jets__p4_fP_fZ[kMax_jets];   //[_jets_]
   Double_t        _jets__p4_fE[kMax_jets];   //[_jets_]
   UInt_t          _jets__vertex_fUniqueID[kMax_jets];   //[_jets_]
   UInt_t          _jets__vertex_fBits[kMax_jets];   //[_jets_]
   Double_t        _jets__vertex_fX[kMax_jets];   //[_jets_]
   Double_t        _jets__vertex_fY[kMax_jets];   //[_jets_]
   Double_t        _jets__vertex_fZ[kMax_jets];   //[_jets_]
   Int_t           _jets__flavor[kMax_jets];   //[_jets_]
   Int_t           _muons_;
   UInt_t          _muons_fUniqueID[kMax_muons];   //[_muons_]
   UInt_t          _muons_fBits[kMax_muons];   //[_muons_]
   UInt_t          _muons__p4_fUniqueID[kMax_muons];   //[_muons_]
   UInt_t          _muons__p4_fBits[kMax_muons];   //[_muons_]
   UInt_t          _muons__p4_fP_fUniqueID[kMax_muons];   //[_muons_]
   UInt_t          _muons__p4_fP_fBits[kMax_muons];   //[_muons_]
   Double_t        _muons__p4_fP_fX[kMax_muons];   //[_muons_]
   Double_t        _muons__p4_fP_fY[kMax_muons];   //[_muons_]
   Double_t        _muons__p4_fP_fZ[kMax_muons];   //[_muons_]
   Double_t        _muons__p4_fE[kMax_muons];   //[_muons_]
   UInt_t          _muons__vertex_fUniqueID[kMax_muons];   //[_muons_]
   UInt_t          _muons__vertex_fBits[kMax_muons];   //[_muons_]
   Double_t        _muons__vertex_fX[kMax_muons];   //[_muons_]
   Double_t        _muons__vertex_fY[kMax_muons];   //[_muons_]
   Double_t        _muons__vertex_fZ[kMax_muons];   //[_muons_]

   // List of branches
   TBranch        *b_event_fUniqueID;   //!
   TBranch        *b_event_fBits;   //!
   TBranch        *b_event__jets_;   //!
   TBranch        *b__jets_fUniqueID;   //!
   TBranch        *b__jets_fBits;   //!
   TBranch        *b__jets__p4_fUniqueID;   //!
   TBranch        *b__jets__p4_fBits;   //!
   TBranch        *b__jets__p4_fP_fUniqueID;   //!
   TBranch        *b__jets__p4_fP_fBits;   //!
   TBranch        *b__jets__p4_fP_fX;   //!
   TBranch        *b__jets__p4_fP_fY;   //!
   TBranch        *b__jets__p4_fP_fZ;   //!
   TBranch        *b__jets__p4_fE;   //!
   TBranch        *b__jets__vertex_fUniqueID;   //!
   TBranch        *b__jets__vertex_fBits;   //!
   TBranch        *b__jets__vertex_fX;   //!
   TBranch        *b__jets__vertex_fY;   //!
   TBranch        *b__jets__vertex_fZ;   //!
   TBranch        *b__jets__flavor;   //!
   TBranch        *b_event__muons_;   //!
   TBranch        *b__muons_fUniqueID;   //!
   TBranch        *b__muons_fBits;   //!
   TBranch        *b__muons__p4_fUniqueID;   //!
   TBranch        *b__muons__p4_fBits;   //!
   TBranch        *b__muons__p4_fP_fUniqueID;   //!
   TBranch        *b__muons__p4_fP_fBits;   //!
   TBranch        *b__muons__p4_fP_fX;   //!
   TBranch        *b__muons__p4_fP_fY;   //!
   TBranch        *b__muons__p4_fP_fZ;   //!
   TBranch        *b__muons__p4_fE;   //!
   TBranch        *b__muons__vertex_fUniqueID;   //!
   TBranch        *b__muons__vertex_fBits;   //!
   TBranch        *b__muons__vertex_fX;   //!
   TBranch        *b__muons__vertex_fY;   //!
   TBranch        *b__muons__vertex_fZ;   //!

   ReadSelSplit(TTree * /*tree*/ =0) { fReadAll = kTRUE; }
   virtual ~ReadSelSplit() { }
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

   ClassDef(ReadSelSplit,0);
};

#endif

#ifdef ReadSelSplit_cxx
void ReadSelSplit::Init(TTree *tree)
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

   fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_event_fUniqueID);
   fChain->SetBranchAddress("fBits", &fBits, &b_event_fBits);
   fChain->SetBranchAddress("_jets", &_jets_, &b_event__jets_);
   fChain->SetBranchAddress("_jets.fUniqueID", _jets_fUniqueID, &b__jets_fUniqueID);
   fChain->SetBranchAddress("_jets.fBits", _jets_fBits, &b__jets_fBits);
   fChain->SetBranchAddress("_jets._p4.fUniqueID", _jets__p4_fUniqueID, &b__jets__p4_fUniqueID);
   fChain->SetBranchAddress("_jets._p4.fBits", _jets__p4_fBits, &b__jets__p4_fBits);
   fChain->SetBranchAddress("_jets._p4.fP.fUniqueID", _jets__p4_fP_fUniqueID, &b__jets__p4_fP_fUniqueID);
   fChain->SetBranchAddress("_jets._p4.fP.fBits", _jets__p4_fP_fBits, &b__jets__p4_fP_fBits);
   fChain->SetBranchAddress("_jets._p4.fP.fX", _jets__p4_fP_fX, &b__jets__p4_fP_fX);
   fChain->SetBranchAddress("_jets._p4.fP.fY", _jets__p4_fP_fY, &b__jets__p4_fP_fY);
   fChain->SetBranchAddress("_jets._p4.fP.fZ", _jets__p4_fP_fZ, &b__jets__p4_fP_fZ);
   fChain->SetBranchAddress("_jets._p4.fE", _jets__p4_fE, &b__jets__p4_fE);
   fChain->SetBranchAddress("_jets._vertex.fUniqueID", _jets__vertex_fUniqueID, &b__jets__vertex_fUniqueID);
   fChain->SetBranchAddress("_jets._vertex.fBits", _jets__vertex_fBits, &b__jets__vertex_fBits);
   fChain->SetBranchAddress("_jets._vertex.fX", _jets__vertex_fX, &b__jets__vertex_fX);
   fChain->SetBranchAddress("_jets._vertex.fY", _jets__vertex_fY, &b__jets__vertex_fY);
   fChain->SetBranchAddress("_jets._vertex.fZ", _jets__vertex_fZ, &b__jets__vertex_fZ);
   fChain->SetBranchAddress("_jets._flavor", _jets__flavor, &b__jets__flavor);
   fChain->SetBranchAddress("_muons", &_muons_, &b_event__muons_);
   fChain->SetBranchAddress("_muons.fUniqueID", _muons_fUniqueID, &b__muons_fUniqueID);
   fChain->SetBranchAddress("_muons.fBits", _muons_fBits, &b__muons_fBits);
   fChain->SetBranchAddress("_muons._p4.fUniqueID", _muons__p4_fUniqueID, &b__muons__p4_fUniqueID);
   fChain->SetBranchAddress("_muons._p4.fBits", _muons__p4_fBits, &b__muons__p4_fBits);
   fChain->SetBranchAddress("_muons._p4.fP.fUniqueID", _muons__p4_fP_fUniqueID, &b__muons__p4_fP_fUniqueID);
   fChain->SetBranchAddress("_muons._p4.fP.fBits", _muons__p4_fP_fBits, &b__muons__p4_fP_fBits);
   fChain->SetBranchAddress("_muons._p4.fP.fX", _muons__p4_fP_fX, &b__muons__p4_fP_fX);
   fChain->SetBranchAddress("_muons._p4.fP.fY", _muons__p4_fP_fY, &b__muons__p4_fP_fY);
   fChain->SetBranchAddress("_muons._p4.fP.fZ", _muons__p4_fP_fZ, &b__muons__p4_fP_fZ);
   fChain->SetBranchAddress("_muons._p4.fE", _muons__p4_fE, &b__muons__p4_fE);
   fChain->SetBranchAddress("_muons._vertex.fUniqueID", _muons__vertex_fUniqueID, &b__muons__vertex_fUniqueID);
   fChain->SetBranchAddress("_muons._vertex.fBits", _muons__vertex_fBits, &b__muons__vertex_fBits);
   fChain->SetBranchAddress("_muons._vertex.fX", _muons__vertex_fX, &b__muons__vertex_fX);
   fChain->SetBranchAddress("_muons._vertex.fY", _muons__vertex_fY, &b__muons__vertex_fY);
   fChain->SetBranchAddress("_muons._vertex.fZ", _muons__vertex_fZ, &b__muons__vertex_fZ);
}

Bool_t ReadSelSplit::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.
   Info("Notify","processing file: %s", fChain->GetCurrentFile()->GetName());

   return kTRUE;
}

#endif // #ifdef ReadSelSplit_cxx
