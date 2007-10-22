// @(#)root/fit:$Id: src/Chi2GradFCN.cxx,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Tue Nov 28 10:53:06 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Implementation file for class Chi2GradFCN

#include "Fit/Chi2GradFCN.h"

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

Chi2GradFCN::Chi2GradFCN(const BinData & data, IGradModelFunction & func) : 
   fData(data), 
   fFunc(func), 
   fNPoints(data.Size()),      
   fNDim(func.NPar() ), 
   fGrad(std::vector<double>(fNDim) )
{
   // Constructor implementation, needs a reference to the data and the function

}

Chi2GradFCN::Chi2GradFCN(const Chi2GradFCN & rhs) : 
   BaseObjFunction::BaseFunc(), 
   BaseObjFunction(),
   fData(rhs.fData),
   fFunc(rhs.fFunc),
   fNPoints(rhs.fNPoints)
{
   // copy constructor (dummy since is private)
}
Chi2GradFCN & Chi2GradFCN::operator = (const Chi2GradFCN & rhs)  {
   // assignment operator (dummy since is private)
   if (this == &rhs) return *this;  // time saving self-test
   return *this;
}

Chi2GradFCN * Chi2GradFCN::Clone() const  {
   Chi2GradFCN * fcn =  new Chi2GradFCN(fData,fFunc); 
   return fcn; 
}

void Chi2GradFCN:: Gradient(const double * p, double * g ) const { 
   // implement derivative of the Chi2

   IGradModelFunction & func = fFunc; 
   const BinData & data = fData; 
   unsigned int n = data.Size();

   double chi2 = 0;
   //int nRejected = 0; 
   // set values of parameters 
   func.SetParameters(p); 
   unsigned int npar = func.NPar(); 
   assert (npar == NDim() );  // npar MUST be  Chi2 dimension
   std::vector<double> gradFunc( npar ); 
   // clear the vector   
   fGrad = std::vector<double>(npar); 

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
      for (int ipar = 0; ipar < npar ; ++ipar) { 

         // avoid singularity in the function (infinity and nan ) in the chi2 sum 
         // eventually add possibility of excluding some points (like singularity) 
         if (  (fval > - std::numeric_limits<double>::max() && fval < std::numeric_limits<double>::max() ) && 
               (gradFunc[ipar] > - std::numeric_limits<double>::max() && 
                gradFunc[ipar] < std::numeric_limits<double>::max() ) )
               { 
                  // calculate derivative point contribution
                  double tmp = - 2.0 * ( y -fval )* invError * invError * gradFunc[ipar];  	  
                  fGrad[ipar] += tmp;
               }
//          else 
//             nRejected++; 
      
      }

   }

//    // reset the number of fitting data points
//    if (nRejected != 0)  nPoints = n - nRejected;
//    if (nPoints != fNPoints)
//       std::cout << "Warning : Number of points differes between Chi2 evaluation and derivatives " << std::endl; 

   // keep internally a copy 
   std::copy(fGrad.begin(), fGrad.end(), g);
   
}


   } // end namespace Fit

} // end namespace ROOT

