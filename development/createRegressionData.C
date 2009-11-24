// plot the variables
#include "TROOT.h"
#include "TMath.h"
#include "TTree.h"
#include "TArrayD.h"
#include "TStyle.h"
#include "TFile.h"
#include "TRandom.h"
#include "Riostream.h"
#include "TCanvas.h"
#include "TMatrixD.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TLegend.h"
#include "TBranch.h"
#include "TRandom1.h"
#include <vector>

void plot( TString fname = "RegressionData.root" ) 
{
   TFile* dataFile = TFile::Open( fname );

   if (!dataFile) {
      cout << "ERROR: cannot open file: " << fname << endl;
      return;
   }

   TTree *treeR = (TTree*)dataFile->Get("TreeR");

   TCanvas* c = new TCanvas( "c", "", 0, 0, 550, 550 );

   TStyle *TMVAStyle = gROOT->GetStyle("Plain"); // our style is based on Plain
   TMVAStyle->SetOptStat(0);
   TMVAStyle->SetPadTopMargin(0.02);
   TMVAStyle->SetPadBottomMargin(0.16);
   TMVAStyle->SetPadRightMargin(0.03);
   TMVAStyle->SetPadLeftMargin(0.15);
   TMVAStyle->SetPadGridX(0);
   TMVAStyle->SetPadGridY(0);
   
   TMVAStyle->SetOptTitle(0);
   TMVAStyle->SetTitleW(.4);
   TMVAStyle->SetTitleH(.10);
   TMVAStyle->SetTitleX(.5);
   TMVAStyle->SetTitleY(.9);
   TMVAStyle->SetMarkerStyle(20);
   TMVAStyle->SetMarkerSize(.4);
   TMVAStyle->cd();

   TMVAStyle->SetNumberContours(99);


   Float_t xmin = treeR->GetMinimum( "var1" );
   Float_t xmax = treeR->GetMaximum( "var1" );
   Float_t ymin = treeR->GetMinimum( "var2" );
   Float_t ymax = treeR->GetMaximum( "var2" );
   Float_t zmin = treeR->GetMinimum( "fvalue" );
   Float_t zmax = treeR->GetMaximum( "fvalue" );

   Int_t nbin = 30;
   TH3F* frame = new TH3F(); // "DataR", "Data", nbin, xmin, xmax, nbin, ymin, ymax, nbin, zmin, zmax  );

   TString  options("prof surf2 fb bb");

   // project trees
   treeR->Draw( Form("fvalue:var2:var1>>DataR(%d,%f,%f,%d,%f,%f,%d,%f,%f)"
		     ,nbin,xmin,xmax,nbin,ymin,ymax,nbin,zmin,zmax ), 
		"", options+TString("goff") );

   frame = (TH3F*)treeR->GetHistogram();
   frame->SetName("DataR");

   // set style
   frame->SetMarkerSize( 0.2 );
   frame->SetMarkerColor( 4 );

   // legend
   frame->SetTitle( "fvalue in var2:var1 plane " );
   frame->GetXaxis()->SetTitle( "var1" );
   frame->GetYaxis()->SetTitle( "var2" );
   frame->GetZaxis()->SetTitle( "fvalue" );

   frame->SetLabelSize( 0.03, "XYZ" );
   frame->SetTitleSize( 0.05, "XYZ" );
   frame->SetTitleOffset( 1.2, "XYZ" );

   // and plot
   frame->Draw(options);


}


// create the data
void createPlane(Int_t N = 20000)
{
   // the true function around which the "sampled" (i.e. measured) values fluctuate
   // is a plane  z = a*x + b*y + c

   const Float_t a=7, b=9, c=2, d = 100.0;
   const Int_t nvar = 2;

   TRandom1 R( 100 );

   Float_t xvar[nvar];
   Float_t fvalue;

   // output flie
   TFile* dataFile = TFile::Open( "RegressionData.root", "RECREATE" );

   // create signal and background trees
   TTree* treeR = new TTree( "TreeR", "TreeR", 1 );   
   for (Int_t ivar=0; ivar<nvar; ivar++) {
      treeR->Branch( TString(Form( "var%i", ivar+1 )).Data(), &xvar[ivar], TString(Form( "var%i/F", ivar+1 )).Data() );
   }
   treeR->Branch( "fvalue", &fvalue, "fvalue/F") ;


   Float_t delta_fvalue;
   
   // event loop
   cout << "start event loop " << endl;
   for (Int_t i=0; i<N; i++) {
      //cout << "i=" << i << endl;
      for (Int_t ivar=0; ivar<nvar; ivar++) {
         xvar[ivar] = R.Rndm()*5.;
         //cout << "xvar["<<ivar<<"] = " << xvar[ivar] << endl;
      }
//      delta_fvalue = R.Rndm()-0.5;
      delta_fvalue = R.Gaus();

      fvalue = a*xvar[0] + b*xvar[1] + c + d*delta_fvalue;
      
      if (i%1000 == 0) cout << "... event: " << i << " (" << N << ")" << endl;
      
      treeR->Fill();
   }

   
   // write tree
   treeR->Write();
   treeR->Show(0);

   dataFile->Close();
   cout << "created data file: " << dataFile->GetName() << endl;

   plot("RegressionData.root");
}

// create the data


void createParabola(Int_t N = 20000)
{
   // the true function around which the "sampled" (i.e. measured) values fluctuate
   // is a parabola  z = a*x*x + b*y*y + c

   const Float_t a=7, b=9, c=2;
   const Int_t nvar = 2;

   TRandom1 R( 100 );

   Float_t xvar[nvar];
   Float_t fvalue;
   Float_t weight;

   // output flie
   TFile* dataFile = TFile::Open( "RegressionData.root", "RECREATE" );

   // create signal and background trees
   TTree* treeR = new TTree( "TreeR", "TreeR", 1 );   
   for (Int_t ivar=0; ivar<nvar; ivar++) {
      treeR->Branch( TString(Form( "var%i", ivar+1 )).Data(), &xvar[ivar], TString(Form( "var%i/F", ivar+1 )).Data() );
   }
   treeR->Branch( "weight", &weight, "weight/F" );
   treeR->Branch( "fvalue", &fvalue, "fvalue/F" );


   Float_t delta_fvalue;
   
   // event loop
   cout << "start event loop " << endl;
   for (Int_t i=0; i<N; i++) {
      //cout << "i=" << i << endl;
      for (Int_t ivar=0; ivar<nvar; ivar++) {
         xvar[ivar] = R.Rndm()*5.;
         //cout << "xvar["<<ivar<<"] = " << xvar[ivar] << endl;
      }
      delta_fvalue = R.Rndm()-0.5;

      fvalue = a*xvar[0]*xvar[0] + b*xvar[1]*xvar[1] + c + delta_fvalue;
      weight = 2*a*xvar[0] + 2*b*xvar[1];
      if (i%1000 == 0) cout << "... event: " << i << " (" << N << ")" << endl;
      
      treeR->Fill();
   }

   
   // write tree
   treeR->Write();
   treeR->Show(0);

   dataFile->Close();
   cout << "created data file: " << dataFile->GetName() << endl;

   plot("RegressionData.root");
}


void createWaves(Int_t N = 10000)
{
   // the true function around which the "sampled" (i.e. measured) values fluctuate
   // is a parabola  z = a*x*x + b*y*y + c

   const Float_t a=21, b=9, c=2, d=1.7, e=0.9, f=-31;
   const Int_t nvar = 2;

   TRandom1 R( 100 );

   Float_t xvar[nvar];
   Float_t fvalue;
   Float_t fvalue2;

   // output flie
   TFile* dataFile = TFile::Open( "RegressionData.root", "RECREATE" );

   // create signal and background trees
   TTree* treeR = new TTree( "TreeR", "TreeR", 1 );   
   for (Int_t ivar=0; ivar<nvar; ivar++) {
      treeR->Branch( TString(Form( "var%i", ivar+1 )).Data(), &xvar[ivar], TString(Form( "var%i/F", ivar+1 )).Data() );
   }
   treeR->Branch( "fvalue",  &fvalue,  "fvalue/F" ) ;
   treeR->Branch( "fvalue2", &fvalue2, "fvalue2/F") ;


   Float_t delta_fvalue;
   
   // event loop
   cout << "start event loop " << endl;
   for (Int_t i=0; i<N; i++) {
      //cout << "i=" << i << endl;
      for (Int_t ivar=0; ivar<nvar; ivar++) {
         xvar[ivar] = R.Rndm()*5.;
         //cout << "xvar["<<ivar<<"] = " << xvar[ivar] << endl;
      }
      delta_fvalue = R.Rndm()-0.5;

      fvalue  = a*sin(xvar[0]*3) + b*cos(xvar[1]*2) + c + d*xvar[0] + e*xvar[1] + f*sin(xvar[0]*xvar[1]*1.1) + delta_fvalue;
      fvalue2 = TMath::Log(TMath::Abs(fvalue)+10);
      
      if (i%1000 == 0) cout << "... event: " << i << " (" << N << ")" << endl;
      
      treeR->Fill();
   }

   
   // write tree
   treeR->Write();
   treeR->Show(0);

   dataFile->Close();
   cout << "created data file: " << dataFile->GetName() << endl;

   plot("RegressionData.root");
}





void createWaves6D(Int_t N = 200000)
{
   // the true function around which the "sampled" (i.e. measured) values fluctuate
   // is a parabola  z = a*x*x + b*y*y + c

   const Float_t a=21, b=9, c=2, d=1.7, e=0.9, f=-31;
   const Int_t nvar = 6;

   TRandom1 R( 100 );

   Float_t xvar[nvar];
   Float_t fvalue;

   // output flie
   TFile* dataFile = TFile::Open( "RegressionData.root", "RECREATE" );

   // create signal and background trees
   TTree* treeR = new TTree( "TreeR", "TreeR", 1 );   
   for (Int_t ivar=0; ivar<nvar; ivar++) {
      treeR->Branch( TString(Form( "var%i", ivar+1 )).Data(), &xvar[ivar], TString(Form( "var%i/F", ivar+1 )).Data() );
   }
   treeR->Branch( "fvalue", &fvalue, "fvalue/F") ;


   Float_t delta_fvalue;
   
   // event loop
   cout << "start event loop " << endl;
   for (Int_t i=0; i<N; i++) {
      //cout << "i=" << i << endl;
      for (Int_t ivar=0; ivar<nvar; ivar++) {
         xvar[ivar] = R.Rndm()*5.;
         //cout << "xvar["<<ivar<<"] = " << xvar[ivar] << endl;
      }
      delta_fvalue = R.Rndm()-0.5;

      fvalue = a*sin(xvar[0]*3) + b*cos(xvar[1]*2) + c + d*xvar[2] + e*xvar[3] + f*sin(xvar[4]*xvar[5]*1.1) + delta_fvalue;
      
      if (i%1000 == 0) cout << "... event: " << i << " (" << N << ")" << endl;
      
      treeR->Fill();
   }

   
   // write tree
   treeR->Write();
   treeR->Show(0);

   dataFile->Close();
   cout << "created data file: " << dataFile->GetName() << endl;

   plot("RegressionData.root");
}

