// @(#)root/test:$Id$
// Author: Lorenzo Moneta   06/2005 
///////////////////////////////////////////////////////////////////////////////////
//
//  MathMore Benchmark test suite
//  ==============================
//
//  This program performs tests : 
//     - mathematical functions in particular the statistical functions by estimating 
//         pdf, cdf and quantiles. 
//     - cdf are estimated directly and compared with integral calulated ones 

#include "Math/DistFunc.h"
#include "Math/IParamFunction.h"
#include "Math/Integrator.h"
#include "Math/Derivator.h"
#include "Math/Functor.h"
#include "Math/RootFinderAlgorithms.h"
#include "Math/RootFinder.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <cmath>
#include "TBenchmark.h"
#include "TROOT.h"
#include "TRandom3.h"
#include "TSystem.h"
#include "TF1.h"

using namespace ROOT::Math; 




//#define DEBUG

bool debug = true;  // print out reason of test failures
bool removeFiles = false; // remove Output root files 

void PrintTest(std::string name) { 
   std::cout << std::left << std::setw(40) << name; 
}

void PrintStatus(int iret) { 
   if (iret == 0) 
      std::cout <<"\t\t................ OK" << std::endl;
   else  
      std::cout <<"\t\t............ FAILED " << std::endl;
}


int compare( std::string name, double v1, double v2, double scale = 2.0) {
  //  ntest = ntest + 1; 

   //std::cout << std::setw(50) << std::left << name << ":\t";   
   
  // numerical double limit for epsilon 
   double eps = scale* std::numeric_limits<double>::epsilon();
   int iret = 0; 
   double delta = v2 - v1;
   double d = 0;
   if (delta < 0 ) delta = - delta; 
   if (v1 == 0 || v2 == 0) { 
      if  (delta > eps ) { 
         iret = 1; 
      }
   }
   // skip case v1 or v2 is infinity
   else { 
      d = v1; 

      if ( v1 < 0) d = -d; 
      // add also case when delta is small by default
      if ( delta/d  > eps && delta > eps ) 
         iret =  1; 
   }

   if (iret) { 
      if (debug) { 
         int pr = std::cout.precision (18);
         std::cout << "\nDiscrepancy in " << name.c_str() << "() :\n  " << v1 << " != " << v2 << " discr = " << int(delta/d/eps) 
                   << "   (Allowed discrepancy is " << eps  << ")\n\n";
         std::cout.precision (pr);
      //nfail = nfail + 1;
      }
   }
   //else  
      //  std::cout <<".";
   
   return iret; 
}

// typedef fot a free function like gamma(double x, double a, double b)
typedef double( * Func ) (double, double, double, double ); 
typedef double( * FuncQ ) (double, double, double ); 

// statistical function class 
const int NPAR = 2; 

class StatFunction : public ROOT::Math::IParamFunction { 

public: 

   StatFunction(Func pdf, Func cdf, FuncQ quant) : fPdf(pdf), fCdf(cdf), fQuant(quant) 
   {
      fScaleIg = 1; //scale for integral test
      fScaleDer = 1;  //scale for der test
      fScaleInv = 100;  //scale for der test
      for(int i = 0; i< NPAR; ++i) fParams[i]=0;
      NFuncTest = 100; 
      xlow = 0; //xup = 0; 
      xmin = 0; xmax = 1; 
      fHasLowRange = false;
   } 



   unsigned int NPar() const { return NPAR; } 
   const double * Parameters() const { return fParams; }
   ROOT::Math::IGenFunction * Clone() const { return new StatFunction(fPdf,fCdf,fQuant); }

   void SetParameters(const double * p) { std::copy(p,p+NPAR,fParams); }

   void SetParameters(double p0, double p1) { *fParams = p0; *(fParams+1) = p1; }

   void SetFuncLowRange(double x) { xlow=x; fHasLowRange = true; }
   void SetTestRange(double x1, double x2) { xmin = x1; xmax = x2; }
   void SetNTest(int n) { NFuncTest = n; }


   double Pdf(double x) const { 
      return DoEval(x); 
   }

   double Cdf(double x) const { 
      return fCdf ( x, fParams[0], fParams[1], 0. ); 
   }

   double Quantile(double x) const { 
      return fQuant( x, fParams[0], fParams[1]  ); 
   }


   // test integral with cdf function
   int TestIntegral(); 

   // test derivative from cdf to pdf function
   int TestDerivative(); 

   // test root finding algorithm for finding inverse of cdf 
   int TestInverse1(); 

   // test root finding algorithm for finding inverse of cdf using drivatives
   int TestInverse2(); 


   
   void SetScaleIg(double s) { fScaleIg = s; }  
   void SetScaleDer(double s) { fScaleDer = s; }
   void SetScaleInv(double s) { fScaleInv = s; }

   //for building a TF1
   using ROOT::Math::IParamFunction::operator();

   double operator()(const double* x, const double *)  { 
      return DoEval(*x);
   }

private: 


   double DoEval(double x) const { 
      return fPdf(x, *fParams, *(fParams+1), 0.); 
   }

   Func fPdf; 
   Func fCdf;
   FuncQ fQuant; 
   double fParams[NPAR];
   double fScaleIg;
   double fScaleDer;
   double fScaleInv;
   int NFuncTest; 
   double xmin; 
   double xmax; 
   double xlow; 
   bool fHasLowRange; 
   //double xup; 
};

// test integral of function

int StatFunction::TestIntegral() {

   int iret = 0; 

   // scan all values from xmin to xmax
   double dx = (xmax-xmin)/NFuncTest; 

   // create Integrator 
   Integrator ig(IntegrationOneDim::ADAPTIVESINGULAR, 1.E-12,1.E-12,100000);
   ig.SetFunction(*this);

   for (int i = 0; i < NFuncTest; ++i) { 
      double v1 = xmin + dx*i;  // value used  for testing
      double q1 = Cdf(v1);
      //std::cout << "v1 " << v1 << " pdf " << (*this)(v1) << " cdf " << q1 << " quantile " << Quantile(q1) << std::endl;  
      // calculate integral of pdf
      double q2 = 0; 

      // lower integral (cdf) 
      if (!fHasLowRange) 
         q2 = ig.IntegralLow(v1); 
      else 
         q2 = ig.Integral(xlow,v1); 
      
      iret = ig.Status(); 
      // use a larger scale (integral error is 10-9)
      double err = ig.Error(); 
      double scale = fScaleIg * err / std::numeric_limits<double>::epsilon();
      iret |= compare("test integral", q1, q2, scale );
      if (iret && debug)  { 
         std::cout << "Failed test for x = " << v1 << " p = "; 
         for (int j = 0; j < NPAR; ++j) std::cout << fParams[j] << "\t"; 
         std::cout << "ig error is " << err << std::endl;
      } 

   }
   return iret; 

}

int StatFunction::TestDerivative() {

   int iret = 0; 

   // scan all values from xmin to xmax
   double dx = (xmax-xmin)/NFuncTest; 
   // create CDF function
   Functor1D<> func(this, &StatFunction::Cdf);
   Derivator d(func); 

   for (int i = 0; i < NFuncTest; ++i) { 
      double v1 = xmin + dx*i;  // value used  for testing
      double q1 = Pdf(v1);
      //std::cout << "v1 " << v1 << " pdf " << (*this)(v1) << " cdf " << q1 << " quantile " << Quantile(q1) << std::endl;  
      // calculate derivative of cdf
      double q2 = d.Eval(v1); 
      iret |= d.Status();
      double err = d.Error(); 

      double scale = fScaleDer * err / std::numeric_limits<double>::epsilon();

      iret |= compare("test Derivative", q1, q2, scale );
      if (iret && debug)  { 
         std::cout << "Failed test for x = " << v1 << " p = "; 
         for (int j = 0; j < NPAR; ++j) std::cout << fParams[j] << "\t"; 
         std::cout << "der error is " << err << std::endl;
      } 

   }
   return iret; 

}

// funciton to be used in ROOT finding algorithm
struct InvFunc { 
   InvFunc(const StatFunction * f, double y) : fFunc(f), fY(y)  {}
   double operator() (double x) { 
      return fFunc->Cdf(x) - fY; 
   }
   const StatFunction * fFunc; 
   double fY; 
};

int StatFunction::TestInverse1() {

   int iret = 0; 

   // scan all values from 0 to 1
   double dx = (1.)/NFuncTest; 

   // test ROOT finder algorithm function without derivative
   RootFinder<Brent> rf1; 

   for (int i = 0; i < NFuncTest; ++i) { 
      double v1 = dx*i;  // value used  for testing
      InvFunc finv(this,v1); 
      Functor1D<> func(finv); 
      rf1.SetFunction(func, 0, 1); 
      iret = rf1.Solve(); 
      if (iret && debug) { 
         std::cout << "Error in solving for inverse" << std::endl;
         break;
      }
      double q1 = rf1.Root(); 
      // test that quantile value correspond: 
      double q2 = Quantile(v1); 
      
      iret |= compare("test Inverse1", q1, q2, fScaleInv );
      if (iret && debug)  { 
         std::cout << "Failed test for x = " << v1 << " p = "; 
         for (int j = 0; j < NPAR; ++j) std::cout << fParams[j] << "\t"; 
         std::cout << "der error is " << err << std::endl;
      } 

   }
   return iret; 

}

int StatFunction::TestInverse2() {

   int iret = 0; 

   // scan all values from 0 to 1
   double dx = (1.)/NFuncTest; 

   // test ROOT finder algorithm function with derivative
   RootFinder<Steffensson> rf1; 

   for (int i = 0; i < NFuncTest; ++i) { 
      double v1 = dx*i;  // value used  for testing
      InvFunc finv(this,v1); 
      //make a gradient function using inv function and derivative (which is pdf)
      Functor1D<ROOT::Math::IGradFunction> func(finv,*this); 
      // use as estimate the quantile at 0.5
      double xstart = Quantile(0.5); 
      rf1.SetFunction(func,xstart ); 
      iret = rf1.Solve(); 
      if (iret && debug) { 
         std::cout << "Error in solving for inverse using derivatives" << std::endl;
         break;
      }
      double q1 = rf1.Root(); 
      // test that quantile value correspond: 
      double q2 = Quantile(v1); 
      
      iret |= compare("test InverseDeriv", q1, q2, fScaleInv );
      if (iret && debug)  { 
         std::cout << "Failed test for x = " << v1 << " p = "; 
         for (int j = 0; j < NPAR; ++j) std::cout << fParams[j] << "\t"; 
         std::cout << "der error is " << err << std::endl;
      } 

   }
   return iret; 

}

// test intergal. derivative and inverse(Rootfinder) 
int testGammaFunction(int n = 100) { 

   int iret = 0; 

   StatFunction dist(gamma_pdf, gamma_cdf, gamma_quantile); 
   dist.SetNTest(n);
   dist.SetFuncLowRange(0.);
   dist.SetTestRange(0.,10.);
   // vary shape of gamma parameter
   for (int i = 1; i < 6; ++i) { 
      double k = i; 
      dist.SetParameters(k,2.);
      std::string name = "Gamma("+Util::ToString(i)+",2) ";

      PrintTest(name + "integral");
      iret |= dist.TestIntegral();

      PrintTest(name + "derivative");
      iret |= dist.TestDerivative();

      PrintTest(name + "inverse1");
      iret |= dist.TestInverse1();

      PrintTest(name + "inverse2");
      iret |= dist.TestInverse2();
   }

   return iret;
}

int stressMathMore(double nscale = 1) { 

   int iret = 0; 

#ifdef __CINT__
   std::cout << "Test must be run in compile mode - please use ACLIC !!" << std::endl; 
   return 0; 
#endif

   TBenchmark bm;
   bm.Start("stressMathMore");

   iret |= testGammaFunction(nscale*100);

   bm.Stop("stressMathMore");
   std::cout <<"******************************************************************************\n";
   bm.Print("stressMathMore");
   //const double reftime = 1.00; // ref time on  macbook pro (intel core duo 2.2 GHz)
   const double reftime = 1.17; // ref time on  imac  (intel dual core 32 bits  2. GHz)
   double rootmarks = 800 * reftime / bm.GetCpuTime("stressMathMore");
   std::cout << " ROOTMARKS = " << rootmarks << " ROOT version: " << gROOT->GetVersion() << "\t" 
             << gROOT->GetSvnBranch() << "@" << gROOT->GetSvnRevision() << std::endl;
   std::cout <<"*******************************************************************************\n";


   if (iret !=0) std::cerr << "stressMathMore Test Failed !!" << std::endl;
   return iret; 
}


int main(int argc,const char *argv[]) { 
   double nscale = 1;
   if (argc > 1) { 
      int scale = atoi(argv[1]);
      nscale = std::pow(10.0,double(scale));
   } 
   return stressMathMore(nscale);
}
