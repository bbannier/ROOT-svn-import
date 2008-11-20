#include <cmath>
#include <ctime>

#include "TRandom2.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TMath.h"

#include "Riostream.h"

// I don't get it to work with extern declaration :S
const unsigned int __DRAW__ = 1;
//#define __DEBUG__ 1

const Double_t minRange = 1;
const Double_t maxRange = 5;

const Double_t minRebin = 3;
const Double_t maxRebin = 7;

const int minBinValue = 1;
const int maxBinValue = 10;

const int nEvents = 1000;

static unsigned int canvasCounter = 1;

// extern declarations
enum compareOptions {
   cmpOptDebug=1,
   cmpOptNoError=2,
   cmpOptStats=4
};

int equals(const char* msg, TH1D* h1, TH1D* h2, int options = 0, double ERRORLIMIT = 1E-15);
int equals(const char* msg, TH2D* h1, TH2D* h2, int options = 0, double ERRORLIMIT = 1E-15);
int equals(Double_t n1, Double_t n2, double ERRORLIMIT = 1E-15);
int compareStatistics( TH1* h1, TH1* h2, bool debug, double ERRORLIMIT = 1E-15);
ostream& operator<<(ostream& out, TH1D* h);

extern TRandom2 r;
// extern declarations - end

TH1D* CreateHistogram(const Int_t nbins, const Int_t seed)
{
   static unsigned int counter = 1;
   char name[10];
   sprintf(name, "h%d", counter++);

   TH1D* h1 = new TH1D(name,"Original Histogram", nbins, minRange, maxRange);
   
   r.SetSeed(seed);
   for ( Int_t i = 0; i < nEvents; ++i )
      h1->Fill( r.Uniform( minRange * .9 , maxRange * 1.1 ) );

   return h1;
}

void DrawHistograms(TH1D* h1, TH1D* h2)
{
   char name[10];
   sprintf(name, "Test %d", canvasCounter++ );
   new TCanvas(name, name);
   h2->SetBarWidth(0.949);
   h2->SetFillColor(4);
   h2->SetBarOffset(0.07);
   h2->SetBarWidth(0.819);
   h2->SetFillStyle(3021);
   h2->GetYaxis()->SetRangeUser(0, h2->GetMaximum() + .5 );
   h2->Draw("BAR");
   h2->Draw();
   h1->SetBarWidth(0.949);
   h1->SetFillColor(1);
   h1->GetYaxis()->SetRangeUser(0, h2->GetMaximum() + .5 );
   h1->Draw("BARSAME");
}

bool testIntegerRebin()
{
   const int rebin = TMath::Nint( r.Uniform(minRebin, maxRebin) );
   Int_t seed = 4632;//time(0);
   TH1D* h1 = CreateHistogram( TMath::Nint( r.Uniform(1, 5) ) * rebin, seed );
   TH1D* h2 = static_cast<TH1D*>( h1->Rebin(rebin, "testIntegerRebin") );

   DrawHistograms(h1, h2);

   TH1D* h3 = new TH1D("testIntegerRebin2", "testIntegerRebin2", 
                       h1->GetNbinsX() / rebin, minRange, maxRange);
   r.SetSeed(seed);
   for ( Int_t i = 0; i < nEvents; ++i )
      h3->Fill( r.Uniform( minRange * .9 , maxRange * 1.1 ) );

   return equals("TestIntegerRebin", h2, h3, cmpOptStats);
}

bool testIntegerRebinNoName()
{
   const int rebin = TMath::Nint( r.Uniform(minRebin, maxRebin) );
   Int_t seed = 4632;//time(0);
   TH1D* h1 = CreateHistogram( TMath::Nint( r.Uniform(1, 5) ) * rebin, seed );
   TH1D* h2 = dynamic_cast<TH1D*>( h1->Clone() );
   h2->Rebin(rebin);

   DrawHistograms(h1, h2);

   TH1D* h3 = new TH1D("testIntegerRebinNoName", "testIntegerRebinNoName", 
                       h1->GetNbinsX() / rebin, minRange, maxRange);
   r.SetSeed(seed);
   for ( Int_t i = 0; i < nEvents; ++i )
      h3->Fill( r.Uniform( minRange * .9 , maxRange * 1.1 ) );

   return equals("TestIntRebinNoName", h2, h3, cmpOptStats);

   // This method fails because the Chi2Test is different of 1 for
   // both of them. We need to look into both the Rebin method and the
   // Chi2Test method to understand better what is going wrong.
}

bool testArrayRebin()
{
   const int rebin = TMath::Nint( r.Uniform(minRebin, maxRebin) ) + 1;
   Int_t seed = 4632;//time(0);
   TH1D* h1 = CreateHistogram( TMath::Nint( r.Uniform(1, 5) ) * rebin * 2, seed ); 

   // Create vector 
   Double_t rebinArray[rebin];
   r.RndmArray(rebin, rebinArray);
   std::sort(rebinArray, rebinArray + rebin);
   for ( Int_t i = 0; i < rebin; ++i ) {
      rebinArray[i] = TMath::Nint( rebinArray[i] * ( h1->GetNbinsX() - 2 ) + 2 );
      rebinArray[i] = h1->GetBinLowEdge( rebinArray[i] );
   }
   

   rebinArray[0] = minRange;
   rebinArray[rebin-1] = maxRange;

   #ifdef __DEBUG__
   for ( Int_t i = 0; i < rebin; ++i ) 
      cout << rebinArray[i] << endl;
   cout << "rebin: " << rebin << endl;
   #endif

   TH1D* h2 = static_cast<TH1D*>( h1->Rebin(rebin - 1, "testArrayRebin", rebinArray) );

   DrawHistograms(h1, h2);

   TH1D* h3 = new TH1D("testArrayRebin2", "testArrayRebin2", rebin - 1, rebinArray );
   r.SetSeed(seed);
   for ( Int_t i = 0; i < nEvents; ++i )
      h3->Fill( r.Uniform( minRange * .9 , maxRange * 1.1 ) );
      
   return equals("TestArrayRebin", h2, h3, cmpOptStats);
}

bool test2DRebin()
{
   Int_t xrebin = TMath::Nint( r.Uniform(minRebin, maxRebin) );
   Int_t yrebin = TMath::Nint( r.Uniform(minRebin, maxRebin) );
   TH2D* h2d = new TH2D("h2d","Original Histogram", 
                       xrebin * TMath::Nint( r.Uniform(1, 5) ), minRange, maxRange, 
                       yrebin * TMath::Nint( r.Uniform(1, 5) ), minRange, maxRange);
   
   Int_t seed = 4632;//time(0);
   r.SetSeed(seed);
   for ( Int_t i = 0; i < nEvents; ++i )
      h2d->Fill( r.Uniform( minRange * .9 , maxRange * 1.1 ), r.Uniform( minRange * .9 , maxRange * 1.1 ) );

   TH2D* h2d2 = (TH2D*) h2d->Rebin2D(xrebin,yrebin, "h2d2");

   // Draw histograms
   char name[10];
   sprintf(name, "Test %d", canvasCounter++ );
   new TCanvas(name, name);
   h2d->Draw("BOX");
   
   h2d2->SetLineStyle(4);
   h2d2->SetLineWidth(3);
   h2d2->Draw("BOXTEXTSAME");

   TH2D* h3 = new TH2D("test2DRebin", "test2DRebin", 
                       h2d->GetNbinsX() / xrebin, minRange, maxRange,
                       h2d->GetNbinsY() / yrebin, minRange, maxRange );
   r.SetSeed(seed);
   for ( Int_t i = 0; i < nEvents; ++i )
      h3->Fill( r.Uniform( minRange * .9 , maxRange * 1.1 ), r.Uniform( minRange * .9 , maxRange * 1.1 ) );

   return equals("TestIntRebin2D", h2d2, h3, cmpOptStats);
}

bool stressRebin()
{
   typedef bool (*pointer2Test)();
   const unsigned int numberOfTests = 4;
   pointer2Test testPointer[numberOfTests] = { testIntegerRebin, 
                                               testIntegerRebinNoName,
                                               testArrayRebin,
                                               test2DRebin };

   bool status = false;
   for ( unsigned int i = 0; i < numberOfTests; ++i )
      status |= testPointer[i]();

   return status;
}
