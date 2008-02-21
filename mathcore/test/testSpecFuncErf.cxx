#include <iostream>
#include <fstream>

#include <TMath.h>
#include <Math/SpecFunc.h>

// #include "SpecFuncCephes.h"

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

void testSpecFuncErf() 
{
   Double_t x[ ARRAYSIZE ];
   Double_t yerf[ ARRAYSIZE ];
   Double_t ymerf[ ARRAYSIZE ];
   Double_t yerfc[ ARRAYSIZE ];
   Double_t ymerfc[ ARRAYSIZE ];
   Double_t yierf[ ARRAYSIZE ];
   Double_t yierfc[ ARRAYSIZE ];
//    Double_t yndtri[ ARRAYSIZE ];

   ofstream outputFile ("values.txt");

   for ( double i = MIN; i < MAX; i += INCREMENT )
   {
      outputFile << "i:"; outputFile.width(5); outputFile << i 
           << " index: "; outputFile.width(5); outputFile << arrayindex(i) 
           << " TMath::Erf(x): "; outputFile.width(10); outputFile << TMath::Erf(i)
           << " ROOT::Math::erf(x): "; outputFile.width(10); outputFile << ROOT::Math::erf(i)
           << " TMath::Erfc(x): "; outputFile.width(10); outputFile << TMath::Erfc(i)
           << " ROOT::Math::erfc(x): "; outputFile.width(10); outputFile << ROOT::Math::erfc(i)
           << " TMath::ErfInverse(x): "; outputFile.width(10); outputFile << TMath::ErfInverse(i)
           << " TMath::ErfcInverse(x): "; outputFile.width(10); outputFile << TMath::ErfcInverse(i)
//            << " ROOT::Math::Cephes::ndtri(x): "; outputFile.width(10); outputFile << ROOT::Math::Cephes::ndtri(i)
           << endl;

      x[arrayindex(i)] = i;
      yerf[arrayindex(i)] = TMath::Erf(i);
      ymerf[arrayindex(i)] = ROOT::Math::erf(i);
      yerfc[arrayindex(i)] = TMath::Erfc(i);
      ymerfc[arrayindex(i)] = ROOT::Math::erfc(i);
      yierf[arrayindex(i)] = TMath::ErfInverse(i);
      yierfc[arrayindex(i)] = TMath::ErfcInverse(i);
//       yndtri[arrayindex(i)] = ROOT::Math::Cephes::ndtri(i);
   }

   TCanvas* c1 = new TCanvas("c1", "Two Graphs", 600, 400); 
   TH2F* hpx = new TH2F("hpx", "Two Graphs(hpx)", ARRAYSIZE, MIN, MAX, ARRAYSIZE, -1,2);
   hpx->SetStats(kFALSE);
   hpx->Draw();

   drawPoints(x, yerf, 14);
   drawPoints(x, ymerf, 5, 7);
   drawPoints(x, yerfc, 2);
   drawPoints(x, ymerfc, 3, 7);
//   drawPoints(x, yierf, 21);
//   drawPoints(x, yierfc, 28);
//   drawPoints(x, yndtri, 9);

   c1->Show();

   cout << "Test Done!" << endl;

   return;
}


int main(int argc, char **argv)
{
   TApplication theApp("App",&argc,argv);
   testSpecFuncErf();
   theApp.Run();

   return 0;
}
