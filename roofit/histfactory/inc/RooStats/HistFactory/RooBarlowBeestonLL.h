/*****************************************************************************
 * Project: RooFit                                                           *
 *                                                                           *
 * Copyright (c) 2000-2005, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/

#ifndef ROOBARLOWBEESTONLL
#define ROOBARLOWBEESTONLL

#include "RooAbsReal.h"
#include "RooRealProxy.h"
#include "RooSetProxy.h"
#include <map>
#include <string>

class RooMinuit ;

class RooBarlowBeestonLL : public RooAbsReal {
public:

  RooBarlowBeestonLL() ;
  RooBarlowBeestonLL(const char *name, const char *title, RooAbsReal& nll, const RooArgSet& observables);
  RooBarlowBeestonLL(const RooBarlowBeestonLL& other, const char* name=0) ;
  virtual TObject* clone(const char* newname) const { return new RooBarlowBeestonLL(*this,newname); }
  virtual ~RooBarlowBeestonLL() ;

  RooArgSet* getParameters(const RooArgSet* depList, Bool_t stripDisconnected=kTRUE) const;

  // void setAlwaysStartFromMin(Bool_t flag) { _startFromMin = flag ; }
  // Bool_t alwaysStartFromMin() const { return _startFromMin ; }

  //RooMinuit* minuit() { return _minuit ; }
  RooAbsReal& nll() { return const_cast<RooAbsReal&>(_nll.arg()) ; }
  // const RooArgSet& bestFitParams() const ;
  // const RooArgSet& bestFitObs() const ;

  virtual RooAbsReal* createProfile(const RooArgSet& paramsOfInterest) ;
  
  virtual Bool_t redirectServersHook(const RooAbsCollection& /*newServerList*/, Bool_t /*mustReplaceAll*/, Bool_t /*nameChange*/, Bool_t /*isRecursive*/) ;

  // void clearAbsMin() { _absMinValid = kFALSE ; }

  // Int_t numEval() const { return _neval ; }

  void setPdf(RooAbsPdf* pdf) { _pdf = pdf; }

  //void FactorizePdf(const RooArgSet &observables, RooAbsPdf &pdf, 
  //	    RooArgList &obsTerms, RooArgList &constraints) const;


protected:

  // void validateAbsMin() const ;

  RooRealProxy _nll ;    // Input -log(L) function
  RooSetProxy _obs ;     // Parameters of profile likelihood
  RooSetProxy _par ;     // Marginialized parameters of likelihood
  RooAbsPdf* _pdf;
  // Bool_t _startFromMin ; // Always start minimization for global minimum?

  TIterator* _piter ; //! Iterator over profile likelihood parameters to be minimized 
  TIterator* _oiter ; //! Iterator of profile likelihood output parameter(s)

  // mutable RooMinuit* _minuit ; //! Internal minuit instance

  // mutable Bool_t _absMinValid ; // flag if absmin is up-to-date
  // mutable Double_t _absMin ; // absolute minimum of -log(L)
  // mutable RooArgSet _paramAbsMin ; // Parameter values at absolute minimum
  // mutable RooArgSet _obsAbsMin ; // Observable values at absolute minimum
  mutable std::map<std::string,bool> _paramFixed ; // Parameter constant status at last time of use
  // mutable Int_t _neval ; // Number evaluations used in last minimization
  Double_t evaluate() const ;


private:

  ClassDef(RooBarlowBeestonLL,0) // Real-valued function representing a Barlow-Beeston minimized profile likelihood of external (likelihood) function
};
 
#endif
