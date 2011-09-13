#define ReadSelSplit_cxx
// The class definition in TestSelSpli.h has been generated automatically
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
// Root > T->Process("ReadSelSplit.C")
// Root > T->Process("ReadSelSplit.C","some options")
// Root > T->Process("ReadSelSplit.C+")
//

#include "ReadSelSplit.h"
#include <TH1D.h>
#include <TParameter.h>
#include <TNamed.h>
#include <TStyle.h>

void ReadSelSplit::HFill(TH1 *h, Double_t x)
{
   // Optimized histogram filling function

   if (!h) {
      Warning("HFill", "histo undefined! Cannot fill for '%f'", x);
      return;
   }
   if (fStdHFill) {
      // Standard fill required
      h->Fill(x);
      return;
   }

   // Get the bin:  (x-xmi) * nbin / (xmx-xmi)
   Double_t xmi = h->GetXaxis()->GetXmin();
   Double_t xma = h->GetXaxis()->GetXmax();
   if (xma <= xmi) {
      // Axis still to be defined: standard fill (in the buffer) for now
      h->Fill(x);
      return;
   }
   Double_t nbi = h->GetXaxis()->GetNbins();
   Int_t bin = Int_t(((x - xmi) * nbi) / (xma - xmi));
   if (bin > -1) h->AddBinContent(bin);
   
   // Done
   return;
}

TObject *ReadSelSplit::Find(const char *n)
{
   // Find object in fInput or gDirectory

   return ((fInput) ? fInput->FindObject(n) : gDirectory->FindObject(n));
}

void ReadSelSplit::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();
/*
 #if 0
   TParameter<Int_t> *ropt = (TParameter<Int_t> *) Find("RTReadOpt");
   if (ropt && ropt->GetVal() == 0) fReadAll = kFALSE;
   Info("Begin", " readall: %d", fReadAll);
#endif
 */

#if 0
	TNamed *nm = (TNamed *) Find("ReadOpt");
	if (nm && !strcmp(nm->GetName(),"")) fReadAll = kFALSE;
	Info("SlaveBegin", " readall: %d", fReadAll);
#endif
}

void ReadSelSplit::SlaveBegin(TTree * /*tree*/)
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

    
   /* TParameter<Int_t> *ropt = (TParameter<Int_t> *) Find("RTReadOpt");
    if (ropt && ropt->GetVal() == 0) fReadAll = kFALSE;
    Info("SlaveBegin", " readall: %d", fReadAll);
   */

    TNamed *nm = (TNamed *) Find("ReadOpt");
    if (nm && !strcmp(nm->GetName(),"")) fReadAll = kFALSE;
    Info("SlaveBegin", " readall: %d", fReadAll);

    nm = (TNamed *) Find("StdHFill");
    if (nm && !strcmp(nm->GetName(),"1")) fStdHFill = kTRUE;
    Info("SlaveBegin", " stdHfill: %d", fStdHFill);
}

Bool_t ReadSelSplit::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either ReadSelSplit::GetEntry() or TBranch::GetEntry()
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

   if (fReadAll) {
      fChain->GetTree()->GetEntry(entry); 
//    Info("Process","entry %lld done ...", entry);
   } else {
      b_event__jets_->GetEntry(entry);
      b__jets__p4_fP_fX->GetEntry(entry);
      b__jets__p4_fP_fY->GetEntry(entry);
      b__jets__p4_fP_fZ->GetEntry(entry);
      b__jets__vertex_fX->GetEntry(entry);
      b__jets__vertex_fY->GetEntry(entry);
      b__jets__vertex_fZ->GetEntry(entry);
      b__jets__flavor->GetEntry(entry);
      b_event__muons_->GetEntry(entry);
      b__muons__p4_fP_fX->GetEntry(entry);
      b__muons__p4_fP_fY->GetEntry(entry);
      b__muons__p4_fP_fZ->GetEntry(entry);
      b__muons__vertex_fX->GetEntry(entry);
      b__muons__vertex_fY->GetEntry(entry);
      b__muons__vertex_fZ->GetEntry(entry);
   }

   _jets->Fill(_jets_);
   for (Int_t i=0;i < _jets_; i++) {
      HFill(_jet_flavor, _jets__flavor[i]);
      HFill(_jet_e, _jets__p4_fE[i]);
      HFill(_jet_px, _jets__p4_fP_fX[i]);
      HFill(_jet_py, _jets__p4_fP_fY[i]);
      HFill(_jet_pz, _jets__p4_fP_fZ[i]);
      HFill(_jet_x, _jets__vertex_fX[i]);
      HFill(_jet_y, _jets__vertex_fY[i]);
      HFill(_jet_z, _jets__vertex_fZ[i]);

   }
   _muons->Fill(_muons_);
   for (Int_t i=0;i < _muons_; i++) {
      HFill(_muon_e, _muons__p4_fE[i]);
      HFill(_muon_px, _muons__p4_fP_fX[i]);
      HFill(_muon_py, _muons__p4_fP_fY[i]);
      HFill(_muon_pz, _muons__p4_fP_fZ[i]);
      HFill(_muon_x, _muons__vertex_fX[i]);
      HFill(_muon_y, _muons__vertex_fY[i]);
      HFill(_muon_z, _muons__vertex_fZ[i]);
   }

   return kTRUE;
}

void ReadSelSplit::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

void ReadSelSplit::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

   Printf("_jets entries: %f", _jets->GetEntries());


}
