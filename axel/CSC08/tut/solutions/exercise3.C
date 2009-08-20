#include "TFile.h"
#include "TTree.h"
#include "TRandom.h"
#include "TClonesArray.h"
#include "myparticle.h"
#include "exercise3.h"

Float_t MyLandau(Double_t mean = 0, Double_t sigma = 1)
{
   Float_t land = 1.0e9;
   while (land > 3.0 * mean) {
      land = gRandom->Landau(mean, sigma);
   }
   return land;
   
}

void exercise3(int type)
{
   Int_t i, j;
   gRandom->SetSeed();
   // first create a file
   Int_t nevents = 1000;
   Int_t split = type != 2 ? 99 : 0;
   bool chain = (type == 3);
   const char *name = "exercise2.root";
   if (type == 2) name = "exercise2_nonsplit.root";
   if (type == 3) name = "exercise3.root";
   TFile* f = new TFile(name, "recreate");
   // create a tree
   TTree *MyTree = new TTree("MyTree","MyEvent Tree");
   if (chain)
	  MyTree->SetMaxTreeSize(50000000);
   // create clone arrays
   TClonesArray *electrons = new TClonesArray("Electron");
   TClonesArray &el = *electrons;

   TClonesArray *muons = new TClonesArray("Muon");
   TClonesArray &mu = *muons;

   TClonesArray *pions = new TClonesArray("Pion");
   TClonesArray &pi = *pions;
   // create a branch holding the vector
   MyTree->Branch("electrons", &electrons, 32000, split);
   // create a branch holding the clones array
   MyTree->Branch("muons", &muons, 32000, split);
   MyTree->Branch("pions", &pions, 32000, split);
   for (i=0;i<nevents;i++) {
      el.Clear();
      mu.Clear();
      pi.Clear();
      // generate a random number of particles
      Int_t npts = Int_t(gRandom->Gaus(1000,200));
      if(npts < 0) npts = 1;
      for (j=0;j<npts;j++) {
         Float_t x = gRandom->Gaus(1.2);
         Float_t y = gRandom->Gaus(0.9);
         Float_t z = gRandom->Gaus();
         Float_t e_e = MyLandau(510,25.0);
         Float_t e_u = MyLandau(1056.6,10.0);
         Float_t e_p = MyLandau(1349.8,13.0);
         Float_t p_e = MyLandau(1.0, 0.1);
         Float_t p_u = MyLandau(2.0, 0.2);
         Float_t p_p = MyLandau(2.7, 0.27);
         // create a new particle in the vector
         Electron *e = new(el[j]) Electron(x, y, z, e_e, p_e);
         Muon     *u = new(mu[j]) Muon(x, y, z, e_u, p_u);
         Pion     *p = new(pi[j]) Pion(x, y, z, e_p, p_p);
      }
      // fill the tree
      MyTree->Fill();
   }
   // print the content of the tree
   MyTree->Print();
   // write it to the file
   MyTree->Write(MyTree->GetName(), TObject::kOverwrite);
   //delete MyTree->GetCurrentFile();
}

