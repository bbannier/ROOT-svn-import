#include <cmath>
#include <ctime>

#include "TRandom2.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TMath.h"

#include "Riostream.h"

// I don't get it to work with extern declaration :S
const Double_t minRange = 1;
const Double_t maxRange = 5;

const int minBinValue = 1;
const int maxBinValue = 10;

const int nEvents = 1000;

const int nbins = 10;

// extern declarations
enum compareOptions {
   cmpOptDebug=1,
   cmpOptNoError=2,
   cmpOptStats=4
};

int equals(const char* msg, TH1D* h1, TH1D* h2, int options = 0, double ERRORLIMIT = 1E-15);
int equals(const char* msg, TH2D* h1, TH2D* h2, int options = 0, double ERRORLIMIT = 1E-15);
int equals(const char* msg, TH3D* h1, TH3D* h2, int options = 0, double ERRORLIMIT = 1E-15);
int equals(Double_t n1, Double_t n2, double ERRORLIMIT = 1E-15);
int compareStatistics( TH1* h1, TH1* h2, bool debug, double ERRORLIMIT = 1E-15);
ostream& operator<<(ostream& out, TH1D* h);

extern TRandom2 r;
// extern declarations - end

bool testAdd1() 
{
   Double_t c1 = r.Rndm();
   Double_t c2 = r.Rndm();

   TH1D* h1 = new TH1D("t1D1-h1", "h1-Title", nbins, minRange, maxRange);
   TH1D* h2 = new TH1D("t1D1-h2", "h2-Title", nbins, minRange, maxRange);
   TH1D* h3 = new TH1D("t1D1-h3", "h3=c1*h1+c2*h2", nbins, minRange, maxRange);

   h1->Sumw2();h2->Sumw2();h3->Sumw2();

   for ( Int_t e = 0; e < nEvents; ++e ) {
      Double_t value = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(value,  1);
      h3->Fill(value, c1);
   }

   for ( Int_t e = 0; e < nEvents; ++e ) {
      Double_t value = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h2->Fill(value,  1);
      h3->Fill(value, c2);
   }

   TH1D* h4 = new TH1D("t1D1-h4", "h4=c1*h1+h2*c2", nbins, minRange, maxRange);
   h4->Add(h1, h2, c1, c2);

   bool ret = equals("Add1D1", h3, h4, cmpOptStats, 1E-13);
   delete h1;
   delete h2;
   delete h3;
   return ret;
}

bool testAdd2() 
{
   Double_t c2 = r.Rndm();

   TH1D* h5 = new TH1D("t1D2-h5", "h5=   h6+c2*h7", nbins, minRange, maxRange);
   TH1D* h6 = new TH1D("t1D2-h6", "h6-Title", nbins, minRange, maxRange);
   TH1D* h7 = new TH1D("t1D2-h7", "h7-Title", nbins, minRange, maxRange);

   h5->Sumw2();h6->Sumw2();h7->Sumw2();

   for ( Int_t e = 0; e < nEvents; ++e ) {
      Double_t value = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h6->Fill(value, 1);
      h5->Fill(value, 1);
   }

   for ( Int_t e = 0; e < nEvents; ++e ) {
      Double_t value = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h7->Fill(value,  1);
      h5->Fill(value, c2);
   }

   h6->Add(h7, c2);
   
   bool ret = equals("Add1D2", h5, h6, cmpOptStats, 1E-13);
   delete h5;
   delete h7;
   return ret;
}

bool testAdd2D1() 
{
   Double_t c1 = r.Rndm();
   Double_t c2 = r.Rndm();

   TH2D* h1 = new TH2D("t2D1-h1", "h1", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   TH2D* h2 = new TH2D("t2D1-h2", "h2", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   TH2D* h3 = new TH2D("t2D1-h3", "h3=c1*h1+c2*h2", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   h1->Sumw2();h2->Sumw2();h3->Sumw2();

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(x, y,  1);
      h3->Fill(x, y, c1);
   }

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h2->Fill(x, y,  1);
      h3->Fill(x, y, c2);
   }

   TH2D* h4 = new TH2D("t2D1-h4", "h4=c1*h1+c2*h2", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);
   h4->Add(h1, h2, c1, c2);
   bool ret = equals("Add2D1", h3, h4, cmpOptStats, 1E-12);
   delete h1;
   delete h2;
   delete h3;
   return ret;
}

bool testAdd2D2() 
{
   Double_t c2 = r.Rndm();

   TH2D* h1 = new TH2D("t2D2-h1", "h1", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   TH2D* h2 = new TH2D("t2D2-h2", "h2", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   TH2D* h3 = new TH2D("t2D2-h3", "h3=h1+c2*h2", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   h1->Sumw2();h2->Sumw2();h3->Sumw2();

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(x, y,  1);
      h3->Fill(x, y,  1);
   }

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h2->Fill(x, y,  1);
      h3->Fill(x, y, c2);
   }

   h1->Add(h2, c2);
   bool ret = equals("Add2D2", h3, h1, cmpOptStats, 1E-12);
   delete h2;
   delete h3;
   return ret;
}

bool testAdd3D1() 
{
   Double_t c1 = r.Rndm();
   Double_t c2 = r.Rndm();

   TH3D* h1 = new TH3D("t3D1-h1", "h1", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   TH3D* h2 = new TH3D("t3D1-h2", "h2", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   TH3D* h3 = new TH3D("t3D1-h3", "h3=c1*h1+c2*h2", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   h1->Sumw2();h2->Sumw2();h3->Sumw2();

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t z = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(x, y, z,  1);
      h3->Fill(x, y, z, c1);
   }

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t z = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h2->Fill(x, y, z,  1);
      h3->Fill(x, y, z, c2);
   }

   TH3D* h4 = new TH3D("t3D1-h4", "h4=c1*h1+c2*h2", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);
   h4->Add(h1, h2, c1, c2);
   bool ret = equals("Add3D1", h3, h4, cmpOptStats, 1E-12);
   delete h1;
   delete h2;
   delete h3;
   return ret;
}

bool testAdd3D2() 
{
   Double_t c2 = r.Rndm();

   TH3D* h1 = new TH3D("t3D2-h1", "h1", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   TH3D* h2 = new TH3D("t3D2-h2", "h2", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   TH3D* h3 = new TH3D("t3D2-h3", "h3=h1+c2*h2", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   h1->Sumw2();h2->Sumw2();h3->Sumw2();

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t z = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(x, y, z,  1);
      h3->Fill(x, y, z,  1);
   }

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t z = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h2->Fill(x, y, z,  1);
      h3->Fill(x, y, z, c2);
   }

   h1->Add(h2, c2);
   bool ret = equals("Add3D2", h3, h1, cmpOptStats, 1E-12);
   delete h2;
   delete h3;
   return ret;
}

bool testMul1() 
{
   Double_t c1 = r.Rndm();
   Double_t c2 = r.Rndm();

   TH1D* h1 = new TH1D("m1D1-h1", "h1-Title", nbins, minRange, maxRange);
   TH1D* h2 = new TH1D("m1D1-h2", "h2-Title", nbins, minRange, maxRange);
   TH1D* h3 = new TH1D("m1D1-h3", "h3=c1*h1*c2*h2", nbins, minRange, maxRange);

   h1->Sumw2();h2->Sumw2();h3->Sumw2();

   UInt_t seed = r.GetSeed();
   // For possible problems
   r.SetSeed(seed);
   for ( Int_t e = 0; e < nEvents; ++e ) {
      Double_t value = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(value, 1);
   }

   for ( Int_t e = 0; e < nEvents; ++e ) {
      Double_t value = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h2->Fill(value,  1);
      h3->Fill(value,  c1*c2*h1->GetBinContent( h1->GetXaxis()->FindBin(value) ) );
   }

   // h3 has to be filled again so that the erros are properly calculated
   r.SetSeed(seed);
   for ( Int_t e = 0; e < nEvents; ++e ) {
      Double_t value = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h3->Fill(value,  c1*c2*h2->GetBinContent( h2->GetXaxis()->FindBin(value) ) );
   }

   // No the bin contents has to be reduced, as it was filled twice!
   for ( Int_t bin = 0; bin <= h3->GetNbinsX() + 1; ++bin ) {
      h3->SetBinContent(bin, h3->GetBinContent(bin) / 2 );
   }

   TH1D* h4 = new TH1D("m1D1-h4", "h4=h1*h2", nbins, minRange, maxRange);
   h4->Multiply(h1, h2, c1, c2);

   bool ret = equals("Multiply1D1", h3, h4, cmpOptStats, 1E-14);
   delete h1;
   delete h2;
   delete h3;
   return ret;
}

bool testMul2() 
{
   TH1D* h1 = new TH1D("m1D2-h1", "h1-Title", nbins, minRange, maxRange);
   TH1D* h2 = new TH1D("m1D2-h2", "h2-Title", nbins, minRange, maxRange);
   TH1D* h3 = new TH1D("m1D2-h3", "h3=h1*h2", nbins, minRange, maxRange);

   h1->Sumw2();h2->Sumw2();h3->Sumw2();

   UInt_t seed = r.GetSeed();
   // For possible problems
   r.SetSeed(seed);
   for ( Int_t e = 0; e < nEvents; ++e ) {
      Double_t value = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(value, 1);
   }

   for ( Int_t e = 0; e < nEvents; ++e ) {
      Double_t value = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h2->Fill(value,  1);
      h3->Fill(value,  h1->GetBinContent( h1->GetXaxis()->FindBin(value) ) );
   }

   r.SetSeed(seed);
   for ( Int_t e = 0; e < nEvents; ++e ) {
      Double_t value = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h3->Fill(value,  h2->GetBinContent( h2->GetXaxis()->FindBin(value) ) );
   }

   for ( Int_t bin = 0; bin <= h3->GetNbinsX() + 1; ++bin ) {
      h3->SetBinContent(bin, h3->GetBinContent(bin) / 2 );
   }

   h1->Multiply(h2);

   bool ret = equals("Multiply1D2", h3, h1, cmpOptStats, 1E-14);
   delete h2;
   delete h3;
   return ret;
}

bool testMul2D1() 
{
   Double_t c1 = r.Rndm();
   Double_t c2 = r.Rndm();

   TH2D* h1 = new TH2D("m2D1-h1", "h1-Title", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);
   TH2D* h2 = new TH2D("m2D1-h2", "h2-Title", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);
   TH2D* h3 = new TH2D("m2D1-h3", "h3=c1*h1*c2*h2",
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   h1->Sumw2();h2->Sumw2();h3->Sumw2();

   UInt_t seed = r.GetSeed();
   // For possible problems
   r.SetSeed(seed);
   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(x, y, 1);
   }

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h2->Fill(x, y,  1);
      h3->Fill(x, y,  c1*c2*h1->GetBinContent( h1->GetXaxis()->FindBin(x),
                                               h1->GetYaxis()->FindBin(y) ) );
   }

   // h3 has to be filled again so that the erros are properly calculated
   r.SetSeed(seed);
   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h3->Fill(x, y,  c1*c2*h2->GetBinContent( h2->GetXaxis()->FindBin(x),
                                               h2->GetYaxis()->FindBin(y) ) );
   }

   // No the bin contents has to be reduced, as it was filled twice!
   for ( Int_t i = 0; i <= h3->GetNbinsX() + 1; ++i ) {
      for ( Int_t j = 0; j <= h3->GetNbinsY() + 1; ++j ) {
         h3->SetBinContent(i, j, h3->GetBinContent(i, j) / 2 );
      }
   }

   TH2D* h4 = new TH2D("m2D1-h4", "h4=h1*h2", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);
   h4->Multiply(h1, h2, c1, c2);

   bool ret = equals("Multiply2D1", h3, h4, cmpOptStats, 1E-12);
   delete h1;
   delete h2;
   delete h3;
   return ret;
}

bool testMul2D2() 
{
   TH2D* h1 = new TH2D("m2D2-h1", "h1-Title", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);
   TH2D* h2 = new TH2D("m2D2-h2", "h2-Title", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);
   TH2D* h3 = new TH2D("m2D2-h3", "h3=h1*h2",
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   h1->Sumw2();h2->Sumw2();h3->Sumw2();

   UInt_t seed = r.GetSeed();
   // For possible problems
   r.SetSeed(seed);
   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(x, y, 1);
   }

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h2->Fill(x, y,  1);
      h3->Fill(x, y,  h1->GetBinContent( h1->GetXaxis()->FindBin(x),
                                         h1->GetYaxis()->FindBin(y) ) );
   }

   // h3 has to be filled again so that the erros are properly calculated
   r.SetSeed(seed);
   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h3->Fill(x, y,  h2->GetBinContent( h2->GetXaxis()->FindBin(x),
                                         h2->GetYaxis()->FindBin(y) ) );
   }

   // No the bin contents has to be reduced, as it was filled twice!
   for ( Int_t i = 0; i <= h3->GetNbinsX() + 1; ++i ) {
      for ( Int_t j = 0; j <= h3->GetNbinsY() + 1; ++j ) {
         h3->SetBinContent(i, j, h3->GetBinContent(i, j) / 2 );
      }
   }

   h1->Multiply(h2);

   bool ret = equals("Multiply2D2", h3, h1, cmpOptStats, 1E-12);
   delete h2;
   delete h3;
   return ret;
}

bool testMul3D1() 
{
   Double_t c1 = r.Rndm();
   Double_t c2 = r.Rndm();

   TH3D* h1 = new TH3D("m3D1-h1", "h1-Title", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);
   TH3D* h2 = new TH3D("m3D1-h2", "h2-Title", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);
   TH3D* h3 = new TH3D("m3D1-h3", "h3=c1*h1*c2*h2",
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   h1->Sumw2();h2->Sumw2();h3->Sumw2();

   UInt_t seed = r.GetSeed();
   // For possible problems
   r.SetSeed(seed);
   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t z = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(x, y, z, 1);
   }

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t z = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h2->Fill(x, y, z,  1);
      h3->Fill(x, y, z,  c1*c2*h1->GetBinContent( h1->GetXaxis()->FindBin(x),
                                                  h1->GetYaxis()->FindBin(y),
                                                  h1->GetZaxis()->FindBin(z) ) );
   }

   // h3 has to be filled again so that the erros are properly calculated
   r.SetSeed(seed);
   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t z = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h3->Fill(x, y, z,  c1*c2*h2->GetBinContent( h2->GetXaxis()->FindBin(x),
                                                  h2->GetYaxis()->FindBin(y),
                                                  h2->GetZaxis()->FindBin(z) ) );
   }

   // No the bin contents has to be reduced, as it was filled twice!
   for ( Int_t i = 0; i <= h3->GetNbinsX() + 1; ++i ) {
      for ( Int_t j = 0; j <= h3->GetNbinsY() + 1; ++j ) {
         for ( Int_t h = 0; h <= h3->GetNbinsZ() + 1; ++h ) {
            h3->SetBinContent(i, j, h, h3->GetBinContent(i, j, h) / 2 );
         }
      }
   }

   TH3D* h4 = new TH3D("m3D1-h4", "h4=h1*h2", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);
   h4->Multiply(h1, h2, c1, c2);

   bool ret = equals("Multiply3D1", h3, h4, cmpOptStats, 1E-13);
   delete h1;
   delete h2;
   delete h3;
   return ret;
}

bool testMul3D2() 
{
   TH3D* h1 = new TH3D("m3D2-h1", "h1-Title", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);
   TH3D* h2 = new TH3D("m3D2-h2", "h2-Title", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);
   TH3D* h3 = new TH3D("m3D2-h3", "h3=h1*h2",
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   h1->Sumw2();h2->Sumw2();h3->Sumw2();

   UInt_t seed = r.GetSeed();
   // For possible problems
   r.SetSeed(seed);
   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t z = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(x, y, z, 1);
   }

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t z = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h2->Fill(x, y, z,  1);
      h3->Fill(x, y, z, h1->GetBinContent( h1->GetXaxis()->FindBin(x),
                                           h1->GetYaxis()->FindBin(y),
                                           h1->GetZaxis()->FindBin(z) ) );
   }

   // h3 has to be filled again so that the erros are properly calculated
   r.SetSeed(seed);
   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t z = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h3->Fill(x, y, z, h2->GetBinContent( h2->GetXaxis()->FindBin(x),
                                           h2->GetYaxis()->FindBin(y),
                                           h2->GetZaxis()->FindBin(z) ) );
   }

   // No the bin contents has to be reduced, as it was filled twice!
   for ( Int_t i = 0; i <= h3->GetNbinsX() + 1; ++i ) {
      for ( Int_t j = 0; j <= h3->GetNbinsY() + 1; ++j ) {
         for ( Int_t h = 0; h <= h3->GetNbinsZ() + 1; ++h ) {
            h3->SetBinContent(i, j, h, h3->GetBinContent(i, j, h) / 2 );
         }
      }
   }

   h1->Multiply(h2);

   bool ret = equals("Multiply3D2", h3, h1, cmpOptStats, 1E-13);
   delete h2;
   delete h3;
   return ret;
}

bool testDivide1() 
{
   Double_t c1 = 1;//r.Rndm();
   Double_t c2 = 1;//r.Rndm();

   TH1D* h1 = new TH1D("d1D1-h1", "h1-Title", nbins, minRange, maxRange);
   TH1D* h2 = new TH1D("d1D1-h2", "h2-Title", nbins, minRange, maxRange);

   h1->Sumw2();h2->Sumw2();

   UInt_t seed = r.GetSeed();
   // For possible problems
   r.SetSeed(seed);
   for ( Int_t e = 0; e < nEvents; ++e ) {
      Double_t value;
      value = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(value, 1);
      value = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h2->Fill(value,  1);
   }

   TH1D* h3 = new TH1D("d1D1-h3", "h3=(c1*h1)/(c2*h2)", nbins, minRange, maxRange);
   h3->Divide(h1, h2, c1, c2);
      
   TH1D* h4 = new TH1D("d1D1-h4", "h4=h3*h2)", nbins, minRange, maxRange);
   h4->Multiply(h2, h3, 1, 1);
   for ( Int_t bin = 0; bin <= h4->GetNbinsX() + 1; ++bin ) {
      Double_t error = h1->GetBinError(bin) * h1->GetBinError(bin);
      error += 2 * h3->GetBinContent(bin)*h3->GetBinContent(bin)*h3->GetBinError(bin)*h3->GetBinError(bin);
      h4->SetBinError( bin, sqrt(error) );
   }

//    cout << h2 << endl;
//    cout << h3 << endl;

   return equals("Divide1D1", h1, h4, cmpOptStats/* | cmpOptDebug*/, 1E-13);
}

bool stressAssign1D()
{
   TH1D* h1 = new TH1D("=1D-h1", "h1-Title", nbins, minRange, maxRange);

   h1->Sumw2();

   for ( Int_t e = 0; e < nEvents; ++e ) {
      Double_t value = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(value, 1);
   }

   TH1D* h2 = new TH1D("=1D-h2", "h2-Title", nbins, minRange, maxRange);
   *h2 = *h1;

   bool ret = equals("Assign Oper '='  1D", h1, h2, cmpOptStats);
   delete h1;
   return ret;
}

bool stressCopyConstructor1D()
{
   TH1D* h1 = new TH1D("cc1D-h1", "h1-Title", nbins, minRange, maxRange);

   h1->Sumw2();

   for ( Int_t e = 0; e < nEvents; ++e ) {
      Double_t value = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(value, 1);
   }

   TH1D* h2 = new TH1D(*h1);

   bool ret = equals("Copy Constructor 1D", h1, h2, cmpOptStats);
   delete h1;
   return ret;
}

bool stressClone1D()
{
   TH1D* h1 = new TH1D("cl1D-h1", "h1-Title", nbins, minRange, maxRange);

   h1->Sumw2();

   for ( Int_t e = 0; e < nEvents; ++e ) {
      Double_t value = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(value, 1);
   }

   TH1D* h2 = static_cast<TH1D*> ( h1->Clone() );

   bool ret = equals("Clone Function   1D", h1, h2, cmpOptStats);
   delete h1;
   return ret;
}

bool stressAssign2D()
{
   TH2D* h1 = new TH2D("=2D-h1", "h1-Title", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   h1->Sumw2();

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(x, y, 1);
   }

   TH2D* h2 = new TH2D("=2D-h2", "h2-Title", 
                       nbins, minRange, maxRange, 
                       nbins, minRange, maxRange);
   *h2 = *h1;

   bool ret = equals("Assign Oper '='  2D", h1, h2, cmpOptStats);
   delete h1;
   return ret;
}

bool stressCopyConstructor2D()
{
   TH2D* h1 = new TH2D("cc2D-h1", "h1-Title", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   h1->Sumw2();

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(x, y, 1);
   }

   TH2D* h2 = new TH2D(*h1);

   bool ret = equals("Copy Constructor 2D", h1, h2, cmpOptStats);
   delete h1;
   return ret;
}

bool stressClone2D()
{
   TH2D* h1 = new TH2D("cl2D-h1", "h1-Title", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   h1->Sumw2();

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(x, y, 1);
   }

   TH2D* h2 = static_cast<TH2D*> ( h1->Clone() );

   bool ret = equals("Clone Function   2D", h1, h2, cmpOptStats);
   delete h1;
   return ret;
}

bool stressAssign3D()
{
   TH3D* h1 = new TH3D("=3D-h1", "h1-Title", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   h1->Sumw2();

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t z = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(x, y, z, 1);
   }

   TH3D* h2 = new TH3D("=3D-h2", "h2-Title", 
                       nbins, minRange, maxRange, 
                       nbins, minRange, maxRange, 
                       nbins, minRange, maxRange);
   *h2 = *h1;

   bool ret = equals("Assign Oper '='  3D", h1, h2, cmpOptStats);
   delete h1;
   return ret;
}

bool stressCopyConstructor3D()
{
   TH3D* h1 = new TH3D("cc3D-h1", "h1-Title", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   h1->Sumw2();

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t z = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(x, y, z, 1);
   }

   TH3D* h2 = new TH3D(*h1);

   bool ret = equals("Copy Constructor 3D", h1, h2, cmpOptStats);
   delete h1;
   return ret;
}

bool stressClone3D()
{
   TH3D* h1 = new TH3D("cl3D-h1", "h1-Title", 
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange,
                       nbins, minRange, maxRange);

   h1->Sumw2();

   for ( Int_t e = 0; e < nEvents * nEvents; ++e ) {
      Double_t x = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t y = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      Double_t z = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h1->Fill(x, y, z, 1);
   }

   TH3D* h2 = static_cast<TH3D*> ( h1->Clone() );

   bool ret = equals("Clone Function   3D", h1, h2, cmpOptStats);
   delete h1;
   return ret;
}

bool stressHistOpts()
{
   r.SetSeed(time(0));
   typedef bool (*pointer2Test)();
   const unsigned int numberOfTests = 22;
   pointer2Test testPointer[numberOfTests] = {  testAdd1,   testAdd2, 
                                                testAdd2D1, testAdd2D2,
                                                testAdd3D1, testAdd3D2, 
                                                testMul1,   testMul2,
                                                testMul2D1, testMul2D2,
                                                testMul3D1, testMul3D2, 
                                                testDivide1,
                                                stressAssign1D, stressCopyConstructor1D, stressClone1D,
                                                stressAssign2D, stressCopyConstructor2D, stressClone2D,
                                                stressAssign3D, stressCopyConstructor3D, stressClone3D
   };

   // Still to do: testDivide2, testDivide2D1, testDivide2D2 and
   // testDivide3D1, testDivide3D2. 

   // It depends on whether we can solve the problem with the 1D test
   // already done. It seems like there is something wrong with the
   // way the errors are being calculated. We have a formula to
   // calculate it by hand. Nevertheless the Divide method errors and
   // the ones calculated by ourselves differ a bit from those (in the
   // order of 1E-1).
   
   bool status = false;
   for ( unsigned int i = 0; i < numberOfTests; ++i )
      status |= testPointer[i]();

   return status;
}
