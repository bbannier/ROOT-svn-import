// @(#)root/fit:$Id: src/LogLikelihoodFCN.cxx,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Fri Aug 17 14:29:24 2007

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2007  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Implementation file for class LogLikelihoodFCN

#include "Fit/LogLikelihoodFCN.h"

#include "Fit/DataVector.h"
//#include "Fit/UnBinPoint.h"
#include "Math/IParamFunction.h"

namespace ROOT { 

namespace Fit { 


LogLikelihoodFCN::LogLikelihoodFCN(const UnBinData & data, IModelFunction & func) : 
   fData(data), 
   fFunc(func), 
   fNDim(func.NPar() ), 
   fNPoints(data.Size()),      
   fNEffPoints(0),
   fNCalls(0)
{
   // constructor implementation.
}

// LogLikelihoodFCN::~LogLikelihoodFCN() 
// {
//    // Destructor implementation.
// }

LogLikelihoodFCN::LogLikelihoodFCN(const LogLikelihoodFCN & rhs) : 
   BaseObjFunction(),
   fData(rhs.fData),
   fFunc(rhs.fFunc)  
{
   // Implementation of copy constructor.
   operator=(rhs);
}

LogLikelihoodFCN & LogLikelihoodFCN::operator = (const LogLikelihoodFCN &rhs) 
{
   // Implementation of assignment operator
   if (this == &rhs) return *this;  // time saving self-test
   //fData    = rhs.fData;  
   fFunc    = rhs.fFunc; 
   fNDim    = rhs.fNDim; 
   fNPoints = rhs.fNPoints;       
   fNEffPoints = rhs.fNEffPoints;
   fNCalls     = rhs.fNCalls;
   return *this;
}

LogLikelihoodFCN * LogLikelihoodFCN::Clone() const  {
   // clone the function
   LogLikelihoodFCN * fcn =  new LogLikelihoodFCN(fData,fFunc); 
   return fcn; 
}

   } // end namespace Fit

} // end namespace ROOT

