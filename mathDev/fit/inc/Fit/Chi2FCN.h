// @(#)root/fit:$Id: inc/Fit/Chi2FCN.h,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Tue Sep  5 09:13:32 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class Chi2FCN

#ifndef ROOT_Fit_Chi2FCN
#define ROOT_Fit_Chi2FCN

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

/** 
@defgroup FitMethodFunc Fit Method Function Classes 
*/


namespace ROOT { 


   namespace Fit { 


/** 
   Chi2FCN class for binnned fits using the leas square methods 

   @ingroup  FitMethodFunc   
*/ 
class Chi2FCN : public ROOT::Math::FitMethodFunction {

public: 

   typedef  ROOT::Math::FitMethodFunction BaseObjFunction; 

   typedef  ROOT::Math::IParamMultiFunction IModelFunction;

   /** 
      Constructor from data set (binned ) and model function 
   */ 
   Chi2FCN (const BinData & data, IModelFunction & func); 

   /** 
      Destructor (no operations)
   */ 
   ~Chi2FCN ()  {}  

private:

   // usually copying is non trivial, so we make this unaccessible

   /** 
      Copy constructor
   */ 
   Chi2FCN(const Chi2FCN &); 

   /** 
      Assignment operator
   */ 
   Chi2FCN & operator = (const Chi2FCN & rhs); 

public: 

   Chi2FCN * Clone() const; 

   unsigned int NDim() const { return fNDim; }

   using BaseObjFunction::operator();

   // count number of function calls
   unsigned int NCalls() const { return fNCalls; } 

   // size of the data
   unsigned int NPoints() const { return fNPoints; }

   // effective points used in the fit
   unsigned int NFitPoints() const { return fNEffPoints; }

   void ResetNCalls() { fNCalls = 0; }

   /// i-th chi-square residual  
   double DataElement(const double * x, unsigned int i) const { 
      return FitUtil::EvaluateChi2Residual(fFunc, fData, x, i); 
   }

protected: 


private: 

   /**
      Evaluation of the  function (required by interface)
    */
   double DoEval (const double * x) const { 
      fNCalls++;
#ifdef PARALLEL
      return FitUtilParallel::EvaluateChi2(fFunc, fData, x, fNEffPoints); 
#else 
      return FitUtil::EvaluateChi2(fFunc, fData, x, fNEffPoints); 
#endif
   } 


   const BinData & fData; 
   mutable IModelFunction & fFunc; 

   unsigned int fNDim; 
   unsigned int fNPoints;   // size of the data
   mutable unsigned int fNEffPoints;  // number of effective points used in the fit 
   mutable unsigned int fNCalls;

}; 

   } // end namespace Fit

} // end namespace ROOT


#endif /* ROOT_Fit_Chi2FCN */
