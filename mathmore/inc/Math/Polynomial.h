// @(#)root/mathmore:$Name:  $:$Id: Polynomial.h,v 1.1 2005/09/18 17:33:47 brun Exp $
// Authors: L. Moneta, A. Zsenei   08/2005 

 /**********************************************************************
  *                                                                    *
  * Copyright (c) 2004 ROOT Foundation,  CERN/PH-SFT                   *
  *                                                                    *
  * This library is free software; you can redistribute it and/or      *
  * modify it under the terms of the GNU General Public License        *
  * as published by the Free Software Foundation; either version 2     *
  * of the License, or (at your option) any later version.             *
  *                                                                    *
  * This library is distributed in the hope that it will be useful,    *
  * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU   *
  * General Public License for more details.                           *
  *                                                                    *
  * You should have received a copy of the GNU General Public License  *
  * along with this library (see file COPYING); if not, write          *
  * to the Free Software Foundation, Inc., 59 Temple Place, Suite      *
  * 330, Boston, MA 02111-1307 USA, or contact the author.             *
  *                                                                    *
  **********************************************************************/

// Header file for class Polynomial
// 
// Created by: Lorenzo Moneta  at Wed Nov 10 17:46:19 2004
// 
// Last update: Wed Nov 10 17:46:19 2004
// 
#ifndef ROOT_Math_Polynomial
#define ROOT_Math_Polynomial

#include <complex>

#include "Math/ParamFunction.h"

namespace ROOT {
namespace Math {

  /**
     Parametric Function class describing polynomials of order n.

     <em>P(x) = p[0] + p[1]*x + p[2]*x**2 + ....... + p[n]*x**n</em>

     The class implements also the derivatives, \a dP(x)/dx and the \a dP(x)/dp[i].

     The class provides also the method to find the roots of the polynomial. 
     

     @ingroup CppFunctions
  */

  class Polynomial : virtual public ParamFunction {

  public: 
  
    /**
       Construct a Polynomial function of order n. 
       The number of Parameters is n+1. 
    */

    Polynomial(unsigned int n); 

    ~Polynomial(); 

    /**
       Copy constructor 
    */
    //Polynomial(const Polynomial &); 

    
    /**
       Copy operator 
    */
    //Polynomial & operator = (const Polynomial &);

    double operator() ( double x ); 
    
    double operator() ( double x, const std::vector<double> & p ); 

    double Gradient (double x); 

    const std::vector<double> & ParameterGradient( double x); 


    /**
       Find the polynomial roots. 
       For n < 3, the roots are found analytically while for larger order an iterative numerical method is used
       The numerical method used is from GSL (see <A HREF="http://www.gnu.org/software/gsl/manual/gsl-ref_6.html#SEC53" )
    */
    const std::vector<std::complex <double> > & FindRoots(); 

    IGenFunction * Clone() const; 
 

  protected: 


  private: 

    // cache order = number of params - 1)
    unsigned int fOrder;

    // cache Parameters for Gradient
    std::vector<double> fDerived_params;
 
    // roots

    std::vector< std::complex < double > > fRoots; 

}; 

} // namespace Math
} // namespace ROOT


#endif /* ROOT_Math_Polynomial */
