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
// BEGIN_HTML
// RooAdaptiveIntegratorND implements an adaptive one-dimensional 
// numerical integration algorithm.
// END_HTML
//


#include "RooFit.h"
#include "Riostream.h"

#include "TClass.h"
#include "RooAdaptiveIntegratorND.h"
#include "RooArgSet.h"
#include "RooRealVar.h"
#include "RooNumber.h"
#include "RooMsgService.h"
#include "RooNumIntFactory.h"
#include "RooMultiGenFunction.h"
#include "Math/AdaptiveIntegratorMultiDim.h"

#include <assert.h>



ClassImp(RooAdaptiveIntegratorND)
;

// Register this class with RooNumIntConfig

//_____________________________________________________________________________
void RooAdaptiveIntegratorND::registerIntegrator(RooNumIntFactory& fact)
{
  // Register RooAdaptiveIntegratorND, its parameters, dependencies and capabilities with RooNumIntFactory

  RooRealVar nDivision2D("nDivision2D","Number of sub-divisions for 2-dim integrals",1000) ;
  RooRealVar nDivision3D("nDivision3D","Number of sub-divisions for 3-dim integrals",10000) ;
  RooRealVar nDivisionND("nDivisionND","Number of sub-divisions for >3-dim integrals",100000) ;

  fact.storeProtoIntegrator(new RooAdaptiveIntegratorND(),RooArgSet(nDivision2D,nDivision3D,nDivisionND)) ;
}
 


//_____________________________________________________________________________
RooAdaptiveIntegratorND::RooAdaptiveIntegratorND()
{
  // Default ctor
  _xmin = 0 ;
  _xmax = 0 ;
  _func = 0 ;
  _integrator = 0 ;
}



//_____________________________________________________________________________
RooAdaptiveIntegratorND::RooAdaptiveIntegratorND(const RooAbsFunc& function, const RooNumIntConfig& config) :
  RooAbsIntegrator(function)
{
  // Constructor of integral on given function binding and with given configuration. The
  // integration limits are taken from the definition in the function binding
  //_func = function.
  _func = new RooMultiGenFunction(function) ;
  _integrator = new ROOT::Math::AdaptiveIntegratorMultiDim(config.epsAbs(),config.epsRel(),10000) ;
  _integrator->SetFunction(*_func) ;

  _xmin = 0 ;
  _xmax = 0 ;
  checkLimits() ;
} 



//_____________________________________________________________________________
RooAbsIntegrator* RooAdaptiveIntegratorND::clone(const RooAbsFunc& function, const RooNumIntConfig& config) const
{
  // Virtual constructor with given function and configuration. Needed by RooNumIntFactory
  
  return new RooAdaptiveIntegratorND(function,config) ;
}




//_____________________________________________________________________________
RooAdaptiveIntegratorND::~RooAdaptiveIntegratorND()
{
  // Destructor
  delete _xmin ;
  delete _xmax ;
  delete _integrator ;
  delete _func ;
}



//_____________________________________________________________________________
Bool_t RooAdaptiveIntegratorND::checkLimits() const 
{
  // Check that our integration range is finite and otherwise return kFALSE.
  // Update the limits from the integrand if requested.
  
  if (!_xmin) {
    _xmin = new Double_t[_func->NDim()] ;
    _xmax = new Double_t[_func->NDim()] ;
  }

  for (UInt_t i=0 ; i<_func->NDim() ; i++) {
    _xmin[i]= integrand()->getMinLimit(i);
    _xmax[i]= integrand()->getMaxLimit(i);
  }

  return kTRUE ;
}




//_____________________________________________________________________________
Double_t RooAdaptiveIntegratorND::integral(const Double_t* /*yvec*/) 
{
  // Evaluate integral at given function binding parameter values
  Double_t ret = _integrator->Integral(_xmin,_xmax) ;  
  //cout << " integral = " << ret << " neval = " << _integrator->NEval() << " error = " << _integrator->Error() << " rel.error = " << _integrator->RelError() << endl ;
  return ret ;
}

