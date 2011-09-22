/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitModels                                                     *
 *    File: $Id: RooGaussian.h,v 1.16 2007/07/12 20:30:49 wouter Exp $
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
#ifndef ROO_GAUSSIAN
#define ROO_GAUSSIAN

#include "RooAbsPdf.h"
#include "RooRealProxy.h"
#include "TMath.h"

class RooRealVar;

class RooGaussian : public RooAbsPdf {
public:
  RooGaussian() {} ;
  RooGaussian(const char *name, const char *title,
	      RooAbsReal& _x, RooAbsReal& _mean, RooAbsReal& _sigma);
  RooGaussian(const RooGaussian& other, const char* name=0) ;
  virtual TObject* clone(const char* newname) const { return new RooGaussian(*this,newname); }
  inline virtual ~RooGaussian() { }

  Int_t getAnalyticalIntegral(RooArgSet& allVars, RooArgSet& analVars, const char* rangeName=0) const ;
  Double_t analyticalIntegral(Int_t code, const char* rangeName=0) const ;

  Int_t getGenerator(const RooArgSet& directVars, RooArgSet &generateVars, Bool_t staticInitOK=kTRUE) const;
  void generateEvent(Int_t code);

  // Used by the template evaluator
  inline Double_t eval(Int_t i) const {
    return eval(x[i],mean[i],sigma[i]) ;
  }


protected:

  RooRealProxy x ;
  RooRealProxy mean ;
  RooRealProxy sigma ;

  inline Double_t eval(const Double_t xValue, const Double_t meanValue, const Double_t sigmaValue) const {
    Double_t arg = xValue - meanValue;
    return TMath::Exp(-0.5*arg*arg/(sigmaValue*sigmaValue)) ;
  }
  
  Double_t evaluate() const ;
  Bool_t evaluateAndNormalizeSIMD(RooAbsReal::DeviceSIMD deviceSIMD, Int_t start, Int_t end, 
				  const RooAbsReal* mother, Double_t integral) const ;


private:

  ClassDef(RooGaussian,1) // Gaussian PDF
};

#endif
