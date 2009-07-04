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
#ifndef ROO_MOMENT
#define ROO_MOMENT

#include "RooAbsReal.h"
#include "RooRealProxy.h"
#include "RooSetProxy.h"


class RooRealVar;
class RooArgList ;

class RooMoment : public RooAbsReal {
public:

  RooMoment() ;
  RooMoment(const char *name, const char *title, RooAbsReal& func, RooRealVar& x, Int_t order=1, Bool_t central=kFALSE) ;
  RooMoment(const char *name, const char *title, RooAbsReal& func, RooRealVar& x, const RooArgSet& nset, Int_t order=1, Bool_t central=kFALSE) ;
  virtual ~RooMoment() ;

  RooMoment(const RooMoment& other, const char* name = 0);
  virtual TObject* clone(const char* newname) const { return new RooMoment(*this, newname); }

  Int_t order() const { return _order ; }
  Bool_t central() const { return _mean.absArg() ? kTRUE : kFALSE ; }

  RooAbsReal* mean() { return (RooAbsReal*) _mean.absArg() ; }

protected:

  Int_t _order ;                         // Moment order
  RooSetProxy  _nset ;                   // Normalization set (optional)
  RooRealProxy _func ;                   // Input function
  RooRealProxy _x     ;                  // Observable
  RooRealProxy _mean ;                   // Mean (if calculated for central moment)

  RooRealProxy _xf ;                     // X*F 
  RooRealProxy _ixf ;                    // Int(X*F(X))dx ;
  RooRealProxy _if ;                     // Int(F(x))dx ;
  Double_t evaluate() const;

  ClassDef(RooMoment,1) // Representation of moment in a RooAbsReal in a given RooRealVar
};

#endif
