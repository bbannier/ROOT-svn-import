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

#include <limits>
#include <cmath>

//#define DEBUG
#ifdef DEBUG
#include <iostream> 
#endif

//todo: 

//  need to implement integral option

namespace ROOT { 

   namespace Fit { 


// for chi2 functions

double FitUtil::EvaluateChi2(IModelFunction & func, const BinData & data, const double * p, unsigned int & nPoints) {  
   // evaluate the chi2 given a  function reference  , the data and returns the value and also in nPoints 
   // the actual number of used points

   
   unsigned int n = data.Size();

#ifdef DEBUG
   std::cout << "\n\nFit data size = " << n << std::endl;
#endif

   double chi2 = 0;
   int nRejected = 0; 
   func.SetParameters(p); 
   for (unsigned int i = 0; i < n; ++ i) { 
      const double * x = data.Coords(i);
      double y = data.Value(i);
      double invError = data.InvError(i); 
      double fval = func ( x ); 

#ifdef DEBUG      
      std::cout << x[0] << "  " << y << "  " << 1./invError << " params : "; 
      for (int ipar = 0; ipar < func.NPar(); ++ipar) 
         std::cout << p[ipar] << "\t";
      std::cout << "\tfval = " << fval << std::endl; 
#endif

      // avoid singularity in the function (infinity and nan ) in the chi2 sum 
      // eventually add possibility of excluding some points (like singularity) 
      if (fval > - std::numeric_limits<double>::max() && fval < std::numeric_limits<double>::max() ) { 
         // calculat chi2 point
         double tmp = ( y -fval )* invError;  	  
         chi2 += tmp*tmp;
      }
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


double FitUtil::EvaluateChi2Residual(IModelFunction & func, const BinData & data, const double * p, unsigned int i) {  
   // evaluate the chi2 contribution (residual term) 

   func.SetParameters(p);
   const double * x = data.Coords(i);
   double y = data.Value(i);
   double invError = data.InvError(i); 
   double fval = func ( x ); 
   if (fval > - std::numeric_limits<double>::max() && fval < std::numeric_limits<double>::max() ) 
      // calculat chi2 point
      return  ( y -fval )* invError;  	  
   else 
      return 0; 
}

void FitUtil::EvaluateChi2Gradient(IModelFunction & f, const BinData & data, const double * p, double * grad, unsigned int & nPoints) { 
   IGradModelFunction * fg = dynamic_cast<IGradModelFunction *>( &f); 
   assert (fg != 0); // must be called by a grad function

   IGradModelFunction & func = *fg; 
   unsigned int n = data.Size();

   //int nRejected = 0; 
   // set values of parameters 
   func.SetParameters(p); 
   unsigned int npar = func.NPar(); 
//   assert (npar == NDim() );  // npar MUST be  Chi2 dimension
   std::vector<double> gradFunc( npar ); 
   // set all vector values to zero
   std::vector<double> g( npar); 
   //for (int i = 0; i < npar; ++i) grad[i] = 0; 

   for (unsigned int i = 0; i < n; ++ i) { 
//       const BinPoint & point = data[i]; 
//       const std::vector<double> & x = point.Coords(); 
//       double y = point.Value();
//       double invError = point.InvError();
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

double FitUtil::EvaluatePdf(IModelFunction & func, const UnBinData & data, const double * p, unsigned int i) {  
   // evaluate the pdf contribution to the logl

   func.SetParameters(p);
   const double * x = data.Coords(i);
   double fval = func ( x ); 
   return EvalLogF(fval);
}

double FitUtil::EvaluateLogL(IModelFunction & func, const UnBinData & data, const double * p, unsigned int &nPoints) {  
   // evaluate the LogLikelihood 

   unsigned int n = data.Size();

#ifdef DEBUG
   std::cout << "\n\nFit data size = " << n << std::endl;
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

      logl -= EvalLogF( fval); 

      
   }
   
   // reset the number of fitting data points
   if (nRejected != 0)  nPoints = n - nRejected;

#ifdef DEBUG
   std::cout << "Logl = " << logl << " np = " << nPoints << std::endl;
#endif
   
   return logl;
}


// for binned log likelihood functions      

double FitUtil::EvaluatePoissonBinPdf(IModelFunction & func, const BinData & data, const double * p, unsigned int i) {  
   // evaluate the pdf contribution to the logl

   func.SetParameters(p);
   const double * x = data.Coords(i);
   double y = data.Value(i);
   double fval = func ( x ); 

   // remove constant term depending on N
   return  fval - y * EvalLogF( fval);  

}

double FitUtil::EvaluatePoissonLogL(IModelFunction & func, const BinData & data, const double * p, unsigned int &nPoints) {  
   // evaluate the Poisson Log Likelihood
   unsigned int n = data.Size();

   
   double loglike = 0;
   int nRejected = 0; 
   func.SetParameters(p); 
   for (unsigned int i = 0; i < n; ++ i) { 
      const double * x = data.Coords(i);
      double y = data.Value(i);
      // need to implement integral option
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

   loglike +=  fval - y * EvalLogF( fval);  
      
      
   }
   
   // reset the number of fitting data points
   if (nRejected != 0)  nPoints = n - nRejected;

#ifdef DEBUG
   std::cout << "Logl = " << logl << " np = " << nPoints << std::endl;
#endif
   
   return loglike;  
}

   
}

} // end namespace ROOT

