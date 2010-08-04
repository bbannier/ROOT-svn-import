// @(#)root/mathcore:$Id$
// Authors: David Gonzalez Maline    01/2008 

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006 , LCG ROOT MathLib Team                         *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

#include "Math/GaussIntegrator.h"
#include "Math/WrappedFunction.h"
#include <cmath>
#include <limits>
#include <iostream>

#define disp(var) std::cout << #var << ": " << var << std::endl;

namespace ROOT {
namespace Math {

bool GaussIntegrator::fgAbsValue = false;

GaussIntegrator::GaussIntegrator()
{
// Default Constructor.

   fEpsilon = 1e-12;
   fLastResult = fLastError = 0;
   fUsedOnce = false;
   fFunction = 0;
}

GaussIntegrator::~GaussIntegrator()
{
   // Destructor. (no - operations)
}

void GaussIntegrator::AbsValue(bool flag)
{   fgAbsValue = flag;  }

double GaussIntegrator::Integral (double a, double b) {
   return DoIntegral(a, b);
}

// double GaussIntegrator::Integral (const IGenFunction& mapping, double a, double b) {
//    const IGenFunction* transformation = &mapping;
//    a = (*transformation)(a);
//    b = (*transformation)(b);
//    return DoIntegral(a, b, transformation);
// }

// double GaussIntegrator::Integral (const IGenFunction& mapping) { // Mind that the mapping has to tranform (-inf, inf) interval strictly to the limited interval (a,b)
//    const IGenFunction* transformation = &mapping;
//    double a = -1. * std::numeric_limits<Double_t>::infinity();
//    double b = std::numeric_limits<Double_t>::infinity();
//    return DoIntegral (a, b, transformation);  
// }

double GaussIntegrator::Integral () {
   ROOT::Math::IGenFunction* transformation = new WrappedMemFunction<GaussIntegrator, double (GaussIntegrator::*)(double)>(*this, &GaussIntegrator::DefaultMapping);
   disp("GaussIntegrator::Integral ()");
   return DoIntegral(0., 1., transformation);  
}

double GaussIntegrator::DoIntegral (double a, double b, const IGenFunction* mapping)
{
   //  Return Integral of function between mapping(a) and mapping(b).

   const double kHF = 0.5;
   const double kCST = 5./1000;

   double x[12] = { 0.96028985649753623,  0.79666647741362674,
                      0.52553240991632899,  0.18343464249564980,
                      0.98940093499164993,  0.94457502307323258,
                      0.86563120238783174,  0.75540440835500303,
                      0.61787624440264375,  0.45801677765722739,
                      0.28160355077925891,  0.09501250983763744};

   double w[12] = { 0.10122853629037626,  0.22238103445337447,
                      0.31370664587788729,  0.36268378337836198,
                      0.02715245941175409,  0.06225352393864789,
                      0.09515851168249278,  0.12462897125553387,
                      0.14959598881657673,  0.16915651939500254,
                      0.18260341504492359,  0.18945061045506850};

   double h, aconst, bb, aa, c1, c2, u, s8, s16, f1, f2;
   double xx[1];
   int i;

   if ( fFunction == 0 )
   {
      MATH_ERROR_MSG("ROOT::Math::GausIntegratorOneDim", "A function must be set first!");
      return 0.0;
   }

   h = 0;
   fUsedOnce = true;
   if (b == a) return h;
   aconst = kCST/std::abs(b-a);
   bb = a;
CASE1:
   aa = bb;
   bb = b;
CASE2:
   c1 = kHF*(bb+aa);
   c2 = kHF*(bb-aa);
   s8 = 0;
   for (i=0;i<4;i++) {
      u     = c2*x[i];
      if (mapping) {
         xx[0] = (*mapping)(c1+u);
         double xxminus[1] = {-1. * xx[0]}; 
         f1    = ((*fFunction)(xx) + (*fFunction)(xxminus)) / std::pow(1. / (*xx + 1) , 2);
      } else {
         xx[0] = c1+u;
         f1    = (*fFunction)(xx);
      }
      if (fgAbsValue) f1 = std::abs(f1);
      if (mapping) {
         xx[0] = (*mapping)(c1-u);
         double xxminus[1] = {-1. * xx[0]};
         f2    = ((*fFunction)(xx) + (*fFunction)(xxminus)) / std::pow(1. / (*xx + 1) , 2);
      } else {
         xx[0] = c1-u;
         f2    = (*fFunction)(xx);
      }
      if (fgAbsValue) f2 = std::abs(f2);
      s8   += w[i]*(f1 + f2);
   }
   s16 = 0;
   for (i=4;i<12;i++) {
      u     = c2*x[i];
      if (mapping) {
         xx[0] = (*mapping)(c1+u);
         double xxminus[1] = {-1. * xx[0]};
         f1    = ((*fFunction)(xx) + (*fFunction)(xxminus)) / std::pow(1. / (*xx + 1) , 2);
      } else {
         xx[0] = c1+u;
         f1    = (*fFunction) (xx);
      }
      if (fgAbsValue) f1 = std::abs(f1);
      if (mapping) {
         xx[0] = (*mapping)(c1-u);
         double xxminus[1] = {-1. * xx[0]};
         f2    = ((*fFunction)(xx) + (*fFunction)(xxminus)) / std::pow(1. / (*xx + 1) , 2);
      } else {
         xx[0] = c1-u;
         f2    = (*fFunction) (xx);
      }
      if (fgAbsValue) f2 = std::abs(f2);
      s16  += w[i]*(f1 + f2);
   }
   s16 = c2*s16;
   if (std::abs(s16-c2*s8) <= fEpsilon*(1. + std::abs(s16))) {
      h += s16;
      if(bb != b) goto CASE1;
   } else {
      bb = c1;
      if(1. + aconst*std::abs(c2) != 1) goto CASE2;
      h = s8;  //this is a crude approximation (cernlib function returned 0 !)
   }

   fLastResult = h;
   fLastError = std::abs(s16-c2*s8);

   return h;
}
   

void GaussIntegrator::SetRelTolerance (double eps)
{   fEpsilon = eps;  }

void GaussIntegrator::SetAbsTolerance (double)
{   MATH_ERROR_MSG("ROOT::Math::GausIntegratorOneDim", "There is no Absolute Tolerance!");  }

double GaussIntegrator::Result () const
{
   // Returns the result of the last Integral calculation.

   if (!fUsedOnce)
      MATH_ERROR_MSG("ROOT::Math::GausIntegratorOneDim", "You must calculate the result at least once!");

   return fLastResult;
}

double GaussIntegrator::Error() const
{   return fLastError;  }

int GaussIntegrator::Status() const
{   return (fUsedOnce) ? 0 : -1;  }

void GaussIntegrator::SetFunction (const IGenFunction & function)
{
   // Set integration function
   fFunction = &function;
   // reset fUsedOne flag
   fUsedOnce = false; 
}

double GaussIntegrator::DefaultMapping(double x) { 
   return 1./ x - 1. ;
}

double GaussIntegrator::IntegralUp (double /*a*/)
{   return 0.0;  }

double GaussIntegrator::IntegralLow (double /*b*/)
{   return 0.0;  }

double GaussIntegrator::Integral (const std::vector< double > &/*pts*/)
{   return 0.0;  }

double GaussIntegrator::IntegralCauchy (double /*a*/, double /*b*/, double /*c*/)
{   return 0.0;  }

} // end namespace Math
   
} // end namespace ROOT
