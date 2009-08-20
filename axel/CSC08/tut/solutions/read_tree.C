#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom.h"
#include "TClonesArray.h"
#include "myparticle.h"
#include "TH1F.h"

void read_tree()
{
   // function reading particles collection in a tree

   // open the Root file
   TFile *f = TFile::Open("http://server/ROOT/exercise2.root");
   if (!f || f->IsZombie()) {
      printf("Problem opening http://server/ROOT/exercise2.root \n");
      return;
   }
   // get the tree from the file
   TTree *myTree = (TTree*)f->Get("MyTree");
   // disable all branches 
   myTree->SetBranchStatus("*", 0);
   // enable only the branch we are interested to read
   myTree->SetBranchStatus("muons*", 1);
   // declare a pointer on a TClonesArray 
   // and initialize it to 0
   TClonesArray *particles = 0;
   // assign the branch to the TClonesArray
   myTree->SetBranchAddress("muons", &particles);
   
   // initialize Root's timing mechanism
   gROOT->Time();
   
   // create the 1D histogram of floats 
   // (100 bins, between 0.0 and 10.0)
   TH1F *h1 = new TH1F("h1", "Pt", 100, 0.0, 10.0);
   // get number of entries in the tree
   Long64_t nentries = myTree->GetEntries();
   // loop over all entries
   for (Long64_t ev=0;ev<nentries;ev++) {
      // get tree entry[ev]
      myTree->GetEntry(ev);
      // get number of particle in the TClonesArray
      Int_t npart = particles->GetEntriesFast();
      // loop over all particles
      for (Int_t i=0;i<npart;++i) {
         // get the ith particle
         MyParticle *particle = (MyParticle *)particles->At(i);
         // fill the histogram with particle's energy value
         h1->Fill(particle->fPt);
      }
   }
   // draw (display) the histogram
   h1->Draw();
   // display the time elapsed since the first call to gROOT->Time()
   gROOT->Time();
}

