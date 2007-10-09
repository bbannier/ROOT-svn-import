// @(#)root/mathcore:$Id$
// Author: Magdalena Slawinska 10/2007


/**********************************************************************
 *                                                                    *
 * Copyright (c) 2007  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class Minimizer

#ifndef ROOT_Math_VirtualIntegrator
#define ROOT_Math_VirtualIntegrator

#ifndef ROOT_Math_IFunctionfwd
#include "Math/IFunctionfwd.h"
#endif

#ifndef ROOT_Math_Error
#include "Math/Error.h"
#endif



namespace ROOT {
namespace Math {


/**
   Interface for numerical integration
   in one and many dimensions

   @ingroup  Integration

*/
class VirtualIntegrator{

public:

   // destructor: no operation
   virtual ~VirtualIntegrator() {}

   //1-dim integration
   virtual double Integrate(double, double) {
      MATH_ERROR_MSG("cannot call VirtualIntegrator::Integrate");
      return 0;
   }


   //multi-dim integration
   virtual double Integrate(unsigned int , const double*, const double*) { 
      MATH_ERROR_MSG("cannot call VirtualIntegrator::Integrate");
      return 0;
   }


   //setting a 1-dim function
   virtual void SetFunction(const IGenFunction &) {}
   
   //setting a multi-dim function
   virtual void SetFunction(const IMultiGenFunction &) {}
   
};


}//namespace Math
}//namespace ROOT


#endif /* ROOT_Math_VirtualIntegrator */
