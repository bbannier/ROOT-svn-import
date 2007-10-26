// @(#)root/fit:$Id: inc/Fit/MinimizerFactory.h,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Fri Dec 22 14:43:33 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class MinimizerFactory

#ifndef ROOT_Fit_MinimizerFactory
#define ROOT_Fit_MinimizerFactory

#include <string>


namespace ROOT { 

   namespace Math { 
      class Minimizer; 
   }

   namespace Fit { 

   
/** 
   MinimizerFactory  to create all type of Minimizer
   use the Plugin Manager to create the minimizers
*/ 
struct MinimizerFactory {

   /**
      create the corrisponding Minimizer given the string
    */
   static ROOT::Math::Minimizer * CreateMinimizer(const std::string & minimizerType = "Minuit2", const std::string & algoType = "Migrad");

}; 

   } // end namespace Fit

} // end namespace ROOT


#endif /* ROOT_Fit_MinimizerFactory */
