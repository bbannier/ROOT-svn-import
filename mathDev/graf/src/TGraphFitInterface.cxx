// @(#)root/graf:$Id$
// Author: L. Moneta Thu Nov 15 17:04:20 2007

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Implementation file for class TGraphFitInterface

#include "TGraphFitInterface.h"

#include "Fit/DataVector.h"

#include "TGraph.h" 
#include "TMultiGraph.h" 
#include "TF1.h" 
#include "TList.h"

//#define DEBUG
#ifdef DEBUG
#include <iostream> 
#endif

#include <cassert> 




namespace ROOT { 

namespace Fit { 



bool IsPointOutOfRange(const TF1 * func, const double * x) { 
   // function to check if a point is outside range
   if (func ==0) return false; 
   return !func->IsInside(x);       
}
bool AdjustError(const DataOptions & option, double & error) {
   // adjust the given error accoring to the option
   //  if false is returned bin must be skipped 
   if (option.fError1) error = 1;
   if (error <= 0 ) { 
      if (option.fUseEmpty) 
         error = 1.; // set error to 1 for empty bins 
      else 
         return false; 
   }
   return true; 
}


void FillData ( BinData  & dv, const TGraph * gr, TF1 * func ) {  
   //  fill the data vector from a TMultiGraph. Pass also the TF1 function which is 
   // needed in case to exclude points rejected by the function
   assert(gr != 0); 

   // get fit option 
   DataOptions & fitOpt = dv.Opt();
   
   
   int  nPoints = gr->GetN();
   double *gx = gr->GetX();
   double *gy = gr->GetY();
 
   // if all errors are zero set option of using errors to 1
   if ( gr->GetEY() == 0) fitOpt.fError1 = true;
  
   // dimension is 1
   double x[1]; 
   dv.Initialize(nPoints,1); 

   for ( int i = 0; i < nPoints; ++i) { 
      
      x[0] = gx[i];
      // neglect error in x (it is a different chi2 function) 

      if (!func->IsInside( x )) continue;

      double errorY = gr->GetErrorY(i); 
      // consider error = 0 as 1 
      if (!AdjustError(fitOpt,errorY) ) continue; 
      dv.Add( *x, gy[i], errorY );
            
   }    

#ifdef DEBUG
   std::cout << "TGraphFitInterface::FillData Graph FitData size is " << dv.Size() << std::endl;
#endif
  
}


void FillData ( BinData  & dv, const TMultiGraph * mg, TF1 * func ) {  
   //  fill the data vector from a TMultiGraph. Pass also the TF1 function which is 
   // needed in case to exclude points rejected by the function
   assert(mg != 0);

   TGraph *gr;
   TIter next(mg->GetListOfGraphs());   
   
   while ((gr = (TGraph*) next())) {
      FillData( dv, gr, func); 
   }

#ifdef DEBUG
   std::cout << "TGraphFitInterface::FillData MultiGraph FitData size is " << dv.Size() << std::endl;
#endif
 

}


} // end namespace Fit

} // end namespace ROOT


