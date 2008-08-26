// @(#)root/mathcore:$Id$
// Author: L. Moneta Wed Aug 30 11:05:34 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Implementation file for class FitResult

#include "Fit/FitResult.h"

#include "Fit/FitConfig.h"

#include "Fit/BinData.h"

#include "Math/Minimizer.h"

#include "Math/IParamFunction.h"
#include "Math/OneDimFunctionAdapter.h"

#include "Math/DistFunc.h"

#include "TMath.h"  
#include "Math/RichardsonDerivator.h"
#include "Math/Error.h"

#include <cassert>
#include <cmath>

namespace ROOT { 

   namespace Fit { 


FitResult::FitResult() : 
   fValid(false), fNormalized(false), fNFree(0), fNdf(0), fNCalls(0), fVal(0), fEdm(0), fChi2(0), fFitFunc(0)
{
   // Default constructor implementation.
}

      FitResult::FitResult(ROOT::Math::Minimizer & min, const FitConfig & fconfig, IModelFunction & func,  bool isValid,  unsigned int sizeOfData, const  ROOT::Math::IMultiGenFunction * chi2func, bool minosErr, unsigned int ncalls ) : 
   fValid(isValid),
   fNormalized(false),
   fNFree(min.NFree() ),
   fNCalls(min.NCalls()),
   fVal (min.MinValue()),  
   fEdm (min.Edm()),  
   fFitFunc(&func), 
   fParams(std::vector<double>(min.X(), min.X() + min.NDim() ) )
{
   // Constructor from a minimizer, fill the data. ModelFunction  is passed as non const 
   // since it will be managed by the FitResult

   if (sizeOfData > 0) fNdf = sizeOfData - min.NFree(); 

   // set right parameters in function (in case minimizer did not do before)
   // do also when fit is not valid
   fFitFunc->SetParameters(&fParams.front());

   if (min.Errors() != 0) 
      fErrors = std::vector<double>(min.Errors(), min.Errors() + min.NDim() ) ; 

   if (chi2func == 0) 
      fChi2 = fVal;
   else { 
      // compute chi2 equivalent
      fChi2 = (*chi2func)(&fParams[0]); 
   }

   // replace ncalls if given (they are taken from the FitMethodFunction)
   if (ncalls !=0) fNCalls = ncalls;

   unsigned int n  = min.NDim(); 
      
//    // fill error matrix
   // cov matrix rank 
   if (fValid) { 

      unsigned int r = n * (  n + 1 )/2;  
      fCovMatrix.reserve(r);
      for (unsigned int i = 0; i < n; ++i) 
         for (unsigned int j = 0; j <= i; ++j)
            fCovMatrix.push_back(min.CovMatrix(i,j) );
      
      assert (fCovMatrix.size() == r ); 

      // normalize errors if requested in configuration
      if (fconfig.NormalizeErrors() ) NormalizeErrors();

      // minos errors 
      if (minosErr) { 
         fMinosErrors.reserve(n);
         for (unsigned int i = 0; i < n; ++i) { 
            double elow, eup; 
            bool ret = min.GetMinosError(0, elow, eup); 
            if (ret) fMinosErrors.push_back(std::make_pair(elow,eup) );
            else fMinosErrors.push_back(std::make_pair(0.,0.) );
         }
      }

      // globalCC
      fGlobalCC.reserve(n);
      for (unsigned int i = 0; i < n; ++i) { 
         double globcc = min.GlobalCC(i); 
         if (globcc < 0) break; // it is not supported by that minimizer
         fGlobalCC.push_back(globcc); 
      }
      
   }

   fMinimType = fconfig.MinimizerType();
   if (fconfig.MinimizerAlgoType() != "") fMinimType += " / " + fconfig.MinimizerAlgoType(); 
}

FitResult::FitResult(const FitResult &rhs) { 
   // Implementation of copy constructor
   (*this) = rhs; 
}

FitResult & FitResult::operator = (const FitResult &rhs) { 
   // Implementation of assignment operator.
   if (this == &rhs) return *this;  // time saving self-test

   // Manages the fitted function 
   if (fFitFunc) delete fFitFunc;
   fFitFunc = 0; 
   if (rhs.fFitFunc != 0 ) {
      fFitFunc = dynamic_cast<IModelFunction *>( (rhs.fFitFunc)->Clone() ); 
      assert(fFitFunc != 0); 
   }

   // copy all other data members 
   fValid = rhs.fValid; 
   fNormalized = rhs.fNormalized;
   fNFree = rhs.fNFree; 
   fNdf = rhs.fNdf; 
   fNCalls = rhs.fNCalls; 
   fVal = rhs.fVal;  
   fEdm = rhs.fEdm; 
   fChi2 = rhs.fChi2;

   fParams = rhs.fParams; 
   fErrors = rhs.fErrors; 
   fCovMatrix = rhs.fCovMatrix; 
   fGlobalCC = rhs.fGlobalCC;
   fMinosErrors = rhs.fMinosErrors; 

   fMinimType = rhs.fMinimType; 
   
   return *this; 

}  


void FitResult::NormalizeErrors() { 
   // normalize errors and covariance matrix according to chi2 value
   if (fNdf == 0 || fChi2 <= 0) return; 
   double s2 = fChi2/fNdf; 
   double s = std::sqrt(fChi2/fNdf); 
   for (unsigned int i = 0; i < fErrors.size() ; ++i) 
      fErrors[i] *= s; 
   for (unsigned int i = 0; i < fCovMatrix.size() ; ++i) 
      fCovMatrix[i] *= s2; 

   fNormalized = true; 
} 

double FitResult::Prob() const { 
   // fit probability
   return ROOT::Math::chisquared_cdf_c(fChi2, static_cast<double>(fNdf) ); 
}

int FitResult::Index(const std::string & name) const { 
   // find index for given parameter name
   unsigned int npar = fParams.size(); 
   for (unsigned int i = 0; i < npar; ++i) 
      if ( fFitFunc->ParameterName(i) == name) return i; 
   
   return -1; // case name is not found
} 

void FitResult::Print(std::ostream & os, bool doCovMatrix) const { 
   // print the result in the given stream 
   // need to add minos errors , globalCC, etc..
   if (!fValid) { 
      os << "\n****************************************\n";
      os << "            Invalid FitResult            ";
      os << "\n****************************************\n";
      return; 
   }

   os << "\n****************************************\n";
   os << "            FitResult                   \n\n";
   os << "Minimizer is " << fMinimType << std::endl;
   unsigned int npar = fParams.size(); 
   if (fVal != fChi2) 
   os << "Likelihood       =\t" << fVal << std::endl;
   os << "Chi2             =\t" << fChi2<< std::endl;
   os << "NDf              =\t" << fNdf << std::endl; 
   os << "Edm              =\t" << fEdm << std::endl; 
   os << "NCalls           =\t" << fNCalls << std::endl; 
   assert(fFitFunc != 0); 
   for (unsigned int i = 0; i < npar; ++i) { 
      os << fFitFunc->ParameterName(i) << "\t\t =\t" << fParams[i] << " \t+/-\t" << fErrors[i] << std::endl; 
   }

   if (doCovMatrix) PrintCovMatrix(os); 
}

void FitResult::PrintCovMatrix(std::ostream &os) const { 
   // print the covariance and correlation matrix 
   if (!fValid) return;
   os << "\n****************************************\n";
   os << "\n            Covariance Matrix            \n\n";
   unsigned int npar = fParams.size(); 
   const int kPrec = 8; 
   const int kWidth = 12; 
   for (unsigned int i = 0; i < npar; ++i) {
      for (unsigned int j = 0; j <= i; ++j) {
         os.precision(kPrec); os.width(kWidth);  os << CovMatrix(i,j); 
      }
      os << std::endl;
   }
   os << "\n            Correlation Matrix         \n\n";
   for (unsigned int i = 0; i < npar; ++i) {
      for (unsigned int j = 0; j <= i; ++j) {
         os.precision(kPrec); os.width(kWidth);  os << Correlation(i,j); 
      }
      os << std::endl;
   }
}

void FitResult::GetConfidenceIntervals(unsigned int n, unsigned int stride1, unsigned int stride2, const double * x, double * ci, double cl ) const {     
   // stride1 stride in coordinate  stride2 stride in dimension space
   // i.e. i-th point in k-dimension is x[ stride1 * i + stride2 * k]
   // compute the confidence interval of the fit on the given data points
   // the dimension of the data points must match the dimension of the fit function
   // confidence intervals are returned in array ci

   // use student quantile
   //double t = - TMath::StudentQuantile((1.-cl)/2, f->GetNDF()); 
   double t = TMath::StudentQuantile(0.5 + cl/2, fNdf); 
   double chidf = TMath::Sqrt(fChi2/fNdf);

   if (!fFitFunc) {
      MATH_ERROR_MSG("FitResult::GetConfidenceIntervals","cannot compute Confidence Intervals without fitter function");
      return;
   }

   unsigned int ndim = fFitFunc->NDim(); 
   unsigned int npar = fFitFunc->NPar(); 

   std::vector<double> xpoint(ndim); 
   std::vector<double> grad(npar); 
   std::vector<double> vsum(npar); 

   // loop on the points
   for (unsigned int ipoint = 0; ipoint < n; ++ipoint) { 

      for (unsigned int kdim = 0; kdim < ndim; ++kdim) { 
         unsigned int i = ipoint * stride1 + kdim * stride2; 
         assert(i < ndim*n); 
         xpoint[kdim] = x[ipoint * stride1 + kdim * stride2]; 
      }

      // calculate gradient of fitted function w.r.t the parameters

      // check first if fFitFunction provides parameter gradient or not 
      
      // does not provide gradient
      // t.b.d : skip calculation for fixed parameters
      ROOT::Math::RichardsonDerivator d; 
      for (unsigned int ipar = 0; ipar < npar; ++ipar) { 
         ROOT::Math::OneDimParamFunctionAdapter<const ROOT::Math::IParamMultiFunction &> fadapter(*fFitFunc,&xpoint.front(),&fParams.front(),ipar);
         d.SetFunction(fadapter); 
         grad[ipar] = d(fParams[ipar] ); // evaluate df/dp
      }

      // multiply covariance matrix with gradient
      vsum.assign(npar,0.0);
      for (unsigned int ipar = 0; ipar < npar; ++ipar) { 
         for (unsigned int jpar = 0; jpar < npar; ++jpar) {
            vsum[ipar] += CovMatrix(ipar,jpar) * grad[jpar]; 
         }
      }
      // multiply gradient by vsum
      double r2 = 0; 
      for (unsigned int ipar = 0; ipar < npar; ++ipar) { 
         r2 += grad[ipar] * vsum[ipar]; 
      }
      double r = std::sqrt(r2); 
      ci[ipoint] = r * t * chidf; 
   }
}

void FitResult::GetConfidenceIntervals(const BinData & data, double * ci, double cl ) const { 
   // implement confidence intervals from a given bin data sets
   // currently copy the data from Bindata. 
   // could implement otherwise directly
   unsigned int ndim = data.NDim(); 
   unsigned int np = data.NPoints(); 
   std::vector<double> xdata( ndim * np ); 
   for (unsigned int i = 0; i < np ; ++i) { 
      const double * x = data.Coords(i); 
      std::vector<double>::iterator itr = xdata.begin()+ ndim * i;
      std::copy(x,x+ndim,itr);
   }
   // points are arraned as x0,y0,z0, ....xN,yN,zN  (stride1=ndim, stride2=1)
   GetConfidenceIntervals(np,ndim,1,&xdata.front(),ci,cl);
}

   } // end namespace Fit

} // end namespace ROOT

