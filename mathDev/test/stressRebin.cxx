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
//#define __DEBUG__ 0

const unsigned int numberOfTests = 4;

TRandom2 r;
const Double_t minRange = 1;
const Double_t maxRange = 5;

const Double_t minRebin = 3;
const Double_t maxRebin = 7;

const int minBinValue = 1;
const int maxBinValue = 10;

static unsigned int canvasCounter = 1;

// extern declarations
enum compareOptions {
   cmpOptDebug=1,
   cmpOptNoError=2,
   cmpOptStats=4
};

int equals(const char* msg, TH2D* h1, TH2D* h2, int options = 0);
int equals(const char* msg, TH1D* h1, TH1D* h2, int options = 0);
int equals(Double_t n1, Double_t n2);
int compareStatistics( TH1* h1, TH1* h2, bool debug );
// extern declarations - end

ostream& operator<<(ostream& out, TH1D* h)
{
   out << h->GetName() << ": [" << h->GetBinContent(1);
   for ( Int_t i = 1; i < h->GetNbinsX(); ++i )
      out << ", " << h->GetBinContent(i);
   out << "] ";

   return out;
}

TH1D* CreateHistogram(const Int_t nbins)
{
   static unsigned int counter = 1;
   char name[10];
   sprintf(name, "h%d", counter++);

   TH1D* h1 = new TH1D(name,"Original Histogram", nbins, minRange, maxRange);
   
   for ( Int_t i = 1; i <= nbins; ++i )
      h1->SetBinContent( i, r.Uniform( minBinValue, maxBinValue ) );

   return h1;
}

void DrawHistograms(TH1D* h1, TH1D* h2)
{
   if ( __DRAW__ ) {
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
}

bool testIntegerRebin()
{
   const int rebin = TMath::Nint( r.Uniform(minRebin, maxRebin) );
   TH1D* h1 = CreateHistogram( TMath::Nint( r.Uniform(1, 5) ) * rebin );
   TH1D* h2 = static_cast<TH1D*>( h1->Rebin(rebin, "testIntegerRebin") );

   DrawHistograms(h1, h2);

   TH1D* h3 = new TH1D("testIntegerRebin2", "testIntegerRebin2", 
                       h1->GetNbinsX() / rebin, minRange, maxRange);
   for ( Int_t i = 1; i <= h3->GetNbinsX(); ++i ) {
      Double_t sum = 0;
      for ( Int_t j = 0; j < rebin; ++j )
         sum += h1->GetBinContent(rebin * i - j);
      // With TH1D::Fill method, it fails :S
      h3->SetBinContent(i, sum);
   }

   return equals("TestIntegerRebin", h2, h3);
}

bool testIntegerRebinNoName()
{
   const int rebin = TMath::Nint( r.Uniform(minRebin, maxRebin) );
   TH1D* h1 = CreateHistogram( TMath::Nint( r.Uniform(1, 5) ) * rebin );
   TH1D* h2 = dynamic_cast<TH1D*>( h1->Clone() );
   h2->Rebin(rebin);

   DrawHistograms(h1, h2);

   TH1D* h3 = new TH1D("testIntegerRebinNoName", "testIntegerRebinNoName", 
                       h1->GetNbinsX() / rebin, minRange, maxRange);
   for ( Int_t i = 1; i <= h3->GetNbinsX(); ++i ) {
      Double_t sum = 0;
      for ( Int_t j = 0; j < rebin; ++j )
         sum += h1->GetBinContent(rebin * i - j);
      // With TH1D::Fill method, it fails :S
      h3->SetBinContent(i, sum);
   }

   return equals("TestIntegerRebinNoName", h2, h3);
}

bool testArrayRebin()
{
   const int rebin = TMath::Nint( r.Uniform(minRebin, maxRebin) ) + 1;
   TH1D* h1 = CreateHistogram( TMath::Nint( r.Uniform(1, 5) ) * rebin * 2 ); 


   // Create vector 
   Double_t rebinArray[rebin];
   r.RndmArray(rebin, rebinArray);
   std::sort(rebinArray, rebinArray + rebin);
   for ( Int_t i = 0; i < rebin; ++i )
      rebinArray[i] = rebinArray[i] * (maxRange-minRange) + minRange;

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

   Int_t oldBin = 1;
   for ( Int_t bin = 1; bin <= rebin; ++bin ) {
      Double_t sum = 0;
      while ( h1->GetBinCenter(oldBin) < h3->GetXaxis()->GetBinUpEdge(bin) )
         sum += h1->GetBinContent(oldBin++);
      h3->SetBinContent(bin, sum);
   }
      
   return equals("testArrayRebin", h2, h3);
}

bool test2DRebin()
{
   Int_t xrebin = TMath::Nint( r.Uniform(minRebin, maxRebin) );
   Int_t yrebin = TMath::Nint( r.Uniform(minRebin, maxRebin) );
   TH2D* h2d = new TH2D("h2d","Original Histogram", 
                       xrebin * TMath::Nint( r.Uniform(1, 5) ), minRange, maxRange, 
                       yrebin * TMath::Nint( r.Uniform(1, 5) ), minRange, maxRange);
   
   for ( Int_t i = 1; i <= h2d->GetNbinsX(); ++i )
      for ( Int_t j = 1; j <= h2d->GetNbinsY(); ++j )
         h2d->SetBinContent( i, j, r.Uniform( minBinValue, maxBinValue ) );

   TH2D* h2d2 = (TH2D*) h2d->Rebin2D(xrebin,yrebin, "h2d2");

   if ( __DRAW__ ) {
      char name[10];
      sprintf(name, "Test %d", canvasCounter++ );
      new TCanvas(name, name);
      h2d->Draw("BOX");

      h2d2->SetLineStyle(4);
      h2d2->SetLineWidth(3);
      h2d2->Draw("BOXTEXTSAME");
   }

   TH2D* h3 = new TH2D("h3", "h3", 
                       h2d->GetNbinsX() / xrebin, minRange, maxRange,
                       h2d->GetNbinsY() / yrebin, minRange, maxRange );

   for ( Int_t x = 1; x <= h3->GetNbinsX(); ++x ) {
      for ( Int_t y = 1; y <= h3->GetNbinsY(); ++y ) {
         Double_t sum = 0;
         for ( Int_t j = 0; j < xrebin; ++j )
            for ( Int_t i = 0; i < yrebin; ++i )
               sum += h2d->GetBinContent(xrebin * x - j, yrebin * y - i);
         // With TH1D::Fill method, it fails :S
         h3->SetBinContent(x, y, sum);
      }
   }

   return equals("TestRebin2D", h2d2, h3, cmpOptDebug);
}

bool stressRebin()
{
   r.SetSeed(time(0));
   typedef bool (*pointer2Test)();
   pointer2Test testPointer[numberOfTests] = { testIntegerRebin, 
                                               testIntegerRebinNoName,
                                               testArrayRebin,
                                               test2DRebin };

   bool status = false;
   for ( unsigned int i = 0; i < numberOfTests; ++i )
      status |= testPointer[i]();

   return status;
}
