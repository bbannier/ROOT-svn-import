// @(#)root/roostats:$Id: SimpleInterval.h 30478 2009-09-25 19:42:07Z schott $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/


/**
   HypoTestInverterResult class

**/


// include header file of this class 
#include "RooStats/HypoTestInverterResult.h"
#include "RooStats/HybridResult.h"

ClassImp(RooStats::HypoTestInverterResult)

using namespace RooStats;


HypoTestInverterResult::HypoTestInverterResult( ) :
  fUseCLs(false)
{
  // default constructor
}


HypoTestInverterResult::HypoTestInverterResult( const char* name,
						const char* title,
						RooRealVar* scannedVariable,
						double cl ) :
   SimpleInterval(name,title,scannedVariable,-9999,+9999), 
   fUseCLs(false)
{
  // constructor
  SetConfidenceLevel(cl);

  fYObjects.SetOwner();
}


HypoTestInverterResult::~HypoTestInverterResult()
{
  // destructor

  // delete fYobjects
}

 
double HypoTestInverterResult::GetXValue( int index ) const
{
  if ( index >= Size() || index<0 ) {
    std::cout << "Problem: You are asking for an impossible array index value\n";
    return -999;
  }

  return fXValues[index];
}

double HypoTestInverterResult::GetYValue( int index ) const
{
  if ( index >= Size() || index<0 ) {
    std::cout << "Problem: You are asking for an impossible array index value\n";
    return -999;
  }

  if (fUseCLs) 
    return ((HybridResult*)fYObjects.At(index))->CLs();
  else 
    return ((HybridResult*)fYObjects.At(index))->AlternatePValue();  // CLs+b
}

double HypoTestInverterResult::GetYError( int index ) const
{
  if ( index >= Size() || index<0 ) {
    std::cout << "Problem: You are asking for an impossible array index value\n";
    return -999;
  }

  if (fUseCLs) 
    return ((HybridResult*)fYObjects.At(index))->CLsError();
  else 
    return ((HybridResult*)fYObjects.At(index))->CLsplusbError();
}

void HypoTestInverterResult::CalculateLimits()
{ 
  // find the 2 objects the closer to the limit and make a linear extrapolation to the target

  double cl = 1-ConfidenceLevel();

  if (Size()<2) {
    std::cout << "not enough points to get the inverted interval\n";
  }

  double v1 = fabs(GetYValue(0)-cl);
  int i1 = 0;
  double v2 = fabs(GetYValue(1)-cl);
  int i2 = 1;

  if (Size()>2)
    for (int i=2; i<Size(); i++) {
      double vt = fabs(GetYValue(i)-cl);
      if ( vt<v1 || vt<v2 ) {
	if ( v1<v2 ) {
	  v2 = vt;
	  i2 = i;
	} else {
	  v1 = vt;
	  i1 = i;
	}
      }
    }

  fLowerLimit = ((RooRealVar*)fParameters->first())->getMin();
  fUpperLimit = GetXValue(i1)+(cl-GetYValue(i1))*(GetXValue(i2)-GetXValue(i1))/(GetYValue(i2)-GetYValue(i1)); // MAYBE TOO MANY GETYVALUE CALLS!

  return;
}
