#include <iostream>
#include <ctime>
#include <algorithm>
#include <vector>

#include <gsl/gsl_permutation.h>

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
const int arraysize = (maxsize-minsize) + 1;

using std::cout;
using std::endl;
using std::vector;

ostream& operator <<(ostream& os, const vector<Int_t>& v)
{
   os << "[ ";
   for ( vector<Int_t>::const_iterator i = v.begin(); i != v.end() ; ++i) {
      os << *i << ' ';
   }
   os << ']';

   return os;
}

template<typename _BidirectionalIterator>
bool
next_permutation(_BidirectionalIterator __first,
                 _BidirectionalIterator __last)//, vector<Int_t> v)
{

   // concept requirements
   __glibcxx_function_requires(_BidirectionalIteratorConcept<
                               _BidirectionalIterator>)
   __glibcxx_function_requires(_LessThanComparableConcept<
         typename iterator_traits<_BidirectionalIterator>::value_type>)
   __glibcxx_requires_valid_range(__first, __last);

   if (__first == __last)
      return false;
   _BidirectionalIterator __i = __first;
   ++__i;
   if (__i == __last)
     return false;
   __i = __last;
   --__i;

//    for(;;)
//    {
//       _BidirectionalIterator __ii = __i;
//       --__i;
//       if (*__i < *__ii)
//       {
//          _BidirectionalIterator __j = __last;
//          while (!(*__i < *--__j))
//          {}
//          std::iter_swap(__i, __j);
//          std::reverse(__ii, __last);
//          return true;
//       }
//       if (__i == __first)
//       {
//          std::reverse(__first, __last);
//          return false;
//       }
//    }

   //cout << "START: " << v << endl;

   _BidirectionalIterator __ii = __i;
   while ( --__i != __first )
   {
      if ( *__i < *__ii )
      {
         //cout << "CORRUPTION: " << *__ii << " " << *__i << endl;
         break;
      }
      __ii = __i;
   }

   __ii = __i;

   __i = __last;
   //--__i;

   while ( --__i != __ii )
   {
      if ( *__ii < *__i )
      {
         std::iter_swap(__i, __ii);
         std::reverse(++__ii, __last);
         return true;
      }
   }
   std::reverse(__ii, __last);
   return false;
}

void initArray(Int_t n, vector<Int_t>& array)
{
   TRandom2 r( time( 0 ) );
   for ( Int_t i = 0; i < n; i++) {
      array[i] = i;
   }
}

void permuteTime(const int n, double* tTMath, double* tStd, double* tNew)
{
   vector<Int_t> original(n);
   vector<Int_t> v(n);

   TStopwatch t; 

   initArray(n, original);

   // TMATH
   t.Start(); 
   for (int j = 0; j < npass; ++j) { 
      copy(original.begin(), original.end(), v.begin());
      while ( TMath::Permute(n, &v[0]) );
   }
   t.Stop(); 
   *tTMath =  t.RealTime();
   cout << "TMath::Permute time :\t " << t.RealTime();

   // STD
   t.Start();
   for (int j = 0; j < npass; ++j) { 
      copy(original.begin(), original.end(), v.begin());
      while ( std::next_permutation(&v[0], &v[n]) );
   }
   t.Stop();
   *tStd = t.RealTime();
   cout << "  std::next_permutation time :\t " << t.RealTime();

   // NEW
   t.Start();
   for (int j = 0; j < npass; ++j) { 
      copy(original.begin(), original.end(), v.begin());
      while ( next_permutation(&v[0], &v[n]) );
   }
   t.Stop();
   *tNew = t.RealTime();
   cout << "  new::next_permutation time :\t " << t.RealTime();
}

void stdp() 
{

//    vector<Int_t> v(5);
//    v[0] = 1; v[1] = 5; v[2] = 4; v[3] = 3; v[4] = 2;
//    cout << v << " ";
//    next_permutation(&v[0], &v[5],v);
//    cout << v << endl;

//    const Int_t n = minsize;

//    vector<Int_t> original(n);
//    vector<Int_t> vM(n);
//    vector<Int_t> vS(n);

//    bool equals = true;

//    initArray(n, original);

//    // TMATH
//    copy(original.begin(), original.end(), vM.begin());
//    copy(original.begin(), original.end(), vS.begin());
//    cout << original << vM << vS << endl;

//    while ( std::next_permutation(&vS[0], &vS[n]) ) {
//       //TMath::Permute(n, &vM[0]);
//       next_permutation(&vM[0], &vM[n], vM);
//       equals &= equal(vM.begin(), vM.end(), vS.begin());
//       cout << vM << " " << vS << " " << equals << endl;
//    }

//    std::next_permutation(vS.begin(), vS.end());
//    //TMath::Permute(n, &vM[0]);
//    next_permutation(&vM[0], &vM[n],vM);
//    equals &= equal(vM.begin(), vM.end(), vS.begin());
//    cout << "kFALSE: " << vM << vS << endl;
//    cout << equals << endl;


   vector<double> tM( arraysize );
   vector<double> tS( arraysize );
   vector<double> tN( arraysize );
   vector<double> index( arraysize );

   for ( int i = minsize; i <= maxsize; i += 1)
   {
      permuteTime(i, &tM[ i - minsize ], &tS[ i -minsize ], &tN[ i -minsize ]);
      index[ i - minsize ] = i;
      cout << endl;
   }

   for ( int i = minsize; i <= maxsize; i += 1)
      cout << tM[ i - minsize ] << ' ' << tS[ i - minsize ] << ' ' << tN[ i - minsize ] << endl;

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

   TGraph* gG = new TGraph(arraysize, &index[0], &tN[0]);
   gG->SetLineColor(4);
   gG->SetLineWidth(3);
   gG->SetTitle("new::next_permutation()");
   gG->Draw("SAME");

   TLegend* legend = new TLegend(0.15,0.72,0.4,0.86);
   legend->AddEntry(gM, "TMath::Permute()");
   legend->AddEntry(gS, "std::next_permutation()");
   legend->AddEntry(gG, "new::next_permutation()");
   legend->Draw();

   hpx->GetXaxis()->SetTitle("Array Size");
   hpx->GetYaxis()->SetTitle("Time");
   

   c1->Show();

   cout << "Test Done!" << endl;
}

int main(int argc, char **argv)
{
   TApplication theApp("App",&argc,argv);
   stdp();
   theApp.Run();

   return 0;
}
