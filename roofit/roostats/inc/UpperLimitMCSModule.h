/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 *    File: $Id: 
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

#ifndef UPPER_LIMIT_MCS_MODULE
#define UPPER_LIMIT_MCS_MODULE

#include "RooAbsMCStudyModule.h"
#include "RooStats/ProfileLikelihoodCalculator.h"
#include "RooRealVar.h"
#include "RooFit.h"
#include <string>

namespace RooStats {

class UpperLimitMCSModule : public RooAbsMCStudyModule {
public:


  UpperLimitMCSModule(RooArgSet* poi, Double_t CL=0.95) ;
  UpperLimitMCSModule(const UpperLimitMCSModule& other) ;
  virtual ~UpperLimitMCSModule() ;

  Bool_t initializeInstance() ; 

  Bool_t initializeRun(Int_t /*numSamples*/) ; 
  RooDataSet* finalizeRun() ;

  //Bool_t processAfterFit(Int_t /*sampleNum*/)  ;
  Bool_t processBetweenGenAndFit(Int_t /*sampleNum*/) ;
	
private:

  std::string _parName ;  // Name of Nsignal parameter
  RooStats::ProfileLikelihoodCalculator* _plc;
  RooRealVar* _ul ;

  RooArgSet* _poi;
  RooDataSet* _data ;     // Summary dataset to store results
  Double_t _cl;
  RooAbsPdf* _model;

  ClassDef(UpperLimitMCSModule,0) // MCStudy module to calculate upperlimit of a given poi
} ;

}

#endif

