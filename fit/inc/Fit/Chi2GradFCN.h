// @(#)root/fit:$Id: inc/Fit/Chi2GradFCN.h,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Tue Nov 28 10:53:06 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class Chi2GradFCN

#ifndef ROOT_Fit_Chi2GradFCN
#define ROOT_Fit_Chi2GradFCN

#ifndef ROOT_Math_IFunction
#include "Math/IFunction.h"
#endif


#ifndef ROOT_Fit_DataVectorfwd
#include "Fit/DataVectorfwd.h"
#endif


#ifndef ROOT_Fit_FitUtil
#include "Fit/FitUtil.h"
#endif

#ifndef ROOT_Math_IParamFunction
#include "Math/IParamFunction.h"
#endif

#include <vector>

namespace ROOT { 

   namespace Fit { 


/** 
   Chi2GradFCN class for chi2 function with analytical derivatives

   @ingroup FitMethodFunction
*/ 
class Chi2GradFCN :  public ROOT::Math::IMultiGradFunction {

public: 

   typedef  ROOT::Math::IMultiGradFunction BaseObjFunction; 

   typedef  ROOT::Math::IParamMultiFunction IModelFunction;
   typedef  ROOT::Math::IParamMultiGradFunction IGradModelFunction;


   /** 
      Default constructor
   */ 
   Chi2GradFCN (const BinData & data, IGradModelFunction & func); 

   /** 
      Destructor (no operations)
   */ 
   ~Chi2GradFCN () {} 

private:
   // usually copying is non trivial, so we make this unaccessible

   /** 
      Copy constructor
   */ 
   Chi2GradFCN(const Chi2GradFCN &); 

   /** 
      Assignment operator
   */ 
   Chi2GradFCN & operator = (const Chi2GradFCN & rhs); 

public: 

   Chi2GradFCN * Clone() const; 

   unsigned int NDim() const { return fNDim; }

   using BaseObjFunction::operator();

   void Gradient(const double * p, double * g ) const; 

private: 


   double DoEval (const double * x) const { 
      return FitUtil::EvaluateChi2( fFunc, fData, x, fNPoints); 
   } 

   double  DoDerivative(const double * x, unsigned int icoord ) const { 
      Gradient(x, &fGrad[0]); 
      return fGrad[icoord]; 
   }


   const BinData & fData; 
   mutable IGradModelFunction & fFunc; 

   mutable unsigned int fNPoints; 
   mutable unsigned int fNDim; 
   mutable std::vector<double> fGrad; 


}; 

   } // end namespace Fit

} // end namespace ROOT


#endif /* ROOT_Fit_Chi2GradFCN */
