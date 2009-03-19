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

#include "RooStats/RooStatsUtils.h"

ClassImp(RooStats::ConfidenceBelt) ;

using namespace RooStats;

//____________________________________________________________________
ConfidenceBelt::ConfidenceBelt() 
{
   // Default constructor
}

//____________________________________________________________________
ConfidenceBelt::ConfidenceBelt(const char* name) :
  TNamed(name,name), fParameterPoints(0)
{
   // Alternate constructor
}

//____________________________________________________________________
ConfidenceBelt::ConfidenceBelt(const char* name, const char* title) :
   TNamed(name,title), fParameterPoints(0)
{
   // Alternate constructor
}

//____________________________________________________________________
ConfidenceBelt::ConfidenceBelt(const char* name, RooTreeData& data) :
   TNamed(name,name), fParameterPoints(&data)
{
   // Alternate constructor
}

//____________________________________________________________________
ConfidenceBelt::ConfidenceBelt(const char* name, const char* title, RooTreeData& data) :
   TNamed(name,title), fParameterPoints(&data)
{
   // Alternate constructor
}



//____________________________________________________________________
ConfidenceBelt::~ConfidenceBelt()
{
   // Destructor

}


//____________________________________________________________________
Double_t ConfidenceBelt::GetAcceptanceRegionMin(RooArgSet& parameterPoint, Double_t cl, Double_t leftside) {

  if(cl>0 || leftside > 0) cout <<"using default cl, leftside for now" <<endl;
  return GetAcceptanceRegion(parameterPoint, cl,leftside)->GetUpperLimit();
}

//____________________________________________________________________
Double_t ConfidenceBelt::GetAcceptanceRegionMax(RooArgSet& parameterPoint, Double_t cl, Double_t leftside) {
  if(cl>0 || leftside > 0) cout <<"using default cl, leftside for now" <<endl;
  return GetAcceptanceRegion(parameterPoint, cl,leftside)->GetLowerLimit();
}

//____________________________________________________________________
vector<Double_t> ConfidenceBelt::ConfidenceLevels() const {
  vector<Double_t> levels;
  return levels;
}

//____________________________________________________________________
void ConfidenceBelt::AddAcceptanceRegion(RooArgSet& parameterPoint, AcceptanceRegion region, 
					 Double_t cl, Double_t leftside){
  
  if(cl>0 || leftside > 0) cout <<"using default cl, leftside for now" <<endl;

  RooDataSet*  tree = dynamic_cast<RooDataSet*>(  fParameterPoints );
  RooDataHist* hist = dynamic_cast<RooDataHist*>( fParameterPoints );

  if( !this->CheckParameters(parameterPoint) )
    std::cout << "problem with parameters" << std::endl;
  
  
  if( hist ) {
    // need a way to get index for given point
    // Can do this by setting hist's internal parameters to desired values
    // need a better way
    //    RooStats::SetParameters(&parameterPoint, const_cast<RooArgSet*>(hist->get())); 
    //    int index = hist->calcTreeIndex(); // get index
    int index = hist->getIndex(parameterPoint); // get index

    // allocate memory if necessary.  numEntries is overkill?
    if(fSamplingSummaries.size() < index) fSamplingSummaries.reserve( hist->numEntries() ); 

    // set the region for this point (check for duplicate?)
    fSamplingSummaries.at(index) = region;
  }
  else if( tree ){
    tree->add( parameterPoint ); // assume it's unique for now
    int index = tree->numEntries() - 1; //check that last point added has index nEntries -1
    // allocate memory if necessary.  numEntries is overkill?
    if(fSamplingSummaries.size() < index) fSamplingSummaries.reserve( tree->numEntries()  ); 

    // set the region for this point (check for duplicate?)
    fSamplingSummaries.at( index ) = region;
  }
}

//____________________________________________________________________
AcceptanceRegion* ConfidenceBelt::GetAcceptanceRegion(RooArgSet &parameterPoint, Double_t cl, Double_t leftside) 
{  
   // Method to determine if a parameter point is in the interval

  if(cl>0 || leftside > 0) cout <<"using default cl, leftside for now" <<endl;

  RooDataSet*  tree = dynamic_cast<RooDataSet*>(  fParameterPoints );
  RooDataHist* hist = dynamic_cast<RooDataHist*>( fParameterPoints );
  
  if( !this->CheckParameters(parameterPoint) ){
    std::cout << "problem with parameters" << std::endl;
    return 0; 
  }
  
  if( hist ) {
    // need a way to get index for given point
    // Can do this by setting hist's internal parameters to desired values
    // need a better way
    //    RooStats::SetParameters(&parameterPoint, const_cast<RooArgSet*>(hist->get())); 
    //    Int_t index = hist->calcTreeIndex(); // get index
    int index = hist->getIndex(parameterPoint); // get index
    return &(fSamplingSummaries.at(index).GetAcceptanceRegion());
  }
  else if( tree ){
    // need a way to get index for given point
    //    RooStats::SetParameters(&parameterPoint, tree->get()); // set tree's parameters to desired values
    Int_t index = 0; //need something like tree->calcTreeIndex(); 
    return &(fSamplingSummaries.at(index).GetAcceptanceRegion());
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
  return (RooArgSet*) fParameterPoints->get()->Clone();
}

//____________________________________________________________________
Bool_t ConfidenceBelt::CheckParameters(RooArgSet &parameterPoint) const
{  

   if (parameterPoint.getSize() != fParameterPoints->get()->getSize() ) {
      std::cout << "size is wrong, parameters don't match" << std::endl;
      return false;
   }
   if ( ! parameterPoint.equals( *(fParameterPoints->get() ) ) ) {
      std::cout << "size is ok, but parameters don't match" << std::endl;
      return false;
   }
   return true;
}
