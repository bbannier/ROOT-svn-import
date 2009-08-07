/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *

 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/

#ifndef ROO_NUISANCE_GENERATION_MCS_MODULE
#define ROO_NUISANCE_GENERATION_MCS_MODULE

#include "RooAbsMCStudyModule.h"
#include "RooRealVar.h"
#include "RooFit.h"
#include "RooAbsPdf.h"
#include <string>

class RooNuisanceGenerationMCSModule : public RooAbsMCStudyModule {
public:


  RooNuisanceGenerationMCSModule(RooArgSet* nuisancepars,RooAbsPdf* priorNuisance) ;
  virtual ~RooNuisanceGenerationMCSModule() ;

  Bool_t initializeInstance() ; 

  Bool_t initializeRun(Int_t /*numSamples*/) ; 


  Bool_t processBeforeGen(Int_t /*sampleNum*/)  ;
 
	
private:


  RooArgSet* _nuisancePars;
  RooAbsPdf* _priorNuisance;
 

  ClassDef(RooNuisanceGenerationMCSModule,0) // MCStudy module to calculate upperlimit of a given poi
} ;


#endif

