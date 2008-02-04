// @(#)root/mathcore:$Id$
// Author: David Gonzalez Maline 2/2008
 /**********************************************************************
  *                                                                    *
  * Copyright (c) 2004 Maline,  CERN/PH-SFT                            *
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

// Header file for class Minimizer1D
// 
// Created by: Maline  at Fri Feb  1 11:30:26 2008
// 

#ifndef ROOT_Math_IMinimizer1D
#define ROOT_Math_IMinimizer1D

/**
   @defgroup Min1D One-dimensional Minimization Interface
   @ingroup NumAlgo
 */

namespace ROOT { 
namespace Math { 

//    namespace Minim1D {
      
//       enum Type { GOLDENSECTION, 
//                   BRENT
//       };
//    }
   
   class IMinimizer1D {
      
   public: 
      
      IMinimizer1D() {} 
      virtual ~IMinimizer1D() {} 
      
   public: 
      
      //virtual int Iterate() = 0;  // Only for GSLMethods
      virtual double XMinimum() const = 0; 
      virtual double XLower() const = 0; 
      virtual double XUpper() const = 0; 
      virtual double FValMinimum() const = 0; 
      virtual double FValLower() const = 0; 
      virtual double FValUpper() const = 0; 
      virtual int Minimize( int maxIter, double absTol, double relTol) = 0; 
      virtual int Iterations() const = 0;

      virtual const char * Name() const = 0;  

   };  // end class IMinimizer1D
   
} // end namespace Math
   
} // end namespace ROOT

#endif /* ROOT_Math_IMinimizer1D */
