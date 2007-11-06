#define PythiaMC_cxx
// The class definition in PythiaMC.h has been generated automatically
// by the ROOT utility TTree::MakeSelector(). This class is derived
// from the ROOT class TSelector. For more information on the TSelector
// framework see $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.

// The following methods are defined in this file:
//    Begin():        called everytime a loop on the tree starts,
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
// Root > T->Process("PythiaMC.C")
// Root > T->Process("PythiaMC.C","some options")
// Root > T->Process("PythiaMC.C+")
//

#include "PythiaMC.h"

#include <TCanvas.h>
#include <TClonesArray.h>
#include <TFile.h>
#include <TH1F.h>
#include <TList.h>
#include <TMath.h>
#include <TMCParticle.h>
#include <TProofFile.h>
#include <TPythia6.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TTree.h>

#define FILENAME   "PythiaMC.root"
#define PDGPIPNUM  211


TDirectory* gSavedir = 0;

PythiaMC::~PythiaMC()
{
   // Destructor
   if (fH1) delete fH1;
   if (fPythia) delete fPythia;
   if (fFile) delete fFile;
   if (fProofFile) delete fProofFile;
   if (fTree) delete fTree;
}

void PythiaMC::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

}

void PythiaMC::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

   fH1 = new TH1F("h1", "p_{#perp}  spectrum for  #pi^{+}", 100, 0, 3);
   fH1->SetXTitle("p_{#perp}");
   fH1->SetYTitle("dN/dp_{#perp}");
   fOutput->Add(fH1);

   // Create an instance of the Pythia event generator ...
   fPythia = new TPythia6();

   // ... and initialise it to run p+p at sqrt(14) TeV in CMS
   fPythia->Initialize("cms", "p", "p", 14000);

   // File for the big output objects
   // Check if an output URL has been given
   TString outf(FILENAME);
   TNamed *out = (TNamed *) fInput->FindObject("PROOF_OUTPUTFILE");
   if (out) {
//      fProofFile = new TProofFile(gSystem->BaseName(TUrl(out->GetTitle()).GetFile()),
//                                  "LOCAL", "SEQUENTIAL");
      fProofFile = new TProofFile(gSystem->BaseName(TUrl(out->GetTitle()).GetFile()));
      fProofFile->SetOutputFileName(out->GetTitle());
   } else {
//      fProofFile = new TProofFile(FILENAME, "LOCAL", "SEQUENTIAL");
      fProofFile = new TProofFile(FILENAME);
   }
   fOutput->Add(fProofFile);

   gSavedir = gDirectory;
   if (!(fFile = fProofFile->OpenFile("RECREATE"))) {
      Warning("SlaveBegin", "problems opening file: %s/%s",
                            fProofFile->GetDir(), fProofFile->GetFileName());
   }

   gSavedir->cd();

   // Make a tree in that file ...
   fTree = new TTree("PythiaMC", "Pythia 6 pp@14 TeV");
   fTree->SetDirectory(fFile);

   // ... and register a the cache of pythia on a branch (It's a
   // TClonesArray of TMCParticle objects. )
   fTree->Branch("fNParticles", &fNParticles, "fNParticles/I");
//   fParticles = new TClonesArray("TParticles");
   fParticles = (TClonesArray *) fPythia->GetListOfParticles();
   fTree->Branch("fParticles", &fParticles);
   fTree->AutoSave();
}

Bool_t PythiaMC::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either PythiaMC::GetEntry() or TBranch::GetEntry()
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

   fPythia->GenerateEvent();
   fParticles = (TClonesArray *) fPythia->ImportParticles();
   fNParticles = fPythia->GetNumberOfParticles();
   fTree->Fill();

   TMCParticle* particle = 0;
   Int_t n = fPythia->GetNumberOfParticles();
   for (Int_t i = 0; i < n; i++) {
      particle = (TMCParticle *)fPythia->GetParticle(i);
      if (particle->GetKF() == PDGPIPNUM )
         // Fill pi+ Pt
         fH1->Fill(TMath::Sqrt(TMath::Power(particle->GetPx(),2) +
                               TMath::Power(particle->GetPy(),2)));
   }

   if (!(entry%100))
      Info("Process","%lld event done", entry);

   return kTRUE;
}

void PythiaMC::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

   if (fFile) {
      gSavedir = gDirectory;
      fFile->cd();
      fTree->Write();
      fTree->SetDirectory(0);
      gDirectory = gSavedir;
      fFile->Close();
   }

   fProofFile->Print();
}

void PythiaMC::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

   if ((fH1 = dynamic_cast<TH1F*>(fOutput->FindObject("h1"))) == 0) {
      Error("Terminate", "Pt histo not found");
      return;
   }
   TCanvas *c = new TCanvas("canvas","Pythia Example",10,10,800,600);
   c->cd(1);
   fH1->DrawCopy();
//   fH1->Fit("expo", "QO+", "", .25, 1.75);

#if 0
   TIter nxo(fOutput);
   TObject *o = 0;
   while ((o = nxo())) {
      if ((fProofFile = dynamic_cast<TProofFile*>(o)))
         break;
   }
   if (fProofFile == 0) {
      Error("Terminate", "Output file info not found");
      return;
   }

   TString outputFile(fProofFile->GetOutputFileName());
   Printf("outputFile: %s", outputFile.Data());
#endif
}
