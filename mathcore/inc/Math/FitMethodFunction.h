// @(#)root/mathcore:$Id$
// Author: L. Moneta Thu Aug 16 15:40:28 2007

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2007  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class FitMethodFunction

#ifndef ROOT_Math_FitMethodFunction
#define ROOT_Math_FitMethodFunction

#ifndef ROOT_Math_IFunction
#include "Math/IFunction.h"
#endif

namespace ROOT { 

   namespace Math { 


/** 
   FitMethodFunction class 
   Interface for objective functions (like chi2 and likelihood used in the fit)
   In addition to normal function interface provide interface for calculating each 
   data contrinution to the function which is required by some algorithm (like Fumili)

   @ingroup  CppFunctions
*/ 
template<class FunctionType>
class BasicFitMethodFunction : public FunctionType {

public:

   typedef  typename FunctionType::BaseFunc BaseFunction; 
   

   /** 
      Virtual Destructor (no operations)
   */ 
   virtual ~BasicFitMethodFunction ()  {}  

   /**
      method returning the data i-th contribution to the fit objective function
      For example the residual for the chi2 
    */
   virtual double DataElement(const double *x, unsigned int i) const = 0; 

   /**
      return the number of data points used in evaluating the function
    */
   virtual unsigned int NPoints() const = 0; 

public: 


protected: 


private: 


}; 

      // define the normal and gradient function
      typedef BasicFitMethodFunction<ROOT::Math::IMultiGenFunction>  FitMethodFunction;      
      typedef BasicFitMethodFunction<ROOT::Math::IMultiGradFunction> FitMethodGradFunction;


   } // end namespace Math

} // end namespace ROOT


#endif /* ROOT_Math_FitMethodFunction */
