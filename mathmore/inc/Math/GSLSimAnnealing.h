// @(#)root/mathcore:$Name:  $:$Id: inc/Math/GSLSimAnnealing.h,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Thu Jan 25 11:13:48 2007

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class GSLSimAnnealing

#ifndef ROOT_Math_GSLSimAnnealing
#define ROOT_Math_GSLSimAnnealing

#include "Math/IFunctionfwd.h"

namespace ROOT { 

   namespace Math { 


/** 
   GSLSimAnnealing class for performing  a simulated annealing search of a multidimensional function

   @ingroup MultiMin
*/ 
class GSLSimAnnealing {

public: 

   /** 
      Default constructor
   */ 
   GSLSimAnnealing ();

   /** 
      Destructor (no operations)
   */ 
   ~GSLSimAnnealing ()  {}  

private:
   // usually copying is non trivial, so we make this unaccessible

   /** 
      Copy constructor
   */ 
   GSLSimAnnealing(const GSLSimAnnealing &) {} 

   /** 
      Assignment operator
   */ 
   GSLSimAnnealing & operator = (const GSLSimAnnealing & rhs)  {
      if (this == &rhs) return *this;  // time saving self-test
      return *this;
   }

public: 


   /**
      solve the simulated annealing given a multi-dim function, the initial vector parameters 
      and a vector containing the scaling factors for the parameters 
   */

   int Solve(const ROOT::Math::IMultiGenFunction & func, const double * x0, const double * scale, double * xmin, bool debug = false); 



protected: 


private: 



}; 

   } // end namespace Math

} // end namespace ROOT


#endif /* ROOT_Math_GSLSimAnnealing */
