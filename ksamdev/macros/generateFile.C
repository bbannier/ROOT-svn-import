
#include "TFile.h"
#include "TTree.h"
#include "TRandom3.h"
#include "TProof.h"
#include "RTEvent.h"
#include "TParameter.h"
#include "stdio.h"

Int_t generateFile(const char *fn, Long64_t nevts = 1000, Int_t comp_level = 1,
                   Int_t split_level = 99, Int_t njets = 10, Int_t nmuons = 5)
{
   // Generate a file with a TTree containing 'nevts' RTEvents, each with average number of jets
   // 'njets', and average number of muons 'nmuons'

   // Open the file
   TFile *fout = TFile::Open(fn, "RECREATE");
   if (!fout || fout->IsZombie()) {
      if (fout) delete fout;
      return -1;
   }

   // Set the compression level
   fout->SetCompressionLevel(comp_level);

   // Create the TTree
   TTree *tree = new TTree("rt", "ROOT Tree");
   tree->SetDirectory(fout);

   RTEvent *event = new RTEvent(njets, nmuons);
   RTEvent *ep = event;
   
   tree->Branch("event", "RTEvent", &ep, 32000, split_level);
   tree->AutoSave();
   // This forces at least one flush for files smaller than the default cache (30 MB).
   // Otherwise the TTreeCache mechanism does not work well in reading
   if (nevts < 20000) tree->SetAutoFlush(5000000);
   
   // Generate the events
   for (Long64_t i = 0 ; i < nevts; i++) {

        // Generate Jets
        for (Int_t j = 0; njets > j; ++j) {
           RTJet *jet = event->add_jets();
           TLorentzVector *p4 = jet->mutable_p4();

           p4->SetE(gRandom->Gaus(172, 10));
           p4->SetPx(gRandom->Gaus(45, 5));
           p4->SetPy(gRandom->Gaus(45, 5));
           p4->SetPz(gRandom->Gaus(45, 5));

           TVector3 *vertex = jet->mutable_vertex();
           
           vertex->SetX(gRandom->Gaus(6, 1));
           vertex->SetY(gRandom->Gaus(7, 1));
           vertex->SetZ(gRandom->Gaus(8, 1));

           RTJet::Flavor flavour = ((RTJet::Flavor) (Int_t) gRandom->Uniform(RTJet::UNKNOWN,RTJet::T));
           jet->setFlavor(flavour);

        }

        // Generate Muons
        //
        for(Int_t m = 0; nmuons > m; ++m)
        {
           RTLepton *muon = event->add_muons();

           TLorentzVector *p4 = muon->mutable_p4();

            p4->SetE(gRandom->Gaus(200,10));
            p4->SetPx(gRandom->Gaus(100,10));
            p4->SetPy(gRandom->Gaus(100,10));
            p4->SetPz(gRandom->Gaus(100,10));

            TVector3 *vertex = muon->mutable_vertex();

            vertex->SetX(gRandom->Gaus(1, 1));
            vertex->SetY(gRandom->Gaus(2, 1));
            vertex->SetZ(gRandom->Gaus(3, 1));
        }

        tree->Fill();
        event->Clear();
   }
   
   fout->cd();
   tree->Write();
   tree->SetDirectory(0);

   fout->Close();
   delete fout;
   fout = 0;
   tree->Delete();
   event->Delete();
  
   return 0;
}

