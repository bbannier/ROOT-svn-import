// @(#)root/fit:$Id: src/Chi2FCN.cxx,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Tue Sep  5 09:13:32 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Implementation file for class Chi2FCN

#include "Fit/Chi2FCN.h"
#include "Fit/DataVector.h"
#include "Fit/BinPoint.h"
#include "Math/IParamFunction.h"

#include <limits>
#include <cmath>


namespace ROOT { 

   namespace Fit { 


Chi2FCN::Chi2FCN(const BinData & data, IModelFunction & func) : 
   fData(data), 
   fFunc(func), 
   fNDim(func.NPar() ), 
   fNPoints(data.Size()),      
   fNEffPoints(0),
   fNCalls(0)
{
   // Constructor implementation, needs a reference to the data and the function

}

Chi2FCN::Chi2FCN(const Chi2FCN & rhs) : 
   BaseObjFunction(), 
   fData(rhs.fData),
   fFunc(rhs.fFunc) 
{
   // copy constructor (dummy since is private)
   operator=(rhs);
}

Chi2FCN & Chi2FCN::operator = (const Chi2FCN & rhs)  {
   // assignment operator (dummy since is private)
   if (this == &rhs) return *this;  // time saving self-test
   //fData    = rhs.fData;  
   fFunc    = rhs.fFunc; 
   fNDim    = rhs.fNDim; 
   fNPoints = rhs.fNPoints;       
   fNEffPoints = rhs.fNEffPoints;
   fNCalls     = rhs.fNCalls;
   return *this;
}

Chi2FCN * Chi2FCN::Clone() const  {
   // clone the function
   Chi2FCN * fcn =  new Chi2FCN(fData,fFunc); 
   return fcn; 
}


   } // end namespace Fit

} // end namespace ROOT

