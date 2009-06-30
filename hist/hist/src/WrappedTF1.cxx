// @(#)root/mathmore:$Id$
// Author: L. Moneta Wed Sep  6 09:52:26 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// implementation file for class WrappedTF1 and WrappedMultiTF1

#include "Math/WrappedTF1.h"
#include "Math/WrappedMultiTF1.h"

#include <cmath>


namespace ROOT { 
   
namespace Math { 

// static value for epsilon used in derivative calculations
double WrappedTF1::fgEps = 0.001; 


WrappedTF1::WrappedTF1 ( TF1 & f  )  : 
   fLinear(false), 
   fPolynomial(false),
   fFunc(&f), 
   fParams(f.GetParameters(),f.GetParameters()+f.GetNpar())
{
   // constructor from a TF1 function pointer.
   
   // init the pointers for CINT
   if (fFunc->GetMethodCall() )  fFunc->InitArgs(fX, &fParams.front() );
   // distinguish case of polynomial functions and linear functions
   if (fFunc->GetNumber() >= 300 && fFunc->GetNumber() < 310) { 
      fLinear = true; 
      fPolynomial = true; 
   }
   // check that in case function is linear the linear terms are not zero
   if (fFunc->IsLinear() ) { 
      unsigned int ip = 0; 
      fLinear = true;
      while (fLinear && ip < fParams.size() )  { 
         fLinear &= (fFunc->GetLinearPart(ip) != 0) ; 
         ip++;
      }
   }      
}

WrappedTF1::WrappedTF1(const WrappedTF1 & rhs) :
   BaseFunc(),
   BaseGradFunc(),
   fLinear(rhs.fLinear), 
   fPolynomial(rhs.fPolynomial),
   fFunc(rhs.fFunc), 
   fParams(rhs.fParams)
{
   // copy constructor
   fFunc->InitArgs(fX,&fParams.front()  );
}
   
WrappedTF1 & WrappedTF1::operator = (const WrappedTF1 & rhs) { 
   // assignment operator 
   if (this == &rhs) return *this;  // time saving self-test
   fLinear = rhs.fLinear;  
   fPolynomial = rhs.fPolynomial; 
   fFunc = rhs.fFunc; 
   fFunc->InitArgs(fX, &fParams.front() );
   fParams = rhs.fParams;
   return *this;
} 

void  WrappedTF1::ParameterGradient(double x, const double * par, double * grad ) const {
   // evaluate the derivative of the function with respect to the parameters 
   if (!fLinear) { 
      // need to set parameter values
      fFunc->SetParameters( par );
      // no need to call InitArgs (it is called in TF1::GradientPar)
      fFunc->GradientPar(&x,grad,fgEps);
   }
   else { 
      unsigned int np = NPar();
      for (unsigned int i = 0; i < np; ++i) 
         grad[i] = DoParameterDerivative(x, par, i);
   }
}

double WrappedTF1::DoDerivative( double  x  ) const { 
   // return the function derivatives w.r.t. x 

   // parameter are passed as non-const in Derivative
   double * p =  (fParams.size() > 0) ? const_cast<double *>( &fParams.front()) : 0;
   return  fFunc->Derivative(x,p,fgEps); 
}
   
double WrappedTF1::DoParameterDerivative(double x, const double * p, unsigned int ipar ) const { 
   // evaluate the derivative of the function with respect to the parameters

   // not very efficient - use ParameterGradient
   if (! fLinear ) {  
      fFunc->SetParameters( p );
      return fFunc->GradientPar(ipar, &x,fgEps);
   }
   else if (fPolynomial) { 
      // case of polynomial function (no parameter dependency)  
      return std::pow(x, static_cast<int>(ipar) );  
   }
   else { 
      // case of general linear function (bbuilt with ++ )
      const TFormula * df = dynamic_cast<const TFormula*>( fFunc->GetLinearPart(ipar) );
      assert(df != 0); 
      fX[0] = x; 
      // hack since TFormula::EvalPar is not const
      return (const_cast<TFormula*> ( df) )->EvalPar( fX ) ; // derivatives should not depend on parameters since func is linear 
   }
}

void WrappedTF1::SetDerivPrecision(double eps) { fgEps = eps; }

double WrappedTF1::GetDerivPrecision( ) { return fgEps; }



// impelmentations for WrappedMultiTF1


WrappedMultiTF1::WrappedMultiTF1 (TF1 & f, unsigned int dim  )  : 
   fFunc(&f),
   fDim(dim),
   fParams(f.GetParameters(),f.GetParameters()+f.GetNpar())
{ 
   // constructor of WrappedMultiTF1 
   // pass a dimension if dimension specified in TF1 does not correspond to real dimension
   // for example in case of multi-dimensional TF1 objects defined as TF1 (i.e. for functions with dims > 3 )
   if (fDim == 0) fDim = fFunc->GetNdim(); 
}


WrappedMultiTF1::WrappedMultiTF1(const WrappedMultiTF1 & rhs) :
   BaseFunc(),
   BaseParamFunc(),
   fFunc(rhs.fFunc),
   fDim(rhs.fDim),
   fParams(rhs.fParams) 
{
   // copy constructor 
}


WrappedMultiTF1 & WrappedMultiTF1::operator= (const WrappedMultiTF1 & rhs) { 
   // Assignment operator
   if (this == &rhs) return *this;  // time saving self-test
   fFunc = rhs.fFunc; 
   fDim = rhs.fDim;
   fParams = rhs.fParams;
   return *this;
} 



} // end namespace Fit

} // end namespace ROOT


