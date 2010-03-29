// @(#)root/mathcore:$Id$
// Author: L. Moneta Fri Sep 22 15:06:47 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// implementation file for class DistSampler

#include "Math/DistSampler.h"
#include "Math/Error.h"

#include "Math/IFunction.h"
#include "Fit/BinData.h"
#include "Fit/UnBinData.h"
#include "Fit/DataRange.h"


namespace ROOT { 
   

namespace Math { 

DistSampler::~DistSampler() { 
   // destructor 
   if (fOwnFunc && fFunc != 0) delete fFunc; 
   if (fRange) delete fRange;
}

void DistSampler::SetRange(double xmin, double xmax, int icoord) {
   if (!fRange) { 
      MATH_ERROR_MSG("DistSampler::SetRange","Need to set function before setting the range");
      return;
   }
   fRange->SetRange(icoord,xmin,xmax);
}

void DistSampler::SetRange(const double * xmin, const double * xmax) {
   // set range specifying a vector for all coordinates 
   if (!fRange) {
      MATH_ERROR_MSG("DistSampler::SetRange","Need to set function before setting the range");
      return;
   }
   for (unsigned int icoord = 0; icoord < NDim(); ++icoord) 
      fRange->SetRange(icoord,xmin[icoord],xmax[icoord]);
}

void DistSampler::SetRange(const ROOT::Fit::DataRange & range) {
   // copy the given range
   *fRange = range;  
}

void DistSampler::DoSetFunction(const ROOT::Math::IMultiGenFunction & func, bool copy) { 
   // set the internal function
   // if a range exists and it is compatible it will be re-used
   if (fOwnFunc && fFunc != 0) delete fFunc; 
   if (copy) {
      fOwnFunc = true; 
      fFunc = func.Clone(); 
   }
   else {
      fOwnFunc = false; 
      fFunc = &func; 
   }
   fData = std::vector<double>(func.NDim());
   // delete a range if exists and it is not compatible
   if (fRange && fRange->NDim() != fData.size() ) {
      delete fRange; 
      fRange = 0; 
   }
   if (!fRange) fRange = new ROOT::Fit::DataRange(func.NDim() );
}

bool DistSampler::IsInitialized()  { 
   // test if sampler is initialized
   // tryying to generate one event (for this cannot be const) 
   if (NDim() == 0) return false; 
   if (fFunc == 0) return false; 
   if (fFunc->NDim() != NDim() ) return false; 
   // test one event 
   if (!Sample(&fData[0]) ) return false; 
   return true;
}

bool DistSampler::Generate(unsigned int nevt, ROOT::Fit::UnBinData & data) { 
   // generate a un-binned data sets (fill the given data set)
   // if dataset has already data append to it 
   int n0 = data.DataSize(); 
   if (n0 > 0 ) { 
      if (data.PointSize() != NDim() ) {
         MATH_ERROR_MSG("DistSampler::Generate","unbin data not consistent with distribution");
         return false; 
      }
   }
   if (!IsInitialized()) { 
         MATH_WARN_MSG("DistSampler::Generate","sampler has not been initialized correctly");
         return false; 
   }

   data.Initialize( n0 + nevt, NDim() );
   for (unsigned int i = 0; i < nevt; ++i) { 
      const double * x = Sample(); 
      data.Add( x ); 
   }
   return true;
}


bool DistSampler::Generate(unsigned int nbins, ROOT::Fit::BinData & data) { 
   // generate a bin data set from given bin center values 
   // bin center values must be present in given data set 
   if (!IsInitialized()) { 
         MATH_WARN_MSG("DistSampler::Generate","sampler has not been initialized correctly");
         return false; 
   }

   data.Initialize(nbins, NDim(), ROOT::Fit::BinData::kValueError);    // store always the error
   // use for the moment bin center (should use bin integral) 
   std::vector<double> dx(NDim() );
   std::vector<double> x(NDim() );
   for (unsigned int j = 0; j < dx.size(); ++j) { 
      double x1 = 0,x2 = 0; 
      fRange->GetRange(j,x1,x2); 
      dx[j] = x2-x1; 
      x[j] = x1 + dx[j]/2;  // use bin centers

   }
   for (unsigned int i = 0; i < nbins; ++i) { 
      //const double * v = Sample(); 
      double val,eval;
      double yval = (ParentPdf())(&x.front());
      SampleBin(yval,val,eval);
      data.Add(&x.front(), val, eval); 
   }
   return true;
}
   
} // end namespace Math
} // end namespace ROOT
