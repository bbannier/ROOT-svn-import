#include <sstream>
#include <cmath>

#include "TH2.h"

#include "TApplication.h"
#include "Riostream.h"

#define ERRORLIMIT 1E-15

const unsigned int __DRAW__ = 0;

// From stressProjection.cxx
int stressProjection(bool testWithoutWeights = true,
                     bool testWithWeights = true);

// From stressRebin.cxx
bool stressRebin();

enum compareOptions {
   cmpOptDebug=1,
   cmpOptNoError=2,
   cmpOptStats=4
};

// Methods for histogram comparisions (later implemented)

int equals(const char* msg, TH1D* h1, TH1D* h2, int options = 0);
int equals(const char* msg, TH2D* h1, TH2D* h2, int options = 0);
int equals(Double_t n1, Double_t n2);
int compareStatistics( TH1* h1, TH1* h2, bool debug );

int main(int argc, char** argv)
{
   TApplication* theApp = 0;

   if ( __DRAW__ )
      theApp = new TApplication("App",&argc,argv);

   bool GlobalStatus = false;
   bool status = false;

   ostringstream output;
   output << "\nTEST RESULTS\n\n";

   cout << "\nstressProjection\n" << endl;
   status = stressProjection();
   GlobalStatus |= status;
   output << "stressProjection Test........" 
          << (status?"FAIL":"OK") << endl;

   cout << "\nstressRebin\n" << endl;
   status = stressRebin();
   GlobalStatus |= status;
   output << "stressRebin Test............."
          << (status?"FAIL":"OK") << endl;

   cout << output.str() << endl;

   if ( __DRAW__ ) {
      theApp->Run();
      delete theApp;
      theApp = 0;
   }

   return status;
}

// Methods for histogram comparisions

int equals(const char* msg, TH2D* h1, TH2D* h2, int options)
{
   bool debug = options & cmpOptDebug;
   bool compareError = ! (options & cmpOptNoError);
   bool compareStats = options & cmpOptStats;
   
   bool differents = 0;
   
   for ( int i = 0; i <= h1->GetNbinsX() + 1; ++i )
      for ( int j = 0; j <= h1->GetNbinsY() + 1; ++j )
      {
         Double_t x = h1->GetXaxis()->GetBinCenter(i);
         Double_t y = h1->GetYaxis()->GetBinCenter(j);
         
         if (debug)
         {
            cout << equals(x, h2->GetXaxis()->GetBinCenter(i)) << " "
                 << equals(y, h2->GetYaxis()->GetBinCenter(j)) << " "
                 << "[" << x << "," << y << "]: " 
                 << h1->GetBinContent(i,j) << " +/- " << h1->GetBinError(i,j) << " | "
                 << h2->GetBinContent(i,j) << " +/- " << h2->GetBinError(i,j)
                 << " | " << equals(h1->GetBinContent(i,j), h2->GetBinContent(i,j))
                 << " "   << equals(h1->GetBinError(i,j)  , h2->GetBinError(i,j) )
                 << " "   << differents
                 << " "   << (fabs(h1->GetBinContent(i,j) - h2->GetBinContent(i,j)))
                 << endl;
         }
         differents |= equals(x, h2->GetXaxis()->GetBinCenter(i));
         differents |= equals(y, h2->GetYaxis()->GetBinCenter(j));
         differents |= equals(h1->GetBinContent(i,j), h2->GetBinContent(i,j));
         if ( compareError )
            differents |= equals(h1->GetBinError(i,j)  , h2->GetBinError(i,j) );
      }
   
   // Statistical tests:
   if ( compareStats )
      differents |= compareStatistics( h1, h2, debug );
   
   cout << msg << ": \t" << (differents?"FAILED":"OK") << endl;
   
   delete h2;
   
   return differents;
}

int equals(const char* msg, TH1D* h1, TH1D* h2, int options)
{
   bool debug = options & cmpOptDebug;
   bool compareError = ! (options & cmpOptNoError);
   bool compareStats = options & cmpOptStats;
   
   bool differents = 0;
   
   for ( int i = 0; i <= h1->GetNbinsX() + 1; ++i )
   {
      Double_t x = h1->GetXaxis()->GetBinCenter(i);
      if ( debug )
      {
         cout << equals(x, h2->GetXaxis()->GetBinCenter(i))
              << " [" << x << "]: " 
              << h1->GetBinContent(i) << " +/- " << h1->GetBinError(i) << " | "
              << h2->GetBinContent(i) << " +/- " << h2->GetBinError(i)
              << " | " << equals(h1->GetBinContent(i), h2->GetBinContent(i))
              << " "   << equals(h1->GetBinError(i),   h2->GetBinError(i)  )
              << " "   << differents
              << endl;
      }
      differents |= equals(x, h2->GetXaxis()->GetBinCenter(i));
      differents |= equals(h1->GetBinContent(i), h2->GetBinContent(i));
      
      if ( compareError )
         differents |= equals(h1->GetBinError(i),   h2->GetBinError(i)  );
   }
   
   // Statistical tests:
   if ( compareStats )
      differents |= compareStatistics( h1, h2, debug );
   
   cout << msg << ": \t" << (differents?"FAILED":"OK") << endl;
   
   delete h2;
   
   return differents;      
}

int equals(Double_t n1, Double_t n2)
{
   return fabs( n1 - n2 ) > ERRORLIMIT * fabs(n1);
}

int compareStatistics( TH1* h1, TH1* h2, bool debug )
{
   bool differents = 0;
   
   differents |= (h1->Chi2Test(h2) < 1);
   differents |= (h2->Chi2Test(h1) < 1);         
   differents |= equals(h1->Chi2Test(h2), h2->Chi2Test(h1));
   if ( debug )
      cout << "Chi2Test " << h1->Chi2Test(h2) << " " << h2->Chi2Test(h1) 
           << " | " << differents
           << endl;
   
   // Mean
   differents |= equals(h1->GetMean(1), h2->GetMean(1));
   if ( debug )
      cout << "Mean: " << h1->GetMean(1) << " " << h2->GetMean(1) 
           << " | " << fabs( h1->GetMean(1) - h2->GetMean(1) ) 
           << " " << differents
           << endl;
   
   // RMS
   differents |= equals( h1->GetRMS(1), h2->GetRMS(1) );
   if ( debug )
      cout << "RMS: " << h1->GetRMS(1) << " " << h2->GetRMS(1) 
           << " | " << fabs( h1->GetRMS(1) - h2->GetRMS(1) ) 
           << " " << differents
           << endl;      
   
   return differents;
}
