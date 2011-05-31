// @(#)root/roostats:$Id$
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//_________________________________________________________________
/**
   HypoTestInverterNew class for performing an hypothesis test inversion by scanning the hypothesis test results of the 
  HybridCalculator  for various values of the parameter of interest. By looking at the confidence level curve of 
 the result  an upper limit, where it intersects the desired confidence level, can be derived.
 The class implements the RooStats::IntervalCalculator interface and returns an  RooStats::HypoTestInverterNewResult class.
 The result is a SimpleInterval, which via the method UpperLimit returns to the user the upper limit value.

The  HypoTestInverterNew implements various option for performing the scan. HypoTestInverterNew::RunFixedScan will scan using a fixed grid the parameter of interest. HypoTestInverterNew::RunAutoScan will perform an automatic scan to find optimally the curve and it will stop until the desired precision is obtained.
The confidence level value at a given point can be done via  HypoTestInverterNew::RunOnePoint.
The class can scan the CLs+b values or alternativly CLs (if the method HypoTestInverterNew::UseCLs has been called).


   New contributions to this class have been written by Matthias Wolf (advanced AutoRun algorithm)
**/

// include other header files

#include "RooAbsData.h"
#
#include "TMath.h"

#include "RooStats/HybridResult.h"


#include "TFile.h"
#include "TF1.h"
#include "TKey.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "RooRealVar.h"
#include "RooArgSet.h"
#include "RooAbsPdf.h"
#include "RooRandom.h"
#include "RooAddPdf.h"
#include "RooConstVar.h"
#include "RooStats/ModelConfig.h"
#include "RooStats/HybridCalculator.h"
#include "RooStats/FrequentistCalculator.h"
#include "RooStats/SimpleLikelihoodRatioTestStat.h"
#include "RooStats/RatioOfProfiledLikelihoodsTestStat.h"
#include "RooStats/ProfileLikelihoodTestStat.h"
#include "RooStats/ToyMCSampler.h"
#include "RooStats/HypoTestPlot.h"

//#include "../interface/Combine.h"
//#include "../interface/RooFitGlobalKillSentry.h"
//#include "../interface/SimplerLikelihoodRatioTestStat.h"
//#include "../interface/ProfiledLikelihoodRatioTestStat.h"

//#include "RooStats/utils.h"

// include header file of this class 
#include "RooStats/HypoTestInverterNew.h"


ClassImp(RooStats::HypoTestInverterNew)

using namespace RooStats;

double HypoTestInverterNew::fgCLAccuracy = 0.05;
unsigned int HypoTestInverterNew::fgNToys = 500;

#ifdef LATER
HypoTestInverterNew::WorkingMode HypoTestInverterNew::workingMode_ = MakeLimit;
double HypoTestInverterNew::rAbsAccuracy_ = 0.1;
double HypoTestInverterNew::rRelAccuracy_ = 0.05;
std::string HypoTestInverterNew::rule_ = "CLs";
std::string HypoTestInverterNew::testStat_ = "LEP";
unsigned int HypoTestInverterNew::nCpu_ = 0; // proof-lite mode
unsigned int HypoTestInverterNew::fork_ = 1; // fork mode
double HypoTestInverterNew::rValue_  = 1.0;
bool HypoTestInverterNew::fUseCLs = false;
bool HypoTestInverterNew::saveHybridResult_  = false;
bool HypoTestInverterNew::readHybridResults_ = false; 
bool HypoTestInverterNew::importanceSamplingNull_ = false;
bool HypoTestInverterNew::importanceSamplingAlt_  = false;
std::string HypoTestInverterNew::algo_ = "logSecant";
bool HypoTestInverterNew::optimizeProductPdf_     = false;
bool HypoTestInverterNew::optimizeTestStatistics_ = true;
std::string HypoTestInverterNew::plot_;
#endif

// helper class to wrap the functionality of the various HypoTestCalculators

template<class HypoTestType> 
struct HypoTestWrapper { 

   static void SetToys(HypoTestType * h, int toyNull, int toyAlt) { h->SetToys(toyNull,toyAlt); }
   
};


HypoTestInverterNew::HypoTestInverterNew( ) :
   fCalculator0(0),
   fScannedVariable(0),
   fResults(0),
   fUseCLs(false),
   fSize(0),
   fVerbose(1),
   fSystematics(1),
   fReadToysFromHere(1)
{
  // default constructor (doesn't do anything) 
}


HypoTestInverterNew::HypoTestInverterNew( HypoTestCalculatorGeneric& hc,
				    RooRealVar& scannedVariable, double size ) :
   fCalculator0(&hc),
   fScannedVariable(&scannedVariable), 
   fResults(0),
   fUseCLs(false),
   fSize(size),
   fVerbose(1),
   fSystematics(1),
   fReadToysFromHere(1),
   fCalcType(kUndefined)
{
   // constructor from a reference to an HypoTestCalculator 
   // (it must be an HybridCalculator type) and a RooRealVar for the variable
   HybridCalculator * hybrid_calc = dynamic_cast< HybridCalculator *> (&hc);
   if (hybrid_calc) fCalcType = kHybrid; 
   FrequentistCalculator * freq_calc = dynamic_cast< FrequentistCalculator *> (&hc);
   if (freq_calc) fCalcType = kFrequentist; 
 
}


HypoTestInverterNew::~HypoTestInverterNew()
{
  // destructor
  
  // delete the HypoTestInverterResult
  if (fResults) delete fResults;
}

void  HypoTestInverterNew::CreateResults() { 
  // create a new HypoTestInverterNewResult to hold all computed results
   if (fResults == 0) {
      TString results_name = "HypoInv_result_";
      results_name += fScannedVariable->GetName();
      fResults = new HypoTestInverterResult(results_name,*fScannedVariable,ConfidenceLevel());
      TString title = "HypoTestInverter Result For ";
      title += fScannedVariable->GetName();
      fResults->SetTitle(title);
   }
   fResults->UseCLs(fUseCLs);
}

#ifdef LATER
std::auto_ptr<RooStats::HypoTestC> HypoTestInverterNew::Create(RooWorkspace *w, RooStats::ModelConfig *mc_s, RooStats::ModelConfig *mc_b, RooAbsData &data, double rVal, HypoTestInverterNew::Setup &setup) {

  
  RooArgSet  poi(*mc_s->GetParametersOfInterest());
  RooRealVar *r = dynamic_cast<RooRealVar *>(poi.first());
  
  r->setVal(rVal); 
  if (testStat_ == "Atlas" || testStat_ == "Profile") {
    r->setConstant(false); r->setMin(0);
  } else {
    r->setConstant(true);
  }
  setup.modelConfig = ModelConfig(*mc_s);
  setup.modelConfig.SetGlobalObservables(RooArgSet()); // NOT for Hybrid
  
  setup.modelConfig_bonly = ModelConfig(*mc_b);
  setup.modelConfig_bonly.SetGlobalObservables(RooArgSet()); // NOT for Hybrid

  if (testStat_ == "Atlas" || testStat_ == "Profile") {
      // these need the S+B snapshot for both
      // must set it here and not later because calling SetSnapshot more than once does not work properly
      setup.modelConfig_bonly.SetSnapshot(poi);
  } else {
      RooArgSet poiZero; 
      setup.modelConfig_bonly.SetSnapshot(poiZero);
  }

  if (testStat_ == "LEP") {
      //SLR is evaluated using the central value of the nuisance parameters, so I believe we have to put them in the snapshots
      RooArgSet snapS; snapS.addClone(poi); 
      if (fSystematics) snapS.addClone(*mc_s->GetNuisanceParameters());
      RooArgSet snapB; snapB.addClone(snapS);
      snapS.setRealValue(r->GetName(), rVal);
      snapB.setRealValue(r->GetName(),    0);
      //if (optimizeTestStatistics_ && !mc_s->GetPdf()->canBeExtended()) {
          //// FIXME
          ////if (fSystematics && optimizeProductPdf_) {
          ////    if (w->pdf("modelObs_b") == 0 || w->pdf("modelObs_s") == 0) 
          ////        throw std::invalid_argument("HypoTestInverterNew: you can't use 'optimizeProduct' if the module does not define 'modelObs_s', 'modelObs_b'");
          ////    setup.qvar.reset(new SimplerLikelihoodRatioTestStat(*w->pdf("modelObs_b"), *w->pdf("modelObs_s"), snapB, snapS));
          ////} else {
          //setup.qvar.reset(new SimplerLikelihoodRatioTestStat(*setup.modelConfig_bonly.GetPdf(),*setup.modelConfig.GetPdf(), snapB, snapS));
          ////}
	  //} else {
          // FIXME
          //if (fSystematics && optimizeProductPdf_) {
          //    if (w->pdf("modelObs_b") == 0 || w->pdf("modelObs_s") == 0) 
          //        throw std::invalid_argument("HypoTestInverterNew: you can't use 'optimizeProduct' if the module does not define 'modelObs_s', 'modelObs_b'");
          //    setup.qvar.reset(new SimpleLikelihoodRatioTestStat(*w->pdf("modelObs_b"), *w->pdf("modelObs_s")));
          //} else {
      setup.qvar.reset(new SimpleLikelihoodRatioTestStat(*setup.modelConfig_bonly.GetPdf(),*setup.modelConfig.GetPdf()));
          //}
      ((SimpleLikelihoodRatioTestStat&)*setup.qvar).SetNullParameters(snapB); // Null is B
      ((SimpleLikelihoodRatioTestStat&)*setup.qvar).SetAltParameters(snapS);
	  //}
  } else if (testStat_ == "TEV") {
      // FIXME
    /*if (optimizeTestStatistics_ && !w->pdf("model_s")->canBeExtended()) {
        setup.qvar.reset(new ProfiledLikelihoodRatioTestStat(*setup.modelConfig_bonly.GetPdf(),*setup.modelConfig.GetPdf(), 
                                                             fSystematics ? w->set("nuisances") : 0, poiZero, poi));
    } else {*/   // turn this off for now, it does not work properly
        setup.qvar.reset(new RatioOfProfiledLikelihoodsTestStat(*setup.modelConfig_bonly.GetPdf(),*setup.modelConfig.GetPdf(), setup.modelConfig.GetSnapshot()));
        ((RatioOfProfiledLikelihoodsTestStat&)*setup.qvar).SetSubtractMLE(false);
    //}
  } else if (testStat_ == "Atlas" || testStat_ == "Profile") {
    setup.qvar.reset(new ProfileLikelihoodTestStat(*setup.modelConfig.GetPdf()));
    if (testStat_ == "Atlas") {
       ((ProfileLikelihoodTestStat&)*setup.qvar).SetOneSided(true);
    }
  }
  
  // FIXME
  //if (fSystematics && optimizeProductPdf_) {
  //    if (w->pdf("modelObs_b") == 0 || w->pdf("modelObs_s") == 0) 
  //        throw std::invalid_argument("HypoTestInverterNew: you can't use 'optimizeProduct' if the module does not define 'modelObs_s', 'modelObs_b'");
  //   setup.modelConfig.SetPdf(*w->pdf("modelObs_s"));
  //   setup.modelConfig_bonly.SetPdf(*w->pdf("modelObs_b"));
  //} 

  setup.toymcsampler.reset(new ToyMCSampler(*setup.qvar, fgNToys));

  if (!mc_b->GetPdf()->canBeExtended()) setup.toymcsampler->SetNEventsPerToy(1);
  
  if (nCpu_ > 0) {
    if (fVerbose > 1) std::cout << "  Will use " << nCpu_ << " CPUs." << std::endl;
    setup.pc.reset(new ProofConfig(*w, nCpu_, "", kFALSE)); 
    setup.toymcsampler->SetProofConfig(setup.pc.get());
  }   
  
  std::auto_ptr<HybridCalculator> hc(new HybridCalculator(data,setup.modelConfig, setup.modelConfig_bonly, setup.toymcsampler.get()));
  if (fSystematics) {
    // setup.nuisancePdf.reset(utils::makeNuisancePdf(*mc_s));
    // hc->ForcePriorNuisanceNull(*setup.nuisancePdf);
    // hc->ForcePriorNuisanceAlt(*setup.nuisancePdf);
  }

  // we need less B toys than S toys
  if (workingMode_ == MakeSignificance) {
      // need only B toys. just keep a few S+B ones to avoid possible divide-by-zero errors somewhere
      hc->SetToys(fgNToys, int(0.01*fgNToys)+1);
  } else if (!fUseCLs) {
      // we need only S+B toys to compute CLs+b
      hc->SetToys(int(0.01*fgNToys)+1, fgNToys);
  } else {
      // need both, but more S+B than B 
      hc->SetToys(int(0.25*fgNToys)+1, fgNToys);
  }

  static const char * istr = "__HypoTestInverterNew__importanceSamplingDensity";
  if(importanceSamplingNull_) {
    if(fVerbose > 1) std::cout << ">>> Enabling importance sampling for null hyp." << std::endl;
    if(!fSystematics) {
      coutE(InputArguments)<<"Importance sampling is not available without systematics";
      return std::auto_ptr<RooStats::HybridCalculator>();
      //throw std::invalid_argument("Importance sampling is not available without systematics");
    }
    RooArgSet importanceSnapshot;
    importanceSnapshot.addClone(poi);
    importanceSnapshot.addClone(*mc_s->GetNuisanceParameters());
    if (fVerbose > 2) importanceSnapshot.Print("V");
    hc->SetNullImportanceDensity(mc_b->GetPdf(), &importanceSnapshot);
  }
  if(importanceSamplingAlt_) {
    if(fVerbose > 1) std::cout << ">>> Enabling importance sampling for alt. hyp." << std::endl;
    if(!fSystematics) {
      coutE(InputArguments)<<"Importance sampling is not available without systematics";
      return std::auto_ptr<RooStats::HybridCalculator>();
    }
    if (w->pdf(istr) == 0) {
      w->factory("__oneHalf__[0.5]");
      RooAddPdf *sum = new RooAddPdf(istr, "fifty-fifty", *mc_s->GetPdf(), *mc_b->GetPdf(), *w->var("__oneHalf__"));
      w->import(*sum); 
    }
    RooArgSet importanceSnapshot;
    importanceSnapshot.addClone(poi);
    importanceSnapshot.addClone(*mc_s->GetNuisanceParameters());
    if (fVerbose > 2) importanceSnapshot.Print("V");
    hc->SetAltImportanceDensity(w->pdf(istr), &importanceSnapshot);
  }

  return hc;
}
#endif

#ifdef LATER
std::pair<double, double> HypoTestInverterNew::Eval(RooWorkspace *w, RooStats::ModelConfig *mc_s, RooStats::ModelConfig *mc_b, RooAbsData &data, double rVal, bool adaptive, double clsTarget) {
   // evaluate from a ws ? 

   // do we need this ???

    HypoTestInverterNew::Setup setup;
    RooRealVar *r = dynamic_cast<RooRealVar *>(mc_s->GetParametersOfInterest()->first());
    r->setVal(rVal);
    if (fVerbose) std::cout << "  " << r->GetName() << " = " << rVal << " +/- " << r->getError() << std::endl;
    std::auto_ptr<RooStats::HybridCalculator> hc(create(w, mc_s, mc_b, data, rVal, setup));
    std::pair<double, double> ret = eval(*hc, rVal, adaptive, clsTarget);

    // add to plot 
    if (limitPlot_.get()) { 
        limitPlot_->Set(limitPlot_->GetN()+1);
        limitPlot_->SetPoint(limitPlot_->GetN()-1, rVal, ret.first); 
        limitPlot_->SetPointError(limitPlot_->GetN()-1, 0, ret.second);
    }

    return ret;
}
#endif

// template<class HypoTestCalcType> 
// void HypoTestInverterNew::AddMoreToys(HypoTestCalcType & hc, HypoTestResult & hcResult, 
//                  double clsTarget, double & clsMid, double & clsMidErr) {
//    // add more toys until desired accuracy is reached 

// }


HypoTestResult * HypoTestInverterNew::Eval(HypoTestCalculatorGeneric &hc, bool adaptive, double clsTarget) {

   // run the hypothesis test 
   HypoTestResult *  hcResult = hc.GetHypoTest();
   if (hcResult == 0) {
      oocoutE((TObject*)0,Eval) << "HypoTestInverter::Eval - HypoTest failed" << std::endl;
      return hcResult; 
   }

   // to be seen.......why CMS codes is  having this - need to check 
#ifdef LATER_TBI
   if (testStat_ == "Atlas" || testStat_ == "Profile") {
      // I need to flip the P-values
      hcResult->SetPValueIsRightTail(!hcResult->GetPValueIsRightTail());
      hcResult->SetTestStatisticData(hcResult->GetTestStatisticData()-1e-9); // issue with < vs <= in discrete models
   } else {
      hcResult->SetTestStatisticData(hcResult->GetTestStatisticData()+1e-9); // issue with < vs <= in discrete models
   }
#endif

   double clsMid    = (fUseCLs ? hcResult->CLs()      : hcResult->CLsplusb());
   double clsMidErr = (fUseCLs ? hcResult->CLsError() : hcResult->CLsplusbError());

   //if (fVerbose) std::cout << (fUseCLs ? "\tCLs = " : "\tCLsplusb = ") << clsMid << " +/- " << clsMidErr << std::endl;
   
   if (adaptive) {
 
      if (fCalcType == kHybrid) HypoTestWrapper<HybridCalculator>::SetToys((HybridCalculator*)&hc, fUseCLs ? fgNToys : 1, 4*fgNToys);
      if (fCalcType == kFrequentist) HypoTestWrapper<FrequentistCalculator>::SetToys((FrequentistCalculator*)&hc, fUseCLs ? fgNToys : 1, 4*fgNToys);

   while (clsMidErr >= fgCLAccuracy && (clsTarget == -1 || fabs(clsMid-clsTarget) < 3*clsMidErr) ) {
      std::auto_ptr<HypoTestResult> more(hc.GetHypoTest());
      
#ifdef LATER_TBI
      if (testStat_ == "Atlas" || testStat_ == "Profile")
         more->SetPValueIsRightTail(!more->GetPValueIsRightTail());
#endif
      hcResult->Append(more.get());
      clsMid    = (fUseCLs ? hcResult->CLs()      : hcResult->CLsplusb());
      clsMidErr = (fUseCLs ? hcResult->CLsError() : hcResult->CLsplusbError());
      if (fVerbose) std::cout << (fUseCLs ? "\tCLs = " : "\tCLsplusb = ") << clsMid << " +/- " << clsMidErr << std::endl;
   }

   }
   if (fVerbose ) {
      std::cout <<
         "\tCLs      = " << hcResult->CLs()      << " +/- " << hcResult->CLsError()      << "\n" <<
         "\tCLb      = " << hcResult->CLb()      << " +/- " << hcResult->CLbError()      << "\n" <<
         "\tCLsplusb = " << hcResult->CLsplusb() << " +/- " << hcResult->CLsplusbError() << "\n" <<
         std::endl;
   }
   
   fPerf_totalToysRun += (hcResult->GetAltDistribution()->GetSize() + hcResult->GetNullDistribution()->GetSize());

    // if (!plot_.empty() && workingMode_ != MakeLimit) {
    //      HypoTestPlot plot(*hcResult, 30);
    //      TCanvas *c1 = new TCanvas("c1","c1");
    //      plot.Draw();
    //      c1->Print(plot_.c_str());
    //      delete c1;
    //  }


   return hcResult;
} 



#ifdef LATER
//______________________________________________________________________________________________________________
bool HypoTestInverterNew::RunSinglePoint(RooWorkspace *w, RooStats::ModelConfig *mc_s, RooStats::ModelConfig *mc_b, RooAbsData &data, double &limit, double &limitErr, const double *hint) {
   // run a single point

    RooRealVar *r = dynamic_cast<RooRealVar *>(mc_s->GetParametersOfInterest()->first()); r->setConstant(true);
    std::pair<double, double> result = eval(w, mc_s, mc_b, data, rValue_, true);
    std::cout << "\n -- Hybrid New -- \n";
    std::cout << (fUseCLs ? "CLs = " : "CLsplusb = ") << result.first << " +/- " << result.second << std::endl;
    limit = result.first;
    limitErr = result.second;
    return true;
}
#endif

bool HypoTestInverterNew::RunFixedScan( int nBins, double xMin, double xMax )
{
   // Run a Fixed scan in npoints between min and max

   CreateResults();
  // safety checks
  if ( nBins<=0 ) {
    std::cout << "Please provide nBins>0\n";
    return false;
  }
  if ( nBins==1 && xMin!=xMax ) {
    std::cout << "nBins==1 -> I will run for xMin (" << xMin << ")\n";
  }
  if ( xMin==xMax && nBins>1 ) { 
    std::cout << "xMin==xMax -> I will enforce nBins==1\n";
    nBins = 1;
  }
  if ( xMin>xMax ) {
    std::cout << "Please provide xMin (" << xMin << ") smaller that xMax (" << xMax << ")\n";
    return false;
  } 
  
  for (int i=0; i<nBins; i++) {
    double thisX = xMin+i*(xMax-xMin)/(nBins-1);
    bool status = RunOnePoint(thisX);
    
    // check if failed status
    if ( status==false ) {
      std::cout << "Loop interupted because of failed status\n";
      return false;
    }
  }

  return true;
}


bool HypoTestInverterNew::RunOnePoint( double rVal)
{
   // run only one point at the given value

   CreateResults();

   // check if rVal is in the range specified for fScannedVariable
   if ( rVal<fScannedVariable->getMin() ) {
     std::cout << "Out of range: using the lower bound on the scanned variable rather than " << rVal<< "\n";
     rVal = fScannedVariable->getMin();
   }
   if ( rVal>fScannedVariable->getMax() ) {
     std::cout << "Out of range: using the upper bound on the scanned variable rather than " << rVal<< "\n";
     rVal = fScannedVariable->getMax();
   }

   // save old value 
   double oldValue = fScannedVariable->getVal();

   // evaluate hybrid calculator at a single point
   fScannedVariable->setVal(rVal);
   // need to set value of rval in hybridcalculator
   const ModelConfig * altModel = fCalculator0->GetAlternateModel();
   RooArgSet poi; poi.add(*altModel->GetParametersOfInterest());
   // set poi to right values 
   poi = RooArgSet(*fScannedVariable);
   const_cast<ModelConfig*>(altModel)->SetSnapshot(poi);

   if (fVerbose > 0) 
      std::cout << "Running for " << fScannedVariable->GetName() << " = " << rVal << endl;


   
   // compute the results
   HypoTestResult* result =   Eval(*fCalculator0,false,-1);

   
   double lastXtested;
   if ( fResults->ArraySize()!=0 ) lastXtested = fResults->GetXValue(fResults->ArraySize()-1);
   else lastXtested = -999;

   if ( lastXtested==rVal ) {
     
     std::cout << "Merge with previous result\n";
     HypoTestResult* prevResult =  fResults->GetResult(fResults->ArraySize()-1);
     prevResult->Append(result);
     delete result; // t.b.c

   } else {
     
     // fill the results in the HypoTestInverterResult array
     fResults->fXValues.push_back(rVal);
     fResults->fYObjects.Add(result);

   }

      // std::cout << "computed value for poi  " << rVal  << " : " << fResults->GetYValue(fResults->ArraySize()-1) 
      //        << " +/- " << fResults->GetYError(fResults->ArraySize()-1) << endl;

   fScannedVariable->setVal(oldValue);
   
   return true;
}



// RooStats::HypoTestResult * HypoTestInverterNew::readToysFromFile(double rValue) {
//     if (!fReadToysFromHere){
//       coutE(InputArguments)<<"Cannot use readHypoTestResult: option toysFile not specified, or input file empty";
//       return HypoTestResult();
//     }

//     TDirectory *toyDir = fReadToysFromHere->GetDirectory("toys");
//     if (!toyDir){       
//       coutE(InputArguments)<<"Cannot use readHypoTestResult: option toysFile not specified, or input file empty";
//       return HypoTestResult();
//     }
//     if (fVerbose) std::cout << "Reading toys for r = " << rValue << std::endl;
//     TString prefix = TString::Format("HypoTestResult_r%g_",rValue);
//     std::auto_ptr<RooStats::HypoTestResult> ret;
//     TIter next(toyDir->GetListOfKeys()); TKey *k;
//     while ((k = (TKey *) next()) != 0) {
//         if (TString(k->GetName()).Index(prefix) != 0) continue;
//         RooStats::HypoTestResult *toy = dynamic_cast<RooStats::HypoTestResult *>(toyDir->Get(k->GetName()));
//         if (toy == 0) continue;
//         if (fVerbose) std::cout << " - " << k->GetName() << std::endl;
//         if (ret.get() == 0) {
//             ret.reset(new RooStats::HypoTestResult(*toy));
//         } else {
//             ret->Append(toy);
//         }
//     }

//     return ret.release();
// }
