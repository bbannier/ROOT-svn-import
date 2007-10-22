// @(#)root/fit:$Id: src/FitConfig.cxx,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Thu Sep 21 16:21:29 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Implementation file for class FitConfig

#include "Fit/FitConfig.h"

#include "Math/IParamFunction.h"

#include "Math/Minimizer.h"
#include "Fit/MinimizerFactory.h"

#include <cmath> 

#include <string> 
#include <sstream> 

#include <iostream>

//#define DEBUG

namespace ROOT { 

namespace Fit { 





FitConfig::FitConfig(unsigned int npar) : 
   fSettings(std::vector<ParameterSettings>(npar) )  
{
   // constructor implementation

   // default minimizer type 
   fMinimizerType = "Minuit2"; // default 
   fMinimAlgoType = "Migrad";  // default i
}

FitConfig::~FitConfig() 
{
   // destructor implementation. No Op
}


void FitConfig::SetParameterSettings(const ROOT::Math::IParamMultiFunction & func) { 
   // set the parameters values from the function
   unsigned int npar = func.NPar(); 
   fSettings.clear(); 
   fSettings.reserve(npar); 
   const double * begin = func.Parameters(); 
   const double * end =  begin+npar; 
   unsigned int i = 0; 
   for (const double * ipar = begin; ipar !=  end; ++ipar) {  
      double val = *ipar; 
      double step = 0.3*std::fabs(val);   // step size is 30% of par value
      //double step = 2.0*std::fabs(val);   // step size is 30% of par value
      if (val ==  0) step  =  0.3; 

      fSettings.push_back( ParameterSettings(func.ParameterName(i), val, step ) ); 
#ifdef DEBUG
      std::cout << "FitConfig: add parameter " <<  func.ParameterName(i) << " val = " << val << std::endl;
#endif
      i++;
   } 

   // set default max of funciton calls according to the number of parameters
   // formula from Minuit2 (adapted)
   int maxfcn = 1000 + 100*npar + 5*npar*npar;
   fMinimizerOpts.SetMaxFunctionCalls(maxfcn); 
}

ROOT::Math::Minimizer * FitConfig::CreateMinimizer() { 
   // create minimizer according to the chosen configuration usinng the 
   // plug-in manager

   ROOT::Math::Minimizer * min = MinimizerFactory::CreateMinimizer(fMinimizerType, fMinimAlgoType); 

   if (min == 0) { 
      std::cout << "FitConfig: Could not create Minimizer " << fMinimizerType << std::endl;
      return 0;
   } 


   // set default minimizer control parameters 
   min->SetPrintLevel( fMinimizerOpts.PrintLevel() ); 
   min->SetMaxFunctionCalls( fMinimizerOpts.MaxFunctionCalls() ); 
   min->SetMaxIterations( fMinimizerOpts.MaxIterations() ); 
   min->SetTolerance( fMinimizerOpts.Tolerance() ); 



   return min; 
} 



   } // end namespace Fit

} // end namespace ROOT

