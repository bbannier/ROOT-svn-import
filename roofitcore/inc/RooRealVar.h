/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 *    File: $Id: RooRealVar.rdl,v 1.40 2004/11/29 12:22:23 wverkerke Exp $
 * Authors:                                                                  *
 *   WV, Wouter Verkerke, UC Santa Barbara, verkerke@slac.stanford.edu       *
 *   DK, David Kirkby,    UC Irvine,         dkirkby@uci.edu                 *
 *                                                                           *
 * Copyright (c) 2000-2004, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/
#ifndef ROO_REAL_VAR
#define ROO_REAL_VAR

#include <iostream>
#include <math.h>
#include <float.h>
#include "TString.h"

#include "RooFitCore/RooAbsRealLValue.hh"
#include "RooFitCore/RooUniformBinning.hh"
#include "RooFitCore/RooNumber.hh"

class RooArgSet ;
class RooErrorVar ;

class RooRealVar : public RooAbsRealLValue {
public:
  // Constructors, assignment etc.
  RooRealVar() ;
  RooRealVar(const char *name, const char *title,
  	   Double_t value, const char *unit= "") ;
  RooRealVar(const char *name, const char *title, Double_t minValue, 
	   Double_t maxValue, const char *unit= "");
  RooRealVar(const char *name, const char *title, Double_t value, 
	   Double_t minValue, Double_t maxValue, const char *unit= "") ;
  RooRealVar(const RooRealVar& other, const char* name=0);
  virtual TObject* clone(const char* newname) const { return new RooRealVar(*this,newname); }
  virtual ~RooRealVar();
  
  // Parameter value and error accessors
  inline virtual Double_t getVal(const RooArgSet* nset=0) const { return _value ; }
  virtual void setVal(Double_t value);
  inline Double_t getError() const { return _error>=0?_error:0. ; }
  inline Bool_t hasError() const { return (_error>=0) ; }
  inline void setError(Double_t value) { _error= value ; }
  inline void removeError() { _error = -1 ; }
  inline Double_t getAsymErrorLo() const { return _asymErrLo<=0?_asymErrLo:0. ; }
  inline Double_t getAsymErrorHi() const { return _asymErrHi>=0?_asymErrHi:0. ; }
  inline Bool_t hasAsymError() const { return (_asymErrHi>=0 && _asymErrLo<=0) ; }
  inline void removeAsymError() { _asymErrLo = 1 ; _asymErrHi = -1 ; }
  inline void setAsymError(Double_t lo, Double_t hi) { _asymErrLo = lo ; _asymErrHi = hi ; }
  RooErrorVar* errorVar() const ;

  // Set/get finite fit range limits
  void setFitMin(Double_t value, const char* name=0) ;
  void setFitMax(Double_t value, const char* name=0) ;
  void setFitRange(Double_t min, Double_t max, const char* name=0) ;
  void setFitBins(Int_t nBins) { setBinning(RooUniformBinning(getFitMin(),getFitMax(),nBins)) ; } 
  void setBinning(const RooAbsBinning& binning, const char* name=0) ;

  // RooAbsRealLValue implementation
  const RooAbsBinning& getBinning(const char* name=0, Bool_t verbose=kTRUE) const ;
  RooAbsBinning& getBinning(const char* name=0, Bool_t verbose=kTRUE) ; 

  // Set infinite fit range limits
  inline void removeFitMin(const char* name=0) { getBinning(name).setMin(-RooNumber::infinity) ; }
  inline void removeFitMax(const char* name=0) { getBinning(name).setMax(RooNumber::infinity) ; }
  inline void removeFitRange(const char* name=0) { getBinning(name).setRange(-RooNumber::infinity,RooNumber::infinity) ; }
 
  // I/O streaming interface (machine readable)
  virtual Bool_t readFromStream(std::istream& is, Bool_t compact, Bool_t verbose=kFALSE) ;
  virtual void writeToStream(std::ostream& os, Bool_t compact) const ;

  // We implement a fundamental type of AbsArg that can be stored in a dataset
  inline virtual Bool_t isFundamental() const { return kTRUE; }

  // Printing interface (human readable)
  virtual void printToStream(std::ostream& stream, PrintOption opt=Standard, TString indent= "") const ;
  TString* format(Int_t sigDigits, const char *options) const ;

  static void printScientific(Bool_t flag=kFALSE) { _printScientific = flag ; }
  static void printSigDigits(Int_t ndig=5) { _printSigDigits = ndig>1?ndig:1 ; }

protected:

  static Bool_t _printScientific ;
  static Int_t  _printSigDigits ;

  virtual Double_t evaluate() const { return _value ; } // dummy because we overloaded getVal()
  virtual void copyCache(const RooAbsArg* source) ;
  virtual void attachToTree(TTree& t, Int_t bufSize=32000) ;
  virtual void fillTreeBranch(TTree& t) ;

  Double_t chopAt(Double_t what, Int_t where) const ;

  Double_t  *_min ;
  Double_t  *_max ;
  TString   *_name ;

  RooLinkedList _altBinning ;  //! Optional alternative ranges and binnings
//   Double_t _fitMin ;    // Minimum of fit range [ obsolete ]
//   Double_t _fitMax ;    // Maximum of fit range [ obsolete ]
//   Int_t    _fitBins ;   // Number of bins in fit range for binned fits [ obsolete ] 
  Double_t _error;      // Symmetric error associated with current value
  Double_t _asymErrLo ; // Low side of asymmetric error associated with current value
  Double_t _asymErrHi ; // High side of asymmetric error associated with current value
  RooAbsBinning* _binning ; 

  ClassDef(RooRealVar,2) // Real-valued variable 
};




#endif
