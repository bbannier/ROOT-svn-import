/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 *    File: $Id: RooAbsReal.h,v 1.75 2007/07/13 21:50:24 wouter Exp $
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
#ifndef ROO_ABS_REAL
#define ROO_ABS_REAL

#include "RooAbsArg.h"
#include "RooCmdArg.h"
#include "RooCurve.h"
#include "RooArgSet.h"

class RooArgList ;
class RooDataSet ;
class RooPlot;
class RooRealVar;
class RooAbsFunc;
class RooAbsCategoryLValue ;
class RooCategory ;
class RooLinkedList ;
class RooNumIntConfig ;
class RooDataHist ;

class TH1;
class TH1F;
class TH2F;
class TH3F;

#include <list>
#include <string>
#include <iostream>

class RooAbsReal : public RooAbsArg {
public:
  // Constructors, assignment etc
  RooAbsReal() ;
  RooAbsReal(const char *name, const char *title, const char *unit= "") ;
  RooAbsReal(const char *name, const char *title, Double_t minVal, Double_t maxVal, 
	     const char *unit= "") ;
  RooAbsReal(const RooAbsReal& other, const char* name=0);
  virtual ~RooAbsReal();

  // Return value and unit accessors
  virtual Double_t getVal(const RooArgSet* set=0) const ;
  inline  Double_t getVal(const RooArgSet& set) const { return getVal(&set) ; }
  Bool_t operator==(Double_t value) const ;
  virtual Bool_t operator==(const RooAbsArg& other) ;
  inline const Text_t *getUnit() const { return _unit.Data(); }
  inline void setUnit(const char *unit) { _unit= unit; }
  TString getTitle(Bool_t appendUnit= kFALSE) const;

  // Lightweight interface adaptors (caller takes ownership)
  RooAbsFunc *bindVars(const RooArgSet &vars, const RooArgSet* nset=0, Bool_t clipInvalid=kFALSE) const;

  // Create a fundamental-type object that can hold our value.
  RooAbsArg *createFundamental(const char* newname=0) const;

  // Analytical integration support
  virtual Int_t getAnalyticalIntegralWN(RooArgSet& allVars, RooArgSet& analVars, const RooArgSet* normSet, const char* rangeName=0) const ;
  virtual Double_t analyticalIntegralWN(Int_t code, const RooArgSet* normSet, const char* rangeName=0) const ;
  virtual Int_t getAnalyticalIntegral(RooArgSet& allVars, RooArgSet& analVars, const char* rangeName=0) const ;
  virtual Double_t analyticalIntegral(Int_t code, const char* rangeName=0) const ;
  virtual Bool_t forceAnalyticalInt(const RooAbsArg& /*dep*/) const { return kFALSE ; }
  virtual void forceNumInt(Bool_t flag=kTRUE) { _forceNumInt = flag ; }

  RooAbsReal* createIntegral(const RooArgSet& iset, const RooCmdArg arg1, const RooCmdArg arg2=RooCmdArg::none(),
                             const RooCmdArg arg3=RooCmdArg::none(), const RooCmdArg arg4=RooCmdArg::none(), const RooCmdArg arg5=RooCmdArg::none(), 
                             const RooCmdArg arg6=RooCmdArg::none(), const RooCmdArg arg7=RooCmdArg::none(), const RooCmdArg arg8=RooCmdArg::none()) const ;

  RooAbsReal* createIntegral(const RooArgSet& iset, const char* rangeName) const 
              { return createIntegral(iset,0,0,rangeName) ; }
  RooAbsReal* createIntegral(const RooArgSet& iset, const RooArgSet& nset, const char* rangeName=0) const 
              { return createIntegral(iset,&nset,0,rangeName) ; }
  RooAbsReal* createIntegral(const RooArgSet& iset, const RooArgSet& nset, const RooNumIntConfig& cfg, const char* rangeName=0) const 
              { return createIntegral(iset,&nset,&cfg,rangeName) ; }
  RooAbsReal* createIntegral(const RooArgSet& iset, const RooNumIntConfig& cfg, const char* rangeName=0) const 
              { return createIntegral(iset,0,&cfg,rangeName) ; }
  virtual RooAbsReal* createIntegral(const RooArgSet& iset, const RooArgSet* nset=0, const RooNumIntConfig* cfg=0, const char* rangeName=0) const ;  

  
  // Optimized accept/reject generator support
  virtual Int_t getMaxVal(const RooArgSet& vars) const ;
  virtual Double_t maxVal(Int_t code) ;


  // Plotting options
  inline Double_t getPlotMin() const { return _plotMin; }
  inline Double_t getPlotMax() const { return _plotMax; }
  virtual Int_t getPlotBins() const { return _plotBins; }
  void setPlotMin(Double_t value) ;
  void setPlotMax(Double_t value) ;
  void setPlotRange(Double_t min, Double_t max) ;
  void setPlotBins(Int_t value) ; 
  void setPlotLabel(const char *label);
  const char *getPlotLabel() const;
  virtual Bool_t inPlotRange(Double_t value) const;

  virtual Double_t defaultErrorLevel() const { return 1.0 ; }

  const RooNumIntConfig* getIntegratorConfig() const ;
  static RooNumIntConfig* defaultIntegratorConfig()  ;
  RooNumIntConfig* specialIntegratorConfig() const ;
  void setIntegratorConfig() ;
  void setIntegratorConfig(const RooNumIntConfig& config) ;

  virtual void fixAddCoefNormalization(const RooArgSet& addNormSet=RooArgSet(),Bool_t force=kTRUE) ;
  virtual void fixAddCoefRange(const char* rangeName=0,Bool_t force=kTRUE) ;


public:

  // User entry point for plotting
  enum ScaleType { Raw, Relative, NumEvent, RelativeExpected } ;
  virtual RooPlot* plotOn(RooPlot* frame, 
			  const RooCmdArg& arg1=RooCmdArg(), const RooCmdArg& arg2=RooCmdArg(),
			  const RooCmdArg& arg3=RooCmdArg(), const RooCmdArg& arg4=RooCmdArg(),
			  const RooCmdArg& arg5=RooCmdArg(), const RooCmdArg& arg6=RooCmdArg(),
			  const RooCmdArg& arg7=RooCmdArg(), const RooCmdArg& arg8=RooCmdArg(),
			  const RooCmdArg& arg9=RooCmdArg(), const RooCmdArg& arg10=RooCmdArg()
              ) const ;

  // Forwarder function for backward compatibility
  virtual RooPlot *plotSliceOn(RooPlot *frame, const RooArgSet& sliceSet, Option_t* drawOptions="L", 
			       Double_t scaleFactor=1.0, ScaleType stype=Relative, const RooAbsData* projData=0) const;

  // Fill an existing histogram
  TH1 *fillHistogram(TH1 *hist, const RooArgList &plotVars,
		     Double_t scaleFactor= 1, const RooArgSet *projectedVars= 0, Bool_t scaling=kTRUE) const;

  // Create 1,2, and 3D histograms from and fill it
  TH1 *createHistogram(const char *name, const RooAbsRealLValue& xvar,
                       const RooCmdArg& arg1=RooCmdArg::none(), const RooCmdArg& arg2=RooCmdArg::none(), 
                       const RooCmdArg& arg3=RooCmdArg::none(), const RooCmdArg& arg4=RooCmdArg::none(), 
                       const RooCmdArg& arg5=RooCmdArg::none(), const RooCmdArg& arg6=RooCmdArg::none(), 
                       const RooCmdArg& arg7=RooCmdArg::none(), const RooCmdArg& arg8=RooCmdArg::none()) const ;

  // Fill a RooDataHist
  RooDataHist* fillDataHist(RooDataHist *hist, Double_t scaleFactor) const ;

  // I/O streaming interface (machine readable)
  virtual Bool_t readFromStream(istream& is, Bool_t compact, Bool_t verbose=kFALSE) ;
  virtual void writeToStream(ostream& os, Bool_t compact) const ;

  // Printing interface (human readable)
  virtual void printValue(ostream& os) const ;
  virtual void printMultiline(ostream& os, Int_t contents, Bool_t verbose=kFALSE, TString indent="") const ;

  static void setCacheCheck(Bool_t flag) ;

  // Evaluation error logging 
  class EvalError {
  public:
    EvalError() { _msg[0] = 0 ; _srvval[0] = 0 ; }
    EvalError(const EvalError& other) { strcpy(_msg,other._msg) ; strcpy(_srvval,other._srvval) ; } ;
    void setMessage(const char* tmp) { strcpy(_msg,tmp) ; }
    void setServerValues(const char* tmp) { strcpy(_srvval,tmp) ; }
    char _msg[1024] ;
    char _srvval[1024] ;
  } ;

  static Bool_t evalErrorLoggingEnabled() { return _doLogEvalError ; }
  static void enableEvalErrorLogging(Bool_t flag) { _doLogEvalError = flag ; }
  void logEvalError(const char* message, const char* serverValueString=0) const ;
  static void printEvalErrors(ostream&os=std::cout, Int_t maxPerNode=10000000) ;
  static Int_t numEvalErrors() ;
  static Int_t numEvalErrorItems() { return _evalErrorList.size() ; }
  static std::map<const RooAbsArg*,std::list<EvalError> >::const_iterator evalErrorIter() { return _evalErrorList.begin() ; }

  static void clearEvalErrorLog() ;

protected:

  // PlotOn with command list
  virtual RooPlot* plotOn(RooPlot* frame, RooLinkedList& cmdList) const ;

  // Hook for objects with normalization-dependent parameters interperetation
  virtual void selectNormalization(const RooArgSet* depSet=0, Bool_t force=kFALSE) ;
  virtual void selectNormalizationRange(const char* rangeName=0, Bool_t force=kFALSE) ;

  // Helper functions for plotting
  Bool_t plotSanityChecks(RooPlot* frame) const ;
  void makeProjectionSet(const RooAbsArg* plotVar, const RooArgSet* allVars, 
			 RooArgSet& projectedVars, Bool_t silent) const ;

  TString integralNameSuffix(const RooArgSet& iset, const RooArgSet* nset=0, const char* rangeName=0) const ;

 public:
  const RooAbsReal* createPlotProjection(const RooArgSet& depVars, const RooArgSet& projVars) const ;
  const RooAbsReal* createPlotProjection(const RooArgSet& depVars, const RooArgSet& projVars, RooArgSet*& cloneSet) const ;
  const RooAbsReal *createPlotProjection(const RooArgSet &dependentVars, const RooArgSet *projectedVars,
				         RooArgSet *&cloneSet, const char* rangeName=0) const;
 protected:

  // Support interface for subclasses to advertise their analytic integration
  // and generator capabilities in their analticalIntegral() and generateEvent()
  // implementations.
  Bool_t matchArgs(const RooArgSet& allDeps, RooArgSet& numDeps, 
		   const RooArgProxy& a) const ;
  Bool_t matchArgs(const RooArgSet& allDeps, RooArgSet& numDeps, 
		   const RooArgProxy& a, const RooArgProxy& b) const ;
  Bool_t matchArgs(const RooArgSet& allDeps, RooArgSet& numDeps, 
		   const RooArgProxy& a, const RooArgProxy& b, const RooArgProxy& c) const ;
  Bool_t matchArgs(const RooArgSet& allDeps, RooArgSet& numDeps, 
		   const RooArgProxy& a, const RooArgProxy& b, 		   
		   const RooArgProxy& c, const RooArgProxy& d) const ;

  Bool_t matchArgs(const RooArgSet& allDeps, RooArgSet& numDeps, 
		   const RooArgSet& set) const ;


  RooAbsReal* createIntObj(const RooArgSet& iset, const RooArgSet* nset, const RooNumIntConfig* cfg, const char* rangeName) const ;
  void findInnerMostIntegration(const RooArgSet& allObs, RooArgSet& innerObs, const char* rangeName) const ;


  // Internal consistency checking (needed by RooDataSet)
  virtual Bool_t isValid() const ;
  virtual Bool_t isValidReal(Double_t value, Bool_t printError=kFALSE) const ;

  // Function evaluation and error tracing
  Double_t traceEval(const RooArgSet* set) const ;
  virtual Bool_t traceEvalHook(Double_t /*value*/) const { return kFALSE ;}
  virtual Double_t evaluate() const = 0 ;

  // Hooks for RooDataSet interface
  friend class RooRealIntegral ;
  virtual void syncCache(const RooArgSet* set=0) { getVal(set) ; }
  virtual void copyCache(const RooAbsArg* source) ;
  virtual void attachToTree(TTree& t, Int_t bufSize=32000) ;
  virtual void setTreeBranchStatus(TTree& t, Bool_t active) ;
  virtual void fillTreeBranch(TTree& t) ;

  Double_t _plotMin ;       // Minimum of plot range
  Double_t _plotMax ;       // Maximum of plot range
  Int_t    _plotBins ;      // Number of plot bins
  mutable Double_t _value ; // Cache for current value of object
  TString  _unit ;          // Unit for objects value
  TString  _label ;         // Plot label for objects value
  Bool_t   _forceNumInt ;   // Force numerical integration if flag set

  mutable Float_t _floatValue ; //! Transient cache for floating point values from tree branches 
  mutable Int_t   _intValue   ; //! Transient cache for integer values from tree branches 
  mutable UChar_t _byteValue  ; //! Transient cache for byte values from tree branches 
  mutable UInt_t  _uintValue  ; //! Transient cache for unsigned integer values from tree branches 

  friend class RooAbsPdf ;
  friend class RooAbsAnaConvPdf ;
  friend class RooRealProxy ;

  RooNumIntConfig* _specIntegratorConfig ; //! Numeric integrator configuration specific for this object

  Bool_t   _treeVar ;       // !do not persist

  static Bool_t _cacheCheck ;

  friend class RooDataProjBinding ;
  friend class RooAbsOptGoodnessOfFit ;
  
  struct PlotOpt {
   PlotOpt() : drawOptions("L"), scaleFactor(1.0), stype(Relative), projData(0), binProjData(kFALSE), projSet(0), precision(1e-3), 
               shiftToZero(kFALSE),projDataSet(0),rangeLo(0),rangeHi(0),postRangeFracScale(kFALSE),wmode(RooCurve::Extended),
               projectionRangeName(0),curveInvisible(kFALSE), curveName(0),addToCurveName(0),addToWgtSelf(1.),addToWgtOther(1.),
               numCPU(1),interleave(kTRUE) {} ;
   Option_t* drawOptions ;
   Double_t scaleFactor ;	 
   ScaleType stype ;
   const RooAbsData* projData ;
   Bool_t binProjData ;
   const RooArgSet* projSet ;
   Double_t precision ;
   Bool_t shiftToZero ;
   const RooArgSet* projDataSet ;
   Double_t rangeLo ;
   Double_t rangeHi ;
   Bool_t postRangeFracScale ;
   RooCurve::WingMode wmode ;
   const char* projectionRangeName ;
   Bool_t curveInvisible ;
   const char* curveName ;
   const char* addToCurveName ;
   Double_t addToWgtSelf ;
   Double_t addToWgtOther ;
   Int_t    numCPU ;
   Bool_t interleave ;
  } ;

  // Plot implementation functions
  virtual RooPlot *plotOn(RooPlot* frame, PlotOpt o) const;
  virtual RooPlot *plotAsymOn(RooPlot *frame, const RooAbsCategoryLValue& asymCat, PlotOpt o) const;

private:

  static Bool_t _doLogEvalError ;
  static std::map<const RooAbsArg*,std::list<EvalError> > _evalErrorList ;

  Bool_t matchArgsByName(const RooArgSet &allArgs, RooArgSet &matchedArgs, const TList &nameList) const;

protected:

  ClassDef(RooAbsReal,1) // Abstract real-valued variable
};

#endif
