// @(#)root/roostats:$Id: SetInterval.cxx 26317 2009-01-13 15:31:05Z cranmer $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/*****************************************************************************
 * Project: RooStats
 * Package: RooFit/RooStats  
 * @(#)root/roofit/roostats:$Id: SetInterval.cxx 26317 2009-01-13 15:31:05Z cranmer $
 * Original Author: Kyle Cranmer
 *   Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
 *
 *****************************************************************************/



//_________________________________________________________
//
// BEGIN_HTML
// SetInterval is a concrete implementation of the ConfInterval interface.  
// It implements simple general purpose interval of arbitrary dimensions and shape.
// It does not assume the interval is connected.
// It uses either a RooDataSet (eg. a list of parameter points in the interval) or
// a RooDataHist (eg. a Histogram-like object for small regions of the parameter space) to
// store the interval.  
// END_HTML
//
//

#ifndef RooStats_SetInterval
#include "RooStats/SetInterval.h"
#endif
#include "RooAbsReal.h"
#include <string>

#include "RooDataSet.h"
#include "RooDataHist.h"

ClassImp(RooStats::SetInterval) ;

using namespace RooStats;

//____________________________________________________________________
SetInterval::SetInterval() 
{
   // Default constructor
}

//____________________________________________________________________
SetInterval::SetInterval(const char* name) :
  ConfInterval(name,name), fParameterPointsInInterval(0)
{
   // Alternate constructor
}

//____________________________________________________________________
SetInterval::SetInterval(const char* name, const char* title) :
   ConfInterval(name,title), fParameterPointsInInterval(0)
{
   // Alternate constructor
}

//____________________________________________________________________
SetInterval::SetInterval(const char* name, RooTreeData& data) :
   ConfInterval(name,name), fParameterPointsInInterval(&data)
{
   // Alternate constructor
}

//____________________________________________________________________
SetInterval::SetInterval(const char* name, const char* title, RooTreeData& data) :
   ConfInterval(name,title), fParameterPointsInInterval(&data)
{
   // Alternate constructor
}



//____________________________________________________________________
SetInterval::~SetInterval()
{
   // Destructor

}


//____________________________________________________________________
Bool_t SetInterval::IsInInterval(RooArgSet &parameterPoint) 
{  
   // Method to determine if a parameter point is in the interval

  RooDataSet*  tree = dynamic_cast<RooDataSet*>(  fParameterPointsInInterval );
  RooDataHist* hist = dynamic_cast<RooDataHist*>( fParameterPointsInInterval );
  
  if( !this->CheckParameters(parameterPoint) )
    return false; 
  
  if(parameterPoint.getSize() != 1 )
    return false;


  if( hist ) {
    if ( hist->weight( parameterPoint , 0 ) > 0 ) // positive value indicates point is in interval
      return true; 
    else
      return false;
  }
  else if( tree ){
    RooArgSet* thisPoint = 0;
    // need to check if the parameter point is the same as any point in tree. 
    for(Int_t i = 0; i<tree->numEntries(); ++i){
      // This method is not complete
      // thisPoint = tree->get(i); 
      // if ( parameterPoint == *thisPoint)
      //    return true; 
      return false;
    }
  }
  else {
      std::cout << "dataset is not initialized properly" << std::endl;
  }

   return true;
  
}

//____________________________________________________________________
RooArgSet* SetInterval::GetParameters() const
{  
   // returns list of parameters
  return (RooArgSet*) fParameterPointsInInterval->get()->Clone();
}

//____________________________________________________________________
Bool_t SetInterval::CheckParameters(RooArgSet &parameterPoint) const
{  

   if (parameterPoint.getSize() != fParameterPointsInInterval->get()->getSize() ) {
      std::cout << "size is wrong, parameters don't match" << std::endl;
      return false;
   }
   if ( ! parameterPoint.equals( *(fParameterPointsInInterval->get() ) ) ) {
      std::cout << "size is ok, but parameters don't match" << std::endl;
      return false;
   }
   return true;
}
