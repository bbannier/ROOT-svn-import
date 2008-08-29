#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom.h"
#include "TClonesArray.h"
#include "myparticle.h"
#include "TH1F.h"

void read_tree2(const char *what = "muons", Bool_t disable_branches = kFALSE, Bool_t use_tree_draw = kFALSE)
{
   // function reading particles collection in a tree

   // open the Root file
   TFile *f = new TFile("http://server/exercise3.root");
   // get the tree from the file
   TTree *myTree = (TTree*)f->Get("MyTree");
   if (disable_branches) {
      // disable all branches 
      myTree->SetBranchStatus("*", 0);
      // enable only the branch we are interested to read
      myTree->SetBranchStatus(Form("%s*",what), 1);
   }
   // initialize Root's timing mechanism
   gROOT->Time();

   if (use_tree_draw) {
      // use TTree::Draw() mechanism
      myTree->Draw(Form("%s.fPt", what), "");
   }
   else {
      // declare a pointer on a TClonesArray 
      // and initialize it to 0
      TClonesArray *particles = 0;
      // assign the branch to the TClonesArray
      myTree->SetBranchAddress(what, &particles);
      // create the 1D histogram of floats 
      // (100 bins, between 0.0 and 10.0)
      TH1F *h1 = new TH1F("h1", Form("%s Pt", what), 100, 0.0, 10.0);
      // get number of entries in the tree
      Long64_t nentries = myTree->GetEntries();
      // loop over all entries
      for (Long64_t ev=0;ev<nentries;ev++) {
         // get tree entry[ev]
         myTree->GetEntry(ev);
         // get number of particle in the TClonesArray
         Int_t npart = particles->GetEntriesFast();
         // loop over all particles
         for (Int_t i=0;i<npart;i++) {
            // get the ith particle
            MyParticle *particle = (MyParticle*)particles->At(i);
            // fill the histogram with particle's Pt value
            h1->Fill(particle->fPt);
         }
      }
      // draw (display) the histogram
      h1->Draw();
   }
   // display the time elapsed since the first call to gROOT->Time()
   gROOT->Time();
}

