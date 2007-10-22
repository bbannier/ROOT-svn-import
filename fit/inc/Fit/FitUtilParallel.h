// @(#)root/fit:$Id: inc/Fit/FitUtil.h,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Tue Nov 28 10:52:47 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class FitUtil

#ifndef ROOT_Fit_FitUtilParallel
#define ROOT_Fit_FitUtilParallel

#ifndef ROOT_Math_IParamFunctionfwd
#include "Math/IParamFunctionfwd.h"
#endif

#ifndef ROOT_Fit_DataVectorfwd
#include "Fit/DataVectorfwd.h"
#endif


namespace ROOT { 

   namespace Fit { 


   typedef  ROOT::Math::IParamMultiFunction IModelFunction;
   

/** 
   FitUtil namespace defining free functions using in Fit parallel mode 
*/ 
namespace FitUtilParallel {



   /** 
       evaluate the Chi2 given a model function and the data at the point x. 
       return also nPoints as the effective number of used points in the Chi2 evaluation
       Use a parallel evaluation spawning multiple threads 
   */ 
   double EvaluateChi2(IModelFunction & func, const BinData & data, const double * x, unsigned int & nPoints);  



} // end namespace FitUtil 

   } // end namespace Fit

} // end namespace ROOT


#endif /* ROOT_Fit_FitUtil */
