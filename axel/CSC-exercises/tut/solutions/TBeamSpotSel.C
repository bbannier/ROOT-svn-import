#define TBeamSpotSel_cxx
// The class definition in TBeamSpotSel.h has been generated automatically
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
// Root > T->Process("TBeamSpotSel.C")
// Root > T->Process("TBeamSpotSel.C","some options")
// Root > T->Process("TBeamSpotSel.C+")
//

#include "TBeamSpotSel.h"
#include "TH3.h"
#include "TH2.h"
#include "TH1.h"
#include "TCanvas.h"


void TBeamSpotSel::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

   Info("SlaveBegin", "called!");
}

void TBeamSpotSel::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   Info("SlaveBegin", "called!");

   Double_t r = 5.; // the size of the histograms: min / max

   fBeamSpotX[0]
      = new TH1F("hBeamSpotX0",
                 "Beam position vs. time;X [cm];Entries",
                 100, -r, r);
   fBeamSpotX[1]
      = new TH1F("hBeamSpotX1",
                 "Beam position vs. time;X [cm];Entries",
                 100, -r, r);

   fBeamSpotY[0]
      = new TH1F("hBeamSpotY0",
                 "Beam position vs. time;Y [cm];Entries",
                 100, -r, r);
   fBeamSpotY[1]
      = new TH1F("hBeamSpotY1",
                 "Beam position vs. time;Y [cm];Entries",
                 100, -r, r);

   fBeamSpotZ[0]
      = new TH1F("hBeamSpotZ0",
                 "Beam position vs. time;Z [cm];Entries",
                 100, -r, r);
   fBeamSpotZ[1]
      = new TH1F("hBeamSpotZ1",
                 "Beam position vs. time;Z [cm];Entries",
                 100, -r, r);

   GetOutputList()->Add(fBeamSpotX[0]);
   GetOutputList()->Add(fBeamSpotX[1]);
   GetOutputList()->Add(fBeamSpotY[0]);
   GetOutputList()->Add(fBeamSpotY[1]);
   GetOutputList()->Add(fBeamSpotZ[0]);
   GetOutputList()->Add(fBeamSpotZ[1]);
}

Bool_t TBeamSpotSel::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either TBeamSpotSel::GetEntry() or TBranch::GetEntry()
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

   // too much noise :-)
   //Info("Process", "called!");

   // Load the entry number "entry" from the tree:
   fChain->GetTree()->GetEntry(entry);

   // Get the time stamp of the entry.
   // We only care about the seconds...
   double_t sec = fEventInfo->Time().GetSecond();
   // ...and their fraction:
   sec += fEventInfo->MuSec() / 1000000.;

   int timerange = -1;
   if (sec > 36) timerange = 1; // after the move
   else if (sec < 35) timerange = 0; // before the move
   else return kTRUE; // somewhat during the move, ignore by going to the next event.

   // fill histograms
   Double_t x = fEventInfo->PrimaryVertex().X();
   Double_t y = fEventInfo->PrimaryVertex().Y();
   Double_t z = fEventInfo->PrimaryVertex().Z();
   fBeamSpotX[timerange]->Fill(x);
   fBeamSpotY[timerange]->Fill(y);
   fBeamSpotZ[timerange]->Fill(z);

   return kTRUE;
}

void TBeamSpotSel::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

   Info("SlaveTerminate", "called!");
}

void TBeamSpotSel::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

   Info("Terminate", "called!");

   // Let's draw one histogram only for now;
   // it's maybe not the most interesting one...
   const char* axisnames[] = {"X", "Y", "Z"};
   TCanvas* canvas = new TCanvas();
   canvas->Divide(2, 3); // create sub-pads in canvas: before / after (=2), x/y/z (=3) 

   for (int axis = 0; axis < 3; ++axis) { // loop over x, y, z
      for (int timerange = 0; timerange < 2; ++timerange) {
         canvas->cd(axis * 2 + timerange + 1); // "move" to a sub-pad in the canvas
         TString name("hBeamSpot"); // "calculate" the name of the histogram for axis, timerange:
         name += axisnames[axis]; // e.g. hBeamSpot -> hBeamSpotX
         name += timerange; // e.g. hBeamSpotX -> hBeamSpotX0
         TH1* h = (TH1*) GetOutputList()->FindObject(name);
         h->Draw(); 
      }
   }
}
