#include <iostream>
#include <ctime>
#include <algorithm>
#include <vector>

#include <TRandom2.h>
#include <TMath.h>
#include <TStopwatch.h>

#include <TApplication.h>
#include <TCanvas.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TLegend.h>
#include <TAxis.h>

const int npass = 2;
const int minsize = 5;
const int maxsize = 12;
const int maxint = 5000;
const int arraysize = (maxsize-minsize) + 1;

using namespace std;

// ostream& operator <<(ostream& os, const Int_t v[])
// {
//    os << '[';
//    for ( Int_t i = 0; i < n - 1; i++) {
//       os << v[i] << ' ';
//    }
//    os << v[n-1] << ']';

//    return os;
// }

void initArray(Int_t n, Int_t array[])
{
   TRandom2 r( time( 0 ) );
   for ( Int_t i = 0; i < n; i++) {
      array[i] = r.Integer( maxint ); 
   }
   sort(array, array + n);
}

bool checkPermute()
{
   Int_t n = minsize;

   Int_t original[n];
   Int_t vM[n];
   Int_t vS[n];

   bool equals = true;

   initArray(n, original);

   // TMATH
   copy(original, original + n, vM);
   copy(original, original + n, vS);
   //cout << original << vM << vS << endl;

   while ( TMath::Permute(n, vM) ) {
      std::next_permutation(vS, vS + n);
      //cout << vM << vS << endl;
      equals &= equal(vM, vM + n, vS);
   }

   TMath::Permute(n, vM);
   std::next_permutation(vS, vS+n);
   //cout << "kFALSE: " << vM << vS << endl;

   return equals;
}

void permuteTime(const int n, double* tTMath, double* tStd)
{
   Int_t original[n];
   Int_t vector[n];

   TStopwatch t; 

   initArray(n, original);

   // TMATH
   t.Start(); 
   for (int j = 0; j < npass; ++j) { 
      copy(original, original + n, vector);
      while ( TMath::Permute(n, vector) );
   }
   t.Stop(); 
   *tTMath =  t.RealTime();
   cout << "TMath::Permute time :\t " << t.RealTime() << endl;

   // STD
   t.Start();
   for (int j = 0; j < npass; ++j) { 
      copy(original, original + n, vector);
      while ( std::next_permutation(vector, vector + n) );
   }
   t.Stop();
   *tStd = t.RealTime();
   cout << "std::next_permutation time :\t " << t.RealTime() << endl;
}

void testPermute() 
{
   vector<double> tM( arraysize );
   vector<double> tS( arraysize );
   vector<double> index( arraysize );

   cout << "checkPermute()...." 
        << (checkPermute()? "OK" : "FAILED")
        << endl;

   for ( int i = minsize; i <= maxsize; i += 1)
   {
      permuteTime(i, &tM[i - minsize ], &tS[ i -minsize ]);
      index[ i - minsize ] = i;
   }

   for ( int i = minsize; i <= maxsize; i += 1)
      cout << tM[ i - minsize ] << ' ' << tS[ i - minsize ] << endl;

   TCanvas* c1 = new TCanvas("c1", "Comparision of Permutation Time", 600, 400);
   TH2F* hpx = new TH2F("hpx", "Comparision of Permutation Time", arraysize, minsize, maxsize, arraysize, tM[0],tS[arraysize-1]);
   hpx->SetStats(kFALSE);
   hpx->Draw();
   
   TGraph* gM = new TGraph(arraysize, &index[0], &tM[0]);
   gM->SetLineColor(2);
   gM->SetLineWidth(3);
   gM->SetTitle("TMath::Permute()");
   gM->Draw("SAME");

   TGraph* gS = new TGraph(arraysize, &index[0], &tS[0]);
   gS->SetLineColor(3);
   gS->SetLineWidth(3);
   gS->SetTitle("std::next_permutation()");
   gS->Draw("SAME");

   TLegend* legend = new TLegend(0.15,0.72,0.4,0.86);
   legend->AddEntry(gM, "TMath::Permute()");
   legend->AddEntry(gS, "std::next_permutation()");
   legend->Draw();

   hpx->GetXaxis()->SetTitle("Array Size");
   hpx->GetYaxis()->SetTitle("Time");
   

   c1->Show();

   cout << "Test Done!" << endl;
}

int main(int argc, char **argv)
{
   TApplication theApp("App",&argc,argv);
   testPermute();
   theApp.Run();

   return 0;
}
