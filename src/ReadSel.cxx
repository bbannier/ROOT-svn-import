#define ReadSel_cxx
// The class definition in ReadSel.h has been generated automatically
// by the ROOT utility TTree::MakeSelector(). This class is derived
// from the ROOT class TSelector. For more information on the TSelector
// framework see $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.

// The following methods are defined in this file:
//    Begin():        called every time a loop on the tree starts,
//                    a convenient place to create your histograms.
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//                    called only on the slave servers.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//
// To use this file, try the following session on your Tree T:
//
// Root > T->Process("ReadSel.C")
// Root > T->Process("ReadSel.C","some options")
// Root > T->Process("ReadSel.C+")
//

#include "ReadSel.h"
#include <TH2.h>
#include <TStyle.h>


void ReadSel::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

}

void ReadSel::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();
    
    _jets = new TH1D("jets", "Number of jets", 20, 0, 20);
    _jet_flavor = new TH1D("jet_flavor", "Flavor of jets", 20, 0, 20);
    _jet_e = new TH1D("jet_e", "Jets e", 10, 0, 10);
    _jet_px = new TH1D("jet_px", "Jets px", 50, 50, 150);
    _jet_py = new TH1D("jet_py", "Jets py", 50, 50, 150);
    _jet_pz = new TH1D("jet_pz", "Jets pz", 50, 50, 150);
    _jet_x = new TH1D("jet_x", "Jets x", 20, -10, 10);
    _jet_y = new TH1D("jet_y", "Jets y", 20, -10, 10);
    _jet_z = new TH1D("jet_z", "Jets z", 20, -10, 10);
    
    fOutput->Add(_jets);
    fOutput->Add(_jet_flavor);
    fOutput->Add(_jet_e);
    fOutput->Add(_jet_px);
    fOutput->Add(_jet_py);
    fOutput->Add(_jet_pz);
    fOutput->Add(_jet_x);
    fOutput->Add(_jet_y);
    fOutput->Add(_jet_z);

    _muons = new TH1D("muons", "Number of muons", 20, 0, 20);
    _muon_e = new TH1D("muon_e", "Muons e", 50, 50, 150);
    _muon_px = new TH1D("muon_px", "Muons px", 50, 0, 100);
    _muon_py = new TH1D("muon_py", "Muons py", 50, 0, 100);
    _muon_pz = new TH1D("muon_pz", "Muons pz", 50, 0, 100);
    _muon_x = new TH1D("muon_x", "Muons x", 20, 0, 20);
    _muon_y = new TH1D("muon_y", "Muons y", 20, 0, 20);
    _muon_z = new TH1D("muon_z", "Muons z", 20, 0, 20);
    
    fOutput->Add(_muons);
    fOutput->Add(_muon_e);
    fOutput->Add(_muon_px);
    fOutput->Add(_muon_py);
    fOutput->Add(_muon_pz);
    fOutput->Add(_muon_x);
    fOutput->Add(_muon_y);
    fOutput->Add(_muon_z);
}

Bool_t ReadSel::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either ReadSel::GetEntry() or TBranch::GetEntry()
   // to read either all or the required parts of the data. When processing
   // keyed objects with PROOF, the object is already loaded and is available
   // via the fObject pointer.
   //
   // This function should contain the "body" of the analysis. It can contain
   // simple or elaborate selection criteria, run algorithms on the data
   // of the event and typically fill histograms.
   //
   // The processing can be stopped by calling Abort().
   //
   // Use fStatus to set the return value of TTree::Process().
   //
   // The return value is currently not used.

   GetEntry(entry); 

   TClonesArray *jets = (TClonesArray *) event->jets();
   if (jets) {
      Int_t njets = jets->GetSize();
      _jets->Fill(njets);
      for (Int_t i = 0; i < njets; i++) {
	RTJet *jet = (RTJet *) jets->At(i);
         if (jet) {
  	     _jet_flavor->Fill(jet->flavor());
	    _jet_e->Fill(jet->p4().E());
	    _jet_px->Fill(jet->p4().Vect().X());
	    _jet_py->Fill(jet->p4().Vect().Y());
	    _jet_pz->Fill(jet->p4().Vect().Z()); 
	    _jet_x->Fill(jet->vertex().X());
	    _jet_y->Fill(jet->vertex().Y());
	    _jet_z->Fill(jet->vertex().Z());
         }
      }
   }

   TClonesArray *muons = (TClonesArray *) event->muons();
   if (muons) {
      Int_t nmuons = muons->GetSize();
      _muons->Fill(nmuons);
      for (Int_t i = 0; i < nmuons; i++) {
	RTLepton *muon = (RTLepton *) muons->At(i);
         if (muon) {
	    _muon_e->Fill(muon->p4().E());
	    _muon_px->Fill(muon->p4().Vect().X());
	    _muon_py->Fill(muon->p4().Vect().Y());
	    _muon_pz->Fill(muon->p4().Vect().Z()); 
	    _muon_x->Fill(muon->vertex().X());
	    _muon_y->Fill(muon->vertex().Y());
	    _muon_z->Fill(muon->vertex().Z());
         }
      }
   }

   return kTRUE;
}

void ReadSel::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

void ReadSel::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

   Printf("_jets entries: %f", _jets->GetEntries());

}
