/*****************************************************************************
 * Project: BaBar detector at the SLAC PEP-II B-factory
 * Package: RooFitCore
 *    File: $Id: RooAbsPdf.rdl,v 1.32 2001/10/01 22:04:20 verkerke Exp $
 * Authors:
 *   DK, David Kirkby, Stanford University, kirkby@hep.stanford.edu
 *   WV, Wouter Verkerke, UC Santa Barbara, verkerke@slac.stanford.edu
 *   AB, Adrian Bevan, Liverpool University, bevan@slac.stanford.edu
 * History:
 *   07-Mar-2001 WV Created initial version
 *   26-Aug-2001 AB Added TH2F * plot methods
 *
 * Copyright (C) 2001 University of California
 *****************************************************************************/
#ifndef ROO_ABS_PDF
#define ROO_ABS_PDF

#include "RooFitCore/RooAbsReal.hh"
#include "RooFitCore/RooRealIntegral.hh"
#include "RooFitCore/RooNameSet.hh"

class RooDataSet;
class RooArgSet ;
class RooRealProxy ;
class RooGenContext ;
class RooFitResult ;
class TPaveText;
class TH1F;
class TH2F;
class TList ;

class RooAbsPdf : public RooAbsReal {
public:

  // Constructors, assignment etc
  inline RooAbsPdf() { }
  RooAbsPdf(const char *name, const char *title) ;
  RooAbsPdf(const char *name, const char *title, Double_t minVal, Double_t maxVal) ;
  // RooAbsPdf(const RooAbsPdf& other, const char* name=0);
  virtual ~RooAbsPdf();

  // Toy MC generation
  RooDataSet *generate(const RooArgSet &whatVars, Int_t nEvents = 0, Bool_t verbose=kFALSE) const;
  RooDataSet *generate(const RooArgSet &whatVars, const RooDataSet &prototype, Bool_t verbose=kFALSE) const;

  virtual RooPlot *plotOn(RooPlot *frame, Option_t* drawOptions="L", Double_t scaleFactor= 1.0, 
			  ScaleType stype=Relative, const RooArgSet* projSet=0) const;

  Double_t analyticalIntegralWN(Int_t code, const RooArgSet* normSet) const ;

  // Built-in generator support
  virtual Int_t getGenerator(const RooArgSet& directVars, RooArgSet &generateVars) const;
  virtual void generateEvent(Int_t code);

  // PDF-specific plotting & display
  TH1F *Scan(RooDataSet* data, RooRealVar &param, Int_t bins= 0) { return 0 ; } 
  TH1F *Scan(RooDataSet& data, RooRealVar &param, Int_t bins= 0) { return 0 ; } 
  TH2F *PlotContours(RooRealVar& var1, RooRealVar& var2,
		     Double_t n1= 1, Double_t n2= 2, Double_t n3= 0) { return 0 ; } 
  TPaveText *Parameters(const char *label= "", Int_t sigDigits = 2,
			Option_t *options = "NELU", Double_t xmin=0.65,
                        Double_t xmax= 0.99,Double_t ymax=0.95) { return 0 ; } 

  // Interactions with a dataset  
  virtual const RooFitResult* fitTo(RooAbsData& data, Option_t *fitOpt = "", Option_t *optOpt = "cpds" ) ;

  // Function evaluation support
  virtual Bool_t traceEvalHook(Double_t value) const ;  
  virtual Double_t getVal(const RooArgSet* set=0) const ;
  Double_t getLogVal(const RooArgSet* set=0) const ;
  virtual Double_t getNorm(const RooArgSet* set=0) const ;
  void resetErrorCounters(Int_t resetValue=10) ;
  void setTraceCounter(Int_t value) ;
  void traceEvalPdf(Double_t value) const ;

  virtual Bool_t selfNormalized() const { return kFALSE ; }

  // Support for extended maximum likelihood, switched off by default
  virtual Bool_t canBeExtended() const { return kFALSE ; } 
  virtual Double_t expectedEvents() const { return 0 ; } 

  // Printing interface (human readable)
  virtual void printToStream(ostream& stream, PrintOption opt=Standard, TString indent= "") const ;

  static void verboseEval(Int_t stat) { _verboseEval = stat ; }

private:

  // This forces definition copy ctor in derived classes 
  RooAbsPdf(const RooAbsPdf& other);

protected:

  RooAbsPdf(const RooAbsPdf& other, const char* name=0);

  friend class RooRealIntegral ;
  friend class RooFitContext ;
  static Int_t _verboseEval ;

  virtual void syncNormalization(const RooArgSet* dset) const ;
  virtual Bool_t syncNormalizationPreHook(RooAbsReal* norm,const RooArgSet* dset) const { return kFALSE ; } ;
  virtual void syncNormalizationPostHook(RooAbsReal* norm,const RooArgSet* dset) const {} ;

  virtual void operModeHook() ;

  virtual Double_t extendedTerm(UInt_t observedEvents) const ;

  friend class RooConvolutedPdf ;
  mutable Double_t _rawValue ;
  mutable RooAbsReal* _norm   ;      // Normalization integral
  mutable RooArgSet* _lastNormSet ;  // Normalization set pointer for which integral was constructed

  mutable Int_t _errorCount ;        // Number of errors remaining to print
  mutable Int_t _traceCount ;        // Number of traces remaining to print
  mutable Int_t _negCount ;          // Number of negative probablities remaining to print

  ClassDef(RooAbsPdf,1) // Abstract PDF with normalization support
};

#endif
