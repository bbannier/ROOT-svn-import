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

#include "Fit/BinData.h"

#include "TGraph.h" 
#include "TMultiGraph.h" 
#include "TF1.h" 
#include "TList.h"
#include "Terror.h"

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
   //if (option.fError1) error = 1;
   if (error <= 0 ) { 
      if (option.fUseEmpty) 
         error = 1.; // set error to 1 for empty bins 
      else 
         return false; 
   }
   return true; 
}


void FillData ( BinData  & dv, const TGraph * gr, bool useErrorX ,  TF1 * func ) {  
   //  fill the data vector from a TGraph. Pass also the TF1 function which is 
   // needed in case to exclude points rejected by the function
   assert(gr != 0); 

   // get fit option 
   DataOptions & fitOpt = dv.Opt();
   
   
   int  nPoints = gr->GetN();
   double *gx = gr->GetX();
   double *gy = gr->GetY();
   double *ex = gr->GetEX();
   double *ey = gr->GetEY();
 
   // if all errors are zero set option of using errors to 1
   if ( ey == 0) 
   if ( ex == 0) useErrorX = false; 
  
   // check for consistency in case of dv has been already filles (case of multi-graph) 
   
   // default case for graphs (when they have errors) 
   BinData::ErrorType type = BinData::kCoordError; 
   if (ey == 0  ) {  
      fitOpt.fError1 = true;
      type =  BinData::kNoError; 
   }
   else if ( ex == 0 || !useErrorX)  { 
      useErrorX = false; 
      type = BinData::kValueError; 
   }
   // if data are filled already do a re-initialization
   if (dv.Size() > 0 && dv.NDim() == 1 ) { 
      // check if size is correct otherwise flag an errors 
      if (dv.PointSize() == 2 && type != BinData::kNoError ) {
         Error("FillData","Inconsistent TGraph with previous data set- skip all graph data"); 
         return;
      }
      if (dv.PointSize() == 3 && type != BinData::kValueError ) {
         Error("FillData","Inconsistent TGraph with previous data set- skip all graph data"); 
         return;
      }
      if (dv.PointSize() == 4 && type != BinData::kCoordError ) {
         Error("FillData","Inconsistent TGraph with previous data set- skip all graph data"); 
         return;
      }
   } 
   
   dv.Initialize(nPoints,1, type); 

   double x[1]; 
   for ( int i = 0; i < nPoints; ++i) { 
      
      x[0] = gx[i];
      // neglect error in x (it is a different chi2 function) 

      if (func && !func->IsInside( x )) continue;

      if (fitOpt.fError1)  
         dv.Add( gx[i], gy[i] ); 
      else if (!useErrorX)  { 
         double errorY =  ey[i];    
         // consider error = 0 as 1 
         if (!AdjustError(fitOpt,errorY) ) continue; 
         dv.Add( gx[i], gy[i], errorY );
      }
      else { // case use error in x
         double errorX =  ex[i];    
         double errorY =  ey[i];    
         if (errorX <= 0 ) { 
            errorX = 0; 
            if (!AdjustError(fitOpt,errorY) ) continue; 
         }
         dv.Add( gx[i], gy[i], errorX, errorY );
      }
                        
   }    

#ifdef DEBUG
   std::cout << "TGraphFitInterface::FillData Graph FitData size is " << dv.Size() << std::endl;
#endif
  
}


void FillData ( BinData  & dv, const TMultiGraph * mg, bool useErrorX, TF1 * func ) {  
   //  fill the data vector from a TMultiGraph. Pass also the TF1 function which is 
   // needed in case to exclude points rejected by the function
   assert(mg != 0);

   TGraph *gr;
   TIter next(mg->GetListOfGraphs());   
   
   while ((gr = (TGraph*) next())) {
      FillData( dv, gr, useErrorX, func); 
   }

#ifdef DEBUG
   std::cout << "TGraphFitInterface::FillData MultiGraph FitData size is " << dv.Size() << std::endl;
#endif
 

}


} // end namespace Fit

} // end namespace ROOT


