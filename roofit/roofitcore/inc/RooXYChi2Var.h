/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 *    File: $Id$
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

#ifndef ROO_XY_CHI2_VAR
#define ROO_XY_CHI2_VAR

#include "RooAbsOptTestStatistic.h"
#include "RooCmdArg.h"
#include "RooDataSet.h"
#include "RooAbsPdf.h"

class RooXYChi2Var : public RooAbsOptTestStatistic {
public:

  // Constructors, assignment etc
  RooXYChi2Var() {} ;
  RooXYChi2Var(const char *name, const char* title, RooAbsReal& func, RooDataSet& data) ;
  RooXYChi2Var(const char *name, const char* title, RooAbsPdf& extPdf, RooDataSet& data) ;
  
  RooXYChi2Var(const RooXYChi2Var& other, const char* name=0);
  virtual TObject* clone(const char* newname) const { return new RooXYChi2Var(*this,newname); }

  virtual RooAbsTestStatistic* create(const char *name, const char *title, RooAbsReal& pdf, RooAbsData& data,
				      const RooArgSet&, const char*, const char*,Int_t, Bool_t,Bool_t, Bool_t) {
    // Virtual constructor
    return new RooXYChi2Var(name,title,pdf,(RooDataSet&)data) ;
  }
  
  virtual ~RooXYChi2Var();

  virtual Double_t defaultErrorLevel() const { 
    // The default error level for MINUIT error analysis for a chi^2 is 1.0
    return 1.0 ; 
  }

protected:

  Bool_t _extended ;

  virtual Double_t evaluatePartition(Int_t firstEvent, Int_t lastEvent, Int_t stepSize) const ;
  
  ClassDef(RooXYChi2Var,1) // Chi^2 function of p.d.f w.r.t a unbinned dataset with X and Y values
};


#endif
