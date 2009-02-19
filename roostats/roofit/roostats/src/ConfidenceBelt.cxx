// @(#)root/roostats:$Id: ConfidenceBelt.cxx 26317 2009-01-13 15:31:05Z cranmer $
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
 * @(#)root/roofit/roostats:$Id: ConfidenceBelt.cxx 26317 2009-01-13 15:31:05Z cranmer $
 * Original Author: Kyle Cranmer
 *   Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
 *
 *****************************************************************************/



//_________________________________________________________
//
// BEGIN_HTML
// ConfidenceBelt is a concrete implementation of the ConfInterval interface.  
// It implements simple general purpose interval of arbitrary dimensions and shape.
// It does not assume the interval is connected.
// It uses either a RooDataSet (eg. a list of parameter points in the interval) or
// a RooDataHist (eg. a Histogram-like object for small regions of the parameter space) to
// store the interval.  
// END_HTML
//
//

#ifndef RooStats_ConfidenceBelt
#include "RooStats/ConfidenceBelt.h"
#endif

#include "RooDataSet.h"
#include "RooDataHist.h"

ClassImp(RooStats::ConfidenceBelt) ;

using namespace RooStats;

//____________________________________________________________________
ConfidenceBelt::ConfidenceBelt() 
{
   // Default constructor
}

//____________________________________________________________________
ConfidenceBelt::ConfidenceBelt(const char* name) :
  TNamed(name,name), fParameterPointsInInterval(0)
{
   // Alternate constructor
}

//____________________________________________________________________
ConfidenceBelt::ConfidenceBelt(const char* name, const char* title) :
   TNamed(name,title), fParameterPointsInInterval(0)
{
   // Alternate constructor
}

//____________________________________________________________________
ConfidenceBelt::ConfidenceBelt(const char* name, RooTreeData& data) :
   TNamed(name,name), fParameterPointsInInterval(&data)
{
   // Alternate constructor
}

//____________________________________________________________________
ConfidenceBelt::ConfidenceBelt(const char* name, const char* title, RooTreeData& data) :
   TNamed(name,title), fParameterPointsInInterval(&data)
{
   // Alternate constructor
}



//____________________________________________________________________
ConfidenceBelt::~ConfidenceBelt()
{
   // Destructor

}


//____________________________________________________________________
Double_t ConfidenceBelt::AcceptanceRegionMin(RooArgSet& parameterPoint, Double_t cl) {
  return AcceptanceRegion(parameterPoint, cl)->first;
}

//____________________________________________________________________
Double_t ConfidenceBelt::AcceptanceRegionMax(RooArgSet& parameterPoint, Double_t cl) {
  return AcceptanceRegion(parameterPoint, cl)->second;
}

//____________________________________________________________________
vector<Double_t> ConfidenceBelt::ConfidenceLevels() const {
  vector<Double_t> levels;
  return levels;
}

//____________________________________________________________________
pair<Double_t, Double_t>* ConfidenceBelt::AcceptanceRegion(RooArgSet &parameterPoint, Double_t cl) 
{  
   // Method to determine if a parameter point is in the interval

  RooDataSet*  tree = dynamic_cast<RooDataSet*>(  fParameterPointsInInterval );
  RooDataHist* hist = dynamic_cast<RooDataHist*>( fParameterPointsInInterval );
  
  if( !this->CheckParameters(parameterPoint) ){
    //    std::cout << "problem with parameters" << std::endl;
    //    return false; 
  }
  
  if(parameterPoint.getSize() != fParameterPointsInInterval->get()->getSize() ){
    std::cout << "problem with parameters" << std::endl;
    return 0;
  }

  if( hist ) {
    // need a way to get index for given point
    //    Int_t index = hist->get(parameterPoint);
    Int_t index = 0;
    Int_t clindex = 0;
    return new RangeType(fBelts.at(clindex).second[index]);    
  }
  else if( tree ){
    // need a way to get index for given point
    //    Int_t index = tree->get(parameterPoint);
    Int_t index = 0;
    Int_t clindex = 0;
    return new RangeType(fBelts.at(clindex).second[index]);    
  }
  else {
      std::cout << "dataset is not initialized properly" << std::endl;
  }

   return 0;
  
}

//____________________________________________________________________
RooArgSet* ConfidenceBelt::GetParameters() const
{  
   // returns list of parameters
  return (RooArgSet*) fParameterPointsInInterval->get()->Clone();
}

//____________________________________________________________________
Bool_t ConfidenceBelt::CheckParameters(RooArgSet &parameterPoint) const
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
