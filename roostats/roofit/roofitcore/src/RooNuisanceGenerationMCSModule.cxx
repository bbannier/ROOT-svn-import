/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *

 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/

// Author: This module has been designed by Nils Ruthmann

//////////////////////////////////////////////////////////////////////////////
// 
// BEGIN_HTML
// This modules allow to randomize nuisance parameters before a dataset generation in the ToyMcStudy framework

// END_HTML
//
//

#include "Riostream.h"
#include "RooDataSet.h"
//#include "RooRealVar.h"
#include "TString.h"
//#include "RooFit.h"
#include "RooFitResult.h"
#include "RooNuisanceGenerationMCSModule.h"
#include "RooMsgService.h"
//#include "RooStats/ConfInterval.h"
//#include "RooStats/PointSetInterval.h"
//#include "TCanvas.h"
#include "TIterator.h"



ClassImp(RooNuisanceGenerationMCSModule)
  ;




//_____________________________________________________________________________
RooNuisanceGenerationMCSModule::RooNuisanceGenerationMCSModule(RooArgSet* nuisancepars,RooAbsPdf* priorNuisance) : 
  RooAbsMCStudyModule("NuisanceGenerationMCSModule","NuisanceGenerationMCSModule"),
  _nuisancePars(nuisancepars),_priorNuisance(priorNuisance)
{
  std::cout<<"RooNuisanceGenerationConstructor nuisanceName:"<<std::endl;
  nuisancepars->Print();
  
  // Constructor of module with parameter to be interpreted as nSignal and the value of the
  // null hypothesis for nSignal (usually zero)
}






//_____________________________________________________________________________
RooNuisanceGenerationMCSModule:: ~RooNuisanceGenerationMCSModule() 
{
  // Destructor

 
  if (_nuisancePars) {
    delete _nuisancePars ;
  }
  if (_priorNuisance) {
    delete _priorNuisance ;
  }
}



//_____________________________________________________________________________
Bool_t RooNuisanceGenerationMCSModule::initializeInstance()
{
  // Initialize module after attachment to RooMCStudy object

  // Check that parameter is also present in fit parameter list of RooMCStudy object
  TIterator* it;
  it =_nuisancePars->createIterator();
  RooRealVar* currentparam=(RooRealVar*) it->Next();

  do {
    if (!fitParams()->find(currentparam->GetName())) {
      coutE(InputArguments) << "RooNuisanceGenerationMCSModule::initializeInstance:: ERROR: No parameter named " << currentparam->GetName() << " in RooMCStudy!" << endl ;
      return kFALSE ;
    }
    currentparam=(RooRealVar*) it->Next();
  }while(currentparam!=0);
  

  return kTRUE ;
}

//_____________________________________________________________________________
Bool_t RooNuisanceGenerationMCSModule::processBeforeGen(Int_t )
{

  RooDataSet* generatedNuisance =_priorNuisance->generate(*_nuisancePars,1);
 
  TIterator* it =_nuisancePars->createIterator();
  RooRealVar* currentparam=(RooRealVar*) it->Next();
  do {
    ((RooRealVar*)fitParams()->find(currentparam->GetName()))->setVal(generatedNuisance->get()->getRealValue(currentparam->GetName()));
    
    
    currentparam=(RooRealVar*) it->Next();
  }while(currentparam!=0);
  /*
  it =_nuisancePars.createIterator();
  currentparam=(RooRealVar*) it->Next();
  do {
    ((RooRealVar*)(fitParams()->find(currentparam->GetName())))->setVal(currentparam->getVal());
    std::cout<<"RooNuisanceGenerationMCSMOdule:  "<<currentparam->GetName()<<"="<<currentparam->getVal()<<std::endl;
    currentparam=(RooRealVar*)it->Next();
  }while(currentparam!=0);
  */

  //fitParams()->Print("v");
  return kTRUE;


}




//_____________________________________________________________________________
Bool_t RooNuisanceGenerationMCSModule::initializeRun(Int_t /*numSamples*/) 
{
  // Initialize module at beginning of RooCMStudy run

  
  return kTRUE ;
}








