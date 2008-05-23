#define procRate_cxx
// The class definition in procRate.h has been generated automatically
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
// Root > T->Process("procRate.C")
// Root > T->Process("procRate.C","some options")
// Root > T->Process("procRate.C+")
//

#include "procRate.h"
#include <TH2.h>
#include <TStyle.h>

#include "TSystem.h"
#include "TFile.h"
#include "TLegend.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TFrame.h"
#include "TROOT.h"
#include "TColor.h"
#include <math.h>

void procRate::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

}

void procRate::SlaveBegin(TTree *tree)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

//   TString option = GetOption();
   Init(tree);
}

Bool_t procRate::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either procRate::GetEntry() or TBranch::GetEntry()
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
   if (fChain) {
      fChain->GetTree()->GetEntry(entry);
      if (PerfEvents->fType == TVirtualPerfStats::kPacket) {
         for(int i = 0; i < round(PerfEvents->fProcTime); i++) {
            // We hope that in case fProcTime == 0.5 the loop is executed
            // once.
            y1[lround(PerfEvents->fTimeStamp.GetSec()
                     + PerfEvents->fTimeStamp.GetNanoSec() * 1e-9 + i)] +=
               (PerfEvents->fEventsProcessed / PerfEvents->fProcTime);
            if (PerfEvents->fTimeStamp.GetSec() + i > fMaxTime)
               fMaxTime = PerfEvents->fTimeStamp.GetSec() + i;
         }
      }
   }

   return kTRUE;
}

void procRate::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

void procRate::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

   gROOT->Reset();
   TCanvas* c1 = new TCanvas("c1", "procRate");
   c1->SetGrid();

   TGraph* gr1 = new TGraph(fMaxTime + 1, x1, y1);
   gr1->Draw("ACP");
   gr1->SetTitle("Processing rate in a job");
   gr1->GetXaxis()->SetTitle("Time [s]");
   gr1->GetYaxis()->SetTitle("Processing Rate [#events/sec]");

   c1->Update();
   c1->Modified();
}

