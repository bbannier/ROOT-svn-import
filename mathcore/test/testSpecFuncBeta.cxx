#include <iostream>
#include <fstream>
#include <vector>

#include <TMath.h>
#include <Math/SpecFunc.h>

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

void drawPoints(Double_t x[], Double_t y[], int color, int style = 1)
{
   TGraph* g = new TGraph(ARRAYSIZE, x, y);
   g->SetLineColor(color);
   g->SetLineStyle(style);
   g->SetLineWidth(3);
   g->Draw("SAME");
}

void testSpecFuncBeta() 
{
   vector<Double_t> x( ARRAYSIZE );
   vector<Double_t> yb( ARRAYSIZE );
   vector<Double_t> ymb( ARRAYSIZE );

   TCanvas* c1 = new TCanvas("c1", "Two Graphs", 600, 400); 
   TH2F* hpx = new TH2F("hpx", "Two Graphs(hpx)", ARRAYSIZE, MIN, MAX, ARRAYSIZE, 0, 5);
   hpx->SetStats(kFALSE);
   hpx->Draw();

   int color = 2;

   for ( double b = 0.9; b < 2; b+=0.4)
   {
      cout << "** b = " << b << " **" << endl;
      for ( double i = MIN; i < MAX; i += INCREMENT )
      {
//          cout << "i:"; cout.width(5); cout << i 
//                     << " index: "; cout.width(5); cout << arrayindex(i) 
//                     << " TMath::Beta(p,b): "; cout.width(10); cout << TMath::Beta(i,b)
//                     << " ROOT::Math::beta(p,b): "; cout.width(10); cout << ROOT::Math::beta(i,b)
//                     << endl;
         
         x[arrayindex(i)] = i;
         yb[arrayindex(i)] = TMath::Beta(i,b);
         ymb[arrayindex(i)] = ROOT::Math::beta(i,b);
      }
      
      drawPoints(&x[0], &yb[0], color++);
      drawPoints(&x[0], &ymb[0], color++, 7);
   }

   c1->Show();

   cout << "Test Done!" << endl;

   return;
}

int main(int argc, char **argv) 
{
   TApplication theApp("App",&argc,argv);
   testSpecFuncBeta();
   theApp.Run();

   return 0;
}
