// @(#)root/mathcore:$Id$
// Authors: David Gonzalez Maline    01/2008 

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006 , LCG ROOT MathLib Team                         *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

#include "Math/GaussIntegrator.h"
#include <cmath>

namespace ROOT {
namespace Math {

bool GaussIntegratorOneDim::fgAbsValue = false;

GaussIntegratorOneDim::GaussIntegratorOneDim()
{
   fEpsilon = 1e-12;
   fLastResult = fLastError = 0;
   fUsedOnce = false;
   fFunctionCopied = false;
   fFunction = 0;
}

GaussIntegratorOneDim::~GaussIntegratorOneDim()
{
   if ( fFunction != 0 && fFunctionCopied )
      delete fFunction;
}

void GaussIntegratorOneDim::AbsValue(bool flag)
{
   // Static function: set the fgAbsValue flag.
   // By default TF1::Integral uses the original function value to compute the integral
   // However, TF1::Moment, CentralMoment require to compute the integral
   // using the absolute value of the function.
   
   fgAbsValue = flag;
}

double GaussIntegratorOneDim::Integral(double a, double b)
{
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

   //InitArgs(xx,params);

   h = 0;
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
      xx[0] = c1+u;
      //f1    = EvalPar(xx,params);
      f1    = (*fFunction)(xx);
      if (fgAbsValue) f1 = std::abs(f1);
      xx[0] = c1-u;
      //f2    = EvalPar(xx,params);
      f2    = (*fFunction) (xx);
      if (fgAbsValue) f2 = std::abs(f2);
      s8   += w[i]*(f1 + f2);
   }
   s16 = 0;
   for (i=4;i<12;i++) {
      u     = c2*x[i];
      xx[0] = c1+u;
      //f1    = EvalPar(xx,params);
      f1    = (*fFunction) (xx);
      if (fgAbsValue) f1 = std::abs(f1);
      xx[0] = c1-u;
      //f2    = EvalPar(xx,params);
      f2    = (*fFunction) (xx);
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

   fUsedOnce = true;
   fLastResult = h;
   fLastError = std::abs(s16-c2*s8);

   return h;
   
   /*
   return ((TF1*)f)->Integral(a,b);
   */
}
   

void GaussIntegratorOneDim::SetRelTolerance (double eps)
{
   fEpsilon = eps;
}

void GaussIntegratorOneDim::SetAbsTolerance (double)
{
   // TODO
   MATH_ERROR_MSG("ROOT::Math::GausIntegratorOneDim", "There is no Absolute Tolerance!");
}

double GaussIntegratorOneDim::Result () const
{
   if (!fUsedOnce)
      MATH_ERROR_MSG("ROOT::Math::GausIntegratorOneDim", "You must calculate the result at least once!");

   return fLastResult;
}

double GaussIntegratorOneDim::Error() const
{
   return fLastError;
}

int GaussIntegratorOneDim::Status() const
{
   // TODO
   return 0;
}

void GaussIntegratorOneDim::SetFunction (const IGenFunction & function, bool copy)
{
   if ( copy )
      fFunction = function.Clone();
   else
      fFunction = &function;

   fFunctionCopied = copy;
}


double GaussIntegratorOneDim::Integral ()
{
   // TODO
   return 0.0;
}

double GaussIntegratorOneDim::IntegralUp (double /*a*/)
{
   // TODO
   return 0.0;
}

double GaussIntegratorOneDim::IntegralLow (double /*b*/)
{
   // TODO
   return 0.0;
}

double GaussIntegratorOneDim::Integral (const std::vector< double > &/*pts*/)
{
   // TODO
   return 0.0;
}

double GaussIntegratorOneDim::IntegralCauchy (double /*a*/, double /*b*/, double /*c*/)
{
   // TODO
   return 0.0;
}


};
};
