// @(#)root/fit:$Id: inc/Fit/LogLikelihoodFCN.h,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Fri Aug 17 14:29:24 2007

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2007  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class LogLikelihoodFCN

#ifndef ROOT_Fit_LogLikelihoodFCN
#define ROOT_Fit_LogLikelihoodFCN

#ifndef ROOT_Math_FitMethodFunction
#include "Math/FitMethodFunction.h"
#endif

#ifndef ROOT_Math_IParamFunction
#include "Math/IParamFunction.h"
#endif

#ifndef ROOT_Fit_DataVector
#include "Fit/DataVector.h"
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
   LogLikelihoodFCN class 
   for likelihood fits 

   it is template to distinguish gradient and non-gradient case

   @ingroup  FitMethodFunc   
*/ 
template<class FunType> 
class LogLikelihoodFCN : public ROOT::Math::BasicFitMethodFunction<FunType>  {

public: 



   typedef  ROOT::Math::BasicFitMethodFunction<FunType> BaseObjFunction; 
   typedef typename  BaseObjFunction::BaseFunction BaseFunction; 

   typedef  ROOT::Math::IParamMultiFunction IModelFunction;


   /** 
      Constructor from unbin data set and model function (pdf)
   */ 
   LogLikelihoodFCN (const UnBinData & data, IModelFunction & func) : 
      fData(data), 
      fFunc(func), 
      fNDim(func.NPar() ), 
      fNPoints(data.Size()),      
      fNEffPoints(0),
      fNCalls(0), 
      fGrad ( std::vector<double> ( func.NPar() ) )
   {}
  

   /** 
      Destructor (no operations)
   */ 
   virtual ~LogLikelihoodFCN () {}

private:
   // usually copying is non trivial, so we make this unaccessible

   /** 
      Dummy Copy constructor (private)
   */ 
   LogLikelihoodFCN(const LogLikelihoodFCN &) {} 

   /** 
      Dummy Assignment operator (private)
   */ 
   LogLikelihoodFCN & operator = (const LogLikelihoodFCN & rhs) { 
      return *this;
   } 

public: 

   LogLikelihoodFCN * Clone() const { return  new LogLikelihoodFCN(fData,fFunc); }

   unsigned int NDim() const { return fNDim; }

   //using BaseObjFunction::operator();

   // count number of function calls
   unsigned int NCalls() const { return fNCalls; } 

   // size of the data
   unsigned int NPoints() const { return fNPoints; }

   // effective points used in the fit
   unsigned int NFitPoints() const { return fNEffPoints; }

   void ResetNCalls() { fNCalls = 0; }

   /// i-th likelihood contribution  
   double DataElement(const double * x, unsigned int i) const { 
      return FitUtil::EvaluatePdf(fFunc, fData, x, i); 
   }

   // need to be virtual to be instantited
   virtual void Gradient(const double *x, double *g) const { 
      // evaluate the chi2 gradient
      FitUtil::EvaluateLogLGradient(fFunc, fData, x, g, fNEffPoints);
   }


protected: 


private:

   /**
      Evaluation of the  function (required by interface)
    */
   double DoEval (const double * x) const { 
      fNCalls++;
#ifdef PARALLEL
      return FitUtilParallel::EvaluateLogL(fFunc, fData, x, fNEffPoints); 
#else 
      return FitUtil::EvaluateLogL(fFunc, fData, x, fNEffPoints); 
#endif
   } 

   // for derivatives 
   virtual double  DoDerivative(const double * x, unsigned int icoord ) const { 
      Gradient(x, &fGrad[0]); 
      return fGrad[icoord]; 
   }

 
      //data member

   const UnBinData & fData; 
   mutable IModelFunction & fFunc; 

   unsigned int fNDim; 
   unsigned int fNPoints;   // size of the data
   mutable unsigned int fNEffPoints;  // number of effective points used in the fit 
   mutable unsigned int fNCalls;

   mutable std::vector<double> fGrad; // for derivatives


}; 

   } // end namespace Fit

} // end namespace ROOT


#endif /* ROOT_Fit_LogLikelihoodFCN */
