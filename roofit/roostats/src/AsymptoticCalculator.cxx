// @(#)root/roostats:$Id$
// Author: Kyle Cranmer, Sven Kreiss   23/05/10
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/**
   Performs hypothesis tests using aysmptotic formula for the profile likelihood and 
   Asimov data set
*/


#include "RooStats/AsymptoticCalculator.h"
#include "RooStats/ToyMCSampler.h"
#include "RooStats/ModelConfig.h"
#include "RooStats/ProfileLikelihoodTestStat.h"
#include "RooStats/RooStatsUtils.h"

#include "RooArgSet.h"
#include "RooArgList.h"
#include "RooProdPdf.h"
#include "RooSimultaneous.h"
#include "RooDataSet.h"
#include "RooCategory.h"
#include "RooRealVar.h"
#include "RooMinimizer.h"
#include "RooNLLVar.h"
#include "Math/MinimizerOptions.h"
#include "RooPoisson.h"
#include "RooUniform.h"
#include <cmath>


using namespace RooStats;

namespace Utils { 

    bool SetAllConstant(const RooAbsCollection &coll, bool constant) {
       // utility function to set all variable constant in a collection
       // (from G. Petrucciani)
       bool changed = false;
       std::auto_ptr<TIterator> iter(coll.createIterator());
       for (RooAbsArg *a = (RooAbsArg *) iter->Next(); a != 0; a = (RooAbsArg *) iter->Next()) {
          RooRealVar *v = dynamic_cast<RooRealVar *>(a);
          if (v && (v->isConstant() != constant)) {
             changed = true;
             v->setConstant(constant);
          }
       }
       return changed;
    }



}



ClassImp(RooStats::AsymptoticCalculator)

AsymptoticCalculator::AsymptoticCalculator(
   const RooAbsData &data,
   const ModelConfig &altModel,
   const ModelConfig &nullModel) :
      HypoTestCalculatorGeneric(data, altModel, nullModel, 0), 
   fOneSided(false), fUseQTilde(false), 
   fNLLObs(0), fNLLAsimov(0), 
   fAsimovData(0)   
{
   // constructor for asymptotic calculator 

   RooAbsPdf * nullPdf = GetNullModel()->GetPdf();
   assert(nullPdf); 



   // evaluate the unconditional nll for alt snapshot and observed data  
   RooAbsData * obsData = const_cast<RooAbsData *>(GetData() );
   assert( obsData );

   fNLLObs = EvaluateNLL( *nullPdf, *obsData);
   
   // compute Asimov data set for the background (alt poi ) value
   const RooArgSet * altSnapshot = GetAlternateModel()->GetSnapshot();
   const RooArgSet * nullSnapshot = GetNullModel()->GetSnapshot();
   if(altSnapshot == NULL) {
      oocoutE((TObject*)0,InputArguments) << "Alt (Background)  model needs a snapshot. Set using modelconfig->SetSnapshot(poi)." << endl;
      return;
   }
   if(nullSnapshot == NULL) {
      oocoutE((TObject*)0,InputArguments) << "Null model needs a snapshot. Set using modelconfig->SetSnapshot(poi)." << endl;
      return;
   }

   RooArgSet poi(*altSnapshot);  // this is the poi snapshot of B (i.e. for mu=0)

   fAsimovData = MakeAsimovData( poi, fAsimovGlobObs);
   if (!fAsimovData) return;

   // set global observables to their Asimov values 
   RooArgSet globObs;
   RooArgSet globObsSnapshot;
   if (GetNullModel()->GetGlobalObservables()  ) {
      globObs.add(*GetNullModel()->GetGlobalObservables());
      assert(globObs.getSize() == fAsimovGlobObs.getSize() );
      // store previous snapshot value
      globObs.snapshot(globObsSnapshot);
      globObs = fAsimovGlobObs; 
   }


   // evaluate  the likelihood. Since we use on Asimov data , conditional and unconditional values should be the same
   fNLLAsimov =  EvaluateNLL( *nullPdf, *fAsimovData, &poi);
   
   // restore previous value 
   globObs = globObsSnapshot;


}

//_________________________________________________________________
Double_t AsymptoticCalculator::EvaluateNLL(RooAbsPdf & pdf, RooAbsData& data,   const RooArgSet *poiSet) {
      
    
    RooFit::MsgLevel msglevel = RooMsgService::instance().globalKillBelow();
    RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);


    RooArgSet* allParams = pdf.getParameters(data);
    RooStats::RemoveConstantParameters(allParams);
    // add constraint terms for all non-constant parameters

    // need to call constrain for RooSimultaneous until stripDisconnected problem fixed
    RooAbsReal* nll = (RooNLLVar*) pdf.createNLL(data, RooFit::CloneData(kFALSE),RooFit::Constrain(*allParams));


    // if poi are specified - do a conditional fit 
    RooArgList paramsSetConstant;
    if (poiSet) { 
       RooArgSet* attachedSet = nll->getVariables();
       
       RooLinkedListIter it = poiSet->iterator();
       RooRealVar* tmpPar = NULL, *tmpParA=NULL;
       while((tmpPar = (RooRealVar*)it.Next())){
          tmpParA =  ((RooRealVar*)attachedSet->find(tmpPar->GetName()));
          tmpParA->setVal( tmpPar->getVal() );
          if (!tmpParA->isConstant() ) { 
             tmpParA->setConstant();
             paramsSetConstant.add(*tmpParA);
          }
       }
       delete attachedSet;
    }


    
    RooMinimizer minim(*nll);
    minim.setStrategy(ROOT::Math::MinimizerOptions::DefaultStrategy());
    //LM: RooMinimizer.setPrintLevel has +1 offset - so subtruct  here -1
    minim.setPrintLevel(ROOT::Math::MinimizerOptions::DefaultPrintLevel()-1);
    int status = -1;
    //	minim.optimizeConst(true);
    for (int tries = 0, maxtries = 4; tries <= maxtries; ++tries) {
       //	 status = minim.minimize(fMinimizer, ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());
       TString minimizer = ROOT::Math::MinimizerOptions::DefaultMinimizerType(); 
       status = minim.minimize(minimizer, "Minimize");
       if (status == 0) {  
          break;
       } else {
          if (tries > 1) {
             printf("    ----> Doing a re-scan first\n");
             minim.minimize(minimizer,"Scan");
          }
          if (tries > 2) {
             printf("    ----> trying with strategy = 1\n");
             minim.setStrategy(1);
          }
       }
    }
    std::cout << "BEST FIT values " << std::endl;
    allParams->Print("V");
    std::cout << "ALL PDF variables " << std::endl;
    pdf.getVariables()->Print("V");
    
    RooMsgService::instance().setGlobalKillBelow(msglevel);

    // return max value of nll
    
    double val =  nll->getVal(); 
    double muHat = 0; 
    if (poiSet) muHat = ( (RooRealVar*) poiSet->first() )->getVal();
    std::cout << "BEST NLL  value " << val << " and poi = " << std::endl;

    // reset the parameter free which where set as constant
    SetAllConstant(paramsSetConstant,false);

    delete allParams;
    delete nll;

    if (status != 0) {
       oocoutE((TObject*)0,Fitting) << "FIT FAILED !- return a NaN NLL " << std::endl;
       return TMath::QuietNaN();
    }

    return val;
}

//____________________________________________________
HypoTestResult* AsymptoticCalculator::GetHypoTest() const {
   // It performs an hypothesis tests using the likelihood function
   // computes the profile likelihood test statistic value 

   if (!fAsimovData) { 
       oocoutE((TObject*)0,InputArguments) << "AsymptoticCalculator::GetHypoTest - Asimov data set has not been generated - return NULL result " << endl;
       return 0;
   }

   assert(GetNullModel() );
   assert(GetData() );

   RooAbsPdf * nullPdf = GetNullModel()->GetPdf();
   assert(nullPdf); 

   // make conditional fit on null snapshot of poi

   const RooArgSet * nullSnapshot = GetNullModel()->GetSnapshot();
   assert(nullSnapshot);

   RooArgSet poi(*nullSnapshot);

   // evaluate the conditional NLL on the observed data  
   double condNLL = EvaluateNLL( *nullPdf, const_cast<RooAbsData&>(*GetData()), &poi);

   double qmu = 2.*(condNLL - fNLLObs); 
   
   // set the one-side condition
   // (only when we have only one params of interest 
   if (fOneSided) { 
      const RooArgSet * paramsOfInterest = GetNullModel()->GetParametersOfInterest();
      if (paramsOfInterest && paramsOfInterest->getSize() == 1) { 
         RooRealVar * mu = dynamic_cast<RooRealVar*> (  paramsOfInterest->first() );
         RooRealVar * muTest = dynamic_cast<RooRealVar*> ( nullSnapshot->find(mu->GetName() ) );
         if ( mu->getVal() > muTest->getVal() ) qmu = 0;
      }
      else { 
         if (!paramsOfInterest)
            oocoutE((TObject*)0,InputArguments) << "Cannot use one-sided test because no POI is defined in ModelConfig " << std::endl;         
         else
            oocoutE((TObject*)0,InputArguments) << "Cannot use one-sided test because model has more than one POI " << std::endl;         
      }
   }

   // ProfileLikelihoodTestStat profileLL( *nullPdf );
   // profileLL.SetOneSided(true);
   // profileLL.SetReuseNLL(false);

   std::cout << " qmu on data = " << qmu << " condNLL = " << condNLL << " uncond " << fNLLObs << std::endl;

   double sqrtqmu = (qmu > 0) ? std::sqrt(qmu) : 0; 

   double pnull = ROOT::Math::normal_cdf_c( sqrtqmu, 1.);
 
   

   // compute conditional ML on Asimov data set
   // (need to const cast because it uses fitTo which is a non const method
   // RooArgSet asimovGlobObs;
   // RooAbsData * asimovData = (const_cast<AsymptoticCalculator*>(this))->MakeAsimovData( poi, asimovGlobObs);
   // set global observables to their Asimov values 
   RooArgSet globObs;
   RooArgSet globObsSnapshot;
   if (GetNullModel()->GetGlobalObservables()  ) {
      globObs.add(*GetNullModel()->GetGlobalObservables());
      // store previous snapshot value
      globObs.snapshot(globObsSnapshot);
      globObs = fAsimovGlobObs; 
   }

   double condNLL_A = EvaluateNLL( *nullPdf, *fAsimovData, &poi);

   // restore previous value 
   globObs = globObsSnapshot;


   double qmu_A = 2.*(condNLL_A - fNLLAsimov  );

   if (fOneSided) { 
      const RooArgSet * paramsOfInterest = GetNullModel()->GetParametersOfInterest();
      if (paramsOfInterest && paramsOfInterest->getSize() == 1) { 
         RooRealVar * mu = dynamic_cast<RooRealVar*> (  paramsOfInterest->first() );
         RooRealVar * muTest = dynamic_cast<RooRealVar*> ( nullSnapshot->find(mu->GetName() ) );
         if ( mu->getVal() > muTest->getVal() ) qmu_A = 0;
      }
   }



   std::cout << " qmu on Asimov = " << qmu_A << " condNLL = " << condNLL_A << " uncond " << fNLLAsimov << std::endl;

   double sqrtqmu_A = (qmu_A > 0) ? std::sqrt(qmu_A) : 0; 

   double palt = ROOT::Math::normal_cdf( sqrtqmu_A - sqrtqmu, 1.);

   if (fUseQTilde && qmu > qmu_A) { 
      pnull = ROOT::Math::normal_cdf_c( (qmu + qmu_A)/(2 * sqrtqmu_A), 1.);
      palt = ROOT::Math::normal_cdf( (qmu - qmu_A)/(2 * sqrtqmu_A), 1.);
   }

   


   // create an HypoTest result but where the sampling distributions are set to zero
   string resultname = "HypoTestAsymptotic_result";
   HypoTestResult* res = new HypoTestResult(resultname.c_str(), pnull, palt);
   res->SetBackgroundAsAlt(true);

   double mu = 0;
   RooRealVar * muVar = dynamic_cast<RooRealVar*>(poi.first() );
   if (muVar) mu = muVar->getVal();
   oocoutI((TObject*)0,Eval) << "poi = " << mu << " qmu = " << qmu << " qmu_A = " << qmu_A 
                             << "  CLsplusb = " << pnull << " CLb = " << palt << " CLs = " <<  res->CLs() << std::endl; 

   return res; 
}

double AsymptoticCalculator::GetExpectedPValues(double pnull, double palt, double nsigma, bool useCls ) { 
   // function given the null and the alt p value - return the expected one given the N - sigma value
   double sqrtqmu =  ROOT::Math::normal_quantile_c( pnull,1.);
   double sqrtqmu_A =  ROOT::Math::normal_quantile( palt,1.) + sqrtqmu;
   double clsplusb = ROOT::Math::normal_cdf_c( sqrtqmu_A - nsigma, 1.);
   if (!useCls) return clsplusb; 
   double clb = ROOT::Math::normal_cdf( nsigma, 1.);
   return (clb == 0) ? -1 : clsplusb / clb;  
}   

// void GetExpectedLimit(double nsigma, double alpha, double &clsblimit, double &clslimit) { 
//    // get expected limit 
//    double 
// }


void AsymptoticCalculator::FillBins(const RooAbsPdf & pdf, const RooArgList &obs, RooAbsData & data, int &index,  double &binVolume, int &ibin) { 
   /// fill bins by looping recursivly on observables 

   bool debug = false; 
   RooRealVar * v = dynamic_cast<RooRealVar*>( &(obs[index]) );
   if (!v) return;

   RooArgSet obstmp(obs);
   double expectedEvents = pdf.expectedEvents(obstmp);

   if (debug) cout << "looping on observable " << v->GetName() << endl;
   for (int i = 0; i < v->getBins(); ++i) {
      v->setBin(i);
      if (index < obs.getSize() -1) {
         index++;  // increase index
         double prevBinVolume = binVolume; 
         binVolume *= v->getBinWidth(i); // increase bin volume
         FillBins(pdf, obs, data, index,  binVolume, ibin);
         index--; // decrease index
         binVolume = prevBinVolume; // decrease also bin volume
      }
      else {

         // this is now a new bin - compute the pdf in this bin 
         double totBinVolume = binVolume * v->getBinWidth(i);
         double fval = pdf.getVal(&obstmp)*totBinVolume;
         if (fval*expectedEvents <= 0)
         {
            cout << "WARNING::Detected bin with zero expected events! Please check your inputs." << endl;
         }
         // have a cut off for overflows ??
         data.add(obs, fval*expectedEvents);

         if (debug) { 
            cout << "bin " << ibin << "\t";
            for (int j=0; j < obs.getSize(); ++j) { cout << "  " <<  ((RooRealVar&) obs[j]).getVal(); }
            cout << endl;
         }
         // RooArgSet xxx(obs);
         // h3->Fill(((RooRealVar&) obs[0]).getVal(), ((RooRealVar&) obs[1]).getVal(), ((RooRealVar&) obs[2]).getVal() ,
         //          pdf->getVal(&xxx) );
         ibin++;
      }
   }
   //reset bin values
   if (debug) 
      cout << "ending loop on .. " << v->GetName() << endl;

   v->setBin(0);
   
}


void AsymptoticCalculator::SetObsToExpected(RooProdPdf &prod, const RooArgSet &obs) 
{
   // iterate a Prod pdf to find the Poisson part to set the observed value to expected one
    std::auto_ptr<TIterator> iter(prod.pdfList().createIterator());
    for (RooAbsArg *a = (RooAbsArg *) iter->Next(); a != 0; a = (RooAbsArg *) iter->Next()) {
        if (!a->dependsOn(obs)) continue;
        RooPoisson *pois = 0;
        if ((pois = dynamic_cast<RooPoisson *>(a)) != 0) {
            SetObsToExpected(*pois, obs);
        } else {
           // should try to add also Gaussian and lognormal case ? 
            RooProdPdf *subprod = dynamic_cast<RooProdPdf *>(a);
            if (subprod) SetObsToExpected(*subprod, obs);
            else {
               oocoutE((TObject*)0,InputArguments) << "Illegal term in counting model: depends on observables, but not Poisson or Product" << endl;
               return;
            }
        }
    }
}

void AsymptoticCalculator::SetObsToExpected(RooPoisson &pois, const RooArgSet &obs) 
{
   // set observed value in Poisson to the expected one
   // need to iterate on the components of the POisson to get n and nu (nu can be a RooAbsReal)
   // (code from G. Petrucciani)
   RooRealVar *myobs = 0;
   RooAbsReal *myexp = 0;
   std::auto_ptr<TIterator> iter(pois.serverIterator());
   for (RooAbsArg *a = (RooAbsArg *) iter->Next(); a != 0; a = (RooAbsArg *) iter->Next()) {
      if (obs.contains(*a)) {
         assert(myobs == 0 && "SinglePdfGenInfo::setToExpected(RooPoisson): Two observables??");
         myobs = dynamic_cast<RooRealVar *>(a);
         assert(myobs != 0 && "SinglePdfGenInfo::setToExpected(RooPoisson): Observables is not a RooRealVar??");
      } else {
         assert(myexp == 0 && "SinglePdfGenInfo::setToExpected(RooPoisson): Two expecteds??");
         myexp = dynamic_cast<RooAbsReal *>(a);
         assert(myexp != 0 && "SinglePdfGenInfo::setToExpected(RooPoisson): Expectedis not a RooAbsReal??");
      }
   }
   assert(myobs != 0 && "SinglePdfGenInfo::setToExpected(RooPoisson): No observable?");
   assert(myexp != 0 && "SinglePdfGenInfo::setToExpected(RooPoisson): No expected?");
   myobs->setVal(myexp->getVal());
}


RooAbsData * AsymptoticCalculator::GenerateCountingAsimovData(RooAbsPdf & pdf, const RooArgSet & observables,  const RooRealVar & , RooCategory * channelCat) { 
   // generate countuing Asimov data for the case when the pdf cannot be extended
   // assume pdf is a RooPoisson or can be decomposed in a product of RooPoisson, 
   // otherwise we cannot know how to make the Asimov data sets in the other cases
    RooArgSet obs(observables);
    RooProdPdf *prod = dynamic_cast<RooProdPdf *>(&pdf);
    RooPoisson *pois = 0;
    if (prod != 0) {
        SetObsToExpected(*prod, observables);
    } else if ((pois = dynamic_cast<RooPoisson *>(&pdf)) != 0) {
        SetObsToExpected(*pois, observables);
    } else {
       oocoutE((TObject*)0,InputArguments) << "A counting model pdf must be either a RooProdPdf or a RooPoisson" << endl;
    }
    int icat = 0;
    if (channelCat) {
       icat = channelCat->getIndex(); 
    }

    RooDataSet *ret = new RooDataSet(TString::Format("CountingAsimovData%d",icat),TString::Format("CountingAsimovData%d",icat), obs);
    ret->add(obs);
    return ret;
}

RooAbsData * AsymptoticCalculator::GenerateAsimovDataSinglePdf(const RooAbsPdf & pdf, const RooArgSet & allobs,  const RooRealVar & weightVar, RooCategory * channelCat) { 
   // compute the asimov data set for an observable of a pdf 
   // use the number of bins sets in the observables 
   // to do :  (possibility to change number of bins)
   // impelment integration over bin content

   int _printLevel = 1;

   // Get observables defined by the pdf associated with this state
   RooArgSet* obs = pdf.getObservables(allobs) ;


   // if pdf cannot be extended assume is then a counting experiment
   if (!pdf.canBeExtended() ) return GenerateCountingAsimovData(const_cast<RooAbsPdf&>(pdf), *obs, weightVar, channelCat);

   RooArgSet obsAndWeight(*obs); 
   obsAndWeight.add(weightVar);

   RooDataSet* asimovData = 0; 
   if (channelCat) {
      int icat = channelCat->getIndex(); 
      asimovData = new RooDataSet(TString::Format("AsimovData%d",icat),TString::Format("combAsimovData%d",icat),
                                  RooArgSet(obsAndWeight,*channelCat),RooFit::WeightVar(weightVar));
   }
   else 
      asimovData = new RooDataSet("AsimovData","AsimovData",RooArgSet(obsAndWeight),RooFit::WeightVar(weightVar));

    // This works ony for 1D observables 
    //RooRealVar* thisObs = ((RooRealVar*)obstmp->first());

    RooArgList obsList(*obs);

    // loop on observables and on the bins 
    cout << "Generating Asimov data for pdf " << pdf.GetName() << endl;
    cout << "list of observables  " << endl;
    obsList.Print();

    int obsIndex = 0; 
    double binVolume = 1; 
    int nbins = 0; 
    FillBins(pdf, obsList, *asimovData, obsIndex, binVolume, nbins);
    cout << "filled from " << pdf.GetName() << "   " << nbins << " nbins " << " volume is " << binVolume << endl;

    // for (int iobs = 0; iobs < obsList.getSize(); ++iobs) { 
    //    RooRealVar * thisObs = dynamic_cast<RooRealVar*> &obsList[i];
    //    if (thisObs == 0) continue; 
    //    // loop on the bin contents
    //    for(int  ibin=0; ibin<thisObs->numBins(); ++ibin){
    //       thisObs->setBin(ibin);

    //   thisNorm=pdftmp->getVal(obstmp)*thisObs->getBinWidth(jj);
    //   if (thisNorm*expectedEvents <= 0)
    //   {
    //     cout << "WARNING::Detected bin with zero expected events! Please check your inputs." << endl;
    //   }
    //   // have a cut off for overflows ??
    //   obsDataUnbinned->add(*mc->GetObservables(), thisNorm*expectedEvents);
    // }
    
    if (_printLevel >= 1)
    {
      asimovData->Print();
      cout <<"sum entries "<< asimovData->sumEntries()<<endl;
    }
    if( TMath::IsNaN(asimovData->sumEntries()) ){
      cout << "sum entries is nan"<<endl;
      assert(0);
      delete asimovData;
      asimovData = 0;
    }

    delete obs;
    return asimovData;

}

RooAbsData * AsymptoticCalculator::GenerateAsimovData(const RooAbsPdf & pdf, const RooArgSet & observables  )  { 
   // generate the asimov data for the observables (not the global ones) 
   // need to deal with the case of a sim pdf 

   int _printLevel = 0;

   RooRealVar * weightVar = new RooRealVar("binWeightAsimov", "binWeightAsimov", 1, 0, 1.E30 );

   if (_printLevel >= 1) cout <<" check expectedData by category"<<endl;
  //RooDataSet* simData=NULL;
   const RooSimultaneous* simPdf = dynamic_cast<const RooSimultaneous*>(&pdf);
   if (!simPdf) { 
      // generate data for non sim pdf
      return GenerateAsimovDataSinglePdf( pdf, observables, *weightVar, 0);
   }

   std::map<std::string, RooDataSet*> asimovDataMap;
    
  //look at category of simpdf 
  RooCategory& channelCat = (RooCategory&)simPdf->indexCat();
  //    TIterator* iter = simPdf->indexCat().typeIterator() ;
  TIterator* iter = channelCat.typeIterator() ;
  RooCatType* tt = NULL;
  int nrIndices = 0;
  while((tt=(RooCatType*) iter->Next())) {
    nrIndices++;
  }
  for (int i=0;i<nrIndices;i++){
    channelCat.setIndex(i);
    //iFrame++;
    // Get pdf associated with state from simpdf
    RooAbsPdf* pdftmp = simPdf->getPdf(channelCat.getLabel()) ;
	
    if (_printLevel >= 1)
    {
      cout << "on type " << channelCat.getLabel() << " " << channelCat.getIndex() << endl;
    }

    RooAbsData * dataSinglePdf = GenerateAsimovDataSinglePdf( *pdftmp, observables, *weightVar, &channelCat);
    //((RooRealVar*)obstmp->first())->Print();
    //cout << "expected events " << pdftmp->expectedEvents(*obstmp) << endl;
    if (!dataSinglePdf) { 
       oocoutE((TObject*)0,Generation) << "Error generating an Asimov data set for pdf " << pdftmp->GetName() << endl;
       return 0;
    }
     

    asimovDataMap[string(channelCat.getLabel())] = (RooDataSet*) dataSinglePdf;

    if (_printLevel >= 1)
    {
      cout << "channel: " << channelCat.getLabel() << ", data: ";
      dataSinglePdf->Print();
      cout << endl;
    }
  }

  RooArgSet obsAndWeight(observables); 
  obsAndWeight.add(*weightVar);


  RooDataSet* asimovData = new RooDataSet("asimovDataFullModel","asimovDataFullModel",RooArgSet(obsAndWeight,channelCat),
                                          RooFit::Index(channelCat),RooFit::Import(asimovDataMap),RooFit::WeightVar(*weightVar));

  delete weightVar; 
  return asimovData;

}

//______________________________________________________________________________
RooAbsData * AsymptoticCalculator::MakeAsimovData(const RooArgSet & paramValues, RooArgSet & asimovGlobObs)  {
   // make the Asimov data set
   // extract from code from Giovanni and AAron

   // inputs: 
   // RooAbsData &realdata, RooAbsCollection &snapshot, double poiValue, int verbose

   // get the model

   int verbose = 2;

   const RooStats::ModelConfig *mc = GetNullModel() ;
   RooAbsData * realData = const_cast<RooAbsData*> (GetData() ); 


   RooArgSet  poi(*mc->GetParametersOfInterest());
   poi = paramValues; 
   //RooRealVar *r = dynamic_cast<RooRealVar *>(poi.first());
   // set poi constant for conditional MLE 
   // need to fit nuisance parameters at their conditional MLE value
   RooLinkedListIter it = poi.iterator();
   RooRealVar*  tmpPar = NULL;
   while((tmpPar = (RooRealVar*)it.Next())){
      tmpPar->setConstant();
   }
   if (mc->GetNuisanceParameters()) {
      RooAbsPdf * pdf = mc->GetPdf();
      pdf->fitTo(*realData, RooFit::Minimizer("Minuit2","minimize"), RooFit::Strategy(1), RooFit::Constrain(*mc->GetNuisanceParameters()));
   } else {
      // Do we have free parameters anyway that need fitting?
      bool hasFloatParams = false;
      std::auto_ptr<RooArgSet> params(mc->GetPdf()->getParameters(*realData));
      std::auto_ptr<TIterator> iter(params->createIterator());
      for (RooAbsArg *a = (RooAbsArg *) iter->Next(); a != 0; a = (RooAbsArg *) iter->Next()) {
         RooRealVar *rrv = dynamic_cast<RooRealVar *>(a);
         if ( rrv != 0 && rrv->isConstant() == false ) { hasFloatParams = true; break; }
      } 
      if (hasFloatParams) mc->GetPdf()->fitTo(*realData, RooFit::Minimizer("Minuit2","minimize"), RooFit::Strategy(1));
   }
   // after the fit the nuisance parameters will have their best fit value
   if (mc->GetNuisanceParameters() && verbose > 1) {
      std::cout << "Nuisance parameters after fit for asimov dataset: " << std::endl;
      mc->GetNuisanceParameters()->Print("V");
   }
   
   // generate the Asimov data set for the observables 
   // need to distinguish  if Simpdf or normal pdf
 
   // toymcoptutils::SimPdfGenInfo newToyMC(*mc->GetPdf(), *mc->GetObservables(), false); 


   RooAbsData * asimov = GenerateAsimovData(*mc->GetPdf() , *mc->GetObservables() );


    // Now need to have in ASIMOV the data sets also the global observables
   // Their values must be the one satisfying the constraint. 
   // to do it make a nuisance pdf with all product of constraints and then 
   // assign to each constraint a glob observable value = to the current fitted nuisance parameter value

   if (mc->GetGlobalObservables() && mc->GetGlobalObservables()->getSize() > 0) {
      RooArgSet gobs(*mc->GetGlobalObservables());

      // snapshot data global observables
      RooArgSet snapGlobalObsData;
      Utils::SetAllConstant(gobs, true);
      gobs.snapshot(snapGlobalObsData);

      RooArgSet nuis(*mc->GetNuisanceParameters());
      // part 1: create the nuisance pdf
      std::auto_ptr<RooAbsPdf> nuispdf(RooStats::MakeNuisancePdf(*mc,"TempNuisPdf") );
      // unfold the nuisance pdf 
      RooProdPdf *prod = dynamic_cast<RooProdPdf *>(nuispdf.get());
      if (prod == 0) { 
         oocoutF((TObject*)0,Generation) << "AsymptoticCalculator::MakeAsimovData: the nuisance pdf is not a RooProdPdf!" << std::endl;
      }
      std::auto_ptr<TIterator> iter(prod->pdfList().createIterator());
      for (RooAbsArg *a = (RooAbsArg *) iter->Next(); a != 0; a = (RooAbsArg *) iter->Next()) {
         RooAbsPdf *cterm = dynamic_cast<RooAbsPdf *>(a); 
         assert(cterm && "AsimovUtils: a factor of the nuisance pdf is not a Pdf!");
         if (!cterm->dependsOn(nuis)) continue; // dummy constraints
         // skip also the case of uniform components
         if (typeid(*cterm) == typeid(RooUniform)) continue;

         std::auto_ptr<RooArgSet> cpars(cterm->getParameters(&gobs));
         std::auto_ptr<RooArgSet> cgobs(cterm->getObservables(&gobs));
         if (cgobs->getSize() != 1) {
            oocoutE((TObject*)0,Generation) << "AsymptoticCalculator::MakeAsimovData: constraint term  " <<  cterm->GetName() 
                                            << " has multiple global observables -cannot generate - skip it" << std::endl;
            continue;
         }
         RooRealVar &rrv = dynamic_cast<RooRealVar &>(*cgobs->first());

         RooAbsReal *match = 0;
         if (cpars->getSize() == 1) {
            match = dynamic_cast<RooAbsReal *>(cpars->first());
         } else {
            std::auto_ptr<TIterator> iter2(cpars->createIterator());
            for (RooAbsArg *a2 = (RooAbsArg *) iter2->Next(); a2 != 0; a2 = (RooAbsArg *) iter2->Next()) {
               RooRealVar *rrv2 = dynamic_cast<RooRealVar *>(a2); 
               if (rrv2 != 0 && !rrv2->isConstant()) {
                  if (match != 0) {
                     oocoutF((TObject*)0,Generation) << "AsymptoticCalculator::MakeAsimovData:constraint term " 
                                                     << cterm->GetName() << " has multiple floating params" << std::endl;
                     return 0; 
                  }
                  match = rrv2;
               }
            }
         }
         if (match == 0) {  
            oocoutF((TObject*)0,Generation) << "AsymptoticCalculator::MakeAsimovData - can't find nuisance for constraint term " << cterm->GetName() << std::endl;
            std::cerr << "Parameters: " << std::endl;
            cpars->Print("V");
            std::cerr << "Observables: " << std::endl;
            cgobs->Print("V");
            return 0;
         }
         rrv.setVal(match->getVal());
      }

      // make a snapshot of global observables 
      // needed this ?? (LM) 

      asimovGlobObs.removeAll();
      Utils::SetAllConstant(gobs, true);
      gobs.snapshot(asimovGlobObs);

      // revert global observables to the data value
      gobs = snapGlobalObsData;
      //Utils::SetAllConstant(paramsSetToConstants, false);

    
      if (verbose > 1) {
         std::cout << "Global observables for data: " << std::endl;
         gobs.Print("V");
         std::cout << "Global observables for asimov: " << std::endl;
         asimovGlobObs.Print("V");
      }



   }

   return asimov;

}






