// @(#)root/mathcore:$Id$
// Authors: David Gonzalez Maline    01/2008 

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006 , LCG ROOT MathLib Team                         *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

#include "Math/BrentRootFinder.h"
#include <cmath>

#ifndef ROOT_Math_Error
#include "Math/Error.h"
#endif

namespace ROOT {
namespace Math {


BrentRootFinder::BrentRootFinder() : fNpx(100)
{
   fFunction = 0;
}

BrentRootFinder::~BrentRootFinder() {}

int BrentRootFinder::SetFunction(const ROOT::Math::IGenFunction& f, double xlow, double xup)
{
   fFunction = &f;

   if (xlow >= xup) 
   {
      double tmp = xlow;
      xlow = xup; 
      xup = tmp;
   }
   fXMin = xlow;
   fXMax = xup;

   return 0;
}

const char* BrentRootFinder::Name() const
{
   return "BrentRootFinder";
}

double BrentRootFinder::Root() const
{
   return fRoot;
}

int BrentRootFinder::Solve(int, double, double /*double xmin, double xmax*/)
{
   // Returns the X value corresponding to the function value fy for (xmin<x<xmax).
   // Method:
   //  First, the grid search is used to bracket the maximum
   //  with the step size = (xmax-xmin)/fNpx. This way, the step size
   //  can be controlled via the SetNpx() function. If the function is
   //  unimodal or if its extrema are far apart, setting the fNpx to
   //  a small value speeds the algorithm up many times.
   //  Then, Brent's method is applied on the bracketed interval

   double fy = 0; // To find the root

   // TODO: Change xmin & xmax by fXMin & fXMax
//    if (xmin >= xmax) 
//    {
//       double tmp = xmin;
//       xmin = xmax; 
//       xmax = tmp;
//    }
   double xmin = fXMin;
   double xmax = fXMax;

   int niter=0;
   double x;
   x = MinimStep(4, xmin, xmax, fy);
   bool ok = true;
   x = MinimBrent(4, xmin, xmax, x, fy, ok);
   while (!ok){
      if (niter>10){
         MATH_ERROR_MSG("Root", "Search didn't converge");
         break;
      }
      x=MinimStep(4, xmin, xmax, fy);
      x = MinimBrent(4, xmin, xmax, x, fy, ok);
      niter++;
   }

   fRoot = x;

   //return x;
   return 1;
}

double BrentRootFinder::MinimStep(int type, double &xmin, double &xmax, double fy) const
{
   //   Grid search implementation, used to bracket the minimum and later
   //   use Brent's method with the bracketed interval
   //   The step of the search is set to (xmax-xmin)/fNpx
   //   type: 0-returns MinimumX
   //         1-returns Minimum
   //         2-returns MaximumX
   //         3-returns Maximum
   //         4-returns X corresponding to fy

   double x,y, dx;
   dx = (xmax-xmin)/(fNpx-1);
   double xxmin = xmin;
   double yymin;
   if (type < 2)
      yymin = (*fFunction)(xmin);
   else if (type < 4)
      yymin = -(*fFunction)(xmin);
   else
      yymin = fabs((*fFunction)(xmin)-fy);

   for (Int_t i=1; i<=fNpx-1; i++) {
      x = xmin + i*dx;
      if (type < 2)
         y = (*fFunction)(x);
      else if (type < 4)
         y = -(*fFunction)(x);
      else
         y = fabs((*fFunction)(x)-fy);
      if (y < yymin) {xxmin = x; yymin = y;}
   }

   xmin = std::max(xmin,xxmin-dx);
   xmax = std::min(xmax,xxmin+dx);

   return std::min(xxmin, xmax);
}


double BrentRootFinder::MinimBrent(int type, double &xmin, double &xmax, double xmiddle, double fy, bool &ok) const
{
   //Finds a minimum of a function, if the function is unimodal  between xmin and xmax
   //This method uses a combination of golden section search and parabolic interpolation
   //Details about convergence and properties of this algorithm can be
   //found in the book by R.P.Brent "Algorithms for Minimization Without Derivatives"
   //or in the "Numerical Recipes", chapter 10.2
   //
   //type: 0-returns MinimumX
   //      1-returns Minimum
   //      2-returns MaximumX
   //      3-returns Maximum
   //      4-returns X corresponding to fy
   //if ok=true the method has converged

   Double_t eps = 1e-10;
   Double_t t = 1e-8;
   Int_t itermax = 100;

   Double_t c = (3.-sqrt(5.))/2.; //comes from golden section
   Double_t u, v, w, x, fv, fu, fw, fx, e, p, q, r, t2, d=0, m, tol;
   v = w = x = xmiddle;
   e=0;

   Double_t a=xmin;
   Double_t b=xmax;
   if (type < 2)
      fv = fw = fx = (*fFunction)(x);
   else if (type < 4)
      fv = fw = fx = -(*fFunction)(x);
   else
      fv = fw = fx = fabs((*fFunction)(x)-fy);

   for (Int_t i=0; i<itermax; i++){
      m=0.5*(a + b);
      tol = eps*(fabs(x))+t;
      t2 = 2*tol;
      if (fabs(x-m) <= (t2-0.5*(b-a))) {
         //converged, return x
         ok=kTRUE;
         if (type==1)
            return fx;
         else if (type==3)
            return -fx;
         else
            return x;
      }

      if (fabs(e)>tol){
         //fit parabola
         r = (x-w)*(fx-fv);
         q = (x-v)*(fx-fw);
         p = (x-v)*q - (x-w)*r;
         q = 2*(q-r);
         if (q>0) p=-p;
         else q=-q;
         r=e;
         e=d;

         if (fabs(p) < fabs(0.5*q*r) || p < q*(a-x) || p < q*(b-x)) {
            //a parabolic interpolation step
            d = p/q;
            u = x+d;
            if (u-a < t2 || b-u < t2)
               //d=TMath::Sign(tol, m-x);
               d=(m-x >= 0) ? fabs(tol) : -fabs(tol);
         } else {
            e=(x>=m ? a-x : b-x);
            d = c*e;
         }
      } else {
         e=(x>=m ? a-x : b-x);
         d = c*e;
      }
      u = (fabs(d)>=tol ? x+d : x+ ((d >= 0) ? fabs(tol) : -fabs(tol)) );
      if (type < 2)
         fu = (*fFunction)(u);
      else if (type < 4)
         fu = -(*fFunction)(u);
      else
         fu = fabs((*fFunction)(u)-fy);
      //update a, b, v, w and x
      if (fu<=fx){
         if (u<x) b=x;
         else a=x;
         v=w; fv=fw; w=x; fw=fx; x=u; fx=fu;
      } else {
         if (u<x) a=u;
         else b=u;
         if (fu<=fw || w==x){
            v=w; fv=fw; w=u; fw=fu;
         }
         else if (fu<=fv || v==x || v==w){
            v=u; fv=fu;
         }
      }
   }
   //didn't converge
   ok = kFALSE;
   xmin = a;
   xmax = b;
   return x;
}

};
};
