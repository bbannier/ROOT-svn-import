#include "Math/Integrator.h"
#include "Math/IntegratorMultiDim.h"
#include "Math/AllIntegrationTypes.h"
#include "Math/Functor.h"
#include "Math/GaussIntegrator.h"
#include "RooRealVar.h"
#include "RooCFunction1Binding.h"
#include "RooRealBinding.h"
#include "RooIntegrator1D.h"
#include "RooAbsReal.h"
#include "RooNumIntConfig.h"
#include "RooAdaptiveGaussKronrodIntegrator1D.h"

#include <cmath>

using namespace RooFit;

const double ERRORLIMIT = 1E-3;
long calls = 0;

double f(double x) { 
   calls++;
   return exp(-x)*pow(x,3)/6; 
} 

double f2(const double * x) { 
   return x[0] + x[1]; 
} 

void printTestResult(std::ostream & os, const char * type, int status) { 
   os << "Test of " << type  << "\t: \t"; 
   if (!status)       os << "OK" << std::endl;
   else               os << "FAILED";
   os << std::endl << std::endl;
}


int testIntegration1D() { 

   const double RESULT = 13.8155;
   int status = 0;
   
   const Double_t lower = 0;
   const Double_t upper = 300;


   ROOT::Math::Functor1D wf(&f);

   ROOT::Math::Integrator ig(ROOT::Math::IntegrationOneDim::kADAPTIVESINGULAR); 
   ig.SetFunction(wf);
   double val = ig.Integral(lower,upper);
   std::cout << "GSL adaptive sing. integral result is \t" << val << " in " << calls << " calls" << std::endl;
   status += std::fabs(val-RESULT) > ERRORLIMIT;
   calls = 0;

   ROOT::Math::Integrator ig2(ROOT::Math::IntegrationOneDim::kNONADAPTIVE); 
   ig2.SetFunction(wf);
   val = ig2.Integral(lower,upper);
   std::cout << "GSL non-adaptiveintegral result is    \t" << val << " in " << calls << " calls" << std::endl;
   //status += std::fabs(val-RESULT) > ERRORLIMIT;
   calls = 0;

   ROOT::Math::Integrator ig3(wf, ROOT::Math::IntegrationOneDim::kADAPTIVE); 
   val = ig3.Integral(lower,upper);
   std::cout << "GSL adaptive integral result is       \t" << val << " in " << calls << " calls" <<  std::endl;
   status += std::fabs(val-RESULT) > ERRORLIMIT;
   calls = 0;

   //ROOT::Math::GaussIntegrator ig4;
   ROOT::Math::Integrator ig4(ROOT::Math::IntegrationOneDim::kGAUSS); 
   ig4.SetFunction(wf);
   val = ig4.Integral(lower,upper);
   std::cout << "Cernlib Gauss integral result is      \t" << val << " in " << calls << " calls" << std::endl;
   status += std::fabs(val-RESULT) > ERRORLIMIT;
   calls = 0;

   ROOT::Math::Integrator ig5(ROOT::Math::IntegrationOneDim::kLEGENDRE); 
   ig5.SetFunction(wf);
   val = ig5.Integral(lower,upper);
   std::cout << "Gauss-Legendre integral result is     \t" << val << " in " << calls << " calls" << std::endl;
   //status += std::fabs(val-RESULT) > ERRORLIMIT;
   calls = 0;

   //std::cout << RooAbsReal::defaultIntegratorConfig()->getConfigSection("RooIntegrator1D") << std::endl;
   RooRealVar x("x", "x", lower, upper);
   RooCFunction1Binding<Double_t, Double_t> fAbsReal("f", "f", &f, x);
   RooRealBinding fAbsFunc(fAbsReal,x);

   RooNumIntConfig *config = (RooNumIntConfig *)RooAbsReal::defaultIntegratorConfig()->Clone();
   config->getConfigSection("RooIntegrator1D").setRealValue("maxSteps", 30);
   std::cout << config->getConfigSection("RooIntegrator1D").getRealValue("maxSteps") << endl;
   
   RooIntegrator1D ig6(fAbsFunc, lower, upper, *RooAbsReal::defaultIntegratorConfig());  

   val = ig6.integral();
   std::cout << "RooIntegrator1D integral result is    \t" << val << " in " << calls << " calls" << std::endl;
   status += std::fabs(val-RESULT) > ERRORLIMIT;

   printTestResult(std::cerr,"one-dimensional integration  ",status);
   return status;
}

int testIntegrationMultiDim() { 

   const double RESULT = 1.0;
   int status = 0;

   ROOT::Math::Functor wf(&f2,2);
   double a[2] = {0,0};
   double b[2] = {1,1};

   ROOT::Math::IntegratorMultiDim ig(ROOT::Math::IntegrationMultiDim::kADAPTIVE); 
   ig.SetFunction(wf);
   double val = ig.Integral(a,b);
   std::cout << "Cernlib Adaptive integral result is " << val << std::endl;
   status += std::fabs(val-RESULT) > ERRORLIMIT;

   ROOT::Math::IntegratorMultiDim ig2(ROOT::Math::IntegrationMultiDim::kVEGAS); 
   ig2.SetFunction(wf);
   val = ig2.Integral(a,b);
   std::cout << "GSL VEGAS integral result is       " << val << std::endl;
   status += std::fabs(val-RESULT) > ERRORLIMIT;

   ROOT::Math::IntegratorMultiDim ig3(wf,ROOT::Math::IntegrationMultiDim::kPLAIN); 
   val = ig3.Integral(a,b);
   std::cout << "GSL PLAIN  integral result is      " << val << std::endl;
   status += std::fabs(val-RESULT) > ERRORLIMIT;

   ROOT::Math::IntegratorMultiDim ig4(wf,ROOT::Math::IntegrationMultiDim::kMISER); 
   val = ig4.Integral(a,b);
   std::cout << "GSL MISER integral result is       " << val << std::endl;
   status += std::fabs(val-RESULT) > ERRORLIMIT;

   printTestResult(std::cerr,"multi-dimensional integration",status);

   return status;
}

int  main() { 
   int status = 0;

   status += testIntegration1D();
   status += testIntegrationMultiDim();

   return status;
}

