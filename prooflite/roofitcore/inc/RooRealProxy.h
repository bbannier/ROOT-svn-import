/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 *    File: $Id: RooRealProxy.h,v 1.23 2007/07/12 20:30:28 wouter Exp $
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
#ifndef ROO_REAL_PROXY
#define ROO_REAL_PROXY

#include "RooAbsReal.h"
#include "RooArgProxy.h"
#include "RooAbsRealLValue.h"

class RooRealProxy : public RooArgProxy {
public:

  // Constructors, assignment etc.
  RooRealProxy() {} ;
  RooRealProxy(const char* name, const char* desc, RooAbsArg* owner, RooAbsReal& ref,
	       Bool_t valueServer=kTRUE, Bool_t shapeServer=kFALSE, Bool_t proxyOwnsArg=kFALSE) ;
  RooRealProxy(const char* name, RooAbsArg* owner, const RooRealProxy& other) ;
  virtual TObject* Clone(const char* newName=0) const { return new RooRealProxy(newName,_owner,*this); }
  virtual ~RooRealProxy();

  // Accessors
  inline operator Double_t() const { return _isFund?((RooAbsReal*)_arg)->_value:((RooAbsReal*)_arg)->getVal(_nset) ; }
  inline const RooAbsReal& arg() const { return (RooAbsReal&)*_arg ; }

protected:

  RooAbsRealLValue* lvptr() const ;

public:

  // LValue operations 
  RooRealProxy& operator=(const Double_t& value) { lvptr()->setVal(value) ; return *this ; }
  Double_t min(const char* name=0) const { return lvptr()->getMin(name) ; }
  Double_t max(const char* name=0) const { return lvptr()->getMax(name) ; }


  ClassDef(RooRealProxy,1) // Proxy for a RooAbsReal object
};

#endif
