/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       * 
 * @(#)root/roofitcore:$Id$
 * Authors:                                                                  *
 *   AL, Alfio Lazzaro,   INFN Milan,        alfio.lazzaro@mi.infn.it        *
 *                                                                           *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/

#ifndef __ROOFIT_NOROOMINIMIZER

#ifndef ROO_MINIMIZER_FCN
#define ROO_MINIMIZER_FCN

#include "Math/IFunction.h"
#include "Fit/ParameterSettings.h"
#include "Fit/FitResult.h"

#include "TMatrixDSym.h"

#include "RooAbsReal.h"
#include "RooArgList.h"

#include <iostream>
#include <fstream>

class RooMinimizer;

class RooMinimizerFcn : public ROOT::Math::IBaseFunctionMultiDim {

 public:

  RooMinimizerFcn(RooAbsReal *funct, const RooMinimizer *context);
  virtual ~RooMinimizerFcn();

  virtual ROOT::Math::IBaseFunctionMultiDim* Clone() const;
  virtual UInt_t NDim() const { return _nDim; }

  RooArgList* GetFloatParamList() { return _floatParamList; }
  RooArgList* GetConstParamList() { return _constParamList; }
  RooArgList* GetInitFloatParamList() { return _initFloatParamList; }
  RooArgList* GetInitConstParamList() { return _initConstParamList; }

  Bool_t Synchronize(std::vector<ROOT::Fit::ParameterSettings>& parameters);
  void BackProp(const ROOT::Fit::FitResult &results);  
  void ApplyCovarianceMatrix(TMatrixDSym& V); 

 private:

  Double_t GetPdfParamVal(Int_t index);
  Double_t GetPdfParamErr(Int_t index);
  void SetPdfParamErr(Int_t index, Double_t value);
  void ClearPdfParamAsymErr(Int_t index);
  void SetPdfParamErr(Int_t index, Double_t loVal, Double_t hiVal);

  inline Bool_t SetPdfParamVal(const Int_t &index, const Double_t &value) const;

  virtual Double_t DoEval(const Double_t *x) const;  

private:
  
  RooAbsReal *_funct;
  const RooMinimizer *_context;
  Int_t _nDim;

  RooArgList* _floatParamList;
  RooArgList* _constParamList;
  RooArgList* _initFloatParamList;
  RooArgList* _initConstParamList;

};

#endif
#endif
