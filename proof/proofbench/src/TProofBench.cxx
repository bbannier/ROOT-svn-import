// @(#)root/proofx:$Id$
// Author: Sangsu Ryu 22/06/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBench                                                          //
//                                                                      //
// Steering class for PROOF benchmarks                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofBench.h"
#include "TProofBenchRunCPU.h"
#include "TProofBenchRunCleanup.h"
#include "TProofBenchRunDataRead.h"
#include "TFile.h"
#include "TFileCollection.h"
#include "TKey.h"
#include "TObjString.h"
#include "TProof.h"
#include "TUrl.h"

#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TH1F.h"
#include "TMath.h"
#include "TProfile.h"
#include "TStyle.h"

ClassImp(TProofBench)

const char* const kPROOF_BenchSelPar    = "ProofBenchSel";  // PAR with bench selectors
const char* const kPROOF_BenchSelParDir = "proof/proofbench/src/";  // Location of ProofBenchSel

//______________________________________________________________________________
TProofBench::TProofBench(const char *url, const char *outfile, const char *proofopt)
            : fOutFile(0), fNtries(4), fRunCPU(0), fRunDS(0)
{
   // Constructor: check PROOF and load selectors PAR
   
   SetBit(kInvalidObject);
   if (!url) {
      Error("TProofBench", "specifying a PROOF master url is mandatory - cannot continue");
      return;
   }
   if (!(fProof = TProof::Open(url, proofopt)) || (fProof && !fProof->IsValid())) {
      Error("TProofBench", "could not open a valid PROOF session - cannot continue");
      return;
   }
   // Load selectors
   TString par = TString::Format("%s%s.par", kPROOF_BenchSelParDir, kPROOF_BenchSelPar);
   Printf("Uploading '%s' ...", par.Data());
   if (fProof->UploadPackage(par) != 0) {
      Error("TProofBench", "problems uploading '%s' - cannot continue", par.Data());
      return;
   }
   Printf("Enabling '%s' ...", kPROOF_BenchSelPar);
   if (fProof->EnablePackage(kPROOF_BenchSelPar) != 0) {
      Error("TProofBench", "problems enabling '%s' - cannot continue", kPROOF_BenchSelPar);
      return;
   }
   ResetBit(kInvalidObject);

   // Load ProofEventProc
   fProof->Load("tutorials/proof/ProofEventProc.C+");

   // Set output file
   if (SetOutFile(outfile, kFALSE) != 0)
      Warning("TProofBench", "problems opening '%s' - ignoring: use SetOutFile to try"
                                   " again or with another file", outfile);
} 

//______________________________________________________________________________
TProofBench::~TProofBench()
{
   // Destructor
   
   CloseOutFile();
   SafeDelete(fRunCPU);
   SafeDelete(fRunDS);
}

//______________________________________________________________________________
Int_t TProofBench::OpenOutFile(Bool_t wrt, Bool_t verbose)
{
   // Set the otuput file
   // Return 0 on success, -1 on error

   Int_t rc = 0;
   if (!fOutFile && fOutFileName.Length() > 0) {
      const char *mode = 0;
      if (wrt)
         mode = gSystem->AccessPathName(fOutFileName) ? "RECREATE" : "UPDATE";
      else
         mode = "READ";
      if (!(fOutFile = TFile::Open(fOutFileName, mode)) || (fOutFile && fOutFile->IsZombie())) {
         if (verbose)
            Warning("OpenOutFile", "problems opening '%s' - ignoring: use SetOutFile to try"
                                   " again or with another file", fOutFileName.Data());
         rc = -1;
      }
   }
   return rc;
}


//______________________________________________________________________________
Int_t TProofBench::SetOutFile(const char *outfile, Bool_t verbose)
{
   // Set the otuput file
   // Return 0 on success, -1 on error

   Int_t rc = 0;
   // Close existing file, if any
   if (fOutFile && !fOutFile->IsZombie()) {
      fOutFile->Close();
      SafeDelete(fOutFile);
   }
   if (outfile && strlen(outfile) > 0) {
      fOutFileName = outfile;
      if ((rc = OpenOutFile(kTRUE, kFALSE)) != 0 && verbose)
         Warning("SetOutFile", "problems opening '%s' - ignoring: use SetOutFile to try"
                               " again or with another file", outfile);
   }
   return rc;
}

//______________________________________________________________________________
Int_t TProofBench::RunCPU(Long64_t nevents, Int_t start, Int_t stop, Int_t step)
{
   // Perform the CPU run
   // Return 0 on success, -1 on error

   SafeDelete(fRunCPU);
   fRunCPU = new TProofBenchRunCPU(kPBHist1D, 16, fOutFile);
   fRunCPU->Run(nevents, start, stop, step, fNtries, -1, -1);
   // Done
   return 0;
}

//______________________________________________________________________________
Int_t TProofBench::RunCPUx(Long64_t nevents, Int_t start, Int_t stop)
{
   // Perform the CPU run scanning over the number of workers per node
   // Return 0 on success, -1 on error

   SafeDelete(fRunCPU);
   fRunCPU = new TProofBenchRunCPU(kPBHist1D, 16, fOutFile);
   fRunCPU->Run(nevents, start, stop, -2, fNtries, -1, -1);
   // Done
   return 0;
}

//______________________________________________________________________________
void TProofBench::DrawCPU(const char *outfile, const char *opt)
{
   // Draw the CPU speedup plot.
   //  opt =    'abs:'           draw the absolute plot
   //           'norm:'          draw the normalized plot
   //

   // Get the TProfile an create the graphs
   TFile *fout = TFile::Open(outfile, "READ");
   if (!fout || (fout && fout->IsZombie())) {
      ::Error("DrawCPU", "could not open file '%s' ...", outfile);
      return;
   }
   TProfile *pf = (TProfile *) fout->Get("RunCPU/hProf_CPUHist1D_QueryResult_Event");
   if (!pf) {
      ::Error("DrawCPU", "could not find 'hProf_CPUHist1D_QueryResult_Event' ...");
      fout->Close();
      delete fout;
      return;
   }
   Bool_t norm = (!strcmp(opt, "norm:")) ? kTRUE : kFALSE;
   Int_t nbins = pf->GetNbinsX();
   TGraphErrors *gr = new TGraphErrors(nbins);
   Double_t xx, ex, yy, ey,
            ynorm = pf->GetBinContent(1), enorm = pf->GetBinError(1), ymi = ynorm, ymx = ynorm;
   Int_t k =1;
   for (;k <= nbins; k++) {
      xx = pf->GetBinCenter(k);
      ex = pf->GetBinWidth(k) * .001;
      yy = pf->GetBinContent(k);
      ey = pf->GetBinError(k);
      if (norm) {
         if (k == 1) {
            ey = 0;
         } else {
            ey = (ey/yy)*(ey/yy) + (enorm/ynorm)*(enorm/ynorm);
            ey = TMath::Sqrt(ey);
         }
         yy = yy / ynorm / xx;
         ey *= yy;
      }
      if (k == 1) {
         ymi = yy;
         ymx = yy;
      } else {
         if (yy < ymi) ymi = yy; 
         if (yy > ymx) ymx = yy;
      }
      gr->SetPoint(k-1, xx, yy);
      gr->SetPointError(k-1, ex, ey);
   }

   // Create the canvas
   TCanvas *cpu = new TCanvas("cpu", "Rate vs wrks",204,69,1050,502);
   cpu->Range(-3.106332,0.7490716,28.1362,1.249867);

   gStyle->SetOptTitle(0);
   gr->SetFillColor(1);
   gr->SetLineColor(13);
   gr->SetMarkerStyle(21);
   gr->SetMarkerSize(1.2);

   TH1F *hgr = new TH1F("Graph-CPU"," CPU speed-up", nbins*4,0,nbins+1);
   if (norm) {
      hgr->SetTitle("Normalized CPU speed-up");
      hgr->SetMaximum(1.2);
      hgr->SetMinimum(0.8);
   } else {
      hgr->SetMaximum(ymx + (ymx-ymi)*0.2);
      hgr->SetMinimum(0);
   }
   hgr->SetDirectory(0);
   hgr->SetStats(0);
//   hgr->CenterTitle(true);
   hgr->GetXaxis()->SetTitle("# workers");
   hgr->GetXaxis()->CenterTitle(true);
   hgr->GetXaxis()->SetLabelSize(0.05);
   hgr->GetXaxis()->SetTitleSize(0.06);
   hgr->GetXaxis()->SetTitleOffset(0.62);
   hgr->GetYaxis()->SetLabelSize(0.06);
   gr->SetHistogram(hgr);

   gr->Print();
   
   gr->Draw("alp");
  
   fout->Close();
}

//______________________________________________________________________________
Int_t TProofBench::RunDataSet(const char *dset, EPBReadType readtype,
                              Int_t start, Int_t stop, Int_t step, TProof *pclnup)
{
   // Perform a test using dataset 'dset'
   // Return 0 on success, -1 on error

#if 0
   TProofBenchRunCleanup *run_clnup = new TProofBenchRunCleanup(kPBCleanupFileAdvise, 0, pclnup);
#endif
   ReleaseCache(dset);
   SafeDelete(fRunDS);
   fRunDS = new TProofBenchRunDataRead(0, 0, readtype, fOutFile); 
   fRunDS->Run(dset, start, stop, step, fNtries, -1, -1);
   // Done
   return 0;
}

//______________________________________________________________________________
void TProofBench::DrawDataSet(const char *outfile, const char *opt, Int_t kseq)
{
   // Draw the CPU speedup plot.
   //  opt =    'mbs:'          draw MB/s scaling plot
   //           'evt:'          draw event/s scaling plot
   //

   // Get the TProfile an create the graphs
   TFile *fout = TFile::Open(outfile, "READ");
   if (!fout || (fout && fout->IsZombie())) {
      ::Error("DrawDataSet", "could not open file '%s' ...", outfile);
      return;
   }
   TDirectory *d = (TDirectory *) fout->Get("RunDataRead");
   if (!d) {
      ::Error("DrawDataSet", "could not find directory 'RunDataRead' ...");
      fout->Close();
      delete fout;
      return;
   }
   d->cd();
   TString hprofn = (!strcmp(opt, "mbs:")) ? "hProf_QueryResult_IO" : "hProf_QueryResult_Event";
   TProfile *pf = 0;
   TList *keylist = d->GetListOfKeys();
   TKey *key = 0;
   TIter nxk(keylist);
   Int_t kk = 0;
   while ((key = (TKey *) nxk())) {
      if (TString(key->GetName()).BeginsWith(hprofn)) {
         kk++;
         if (kk >= kseq) {
            pf = (TProfile *) d->Get(key->GetName());
            break;
         }
      }
   }
   if (!pf) {
      ::Error("DrawDataSet", "could not find '%s' ...", hprofn.Data());
      fout->Close();
      delete fout;
      return;
   }
   Int_t nbins = pf->GetNbinsX();
   TGraphErrors *gr = new TGraphErrors(nbins);
   Double_t xx, ex, yy, ey,
            ymi = pf->GetBinContent(1), ymx = ymi;
   Int_t k =1;
   for (;k <= nbins; k++) {
      xx = pf->GetBinCenter(k);
      ex = pf->GetBinWidth(k) * .001;
      yy = pf->GetBinContent(k);
      ey = pf->GetBinError(k);
      if (k == 1) {
         ymi = yy;
         ymx = yy;
      } else {
         if (yy < ymi) ymi = yy; 
         if (yy > ymx) ymx = yy;
      }
      gr->SetPoint(k-1, xx, yy);
      gr->SetPointError(k-1, ex, ey);
      Printf("%d %f %f", (Int_t)xx, yy, ey);
   }

   // Create the canvas
   TCanvas *cpu = new TCanvas("dataset", "Rate vs wrks",204,69,1050,502);
   cpu->Range(-3.106332,0.7490716,28.1362,1.249867);

   gStyle->SetOptTitle(0);
   gr->SetFillColor(1);
   gr->SetLineColor(13);
   gr->SetMarkerStyle(21);
   gr->SetMarkerSize(1.2);

   TH1F *hgr = new TH1F("Graph-DataSet"," Data Read speed-up", nbins*4,0,nbins+1);
   hgr->SetMaximum(ymx + (ymx-ymi)*0.2);
   hgr->SetMinimum(0);
   hgr->SetDirectory(0);
   hgr->SetStats(0);
//   hgr->CenterTitle(true);
   hgr->GetXaxis()->SetTitle("# workers");
   hgr->GetXaxis()->CenterTitle(true);
   hgr->GetXaxis()->SetLabelSize(0.05);
   hgr->GetXaxis()->SetTitleSize(0.06);
   hgr->GetXaxis()->SetTitleOffset(0.62);
   hgr->GetYaxis()->SetLabelSize(0.06);
   gr->SetHistogram(hgr);

   gr->Print();
   
   gr->Draw("alp");
  
   fout->Close();
}

//______________________________________________________________________________
Int_t TProofBench::ReleaseCache(const char *dset)
{
   // Release fiole cache for dataset 'dset'
   // Return 0 on success, -1 on error

   // Check input
   if (!dset || (dset && strlen(dset) <= 0)) {
      Error("ReleaseCache", "a valid dataset name is mandatory");
      return -1;
   }

   // The dataset must exist
   if (!fProof || (fProof && !fProof->ExistsDataSet(dset))) {
      Error("ReleaseCache", "dataset '%s' does not exist", dset);
      return -1;
   }

   // Get the dataset
   TFileCollection *fc = fProof->GetDataSet(dset);
   if (!fc) {
      Error("ReleaseCache", "TFileCollection object for dataset '%s' could not be retrieved", dset);
      return -1;
   }

   // Get information per server
   TMap *fcmap = fc->GetFilesPerServer();
   if (!fcmap) {
      Error("ReleaseCache", "could not create map with per-server info for dataset '%s'", dset);
      return -1;
   }
   fcmap->Print();

   // Add map in the input list
   fcmap->SetName("PROOF_FilesToProcess");
   fProof->AddInput(fcmap);

   // Set parameters for processing
   TString oldpack;
   if (TProof::GetParameter(fProof->GetInputList(), "PROOF_Packetizer", oldpack) != 0) oldpack = "";
   fProof->SetParameter("PROOF_Packetizer", "TPacketizerFile");
   Int_t oldnotass = -1;
   if (TProof::GetParameter(fProof->GetInputList(), "PROOF_ProcessNotAssigned", oldnotass) != 0) oldnotass = -1;
   fProof->SetParameter("PROOF_ProcessNotAssigned", (Int_t)0);

   // Process
   fProof->Process("TSelCacheRelease", (Long64_t) fc->GetNFiles());

   // Restore parameters
   if (!oldpack.IsNull())
      fProof->SetParameter("PROOF_Packetizer", oldpack);
   else
      fProof->DeleteParameters("PROOF_Packetizer");
   if (oldnotass != -1)
      fProof->SetParameter("PROOF_ProcessNotAssigned", oldnotass);
   else
      fProof->DeleteParameters("PROOF_ProcessNotAssigned");
   
   // Cleanup
   fProof->GetInputList()->Remove(fcmap);
   delete fcmap;
   delete fc;
   
   // Done
   return 0;
}
