// @(#)root/tmva $Id$
// Author: Tancredi Carli, Dominik Dannheim, Alexander Voigt

/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate Data analysis       *
 * Package: TMVA                                                                  *
 * Class  : MethodBPDEFoam                                                        *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Implementation (see header for description)                               *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      Tancredi Carli   - CERN, Switzerland                                      *
 *      Dominik Dannheim - CERN, Switzerland                                      *
 *      Alexander Voigt  - CERN, Switzerland                                      *
 *      Peter Speckmayer - CERN, Switzerland                                      *
 *                                                                                *
 * Original author of the TFoam implementation:                                   *
 *      S. Jadach - Institute of Nuclear Physics, Cracow, Poland                  *
 *                                                                                *
 * Copyright (c) 2008:                                                            *
 *      CERN, Switzerland                                                         *
 *      MPI-K Heidelberg, Germany                                                 *
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/

//_______________________________________________________________________

#include <sstream>

#ifndef ROOT_TMVA_ClassifierFactory
#include "TMVA/ClassifierFactory.h"
#endif
#ifndef ROOT_TMVA_Tools
#include "TMVA/Tools.h"
#endif
#ifndef ROOT_TMVA_Results
#include "TMVA/Results.h"
#endif
#ifndef ROOT_TMVA_MethodBPDEFoam
#include "TMVA/MethodBPDEFoam.h"
#endif
#include "TMath.h"

using namespace std;

REGISTER_METHOD(BPDEFoam)

ClassImp(TMVA::MethodBPDEFoam)

//_______________________________________________________________________
TMVA::MethodBPDEFoam::MethodBPDEFoam( const TString& jobName,
				      const TString& methodTitle,
				      DataSetInfo& dsi,
				      const TString& theOption,
				      TDirectory* theTargetDir ) :
MethodPDEFoam( jobName, methodTitle, dsi, theOption, theTargetDir, Types::kBPDEFoam ),
   fMonitorHist(0),
   fTrainingHist_S(0),
   fTrainingHist_B(0),
   fMVAHist_S(0),
   fMVAHist_B(0),
   fBoostWeightGraph(0),
   fBoostNum(100),
   fWeightType("Quadratic"),
   fMethodError(0),
   fOrigMethodError(0)
{
   // init MethodPDEFoam
}

//_______________________________________________________________________
TMVA::MethodBPDEFoam::MethodBPDEFoam( DataSetInfo& dsi,
				      const TString& theWeightFile,
				      TDirectory* theTargetDir ) :
   MethodPDEFoam( dsi, theWeightFile, theTargetDir, Types::kBPDEFoam ),
   fMonitorHist(0),
   fTrainingHist_S(0),
   fTrainingHist_B(0),
   fMVAHist_S(0),
   fMVAHist_B(0),
   fBoostWeightGraph(0),
   fBoostNum(100),
   fWeightType("Quadratic"),
   fMethodError(0),
   fOrigMethodError(0)
{
   // constructor from weight file
}

//_______________________________________________________________________
TMVA::MethodBPDEFoam::~MethodBPDEFoam()
{
   // destructor
   if (fMonitorHist)  delete fMonitorHist;
   if (fTrainingHist_S) delete fTrainingHist_S;
   if (fTrainingHist_B) delete fTrainingHist_B;
   if (fMVAHist_S) delete fMVAHist_S;
   if (fMVAHist_B) delete fMVAHist_B;
   if (fBoostWeightGraph) delete fBoostWeightGraph;
}

//_______________________________________________________________________
Bool_t TMVA::MethodBPDEFoam::HasAnalysisType( Types::EAnalysisType type, UInt_t numberClasses, UInt_t /*numberTargets*/ )
{
   // BPDEFoam can handle classification with 2 classes and regression
   // with one or more regression-targets
   if (type == Types::kClassification && numberClasses == 2) return kTRUE;
   return kFALSE;
}

//_______________________________________________________________________
void TMVA::MethodBPDEFoam::DeclareOptions()
{
   //
   // Declare MethodBPDEFoam options
   //
   TMVA::MethodPDEFoam::DeclareOptions();
   
   // MethodBPDEFoam specific options
   DeclareOptionRef( fBoostNum = 100, "BoostNum", "Maximum number of boosts");

   DeclareOptionRef( fWeightType = "Quadratic", "BoostWeightType", "How to weight missclassified events");
   AddPreDefVal(TString("Quadratic"));
   AddPreDefVal(TString("Gauss"));
}

//_______________________________________________________________________
void TMVA::MethodBPDEFoam::ProcessOptions() 
{
   TMVA::MethodPDEFoam::ProcessOptions();
   
   // MethodBPDEFoam specific options
   if (fBoostNum < 1) {
      Log() << kWARNING << "Boost_Num = " << fBoostNum 
	    << "is too small, setting it to 100" << Endl;
      fBoostNum = 100;
   }
}

//_______________________________________________________________________
void TMVA::MethodBPDEFoam::Train( void )
{
   // Train boosted classifier

   // init histograms
   InitMonitorHistograms();
   
   // boost loop
   for (UInt_t i=0; i<=fBoostNum; i++){
      Log() << kINFO << "Boost number " << i << Endl;
      // Reset old PDEFoam
      ResetFoams();
      // Train PDEFoam
      TMVA::MethodPDEFoam::Train();
      // Test method
      SingleTest(i);
      FillHistograms(i);
      // boost the method
      Boost(i);
   }
   
   // delete all histograms stored in the Results class
   Data()->GetResults(GetMethodName(), Types::kTesting, GetAnalysisType())->Clear();

   // write histograms
   WriteMonitorHistograms();
}

//_______________________________________________________________________
void TMVA::MethodBPDEFoam::Boost( UInt_t boost_num )
{
   Event * ev; Float_t w,v,wo; Bool_t sig=kTRUE;
   Double_t sumAll=0, sumWrong=0, sumAllOrig=0, sumWrongOrig=0;
   Double_t sumAllnewBoost=0.;

   // debug variables
   vector<Float_t> var1, BoostWeight;

   // finding the wrong classified events and reweight them, depending
   // on the specified option
   for (Long64_t ievt=0; ievt<Data()->GetNEvents(); ievt++) {
      ev  = Data()->GetEvent(ievt);
      sig = DataInfo().IsSignal(ev);
      v   = TMVA::MethodPDEFoam::GetMvaValue();
      w   = ev->GetWeight();
      wo  = ev->GetOriginalWeight();
      sumAll     += w;
      sumAllOrig += wo;
      if (sig != IsSignalLike()) {
	 // the event was classified wrong
	 sumWrong     += w;
	 sumWrongOrig += wo;
	 // reweight the misclassified training event
	 if (fWeightType == "Quadratic")
	    ev->SetBoostWeight( -TMath::Power(v-0.5,2) + 1.0 );
	 else if (fWeightType == "Gauss")
	    ev->SetBoostWeight( TMath::Exp( -TMath::Power(v-0.5,2)/0.1 ) );
	 // fill debug variables
	 var1.push_back(ev->GetValue(0));
	 BoostWeight.push_back(ev->GetBoostWeight());
      }
      sumAllnewBoost += ev->GetBoostWeight(); // new boost weights
   }
   // calculate missclassification rate
   fMethodError = sumWrong/sumAll;
   // calculate missclassification rate by original weight
   fOrigMethodError = sumWrongOrig/sumAllOrig;

   // renormalize the events
   sumAllnewBoost = sumAll/sumAllnewBoost; // rescaling factor
   for (Long64_t ievt=0; ievt<Data()->GetNEvents(); ievt++)
      Data()->GetEvent(ievt)->ScaleBoostWeight(sumAllnewBoost);

   // fill TGraph
   fBoostWeightGraph->push_back(new TGraph(var1.size(), &var1[0], &BoostWeight[0]));
}

//_______________________________________________________________________
void TMVA::MethodBPDEFoam::SingleTest( UInt_t i )
{
   // Test the method to be able to calculate the ROC integral

   Data()->SetCurrentType(Types::kTesting);

   // Delete the old testing results, which were created during the
   // last boost iteration
   Data()->GetResults(GetMethodName(), Types::kTesting, GetAnalysisType())->Clear();

   // Evaluate method on testing sample
   AddOutput( Types::kTesting, GetAnalysisType() );
   // Evaluate performance of method
   TestClassification();

   // save classifier response (MVA value)
   TH1F* hMVA_S = (TH1F*) Data()->GetResults(GetMethodName(), Types::kTesting, GetAnalysisType())->GetObject("MVA_S")->Clone();
   TH1F* hMVA_B = (TH1F*) Data()->GetResults(GetMethodName(), Types::kTesting, GetAnalysisType())->GetObject("MVA_B")->Clone();

   TString hname = GetMethodName() + Form("_MVA_testing_S_%i",i);
   hMVA_S->SetNameTitle(hname.Data(), hname.Data());
   hname = GetMethodName() + Form("_MVA_testing_B_%i",i);
   hMVA_B->SetNameTitle(hname.Data(), hname.Data());

   fMVAHist_S->push_back( hMVA_S );
   fMVAHist_B->push_back( hMVA_B );

   Data()->SetCurrentType(Types::kTraining);
}

//_______________________________________________________________________
void TMVA::MethodBPDEFoam::ResetFoams( void )
{
   // delete all foams
   for (UInt_t i=0; i<FOAM_NUMBER; i++){
      if(foam[i]) {
	 delete foam[i];
	 foam[i] = NULL;
      }
   }
}

//_______________________________________________________________________
void TMVA::MethodBPDEFoam::ResetBoostWeights( void )
{
   // reset boost weights of training sample to 1.0

   Data()->SetCurrentType(Types::kTraining);

   for (Long64_t ievt=0; ievt<Data()->GetNEvents(); ievt++)
      Data()->GetEvent(ievt)->ScaleBoostWeight(1.0);
}

//_______________________________________________________________________
Double_t TMVA::MethodBPDEFoam::GetMvaValue( Double_t* err )
{
   // Return Mva-Value for method
   return TMVA::MethodPDEFoam::GetMvaValue( err );
}

//_______________________________________________________________________
void TMVA::MethodBPDEFoam::InitMonitorHistograms( void )
{
   if (fMonitorHist) delete fMonitorHist;
   if (fTrainingHist_S) delete fTrainingHist_S;
   if (fTrainingHist_B) delete fTrainingHist_B;
   if (fMVAHist_S) delete fMVAHist_S;
   if (fMVAHist_B) delete fMVAHist_B;

   fMonitorHist = new std::vector<TH1F*>();

   fMonitorHist->push_back(new TH1F("ROCIntegral","ROC integral",fBoostNum,0,fBoostNum));
   fMonitorHist->back()->GetXaxis()->SetTitle("Index of boosted classifier");
   fMonitorHist->back()->GetYaxis()->SetTitle("ROC integral");

   fMonitorHist->push_back(new TH1F("Separation","Separation",fBoostNum,0,fBoostNum));
   fMonitorHist->back()->GetXaxis()->SetTitle("Index of boosted classifier");
   fMonitorHist->back()->GetYaxis()->SetTitle("Separation");

   fMonitorHist->push_back(new TH1F("MisclassRate","Misclassification rate",fBoostNum,0,fBoostNum));
   fMonitorHist->back()->GetXaxis()->SetTitle("Index of boosted classifier");
   fMonitorHist->back()->GetYaxis()->SetTitle("Misclassification rate");

   fMonitorHist->push_back(new TH1F("OrigMisclassRate","Misclassification rate by orig. weights",fBoostNum,0,fBoostNum));
   fMonitorHist->back()->GetXaxis()->SetTitle("Index of boosted classifier");
   fMonitorHist->back()->GetYaxis()->SetTitle("Misclassification rate by orig. weights");

   fTrainingHist_S = new std::vector<TH1F*>();
   fTrainingHist_B = new std::vector<TH1F*>();
   fMVAHist_S = new std::vector<TH1F*>();
   fMVAHist_B = new std::vector<TH1F*>();
   fBoostWeightGraph = new std::vector<TGraph*>();
}

//_______________________________________________________________________
void TMVA::MethodBPDEFoam::FillHistograms( Int_t BoostIndex )
{
   fMonitorHist->at(0)->SetBinContent(BoostIndex, GetROCIntegral());
   fMonitorHist->at(1)->SetBinContent(BoostIndex, GetSeparation());
   fMonitorHist->at(2)->SetBinContent(BoostIndex, fMethodError);
   fMonitorHist->at(3)->SetBinContent(BoostIndex, fOrigMethodError);

   // save the reweighted training sample
   fTrainingHist_S->push_back(new TH1F(Form("Reweighted_Training_Sample_var1_S_%i",BoostIndex),"Reweighted training sample (signal)", 100, 1.0, -1.0));
   fTrainingHist_S->back()->GetXaxis()->SetTitle("var1 (signal)");

   fTrainingHist_B->push_back(new TH1F(Form("Reweighted_Training_Sample_var1_B_%i",BoostIndex),"Reweighted training sample (background)", 100, 1.0, -1.0));
   fTrainingHist_B->back()->GetXaxis()->SetTitle("var1 (background)");

   // loop over training sample and fill reweighted events
   for (Long64_t ievt=0; ievt<Data()->GetNEvents(); ievt++) {
      Event * ev = Data()->GetEvent(ievt);
      Bool_t sig = DataInfo().IsSignal(ev);
      if (sig)
	 fTrainingHist_S->back()->Fill( Data()->GetEvent(ievt)->GetValue(0),
					Data()->GetEvent(ievt)->GetWeight() );
      else
	 fTrainingHist_B->back()->Fill( Data()->GetEvent(ievt)->GetValue(0),
					Data()->GetEvent(ievt)->GetWeight() );
   }
}

//_______________________________________________________________________
void TMVA::MethodBPDEFoam::WriteMonitorHistograms( void )
{
   BaseDir()->cd();
   for (UInt_t i=0; i<fMonitorHist->size(); i++) {
      fMonitorHist->at(i)->SetName(Form("Booster_%s",fMonitorHist->at(i)->GetName()));
      fMonitorHist->at(i)->Write();
   }

   for (UInt_t i=0; i<fTrainingHist_S->size(); i++) {
      fTrainingHist_S->at(i)->BufferEmpty();
      fTrainingHist_S->at(i)->SetName(Form("Booster_ReweightedTrainingSample_var1_S_%i",i));
      fTrainingHist_S->at(i)->Write();
   }

   for (UInt_t i=0; i<fTrainingHist_B->size(); i++) {
      fTrainingHist_B->at(i)->BufferEmpty();
      fTrainingHist_B->at(i)->SetName(Form("Booster_ReweightedTrainingSample_var1_B_%i",i));
      fTrainingHist_B->at(i)->Write();
   }

   for (UInt_t i=0; i<fMVAHist_S->size(); i++) {
      fMVAHist_S->at(i)->BufferEmpty();
      fMVAHist_S->at(i)->Write();
   }

   for (UInt_t i=0; i<fMVAHist_B->size(); i++) {
      fMVAHist_B->at(i)->BufferEmpty();
      fMVAHist_B->at(i)->Write();
   }

   // Write number of boosts to file
   stringstream ss (stringstream::in | stringstream::out);
   ss << fBoostNum;
   TObjString *os = new TObjString(ss.str().c_str());
   os->Write("fBoostNum", kWriteDelete);

   // write boost weights graph
   for (UInt_t i=0; i<fBoostWeightGraph->size(); i++) {
      stringstream gname (stringstream::in | stringstream::out);
      gname << "BoostWeights_var1_" << i;
      fBoostWeightGraph->at(i)->GetXaxis()->SetTitle("var1");
      fBoostWeightGraph->at(i)->GetYaxis()->SetTitle("boost weight");
      fBoostWeightGraph->at(i)->Write(gname.str().c_str());
   }
}

//_______________________________________________________________________
void TMVA::MethodBPDEFoam::GetHelpMessage() const
{
   // provide help message
   Log() << Endl;
   Log() << gTools().Color("bold") << "--- Short description:" << gTools().Color("reset") << Endl;
   Log() << Endl;
   Log() << "BPDEFoam implemets a PDEFoam specific boosting algorithm." << Endl;
   Log() << Endl;
   Log() << gTools().Color("bold") << "--- Use of booking options:" << gTools().Color("reset") << Endl;
   Log() << Endl;
   Log() << "The following options can be set (the listed values are found to be a" << Endl;
   Log() << "good starting point for most applications):" << Endl;
   Log() << Endl;
   Log() << "  Boost_Num          100            Maximal number of boosts" << Endl;
   Log() << "  Boost_WeightType   OneOverDiscr   Weight for the misclassified events" << Endl;
}
