#include <iostream>
#include <fstream>

#include <TMath.h>
#include <Math/SpecFuncMathCore.h>

#include <TApplication.h>

#include <TCanvas.h>
#include <TH2F.h>
#include <TGraph.h>

const double MIN = -2.5;
const double MAX = +2.5;
const double INCREMENT = 0.01;
const int ARRAYSIZE = (int) (( MAX - MIN ) / INCREMENT);
inline int arrayindex(double i) { return ARRAYSIZE - (int) ( (MAX - i) / INCREMENT ) -1 ; };

using namespace std;

void drawPoints(Double_t x[ARRAYSIZE], Double_t y[ARRAYSIZE], int color, int style = 1)
{
   TGraph* g = new TGraph(ARRAYSIZE, x, y);
   g->SetLineColor(color);
   g->SetLineStyle(style);
   g->SetLineWidth(3);
   g->Draw("SAME");
}

void testSpecFuncGamma() 
{
   Double_t x[ ARRAYSIZE ];
   Double_t yg[ ARRAYSIZE ];
   Double_t ymtg[ ARRAYSIZE ];
   Double_t yga[ ARRAYSIZE ];
   Double_t ymga[ ARRAYSIZE ];
   Double_t ylng[ ARRAYSIZE ];
   Double_t ymlng[ ARRAYSIZE ];

   Double_t a = 0.56;

   //ofstream cout ("values.txt");

   for ( double i = MIN; i < MAX; i += INCREMENT )
   {
//       cout << "i:"; cout.width(5); cout << i 
//            << " index: "; cout.width(5); cout << arrayindex(i) 
//            << " TMath::Gamma(x): "; cout.width(10); cout << TMath::Gamma(i)
//            << " ROOT::Math::tgamma(x): "; cout.width(10); cout << ROOT::Math::tgamma(i)
//            << " TMath::Gamma(a, x): "; cout.width(10); cout << TMath::Gamma(a, i)
//            << " ROOT::Math::Inc_Gamma(a, x): "; cout.width(10); cout << ROOT::Math::inc_gamma(a, i)
//            << " TMath::LnGamma(x): "; cout.width(10); cout << TMath::LnGamma(i)
//            << " ROOT::Math::lgamma(x): "; cout.width(10); cout << ROOT::Math::lgamma(i)
//            << endl;

      x[arrayindex(i)] = i;
      yg[arrayindex(i)] = TMath::Gamma(i);
      ymtg[arrayindex(i)] = ROOT::Math::tgamma(i);
      yga[arrayindex(i)] = TMath::Gamma(a, i);
      ymga[arrayindex(i)] = ROOT::Math::inc_gamma(a, i);
      ylng[arrayindex(i)] = TMath::LnGamma(i);
      ymlng[arrayindex(i)] = ROOT::Math::lgamma(i);
   }

   TCanvas* c1 = new TCanvas("c1", "Two Graphs", 600, 400); 
   TH2F* hpx = new TH2F("hpx", "Two Graphs(hpx)", ARRAYSIZE, MIN, MAX, ARRAYSIZE, -1,5);
   hpx->SetStats(kFALSE);
   hpx->Draw();

   drawPoints(x, yg, 1);
   drawPoints(x, ymtg, 2, 7);
   drawPoints(x, yga, 3);
   drawPoints(x, ymga, 4, 7);
   drawPoints(x, ylng, 5);
   drawPoints(x, ymlng, 6, 7);

   c1->Show();

   cout << "Test Done!" << endl;

   return;
}


int main(int argc, char **argv) 
{
   TApplication theApp("App",&argc,argv);
   testSpecFuncGamma();
   theApp.Run();

   return 0;
}
