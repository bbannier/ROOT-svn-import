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
int equals(const char* msg, TH3D* h1, TH3D* h2, int options, double ERRORLIMIT);
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

   return equals("Add1D1", h3, h4, cmpOptStats, 1E-13);
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
   
   return equals("Add1D2", h5, h6, cmpOptStats, 1E-13);
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
   return equals("Add2D1", h3, h4, cmpOptStats, 1E-12);
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
   return equals("Add2D2", h3, h1, cmpOptStats, 1E-12);
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
   return equals("Add3D1", h3, h4, cmpOptStats, 1E-12);
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
   return equals("Add3D2", h3, h1, cmpOptStats, 1E-12);
}

bool testMul1() 
{
   Double_t c1 = r.Rndm();
   Double_t c2 = r.Rndm();

   TH1D* h1 = new TH1D("m1D1-h1", "h1-Title", nbins, minRange, maxRange);
   TH1D* h2 = new TH1D("m1D1-h2", "h2-Title", nbins, minRange, maxRange);
   TH1D* h3 = new TH1D("m1D1-h3", "h3=h1*h2", nbins, minRange, maxRange);

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

   r.SetSeed(seed);
   for ( Int_t e = 0; e < nEvents; ++e ) {
      Double_t value = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h3->Fill(value,  c1*c2*h2->GetBinContent( h2->GetXaxis()->FindBin(value) ) );
   }

   for ( Int_t bin = 0; bin <= h3->GetNbinsX() + 1; ++bin ) {
      h3->SetBinContent(bin, h3->GetBinContent(bin) / 2 );
   }

   TH1D* h4 = new TH1D("m1D1-h4", "h4=h1*h2", nbins, minRange, maxRange);
   h4->Multiply(h1, h2, c1, c2);

   return equals("Multiply1D1", h3, h4, cmpOptStats, 1E-14);
}

bool testMul2() 
{
   Double_t c1 = r.Rndm();
   Double_t c2 = r.Rndm();

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
      h3->Fill(value,  c1*c2*h1->GetBinContent( h1->GetXaxis()->FindBin(value) ) );
   }

   r.SetSeed(seed);
   for ( Int_t e = 0; e < nEvents; ++e ) {
      Double_t value = r.Uniform(0.9 * minRange, 1.1 * maxRange);
      h3->Fill(value,  c1*c2*h2->GetBinContent( h2->GetXaxis()->FindBin(value) ) );
   }

   for ( Int_t bin = 0; bin <= h3->GetNbinsX() + 1; ++bin ) {
      h3->SetBinContent(bin, h3->GetBinContent(bin) / 2 );
   }

   TH1D* h4 = new TH1D("m1D2-h4", "h4=h1*h2", nbins, minRange, maxRange);
   h4->Multiply(h1, h2, c1, c2);

   return equals("Multiply1D2", h3, h4, cmpOptStats, 1E-14);
}

bool stressAdd()
{
   r.SetSeed(time(0));
   typedef bool (*pointer2Test)();
   const unsigned int numberOfTests = 8;
   pointer2Test testPointer[numberOfTests] = {  testAdd1,   testAdd2, 
                                                testAdd2D1, testAdd2D2,
                                                testAdd3D1, testAdd3D2,
                                                testMul1, testMul2 };
   
   bool status = false;
   for ( unsigned int i = 0; i < numberOfTests; ++i )
      status |= testPointer[i]();

   return status;
}
