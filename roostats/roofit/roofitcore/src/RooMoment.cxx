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
// RooMoment represents the first, second, or third order derivative
// of any RooAbsReal as calculated (numerically) by the MathCore Richardson
// derivator class.
// END_HTML
//


#include "RooFit.h"

#include "Riostream.h"
#include "Riostream.h"
#include <math.h>
#include <string>

#include "RooMoment.h"
#include "RooAbsReal.h"
#include "RooAbsPdf.h"
#include "RooErrorHandler.h"
#include "RooArgSet.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooFunctor.h"
#include "RooFormulaVar.h"
#include "RooGlobalFunc.h"
#include "RooConstVar.h"
#include <string>
using namespace std ;


ClassImp(RooMoment)
;


//_____________________________________________________________________________
RooMoment::RooMoment() 
{
  // Default constructor
}



//_____________________________________________________________________________
RooMoment::RooMoment(const char* name, const char* title, RooAbsReal& func, RooRealVar& x, Int_t orderIn, Bool_t centr, Bool_t takeRoot) :
  RooAbsReal(name, title),
  _order(orderIn),
  _takeRoot(takeRoot),
  _nset("nset","nset",this,kFALSE,kFALSE),
  _func("function","function",this,func,kFALSE,kFALSE),
  _x("x","x",this,x,kFALSE,kFALSE),
  _mean("!mean","!mean",this,kFALSE,kFALSE),
  _xf("!xf","xf",this,kFALSE,kFALSE),
  _ixf("!ixf","ixf",this),
  _if("!if","if",this)
{
  
  string pname=Form("%s_product",name) ;

  RooFormulaVar* XF ;
  if (centr) {
    string formula=Form("pow((@0-@1),%d)*@2",_order) ;
    string m1name=Form("%s_moment1",GetName()) ;
    RooAbsReal* mom1 = new RooMoment(m1name.c_str(),m1name.c_str(),func,x,1,kFALSE) ;
    XF = new RooFormulaVar(pname.c_str(),formula.c_str(),RooArgList(x,*mom1,func)) ;
    addOwnedComponents(*mom1) ;
    _mean.setArg(*mom1) ;
  } else {
    string formula=Form("pow(@0*@1,%d)",_order) ;
    XF = new RooFormulaVar(pname.c_str(),formula.c_str(),RooArgSet(x,func)) ;
  }
  RooAbsReal* intXF = XF->createIntegral(x) ;
  RooAbsReal* intF =  func.createIntegral(x) ;
  _xf.setArg(*XF) ;
  _ixf.setArg(*intXF) ;
  _if.setArg(*intF) ;
  addOwnedComponents(RooArgSet(*XF,*intXF,*intF)) ;
}

//_____________________________________________________________________________
RooMoment::RooMoment(const char* name, const char* title, RooAbsReal& func, RooRealVar& x, const RooArgSet& nset, 
		     Int_t orderIn, Bool_t centr, Bool_t takeRoot, Bool_t intNSet) :
  RooAbsReal(name, title),
  _order(orderIn),
  _takeRoot(takeRoot),
  _nset("nset","nset",this,kFALSE,kFALSE),
  _func("function","function",this,func),
  _x("x","x",this,x),
  _mean("!mean","!mean",this,kFALSE,kFALSE),
  _xf("!xf","xf",this,kFALSE,kFALSE),
  _ixf("!ixf","ixf",this),
  _if("!if","if",this)
{
  _nset.add(nset) ;

  string pname=Form("%s_product",name) ;
  RooFormulaVar* XF ;
  if (centr) {
    string formula=Form("pow((@0-@1),%d)*@2",_order) ;
    string m1name=Form("%s_moment1",GetName()) ;
    RooAbsReal* mom1 = new RooMoment(m1name.c_str(),m1name.c_str(),func,x,1,kFALSE,intNSet) ;
    XF = new RooFormulaVar(pname.c_str(),formula.c_str(),RooArgList(x,*mom1,func)) ;
    addOwnedComponents(*mom1) ;
    _mean.setArg(*mom1) ;
  } else {
    string formula=Form("pow(@0*@1,%d)",_order) ;
    XF = new RooFormulaVar(pname.c_str(),formula.c_str(),RooArgSet(x,func)) ;
  }

  RooArgSet intSet(x) ;
  if (intNSet) intSet.add(_nset,kTRUE) ;

  RooAbsReal* intXF = XF->createIntegral(intSet,&_nset) ;
  RooAbsReal* intF =  func.createIntegral(intSet,&_nset) ;
  _xf.setArg(*XF) ;
  _ixf.setArg(*intXF) ;
  _if.setArg(*intF) ;
  addOwnedComponents(RooArgSet(*XF,*intXF,*intF)) ;
}



//_____________________________________________________________________________
RooMoment::RooMoment(const RooMoment& other, const char* name) :
  RooAbsReal(other, name), 
  _order(other._order),  
  _nset("nset",this,other._nset),
  _func("function",this,other._func),
  _x("x",this,other._x),
  _mean("!mean","!mean",this,kFALSE,kFALSE),
  _xf("xf",this,other._xf),
  _ixf("ixf",this,other._ixf),
  _if("if",this,other._if)
{
}



//_____________________________________________________________________________
RooMoment::~RooMoment() 
{
  // Destructor
}



//_____________________________________________________________________________
Double_t RooMoment::evaluate() const 
{
  // Calculate value  
  Double_t ratio = _ixf / _if ;
  return _takeRoot ? pow(ratio,1.0/_order) : ratio ;
}


