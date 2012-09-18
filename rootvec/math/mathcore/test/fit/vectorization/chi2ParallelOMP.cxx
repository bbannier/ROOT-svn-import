
// Author: Michael N. Borinsky 
//
// Testing/Benchmark program for the parallelization
// features of fitting in ROOT (ROOT::Fitting)
// This program runs a chi2-fit on a simple histogram.
// The data in the historgram is generated with
// a gaussian pdf and the fit-function is also 
// a gaussian pdf. 
// The vdt library can be used to speed up the 
// computation.
//
// The ROOT_VECTORIZED_TF1 macro must be defined 
// in TF1.h. It indicates, that TF1 brings the 
// corresponding callback interface for vectorized 
// functions. 
//
// Additionally this program and root must be 
// compiled with openmp for the parallelized 
// version to be used.

#include <iostream>
#include <cmath>
#include <time.h>
#include <fstream>

#include "TH1.h"
#include "TF1.h"
#include "Math/WrappedMultiTF1.h"
#include "Fit/BinData.h"
#include "HFitInterface.h"
#include "Fit/Fitter.h"
#include "TPad.h"
#include "Math/DistFunc.h"

//#ifndef __INTEL_COMPILER
//#include "vdt/exp.h"
//#endif

#include <sys/times.h>
#include <unistd.h>


// some helper functions: 
void updatePercentView( int i, int iterations )
{
   if ( iterations > 100 )
   {
      if ( 0 == (i % ((iterations-1)/100)) )
      {
         std::cout << i*100 / (iterations-1) << "%\r";
         std::cout.flush();
      }
   }
   else
   {
      std::cout << i*100 / (iterations-1) << "%\r";
      std::cout.flush();
   }
}

double sqr( double x )
{
   return x*x;
}

void createFilledHistogram( TH1F** ph1f, int NBins )
{
   TH1F* h1f = new TH1F("h1f","Test random numbers", NBins, -4.0, 4.0);
   h1f->FillRandom("gaus", NBins*32);

   *ph1f = h1f;
}


// scalar version of the fit function
double myfunc_scalar( double* x, double* p )
{
   double val = *x;

   double c = p[0];
   double mu = p[1];
   double sigma = p[2];

   double z = ( val - mu ) / sigma;

   return c*std::exp( - z*z / 2.0 );
}


// vectorized version of the fit function
void myfunc_vector( int n, double const * const * ppIn, const double* p, double* pOut )
{
   double c = p[0];
   double mu = p[1];
   double sigma = p[2];

   for( int i=0; i<n;i++ )
   {
      double val = ppIn[0][i];

      double z = ( val - mu ) / sigma;
#ifdef __INTEL_COMPILER
      pOut[i] = c * std::exp( - z*z / 2.0 );
#else 
      //pOut[i] = c * vdt::fast_exp( - z*z / 2.0 );
      pOut[i] = c * std::exp( - z*z / 2.0 );
#endif
   }
}

// makeFitPerfTest
//
// Makes a fit on a generated histogram and measures the time 
// it takes to do so. 
// NBins            # of bins
// vectorized       true to use the vectorized version ( has no 
//                    effect if ROOT_VECTORIZED_TF1 is not defined! )
// pnCalls          stores the total number if calls 
// time             stores the total time the fit took

double makeFitPerfTest( int NBins, bool vectorized, unsigned long* pnCalls, double* time )
{
   TH1F* h1f;
   createFilledHistogram( &h1f, NBins );

   TF1 *f1;
#ifdef ROOT_VECTORIZED_TF1
   if( vectorized )
   {
      f1 = new TF1( "", &myfunc_vector, -10, 10, 3 );
   }
   else
#endif
   {
      f1 = new TF1( "", &myfunc_scalar, -10, 10, 3 );
   }

   ROOT::Fit::BinData d; 
   ROOT::Fit::FillData(d, h1f, f1);

   f1->SetParameters( 2.0, 0.5, 2.0 );

   ROOT::Math::WrappedMultiTF1 wf(*f1); 
   ROOT::Math::IParamMultiFunction & f = wf; 

   ROOT::Fit::Fitter fitter; 

   struct timespec time1, time2;
   clock_gettime( CLOCK_REALTIME, &time1 );

   fitter.Fit( d, f );

   clock_gettime( CLOCK_REALTIME, &time2 );

   double testDurationReal = (double)(time2.tv_sec - time1.tv_sec) + 
      1e-9*(double)(time2.tv_nsec - time1.tv_nsec);

   double chi2 = fitter.Result().Chi2(); 

   delete f1; 
   delete h1f;
   *pnCalls = fitter.Result().NCalls();

   *time = testDurationReal;

   return chi2;
}

const int statRuns = 32;

// This function performes multiple measurements on the fit time. 
// The constant statRuns defines the number of repetitions. 
// The mean value and the standard deviation of the measured 
// fit time is computed and written to cout. 
// Besides that the total number of calls is written to cout.
// Also, the fit time is written to the stream out. This stream 
// is used for output into files, which can be easily read by 
// data analysis tools.
void stressVector( std::ostream& out, int NBins, bool vectorized )
{
   double fitTimeRealSum= 0.0;
   double fitTimeRealSqrSum= 0.0;

   double callsSum = 0.0;
   double callsSumSqr = 0.0;

   double chi2Sum = 0.0;
   for ( int i=0;i<statRuns;i++ )
   {
      double testDurationReal = 0.0;
      unsigned long nCalls = 0;
      double res = makeFitPerfTest( NBins, vectorized, &nCalls, &testDurationReal );

      callsSum += (double)nCalls;
      callsSumSqr += sqr((double)nCalls);

      fitTimeRealSum+= testDurationReal;
      fitTimeRealSqrSum+= sqr(testDurationReal);

      chi2Sum+= res;
      updatePercentView( i, statRuns );
   }

   struct timespec res; 
   clock_getres( CLOCK_REALTIME, &res );
   double clockError = (double)res.tv_sec + 1e-9*(double)res.tv_nsec;

   double chi2 = chi2Sum / (double)statRuns;
   double averageCalls= callsSum / (double)statRuns;
   double averageCallsSqr=  callsSumSqr / (double)statRuns;
   double fitTimeReal= fitTimeRealSum / (double)statRuns;
   double fitTimeRealSqr= fitTimeRealSqrSum / (double)statRuns;

   double fitTimeRealError = sqrt( ( sqr(clockError) + fitTimeRealSqr - sqr(fitTimeReal))/((double)statRuns-1.0) );

   double averageCallsError = sqrt( (averageCallsSqr - sqr(averageCalls))/((double)statRuns-1.0) );  	  

   std::cout << std::scientific; 
   std::cout << "FitTime: " << fitTimeReal << " +- " << fitTimeRealError << " seconds" << std::endl;

   std::cout << "FitTimePerBin: " << fitTimeReal/(double)NBins << " seconds" << std::endl;

   std::cout << "Average Calls per Fit: " << averageCalls << " +- " << averageCallsError << " calls" << std::endl;
   std::cout << "Average Call Time: " << fitTimeReal / averageCalls << " seconds" << std::endl; 
   std::cout << "Average Call Time Per bin: " << fitTimeReal / averageCalls /(double)NBins<< " seconds" << std::endl; 
   
   std::cout << "Chi2 mean: " << chi2 << std::endl;

   out << fitTimeReal << " ";
   out << fitTimeRealError << " ";
   out << averageCalls << " ";
   out << averageCallsError << " ";


}

// calls stressVector for fitting with thread number:
// Here from 1 to 32.
//
// The results are written to the file given as the first argument or 
// to the file "result_parallel.txt" by default. 
//
int main(int argc, char *argv[])
{
   fstream outFile;
   if ( argc > 1 )
      outFile.open( argv[1], fstream::out );
   else
      outFile.open( "result_parallel.txt", fstream::out );

   outFile << "#threads ";
   outFile << "#bins ";
#ifdef ROOT_VECTORIZED_TF1
   outFile << "vecFitTime vecFitTimeErr ";
#else
   outFile << "nonVecFitTime nonVecFitTimeErr ";
#endif

   outFile << std::endl;

   const int maxThreads= 32;
   for( int j=1; j <= maxThreads; j++ )
   {
#ifdef _OPENMP
      omp_set_num_threads( j );
#else
	#error You have to compile root with -fopenmp
#endif
      const int MaxBins = 1 << 16; 
      int NBins = MaxBins;

      std::cout << "Performance Test with: " << NBins << " and " << j << " Threads " << std::endl;

      outFile << j << " ";
      outFile << NBins << " ";
#ifdef ROOT_VECTORIZED_TF1
      std::cout << "Vectorized version: " << std::endl;
      stressVector( outFile, NBins, true );
      std::cout << std::endl;
#else
      std::cout << "Non-Vectorized version: " << std::endl;
      stressVector( outFile, NBins, false );
      std::cout << std::endl;
#endif

      outFile << std::endl;
   }
   outFile.close();

   return 0;
}
