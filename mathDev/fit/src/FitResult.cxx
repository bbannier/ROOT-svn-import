// @(#)root/fit:$Id: src/FitResult.cxx,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Wed Aug 30 11:05:34 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Implementation file for class FitResult

#include "Fit/FitResult.h"

#include "Math/Minimizer.h"

#include "Math/IParamFunction.h"

#include <cassert>

namespace ROOT { 

   namespace Fit { 


FitResult::FitResult() 
{
   // Default constructor implementation.
}

FitResult::FitResult(const ROOT::Math::Minimizer & min, const IModelFunction & func, unsigned int sizeOfData, bool isValid, bool chi2fit ) : 
   fValid(isValid),
   fVal (min.MinValue()),  
   fEdm (min.Edm()),  
   fNCalls(min.NCalls()),
   fParams(std::vector<double>(min.X(), min.X() + min.NDim() ) ),  
   fFitFunc(&func)
{
   // Constructor from a minimizer, fill the data

   fNdf = sizeOfData - min.NFree(); 

   if (min.Errors() != 0) 
      fErrors = std::vector<double>(min.Errors(), min.Errors() + min.NDim() ) ; 

   if (chi2fit) 
      fChi2 = fVal;
   else 
      fChi2 = 0; // to be done - must compulte chi2 equivalent
      
//    // fill error matrix
   // cov matrix rank 
   if (fValid) { 
      unsigned int n  = min.NDim(); 
      unsigned int r = n * (  n + 1 )/2;  
      fCovMatrix.reserve(r);
      for (unsigned int i = 0; i < n; ++i) 
         for (unsigned int j = 0; j <= i; ++j)
            fCovMatrix.push_back(min.CovMatrix(i,j) );
      
      assert (fCovMatrix.size() == r ); 
   }
                              
                              
}

int FitResult::Index(const std::string & name) const { 
   // find index for given parameter name
   unsigned int npar = fParams.size(); 
   for (unsigned int i = 0; i < npar; ++i) 
      if ( fFitFunc->ParameterName(i) == name) return i; 
   
   return -1; // case name is not found
} 

void FitResult::Print(std::ostream & os) const { 
   // print the result in the given stream 
   os << "\n****************************************\n";
   os << "            FitResult                   \n\n";
   unsigned int npar = fParams.size(); 
   os << "Chi2/Likelihood  =\t" << fVal << std::endl;
   os << "NDf              =\t" << fNdf << std::endl; 
   os << "Edm              =\t" << fEdm << std::endl; 
   os << "NCalls           =\t" << fNCalls << std::endl; 
   assert(fFitFunc != 0); 
   for (unsigned int i = 0; i < npar; ++i) { 
      os << fFitFunc->ParameterName(i) << "\t\t =\t" << fParams[i] << " \t+/-\t" << fErrors[i] << std::endl; 
   }
}

   } // end namespace Fit

} // end namespace ROOT

