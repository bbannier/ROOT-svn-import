//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Aug 25 14:34:20 2008 by ROOT version 5.21/01
// from TTree TBeamSpotSel/beam position
// found on file: TBeamSpotSel.root
//////////////////////////////////////////////////////////

#ifndef TBeamSpotSel_h
#define TBeamSpotSel_h

#include "TTree.h"
#include "TSelector.h"
#include "TDatime.h"
#include "TVector3.h"

class TH1;

class Pos3D {
public:
   Pos3D(Double_t x = 0., Double_t y = 0., Double_t z = 0.):
      fX(x), fY(y), fZ(z) {}

   void Set(Double_t x, Double_t y, Double_t z) {
      fX = x; fY = y; fZ = z;
   }

   Double_t X() const { return fX;}
   Double_t Y() const { return fY;}
   Double_t Z() const { return fZ;}

private:
   Double32_t fX, fY, fZ;
   ClassDef(Pos3D, 1); // position in 3D
};

class Spot {
public:
   Spot(): fMuSec(0) {}

   const Pos3D&   PrimaryVertex() const { return fVtx0; }
   const TDatime& Time() const { return fTime; }
         Float_t  MuSec() const { return fMuSec; }

   void SetPos(const Pos3D& pos) { fVtx0 = pos; }
   void SetTime(const TDatime& time) { fTime = time; }
   void SetMuSec(Float_t& musec) { fMuSec = musec; }

private:
   Pos3D    fVtx0; // primary vertex position
   TDatime  fTime; // time stamp of event (date + time up to seconds)
   Float_t  fMuSec; // micro seconds of fTime
};


class TBeamSpotSel : public TSelector {
private :
   TTree*   fChain;   //!pointer to the analyzed TTree or TChain
   Spot*    fEventInfo; // will contain the data stored in the tree
   TBranch* fBranchEventInfo;   //!
   TH1*     fBeamSpotX[2]; // vtx0.x before and after the detector move
   TH1*     fBeamSpotY[2]; // vtx0.y before and after the detector move
   TH1*     fBeamSpotZ[2]; // vtx0.z before and after the detector move

public:
   TBeamSpotSel(TTree * /*tree*/ =0):
      fChain(0), fEventInfo(0), fBranchEventInfo(0),
      fBeamSpotX(), fBeamSpotY(), fBeamSpotZ()
   { }
   virtual ~TBeamSpotSel() { }
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

   ClassDef(TBeamSpotSel,0);
};

#endif

#ifdef TBeamSpotSel_cxx
void TBeamSpotSel::Init(TTree *tree)
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
   fChain->SetBranchAddress("eventinfo", &fEventInfo, &fBranchEventInfo);
}

Bool_t TBeamSpotSel::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

#endif // #ifdef TBeamSpotSel_cxx
