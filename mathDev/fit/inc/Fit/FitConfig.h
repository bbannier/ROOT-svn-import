// @(#)root/fit:$Id: inc/Fit/FitConfig.h,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Thu Sep 21 16:21:29 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class FitConfig

#ifndef ROOT_Fit_FitConfig
#define ROOT_Fit_FitConfig


#ifndef ROOT_Fit_ParameterSettings
#include "Fit/ParameterSettings.h"
#endif

#ifndef ROOT_Fit_MinimizerControlParams
#include "Fit/MinimizerControlParams.h"
#endif

#ifndef ROOT_Math_IParamFunctionfwd
#include "Math/IParamFunctionfwd.h"
#endif


#include <vector>

namespace ROOT { 

   namespace Math { 

      class Minimizer;
   }

   namespace Fit { 


/** 
   FitConfig class describing the configuration of the fit, options and parameter settings
   using the ROOT::Fit::ParameterSettings class 
*/ 
class FitConfig {

public: 

   /** 
      Default constructor
   */ 
   FitConfig (unsigned int npar = 0); 

   /** 
      Destructor 
   */ 
   ~FitConfig ();    

private:
   // usually copying is non trivial, so we make this unaccessible

   /** 
      Copy constructor
   */ 
   FitConfig(const FitConfig &) {} 

   /** 
      Assignment operator
   */ 
   FitConfig & operator = (const FitConfig & rhs)  {
      if (this == &rhs) return *this;  // time saving self-test
      return *this;
   }

public: 

   ///  get the parameter settings for the i-th parameter (const method)
   const ParameterSettings & ParSettings(unsigned int i) const { return fSettings[i]; }

   ///  get the parameter settings for the i-th parameter (non-const method)
   ParameterSettings & ParSettings(unsigned int i) { return fSettings[i]; }

   ///  get the vector of parameter settings  (const method)
   const std::vector<ParameterSettings> & ParamsSettings() const { return fSettings; }

   ///  get the vector of parameter settings  (non-const method)
   std::vector<ParameterSettings> & ParamsSettings() { return fSettings; }



   /// set the parameter settings from a function
   void SetParameterSettings(const ROOT::Math::IParamMultiFunction & func); 

   /// create a new minimizer according to chosen configuration
   ROOT::Math::Minimizer * CreateMinimizer(); 


   /// access to the minimizer  control parameter (const method) 
   const MinimizerControlParams & MinimizerOptions() const { return fMinimizerOpts; } 

   /// access to the minimizer  control parameter (non const method) 
   MinimizerControlParams & MinimizerOptions()  { return fMinimizerOpts; } 

   
   /// set minimizer type 
   void SetMinimizer(const std::string & type, std::string algo = "") { 
      fMinimizerType = type; 
      if (algo != "") fMinimAlgoType = algo; 
   } 

   const std::string & MinimizerType() { return fMinimizerType; } 


protected: 


private: 


   std::vector<ParameterSettings> fSettings; 

   std::string fMinimizerType;  // minimizer type (MINUIT, MINUIT2, etc..)
   std::string fMinimAlgoType;  // algorithm type (MIGRAD, SIMPLEX, etc..)
   MinimizerControlParams fMinimizerOpts; 

}; 

   } // end namespace Fit

} // end namespace ROOT


#endif /* ROOT_Fit_FitConfig */
