#include <sstream>
#include <cmath>

#include "TH2.h"
#include "TH3.h"

#include "TApplication.h"
#include "Riostream.h"
#include "TRandom2.h"

const unsigned int __DRAW__ = 0;

// From stressProjection.cxx
int stressProjection(bool testWithoutWeights = true,
                     bool testWithWeights = true);

// From stressRebin.cxx
bool stressRebin();

// From stressAdd.cxx
bool stressAdd();

enum compareOptions {
   cmpOptDebug=1,
   cmpOptNoError=2,
   cmpOptStats=4
};

TRandom2 r;

// Methods for histogram comparisions (later implemented)

int equals(const char* msg, TH1D* h1, TH1D* h2, int options = 0, double ERRORLIMIT = 1E-15);
int equals(const char* msg, TH2D* h1, TH2D* h2, int options = 0, double ERRORLIMIT = 1E-15);
int equals(const char* msg, TH3D* h1, TH3D* h2, int options, double ERRORLIMIT);
int equals(Double_t n1, Double_t n2, double ERRORLIMIT = 1E-15);
int compareStatistics( TH1* h1, TH1* h2, bool debug, double ERRORLIMIT = 1E-15);

int main(int argc, char** argv)
{
   r.SetSeed(time(0));

   TApplication* theApp = 0;

   if ( __DRAW__ )
      theApp = new TApplication("App",&argc,argv);

   bool GlobalStatus = false;
   bool status = false;

   ostringstream output;
   output << "\nTEST RESULTS\n\n";

//    cout << "\nstressProjection\n" << endl;
//    status = stressProjection();
//    GlobalStatus |= status;
//    output << "stressProjection Test........" 
//           << (status?"FAILED":"OK") << endl;

//    cout << "\nstressRebin\n" << endl;
//    status = stressRebin();
//    GlobalStatus |= status;
//    output << "stressRebin Test............."
//           << (status?"FAILED":"OK") << endl;

   cout << "\nstressAdd\n" << endl;
   status = stressAdd();
   GlobalStatus |= status;
   output << "stressAdd Test............."
          << (status?"FAILED":"OK") << endl;

   cout << output.str() << endl;

   if ( __DRAW__ ) {
      theApp->Run();
      delete theApp;
      theApp = 0;
   }

   return status;
}

ostream& operator<<(ostream& out, TH1D* h)
{
   out << h->GetName() << ": [" << h->GetBinContent(1);
   for ( Int_t i = 1; i < h->GetNbinsX(); ++i )
      out << ", " << h->GetBinContent(i);
   out << "] ";

   return out;
}

// Methods for histogram comparisions

int equals(const char* msg, TH3D* h1, TH3D* h2, int options, double ERRORLIMIT)
{
   bool debug = options & cmpOptDebug;
   bool compareError = ! (options & cmpOptNoError);
   bool compareStats = options & cmpOptStats;
   
   bool differents = 0;
   
   for ( int i = 0; i <= h1->GetNbinsX() + 1; ++i )
      for ( int j = 0; j <= h1->GetNbinsY() + 1; ++j )
         for ( int h = 0; h <= h1->GetNbinsY() + 1; ++h )
      {
         Double_t x = h1->GetXaxis()->GetBinCenter(i);
         Double_t y = h1->GetYaxis()->GetBinCenter(j);
         Double_t z = h1->GetZaxis()->GetBinCenter(h);
         
         if (debug)
         {
            cout << equals(x, h2->GetXaxis()->GetBinCenter(i), ERRORLIMIT) << " "
                 << equals(y, h2->GetYaxis()->GetBinCenter(j), ERRORLIMIT) << " "
                 << equals(z, h2->GetZaxis()->GetBinCenter(h), ERRORLIMIT) << " "
                 << "[" << x << "," << y << "," << z << "]: " 
                 << h1->GetBinContent(i,j,h) << " +/- " << h1->GetBinError(i,j,h) << " | "
                 << h2->GetBinContent(i,j,h) << " +/- " << h2->GetBinError(i,j,h)
                 << " | " << equals(h1->GetBinContent(i,j,h), h2->GetBinContent(i,j,h), ERRORLIMIT)
                 << " "   << equals(h1->GetBinError(i,j,h)  , h2->GetBinError(i,j,h),   ERRORLIMIT)
                 << " "   << differents
                 << " "   << (fabs(h1->GetBinContent(i,j,h) - h2->GetBinContent(i,j,h)))
                 << endl;
         }
         differents |= equals(x, h2->GetXaxis()->GetBinCenter(i), ERRORLIMIT);
         differents |= equals(y, h2->GetYaxis()->GetBinCenter(j), ERRORLIMIT);
         differents |= equals(z, h2->GetZaxis()->GetBinCenter(h), ERRORLIMIT);
         differents |= equals(h1->GetBinContent(i,j,h), h2->GetBinContent(i,j,h), ERRORLIMIT);
         if ( compareError )
            differents |= equals(h1->GetBinError(i,j,h)  , h2->GetBinError(i,j,h), ERRORLIMIT);
      }
   
   // Statistical tests:
   if ( compareStats )
      differents |= compareStatistics( h1, h2, debug, ERRORLIMIT);
   
   cout << msg << ": \t" << (differents?"FAILED":"OK") << endl;
   
   delete h2;
   
   return differents;
}

int equals(const char* msg, TH2D* h1, TH2D* h2, int options, double ERRORLIMIT)
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
            cout << equals(x, h2->GetXaxis()->GetBinCenter(i), ERRORLIMIT) << " "
                 << equals(y, h2->GetYaxis()->GetBinCenter(j), ERRORLIMIT) << " "
                 << "[" << x << "," << y << "]: " 
                 << h1->GetBinContent(i,j) << " +/- " << h1->GetBinError(i,j) << " | "
                 << h2->GetBinContent(i,j) << " +/- " << h2->GetBinError(i,j)
                 << " | " << equals(h1->GetBinContent(i,j), h2->GetBinContent(i,j), ERRORLIMIT)
                 << " "   << equals(h1->GetBinError(i,j)  , h2->GetBinError(i,j),   ERRORLIMIT)
                 << " "   << differents
                 << " "   << (fabs(h1->GetBinContent(i,j) - h2->GetBinContent(i,j)))
                 << endl;
         }
         differents |= equals(x, h2->GetXaxis()->GetBinCenter(i), ERRORLIMIT);
         differents |= equals(y, h2->GetYaxis()->GetBinCenter(j), ERRORLIMIT);
         differents |= equals(h1->GetBinContent(i,j), h2->GetBinContent(i,j), ERRORLIMIT);
         if ( compareError )
            differents |= equals(h1->GetBinError(i,j)  , h2->GetBinError(i,j), ERRORLIMIT);
      }
   
   // Statistical tests:
   if ( compareStats )
      differents |= compareStatistics( h1, h2, debug, ERRORLIMIT);
   
   cout << msg << ": \t" << (differents?"FAILED":"OK") << endl;
   
   delete h2;
   
   return differents;
}

int equals(const char* msg, TH1D* h1, TH1D* h2, int options, double ERRORLIMIT)
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
         cout << equals(x, h2->GetXaxis()->GetBinCenter(i), ERRORLIMIT)
              << " [" << x << "]: " 
              << h1->GetBinContent(i) << " +/- " << h1->GetBinError(i) << " | "
              << h2->GetBinContent(i) << " +/- " << h2->GetBinError(i)
              << " | " << equals(h1->GetBinContent(i), h2->GetBinContent(i), ERRORLIMIT)
              << " "   << equals(h1->GetBinError(i),   h2->GetBinError(i),   ERRORLIMIT)
              << " "   << differents
              << endl;
      }
      differents |= equals(x, h2->GetXaxis()->GetBinCenter(i), ERRORLIMIT);
      differents |= equals(h1->GetBinContent(i), h2->GetBinContent(i), ERRORLIMIT);
      
      if ( compareError )
         differents |= equals(h1->GetBinError(i),   h2->GetBinError(i), ERRORLIMIT);
   }
   
   // Statistical tests:
   if ( compareStats )
      differents |= compareStatistics( h1, h2, debug, ERRORLIMIT);
   
   cout << msg << ": \t" << (differents?"FAILED":"OK") << endl;
   
   delete h2;
   
   return differents;      
}

int equals(Double_t n1, Double_t n2, double ERRORLIMIT)
{
   return fabs( n1 - n2 ) > ERRORLIMIT * fabs(n1);
}

int compareStatistics( TH1* h1, TH1* h2, bool debug, double ERRORLIMIT)
{
   bool differents = 0;
   
   differents |= (h1->Chi2Test(h2, "WW") < 1);
   differents |= (h2->Chi2Test(h1, "WW") < 1);         
   differents |= equals(h1->Chi2Test(h2, "WW"), h2->Chi2Test(h1, "WW"), ERRORLIMIT);
   if ( debug )
      cout << "Chi2Test " << h1->Chi2Test(h2, "WW") << " " << h2->Chi2Test(h1, "WW") 
           << " | " << differents
           << endl;
   // Mean
   differents |= equals(h1->GetMean(1), h2->GetMean(1), ERRORLIMIT);
   if ( debug )
      cout << "Mean: " << h1->GetMean(1) << " " << h2->GetMean(1) 
           << " | " << fabs( h1->GetMean(1) - h2->GetMean(1) ) 
           << " " << differents
           << endl;
   
   // RMS
   differents |= equals( h1->GetRMS(1), h2->GetRMS(1), ERRORLIMIT);
   if ( debug )
      cout << "RMS: " << h1->GetRMS(1) << " " << h2->GetRMS(1) 
           << " | " << fabs( h1->GetRMS(1) - h2->GetRMS(1) ) 
           << " " << differents
           << endl;      
   
   return differents;
}
