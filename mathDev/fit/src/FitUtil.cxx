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



namespace ROOT { 

   namespace Fit { 


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
      

double FitUtil::EvaluatePdf(IModelFunction & func, const UnBinData & data, const double * p, unsigned int i) {  
   // evaluate the pdf contribution to the logl

   func.SetParameters(p);
   const double * x = data.Coords(i);
   double fval = func ( x ); 
   if (fval  >  std::numeric_limits<double>::min() ) 
      return  std::log( fval); 
   else // for too small values of fval 
      return std::log(  std::numeric_limits<double>::min() ); // log(min) 
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

      // take care of  too small values in the pdf which results in infinities
      if (fval  >  std::numeric_limits<double>::min() ) 
         logl -=  std::log( fval); 
      else // for too small values of fval 
         logl -= std::log(  std::numeric_limits<double>::min() ); // log(min) 

      
   }
   
   // reset the number of fitting data points
   if (nRejected != 0)  nPoints = n - nRejected;

#ifdef DEBUG
   std::cout << "Logl = " << logl << " np = " << nPoints << std::endl;
#endif
   
   return logl;
}

      


   } // end namespace Fit

} // end namespace ROOT

