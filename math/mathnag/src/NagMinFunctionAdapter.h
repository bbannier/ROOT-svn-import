// @(#)root/mathmore:$Id$
// Authors: L. Moneta, 12/2006 

 /**********************************************************************
  *                                                                    *
  * Copyright (c) 2007 ROOT Foundation,  CERN/PH-SFT                   *
  *                                                                    *
  *                                                                    *
  **********************************************************************/

// Header file for class NagMinFunctionAdapter
//
// Generic adapter for gsl_multimin_function signature 
// usable for any c++ class which defines operator( ) 
// 
// Created by: Lorenzo Moneta  at Fri Nov 12 16:58:51 2004
// 
// Last update: Fri Nov 12 16:58:51 2004
// 
#ifndef ROOT_Math_NagMinFunctionAdapter
#define ROOT_Math_NagMinFunctionAdapter

#include <nag.h>
#include <nag_stdlib.h>
#include <nage04.h>
#include <nagx04.h>

#include "Math/IFunction.h"
#include "Math/Minimizer.h"

#include <cassert>

namespace ROOT {
namespace Math {




  /**
     Class for adapting any multi-dimension C++ functor class to C function pointers used by 
     Nag minimization algorithm (nag_opt_solve) 

    See the 
    <A HREF="http://www.nag.co.uk/numeric/CL/nagdoc_cl08/pdf/E04/e04wdc.pdf:>Nag documentation</A> for more details.

    @ingroup MultiMin

  */ 
     

  template<class UserFunc> 
  struct  NagMinFunctionAdapter {

     // case of function not providing gradient calculation
     // not supported now 
     static void ObjFun( Integer * mode, Integer /* n */, const double x[], double *objf, double [] /* grad[] */, 
                         Integer /* nstate */ , Nag_Comm * comm) 
     { 

        // p is the void * pointer stored in the Nag_comm structure
        UserFunc * function = reinterpret_cast< UserFunc *> (comm->p);
        assert(function != 0); 

        if ( *mode == 0 || *mode == 2) {  
         // evaluate the function 
           *objf =  (*function)( x ); 
        }
    }
  }; 
   // specialize structure for the gradient calculation
  template<> 
  struct  NagMinFunctionAdapter<ROOT::Math::IMultiGradFunction> {

     // case of function providing gradient calculation      
     static void ObjFun( Integer * mode, Integer /* n */, const double x[], double *objf, double grad[], 
                         Integer /* nstate */, Nag_Comm * comm) 
     { 
        // p is the void * pointer stored in the Nag_comm structure
        ROOT::Math::IMultiGradFunction * function = reinterpret_cast<ROOT::Math::IMultiGradFunction *> (comm->p);
        assert(function != 0); 

        if (*mode == 0 ) {  
           // evaluate only the function 
           *objf =  (*function)( x ); 
        }
        else if (*mode == 1 ) {  
            // evaluate only the gradient           
           (*function).Gradient( x , grad );
        }
        else if (*mode == 2 ) {  
            // evaluate function and  the gradient           
           (*function).FdF( x , *objf, grad); 
        }

     }
     

  };


} // namespace Math
} // namespace ROOT


#endif /* ROOT_Math_NagMinFunctionAdapter */
