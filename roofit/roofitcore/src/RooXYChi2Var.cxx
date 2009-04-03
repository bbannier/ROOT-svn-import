/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 * @(#)root/roofitcore:$Id$
 * Authors:                                                                  *
 *   WV, Wouter Verkerke, UC Santa Barbara, verkerke@slac.stanford.edu       *
 *   DK, David Kirkby,    UC Irvine,         dkirkby@uci.edu                 *
 *                                                                           *
 * Copyright (c) 2000-2005, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// 
// Class RooXYChi2Var implements a simple chi^2 calculation from a unbinned
// dataset with values x,y with errors on y (and optionally on x) and a function.
// The function can be either a RooAbsReal, or an extended RooAbsPdf where
// the function value is calculated as the probability density times the
// expected number of events
// The chi^2 is calculated as 
//
//              / (Data[y]-) - func \+2
//  Sum[point] |  ------------------ |
//              \     Data[ErrY]    /
//
//

#include "RooFit.h"

#include "RooXYChi2Var.h"
#include "RooDataSet.h"
#include "RooAbsReal.h"

#include "Riostream.h"

#include "RooRealVar.h"


ClassImp(RooXYChi2Var)
;


//_____________________________________________________________________________
RooXYChi2Var::RooXYChi2Var(const char *name, const char* title, RooAbsReal& func, RooDataSet& xydata) :
  RooAbsOptTestStatistic(name,title,func,xydata,RooArgSet(),0,0,1,1,0,0)
{
  //
  //  RooXYChi2Var constructor with function and X-Y values dataset
  //
  // An X-Y dataset is a weighted dataset with one or more observables X where the weight is interpreted
  // as the Y value and the weight error is interpreted as the Y value error. The weight must have an
  // non-zero error defined at each point for the chi^2 calculation to be meaningful.
  //
  // To store errors associated with the x and y values in a RooDataSet, call RooRealVar::setAttribute("StoreError")
  // on each X-type observable for which the error should be stored and add datapoints to the dataset as follows
  //
  // RooDataSet::add(xset,yval,yerr) where xset is the RooArgSet of x observables (with or without errors) and yval and yerr
  //                                 are the Double_t values that correspond to the Y and its error
  //
  _extended = kFALSE ;

}


//_____________________________________________________________________________
RooXYChi2Var::RooXYChi2Var(const char *name, const char* title, RooAbsPdf& extPdf, RooDataSet& xydata) :
  RooAbsOptTestStatistic(name,title,extPdf,xydata,RooArgSet(),0,0,1,1,0,0)
{
  //
  // RooXYChi2Var constructor with an extended p.d.f. and X-Y values dataset
  // The value of the function that defines the chi^2 in this form is takes as
  // the p.d.f. times the expected number of events
  //
  // An X-Y dataset is a weighted dataset with one or more observables X where the weight is interpreted
  // as the Y value and the weight error is interpreted as the Y value error. The weight must have an
  // non-zero error defined at each point for the chi^2 calculation to be meaningful.
  //
  // To store errors associated with the x and y values in a RooDataSet, call RooRealVar::setAttribute("StoreError")
  // on each X-type observable for which the error should be stored and add datapoints to the dataset as follows
  //
  // RooDataSet::add(xset,yval,yerr) where xset is the RooArgSet of x observables (with or without errors) and yval and yerr
  //                                 are the Double_t values that correspond to the Y and its error
  //
  if (!extPdf.canBeExtended()) {
    throw(string(Form("RooXYChi2Var::ctor(%s) ERROR: Input p.d.f. must be an extendible"))) ;
  }
  _extended = kTRUE ;
}




//_____________________________________________________________________________
RooXYChi2Var::RooXYChi2Var(const RooXYChi2Var& other, const char* name) : 
  RooAbsOptTestStatistic(other,name),
  _extended(other._extended)
{
  // Copy constructor
}



//_____________________________________________________________________________
RooXYChi2Var::~RooXYChi2Var()
{
  // Destructor
}



//_____________________________________________________________________________
Double_t RooXYChi2Var::evaluatePartition(Int_t firstEvent, Int_t lastEvent, Int_t stepSize) const 
{
  // Calculate chi^2 in partition from firstEvent to lastEvent using given stepSize

  Double_t result(0) ;

  // Loop over bins of dataset
  RooDataSet* data = (RooDataSet*) _dataClone ;

  for (Int_t i=firstEvent ; i<lastEvent ; i+=stepSize) {
    
    // get the data values for this event
    data->get(i);
    
    if (!data->valid()) {
      continue ;
    }

    // Get function value
    Double_t yfunc = _funcClone->getVal(_dataClone->get()) ;
    if (_extended) {      
      RooAbsPdf* pdf = (RooAbsPdf*) _funcClone ;
      // Multiply with expected number of events
      yfunc *= pdf->expectedEvents(_dataClone->get()) ;
    }      

    // Get data value and error
    Double_t ydata = data->weight() ;
    Double_t eylo,eyhi ;
    data->weightError(eylo,eyhi) ;

    // Calculate external error
    Double_t eExt = yfunc-ydata ;

    // Pick upper or lower error bar depending on sign of external error
    Double_t eInt = (eExt>0) ? eyhi : eylo ;

    // Return 0 if eInt=0, special handling in MINUIT will follow
    if (eInt==0.) {
      coutE(Eval) << "RooXYChi2Var::RooXYChi2Var(" << GetName() << ") INFINITY ERROR: data point " << i 
		  << " has zero error, but function is not zero (" << yfunc << ")" << endl ;
      return 0 ;
    }

    // Add chi2 term
    result += eExt*eExt/(eInt*eInt) ;
  }
  

  return result ;
}



