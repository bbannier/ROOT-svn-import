// @(#)root/mathmore:$Id$
// Authors: L. Moneta, 12/2006 

 /**********************************************************************
  *                                                                    *
  * Copyright (c) 2007 ROOT Foundation,  CERN/PH-SFT                   *
  *                                                                    *
  *                                                                    *
  **********************************************************************/

// Header file for class NagMinWorkSpace
//
// Generic adapter for gsl_multimin_function signature 
// usable for any c++ class which defines operator( ) 
// 
// Created by: Lorenzo Moneta  at Fri Nov 12 16:58:51 2004
// 
// Last update: Fri Nov 12 16:58:51 2004
// 
#ifndef ROOT_Math_NagMinWorkSpace
#define ROOT_Math_NagMinWorkSpace

#include <nag.h>
#include <nag_stdlib.h>
#include <nage04.h>
#include <nagx04.h>

#include "Math/Error.h"

namespace ROOT {
namespace Math {




  /**
     Class for adapting any multi-dimension C++ functor class to C function pointers used by 
     Nag minimization algorithm (nag_opt_solve) 

    See the 
    <A HREF="http://www.nag.co.uk/numeric/CL/nagdoc_cl08/pdf/E04/e04wdc.pdf">Nag documentation</A> for more details.

    @ingroup MultiMin

  */ 
     

   /**
      class holding the minimizer state 
   */ 
  struct NagMinWorkSpace { 

      /// constructor
      /// construct from number of dimension, number of linear and non linear constraints
      NagMinWorkSpace(unsigned int n, unsigned int nclin = 0, unsigned int ncnln = 0) : 
	a(0),
	bl(0), 
	bu(0), 
	ccon(0),
	cjac(0),
	clamda(0),
	grad(0),
	hess(0),
	x(0),
	istate(0)
    { 
      unsigned int nctotal = n + nclin + ncnln; 
      if ( !(a = NAG_ALLOC(ncnln*n, double)) ||
	   !(bl = NAG_ALLOC(nctotal, double)) ||
	   !(bu = NAG_ALLOC(nctotal, double)) ||
	   !(ccon = NAG_ALLOC(ncnln, double)) ||
	   !(cjac = NAG_ALLOC(ncnln*n, double)) ||
	   !(clamda = NAG_ALLOC(nctotal, double)) ||
	   !(grad = NAG_ALLOC(n, double)) ||
	   !(hess = NAG_ALLOC(n*n, double)) ||
	   !(x = NAG_ALLOC(n, double)) ||
	   !(istate = NAG_ALLOC(nctotal, Integer))  )
	{
	  MATH_ERROR_MSG("NagMinimizer","allocation of of nag_opt_nlp_init failed"); 
	  if (x) NAG_FREE(x);
	  x = 0; // set  to zero to flag it
	}
    }

    /// destructor - free the memory 
    ~NagMinWorkSpace() { 
      if (a) NAG_FREE(a);
      if (bl) NAG_FREE(bl);
      if (bu) NAG_FREE(bu);
      if (ccon) NAG_FREE(ccon);
      if (cjac) NAG_FREE(cjac);
      if (clamda) NAG_FREE(clamda);
      if (grad) NAG_FREE(grad);
      if (hess) NAG_FREE(hess);
      if (x) NAG_FREE(x);
      if (istate) NAG_FREE(istate);            
    }

    Nag_E04State * State() { return &nagState; } 
    
    NagError * ErrState() { return &nagErrorState; }
         
      // data members
    double *a;    //  array of linear constraints 
    double *bl;   // lower bounds
    double *bu;   // upper bounds
    double *ccon;  //  exit values of non linear constraints
    double *cjac;   // exit Jacobian matrix of non linear constraints
    double *clamda;   // on exit values of QP multipliers
    double *grad;      // on exit the gradient values
    double *hess;      // on exit the hessian values 
    double *x;         // on exit value at the minimum (entry: initial values) 
    Integer *istate; 
    //Integer majits;    // on exit major numer of iterations
    
    Nag_E04State nagState;  // internal info used by Nag
    NagError  nagErrorState;   // nag error state

  }; 




} // namespace Math
} // namespace ROOT


#endif /* ROOT_Math_NagMinWorkSpace */
