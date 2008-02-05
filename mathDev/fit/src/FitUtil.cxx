// @(#)root/fit:$Id: src/FitUtil.cxx,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Tue Nov 28 10:52:47 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Implementation file for class FitUtil

#include "Fit/FitUtil.h"

#include "Fit/DataVector.h"
#include "Fit/BinPoint.h"

#include "Math/IParamFunction.h"
#include "Math/Integrator.h"
#include "Math/IntegratorMultiDim.h"

#include <limits>
#include <cmath>
#include <cassert> 

//#define DEBUG
#ifdef DEBUG
#include <iostream> 
#endif

//todo: 

//  need to implement integral option

namespace ROOT { 

   namespace Fit { 

      namespace FitUtil { 

         // internal class to evaluate the function or the integral 
         // and cached internal integration details
         // if useIntegral is false no allocation is done
         // and this is a dummy class
         struct IntegralEvaluator { 

            IntegralEvaluator(bool useIntegral, const ROOT::Math::IMultiGenFunction & f) : 
               fDim(f.NDim()),
               fIg1Dim(0), 
               fIgNDim(0) 
            { 
               if (useIntegral) { 
                  if (fDim == 1) { 
                     fIg1Dim = new ROOT::Math::IntegratorOneDim(); 
                     fIg1Dim->SetFunction(f);
                  } 
                  else if (fDim > 1) 
                     fIgNDim = new ROOT::Math::IntegratorMultiDim(f);
                  else
                     assert(fDim > 0); 
               }
            }

            ~IntegralEvaluator() { 
               if (fIg1Dim) delete fIg1Dim; 
               if (fIgNDim) delete fIgNDim; 
            }

            double operator()(const double *x1, const double * x2) { 
               // return normalized integral, divided by bin volume (dx1*dx...*dxn) 
               if (fIg1Dim) { 
                  double dV = *x2 - *x1;
                  return fIg1Dim->Integral( *x1, *x2)/dV; 
               }
               else if (fIgNDim) { 
                  double dV = 1; 
                  for (unsigned int i = 0; i < fDim; ++i) 
                     dV *= ( x2[i] - x1[i] ); 
                  return fIgNDim->Integral( x1, x2)/dV; 
               }
               else 
                  assert(1.); // should never be here
               return 0;
            }

            unsigned int fDim; 
            ROOT::Math::IntegratorOneDim * fIg1Dim; 
            ROOT::Math::IntegratorMultiDim * fIgNDim; 
         }; 

      }


// for chi2 functions

double FitUtil::EvaluateChi2(IModelFunction & func, const BinData & data, const double * p, unsigned int & nPoints) {  
   // evaluate the chi2 given a  function reference  , the data and returns the value and also in nPoints 
   // the actual number of used points

   
   unsigned int n = data.Size();

#ifdef DEBUG
   std::cout << "\n\nFit data size = " << n << std::endl;
   std::cout << "evaluate chi2 using function " << &func << "  " << p << std::endl; 
#endif

   double chi2 = 0;
   int nRejected = 0; 
   

   func.SetParameters(p); 

   // get fit option and check case of using integral of bins
   const DataOptions & fitOpt = data.Opt();
   IntegralEvaluator igEval( fitOpt.fIntegral, func); 

   bool useCoordErrors = data.UseCoordErrors();

   for (unsigned int i = 0; i < n; ++ i) { 

//#define OLD
#ifdef OLD
      const double * x = data.Coords(i);
      double y = data.Value(i);
      double invError = data.InvError(i); 
#else 

      double y, invError; 
      const double * x = data.GetPoint(i,y, invError);
//       const BinPoint & point = data.GetPoint(i); 
//       const double * x = point.Coords();
//       double y = point.Value();
//       double invError = point.InvError(); 

#endif 

      double fval = 0;

      if (!fitOpt.fIntegral )
         fval = func ( x ); 
      else { 
         // calculate normalized integral (divided by bin volume)
         fval = igEval( x, data.Coords(i+1) ) ; 
      }


#ifdef DEBUG      
      std::cout << x[0] << "  " << y << "  " << 1./invError << " params : "; 
      for (int ipar = 0; ipar < func.NPar(); ++ipar) 
         std::cout << p[ipar] << "\t";
      std::cout << "\tfval = " << fval << std::endl; 
#endif


//#ifdef LATER
      double  resval = 0; 
      if (!useCoordErrors) { 
            double tmp = ( y -fval )* invError;  	  
            resval = tmp * tmp;
      }
         
      if (useCoordErrors ) { 
         double ey = 1;
         const double * ex = data.GetPointError(i, ey); 
         double e2 = ey * ey; 
         for (unsigned int icoord = 0; icoord < func.NDim(); ++icoord) { 
            e2 += ex[icoord] * ex[icoord];  // need to multuply by the derivative
         } 
         double w2 = (e2 > 0) ? 1.0/e2 : 0;  
         resval = w2 * ( y - fval ) *  ( y - fval); 
      }
      
      // avoid (infinity and nan ) in the chi2 sum 
      // eventually add possibility of excluding some points (like singularity) 
      if ( resval < std::numeric_limits<double>::max() )  
         chi2 += resval; 
      else 
         nRejected++; 
      
   }
   
   // reset the number of fitting data points
   if (nRejected != 0)  nPoints = n - nRejected;

#ifdef DEBUG
   std::cout << "chi2 = " << chi2 << " n = " << nRejected << std::endl;
#endif
   
   return chi2;

}


      double FitUtil::EvaluateChi2Residual(IModelFunction & func, const BinData & data, const double * p, unsigned int i, double * g) {  
   // evaluate the chi2 contribution (residual term) 
   // need to implement integral calculation

   func.SetParameters(p);
   const double * x = data.Coords(i);
   double y = data.Value(i);
   double invError = data.InvError(i); 
   double fval = func ( x ); 
   double resval = 0; 
   if (fval > - std::numeric_limits<double>::max() && fval < std::numeric_limits<double>::max() ) 
      // calculat chi2 point
      resval =   ( y -fval )* invError;  	  
   
   // estimate gradient 
   if (g == 0) return resval; 


   unsigned int npar = func.NPar(); 
   IGradModelFunction * gfunc = dynamic_cast<IGradModelFunction *>( &func); 

   if (gfunc != 0) { 
      //case function provides gradient
      gfunc->ParameterGradient(  x , g );  
      // mutiply by - 1 * weihgt
      for (unsigned int k = 0; k < npar; ++k) {
         g[k] *= - invError;
      }       
   }
   else { 
      // estimate gradient numerically with simple 2 point rule 
      static const double kEps = 1.0E-4;      
      std::vector<double> p2(p,p+npar);
      double f0 = resval;
      for (unsigned int k = 0; k < npar; ++k) {
         p2[k] += kEps;
         // t.b.d : treat case of infinities 
         //if (fval > - std::numeric_limits<double>::max() && fval < std::numeric_limits<double>::max() ) 
         double f2 = invError * ( y - func(x,&p2.front()) );
         g[k] = ( f2 - f0 )/kEps;
         p2[k] = p[k]; // restore original p value
      }       
   }

   return resval; 

}


void FitUtil::EvaluateChi2Gradient(IModelFunction & f, const BinData & data, const double * p, double * grad, unsigned int & ) { 
   // evaluate gradient of chi2
   // need to implement case with integral option

   IGradModelFunction * fg = dynamic_cast<IGradModelFunction *>( &f); 
   assert (fg != 0); // must be called by a grad function

   IGradModelFunction & func = *fg; 
   unsigned int n = data.Size();

#ifdef DEBUG
   std::cout << "\n\nFit data size = " << n << std::endl;
   std::cout << "evaluate chi2 using function gradient " << &func << "  " << p << std::endl; 
#endif

   //int nRejected = 0; 
   // set values of parameters 
   func.SetParameters(p); 
   unsigned int npar = func.NPar(); 
//   assert (npar == NDim() );  // npar MUST be  Chi2 dimension
   std::vector<double> gradFunc( npar ); 
   // set all vector values to zero
   std::vector<double> g( npar); 

   for (unsigned int i = 0; i < n; ++ i) { 
      const double * x = data.Coords(i);
      double y = data.Value(i);
      double invError = data.InvError(i); 
      double fval = func ( x ); 
      func.ParameterGradient(  x , &gradFunc[0] );  

#ifdef DEBUG      
      std::cout << x[0] << "  " << y << "  " << 1./invError << " params : "; 
      for (int ipar = 0; ipar < npar; ++ipar) 
         std::cout << p[ipar] << "\t";
      std::cout << "\tfval = " << fval << std::endl; 
#endif

      // loop on the parameters
      for (unsigned int ipar = 0; ipar < npar ; ++ipar) { 

         // avoid singularity in the function (infinity and nan ) in the chi2 sum 
         // eventually add possibility of excluding some points (like singularity) 
         if (  (fval > - std::numeric_limits<double>::max() && fval < std::numeric_limits<double>::max() ) && 
               (gradFunc[ipar] > - std::numeric_limits<double>::max() && 
                gradFunc[ipar] < std::numeric_limits<double>::max() ) )
               { 
                  // calculate derivative point contribution
                  double tmp = - 2.0 * ( y -fval )* invError * invError * gradFunc[ipar];  	  
                  g[ipar] += tmp;
               }
//          else 
//             nRejected++; 
      
      }

   }

//    // reset the number of fitting data points
//    if (nRejected != 0)  nPoints = n - nRejected;
//    if (nPoints != fNPoints)
//       std::cout << "Warning : Number of points differes between Chi2 evaluation and derivatives " << std::endl; 

   // copy result 
   std::copy(g.begin(), g.end(), grad);

}


      

// utility function used by the likelihoods 

inline double EvalLogF(double fval) { 
   // evaluate the log with a protections against negative argument to the log 
   // smooth linear extrapolation below function values smaller than  epsilon
   // (better than a simple cut-off)
   const static double epsilon = 2.*std::numeric_limits<double>::min();
   if(fval<= epsilon) 
      return fval/epsilon + std::log(epsilon) - 1; 
   else      
      return std::log(fval);
}

// for LogLikelihood functions

      double FitUtil::EvaluatePdf(IModelFunction & func, const UnBinData & data, const double * p, unsigned int i, double * g) {  
   // evaluate the pdf contribution to the logl

   func.SetParameters(p);
   const double * x = data.Coords(i);
   double fval = func ( x ); 
   //return EvalLogF(fval);
   if (g == 0) return fval;

   unsigned int npar = func.NPar(); 
   IGradModelFunction * gfunc = dynamic_cast<IGradModelFunction *>( &func); 

   // gradient  calculation
   if (gfunc != 0) { 
      //case function provides gradient
      gfunc->ParameterGradient(  x , g );  
   }
   else { 
      // estimate gradient numerically with simple 2 point rule 
      static const double kEps = 1.0E-4;      
      std::vector<double> p2(p,p+npar);
      for (unsigned int k = 0; k < npar; ++k) {
         p2[k] += kEps;
         // t.b.d : treat case of infinities 
         //if (fval > - std::numeric_limits<double>::max() && fval < std::numeric_limits<double>::max() ) 
         double df = func(x,&p2.front() )  - fval;
         g[k] = df/kEps;
         p2[k] = p[k]; // restore original p value
      }       
   }

#ifdef DEBUG
   std::cout << x[i] << "\t"; 
   std::cout << "\tpar = [ " << func.NPar() << " ] =  "; 
   for (int ipar = 0; ipar < func.NPar(); ++ipar) 
      std::cout << p[ipar] << "\t";
   std::cout << "\tfval = " << fval;
   std::cout << "\tgrad = [ "; 
   for (int ipar = 0; ipar < func.NPar(); ++ipar) 
      std::cout << g[ipar] << "\t";
   std::cout << " ] "   << std::endl; 
#endif


   return fval;
}

double FitUtil::EvaluateLogL(IModelFunction & func, const UnBinData & data, const double * p, unsigned int &nPoints) {  
   // evaluate the LogLikelihood 

   unsigned int n = data.Size();

#ifdef DEBUG
   std::cout << "\n\nFit data size = " << n << std::endl;
   std::cout << "func pointer is " << typeid(func).name() << std::endl;
#endif

   double logl = 0;
   int nRejected = 0; 
   func.SetParameters(p); 
   for (unsigned int i = 0; i < n; ++ i) { 
      const double * x = data.Coords(i);
      double fval = func ( x ); 

#ifdef DEBUG      
      std::cout << "x [ " << data.PointSize() << " ] = "; 
      for (unsigned int j = 0; j < data.PointSize(); ++j)
         std::cout << x[j] << "\t"; 
      std::cout << "\tpar = [ " << func.NPar() << " ] =  "; 
      for (int ipar = 0; ipar < func.NPar(); ++ipar) 
         std::cout << p[ipar] << "\t";
      std::cout << "\tfval = " << fval << std::endl; 
#endif
      if (fval < 0) { 
         nRejected++; // reject points with negative pdf (cannot exist)
      }
      else 
         logl += EvalLogF( fval); 
      
   }
   
   // reset the number of fitting data points
   if (nRejected != 0)  nPoints = n - nRejected;

#ifdef DEBUG
   std::cout << "Logl = " << logl << " np = " << nPoints << std::endl;
#endif
   
   return -logl;
}

void FitUtil::EvaluateLogLGradient(IModelFunction & f, const UnBinData & data, const double * p, double * grad, unsigned int & ) { 
   // evaluate the gradient of the log likelihood function

   IGradModelFunction * fg = dynamic_cast<IGradModelFunction *>( &f); 
   assert (fg != 0); // must be called by a grad function
   IGradModelFunction & func = *fg; 

   unsigned int n = data.Size();
   //int nRejected = 0; 
   func.SetParameters(p); 
   unsigned int npar = func.NPar(); 
   std::vector<double> gradFunc( npar ); 
   std::vector<double> g( npar); 

   for (unsigned int i = 0; i < n; ++ i) { 
      const double * x = data.Coords(i);
      double fval = func ( x ); 
      if (fval > 0) { 
         func.ParameterGradient( x, &gradFunc[0] );
         for (unsigned int kpar = 0; kpar < npar; ++ kpar) { 
            g[kpar] -= 1./fval * gradFunc[ kpar ]; 
         }
            
      }
    // copy result 
   std::copy(g.begin(), g.end(), grad);
   }
}

// for binned log likelihood functions      

double FitUtil::EvaluatePoissonBinPdf(IModelFunction & func, const BinData & data, const double * p, unsigned int i, double * g ) {  
   // evaluate the pdf contribution to the logl
   // t.b.d. implement integral option

   func.SetParameters(p);
   const double * x = data.Coords(i);
   double y = data.Value(i);
   double fval = func ( x ); 

   // remove constant term depending on N
   double logPdf =   y * EvalLogF( fval) - fval;  
   // need to return the pdf contribution (not the log)

   double pdfval =  std::exp(logPdf);

   if (g == 0) return pdfval; 

   unsigned int npar = func.NPar(); 
   IGradModelFunction * gfunc = dynamic_cast<IGradModelFunction *>( &func); 

   // gradient  calculation
   if (gfunc != 0) { 
      //case function provides gradient
      gfunc->ParameterGradient(  x , g );  
      // correct g[] do be derivative of poisson term 
      for (unsigned int k = 0; k < npar; ++k) {
         g[k] *= ( y/fval - 1.) * pdfval; 
      }       
   }
   else { 
      // estimate gradient numerically with simple 2 point rule 
      static const double kEps = 1.0E-4;      
      std::vector<double> p2(p,p+npar);
      for (unsigned int k = 0; k < npar; ++k) {
         p2[k] += kEps;
         // t.b.d : treat case of infinities 
         // make derivatives of the log (more stables ) and correct afterwards
         double fval2 = func(x,&p2.front() );    
         double logPdf2 = y * EvalLogF( fval2) - fval2; 
         double dlogPdf = (logPdf2 - logPdf)/kEps;
         g[k] = dlogPdf * pdfval;
         p2[k] = p[k]; // restore original p value

      }       
   }

   return pdfval;
}

double FitUtil::EvaluatePoissonLogL(IModelFunction & func, const BinData & data, const double * p, unsigned int &nPoints) {  
   // evaluate the Poisson Log Likelihood
   unsigned int n = data.Size();

   
   double loglike = 0;
   int nRejected = 0; 
   func.SetParameters(p); 

   // get fit option and check case of using integral of bins
   const DataOptions & fitOpt = data.Opt();
   IntegralEvaluator igEval( fitOpt.fIntegral, func); 

   for (unsigned int i = 0; i < n; ++ i) { 
      const double * x = data.Coords(i);
      double y = data.Value(i);

      double fval = 0;   
      if (!fitOpt.fIntegral )
         fval = func ( x ); 
      else { 
         // calculate normalized integral (divided by bin volume)
         fval = igEval( x, data.Coords(i+1) ) ; 
      }


      loglike +=  fval - y * EvalLogF( fval);  
      
      
   }
   
   // reset the number of fitting data points
   if (nRejected != 0)  nPoints = n - nRejected;

#ifdef DEBUG
   std::cout << "Logl = " << logl << " np = " << nPoints << std::endl;
#endif
   
   return loglike;  
}

void FitUtil::EvaluatePoissonLogLGradient(IModelFunction & f, const BinData & data, const double * p, double * grad ) { 
   // evaluate the gradient of the log likelihood function
   // t.b.d. add integral option

   IGradModelFunction * fg = dynamic_cast<IGradModelFunction *>( &f); 
   assert (fg != 0); // must be called by a grad function
   IGradModelFunction & func = *fg; 

   unsigned int n = data.Size();

   func.SetParameters(p); 
   unsigned int npar = func.NPar(); 
   std::vector<double> gradFunc( npar ); 
   std::vector<double> g( npar); 

   for (unsigned int i = 0; i < n; ++ i) { 
      const double * x = data.Coords(i);
      double y = data.Value(i);
      double fval = func ( x ); 
      if (fval > 0) { 
         func.ParameterGradient( x, &gradFunc[0] );
         for (unsigned int kpar = 0; kpar < npar; ++ kpar) { 
            // df/dp * (1.  - y/f )
            g[kpar] += gradFunc[ kpar ] * ( 1. - y/fval ); 
         }            
      }
    // copy result 
   std::copy(g.begin(), g.end(), grad);
   }
}
   
}

} // end namespace ROOT

