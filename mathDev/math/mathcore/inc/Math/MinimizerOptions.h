// @(#)root/mathcore:$Id$ 
// Author: L. Moneta Fri Aug 15 2008

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2008  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

#ifndef ROOT_Math_MinimizerOptions
#define ROOT_Math_MinimizerOptions

#include <string>

namespace ROOT { 
   

   namespace Math { 

//_______________________________________________________________________________
/** 
    Minimizer options structure

    @ingroup MultiMin
*/
struct MinimizerOptions {

   // default options 
   MinimizerOptions() : 
      MinimType("Minuit2"), 
      AlgoType("Migrad"),
      Tolerance(1.E-4),
      MaxFunctionCalls(0), 
      MaxIterations(0), // 0 means leave default values 
      Strategy(1), 
      ErrorDef(1.),
      PrintLevel(0)
   {}


   std::string MinimType;   // Minimizer type (Minuit, Minuit2, etc..
   std::string AlgoType;    // Minimizer algorithmic specification (Migrag. Minimize, ...)
   double Tolerance;        // minimize tolerance to reach solution
   int MaxFunctionCalls;    // maximum number of function calls
   int MaxIterations;       // maximum number of iterations
   int Strategy;            // minimizer strategy (used by Minuit)
   double ErrorDef;         // error definition (=1. for getting 1 sigma error for chi2 fits)
   int PrintLevel;          // debug print level 

     
};

   } // end namespace Math

} // end namespace ROOT

#endif
