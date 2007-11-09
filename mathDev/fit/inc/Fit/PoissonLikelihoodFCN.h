// @(#)root/fit:$Id: inc/Fit/LogLikelihoodFCN.h,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Fri Aug 17 14:29:24 2007

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2007  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class PoissonLikelihoodFCN

#ifndef ROOT_Fit_PoissonLikelihoodFCN
#define ROOT_Fit_PoissonLikelihoodFCN

#ifndef ROOT_Math_FitMethodunction
#include "Math/FitMethodFunction.h"
#endif

#ifndef ROOT_Math_IParamFunctionfwd
#include "Math/IParamFunctionfwd.h"
#endif

#ifndef ROOT_Fit_DataVectorfwd
#include "Fit/DataVectorfwd.h"
#endif

#ifndef ROOT_Fit_FitUtil
#include "Fit/FitUtil.h"
#endif
//#define PARALLEL
#ifdef PARALLEL
#ifndef ROOT_Fit_FitUtilParallel
#include "Fit/FitUtilParallel.h"
#endif
#endif

namespace ROOT { 

   namespace Fit { 


/** 
   class evaluating the log likelihood  
   for binned Poisson likelihood fits 

   @ingroup  FitMethodFunc   
*/ 
class PoissonLikelihoodFCN : public ROOT::Math::FitMethodFunction  {

public: 


   typedef  ROOT::Math::FitMethodFunction BaseObjFunction; 

   typedef  ROOT::Math::IParamMultiFunction IModelFunction;


   /** 
      Constructor from unbin data set and model function (pdf)
   */ 
   PoissonLikelihoodFCN (const BinData & data, IModelFunction & func);  

   /** 
      Destructor (no operations)
   */ 
   ~PoissonLikelihoodFCN () {}

private:
   // usually copying is non trivial, so we make this unaccessible

   /** 
      Copy constructor
   */ 
   PoissonLikelihoodFCN(const PoissonLikelihoodFCN &); 

   /** 
      Assignment operator
   */ 
   PoissonLikelihoodFCN & operator = (const PoissonLikelihoodFCN & rhs); 

public: 

   PoissonLikelihoodFCN * Clone() const; 

   unsigned int NDim() const { return fNDim; }

   using BaseObjFunction::operator();

   // count number of function calls
   unsigned int NCalls() const { return fNCalls; } 

   // size of the data
   unsigned int NPoints() const { return fNPoints; }

   // effective points used in the fit
   unsigned int NFitPoints() const { return fNEffPoints; }

   void ResetNCalls() { fNCalls = 0; }

   /// i-th element   
   double DataElement(const double * x, unsigned int i) const { 
      return FitUtil::EvaluatePoissonBinPdf(fFunc, fData, x, i); 
   }



protected: 


private:

   /**
      Evaluation of the  function (required by interface)
    */
   double DoEval (const double * x) const { 
      fNCalls++;
#ifdef PARALLEL
//      return FitUtilParallel::EvaluateLogL(fFunc, fData, x, fNEffPoints); 
#else 
      return FitUtil::EvaluatePoissonLogL(fFunc, fData, x, fNEffPoints); 
#endif
   } 
 
      //data member

   const BinData & fData; 
   mutable IModelFunction & fFunc; 

   unsigned int fNDim; 
   unsigned int fNPoints;   // size of the data
   mutable unsigned int fNEffPoints;  // number of effective points used in the fit 
   mutable unsigned int fNCalls;


}; 

   } // end namespace Fit

} // end namespace ROOT


#endif /* ROOT_Fit_PoissonLikelihoodFCN */
