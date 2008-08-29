//
// This script helps you run the tutorials for the third course.
// 
//

//
// Run
//   $ xrootd -c $ROOTSYS/etc/proof/xpd.cf.sample -b
// to start the PROOF daemon before using proof in here!
//

void run() {
   TChain* chain = new TChain("tree3");
   chain->Add("beamspot*.root");
   TProof::Open("localhost");
   chain->SetProof();
   chain->Process("TBeamSpotSel.C+");
}
