#include "RooRandom.h"
#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooPolynomial.h"
#include "RooArgSet.h"
#include "RooAddPdf.h"
#include "RooDataSet.h"
#include "RooExtendPdf.h"
#include "RooConstVar.h"

#ifndef __CINT__  // problem including this file with CINT
#include "RooGlobalFunc.h"
#endif

#include "RooStats/HybridCalculator.h"
#include "RooStats/HybridResult.h"
#include "RooStats/HybridPlot.h"

void rs203_HybridCalculator_with_ModelConfig()
{
  //***********************************************************************//
  // This macro show an example on how to use RooStats/HybridCalculator    //
  //***********************************************************************//
  //
  // With this example, you should get: CL_sb = 0.130 and CL_b = 0.946
  // (if data had -2lnQ = -3.0742). You can compare to the expected plot:
  // http://www-ekp.physik.uni-karlsruhe.de/~schott/roostats/hybridplot_example.png

  using namespace RooFit;
  using namespace RooStats;

  /// set RooFit random seed
  RooRandom::randomGenerator()->SetSeed(3007);

  /// build the models for background and signal+background
  RooRealVar x("x","",-3,3);
  RooArgList observables(x); // variables to be generated

  // gaussian signal
  RooRealVar sig_mean("sig_mean","",0);
  RooRealVar sig_sigma("sig_sigma","",0.8);
  RooGaussian sig_pdf("sig_pdf","",x,sig_mean,sig_sigma);
  RooRealVar sig_yield("sig_yield","",20,0,300);

  // flat background (extended PDF)
  RooRealVar bkg_slope("bkg_slope","",0);
  RooPolynomial bkg_pdf("bkg_pdf","",x,bkg_slope);
  RooRealVar bkg_yield("bkg_yield","",40,0,300);
  RooExtendPdf bkg_ext_pdf("bkg_ext_pdf","",bkg_pdf,bkg_yield);

  // total sig+bkg (extended PDF)
  RooAddPdf tot_pdf("tot_pdf","",RooArgList(sig_pdf,bkg_ext_pdf),RooArgList(sig_yield,bkg_yield));

  /// this does not work , why ??
  //RooAddPdf tot_pdf("tot_pdf","",RooArgList(sig_pdf,bkg_pdf),RooArgList(sig_yield,bkg_yield));

  /// build the prior PDF on the parameters to be integrated
  // gaussian contraint on the background yield ( N_B = 40 +/- 10  ie. 25% )
  RooGaussian bkg_yield_prior("bkg_yield_prior","",bkg_yield,RooConst(bkg_yield.getVal()),RooConst(10.));

  RooArgSet nuisance_parameters(bkg_yield); // variables to be integrated
  //nuisance_parameters

  /// generate a data sample
  RooDataSet* data = tot_pdf.generate(observables,RooFit::Extended());

  //***********************************************************************//

  std::cout << "create first model " << std::endl;

  ModelConfig model1("S+B model","S+B ModelConfig of the analysis");
  model1.SetPdf(tot_pdf);

  std::cout << "create second model " << std::endl;

  ModelConfig model2("B model","B ModelConfig of the analysis");
  model2.SetPdf(bkg_ext_pdf);
  model2.GetWS()->Print();
  model2.SetNuisanceParameters(nuisance_parameters);
  model2.SetPriorPdf(bkg_yield_prior);
 
  //***********************************************************************//

  /// run HybridCalculator on those inputs 
  HybridCalculator myHybridCalc(*data, model1, model2 );
  //myHybridCalc.SetModel(&model); // alternative

  myHybridCalc.SetNumberOfToys(3000); 
  //myHybridCalc.UseNuisance(false);                            

  // here I use the default test statistics: 2*lnQ (optional)
  myHybridCalc.SetTestStatistic(1);

  // calculate by running toys for the S+B and B hypothesis and retrieve the result
  HybridResult* myHybridResult = myHybridCalc.GetHypoTest(); 
  if (! myHybridResult) { 
     std::cerr << "\nError returned from Hypothesis test" << std::endl;
     return;
  }

  /// nice plot of the results
  HybridPlot* myHybridPlot = myHybridResult->GetPlot("myHybridPlot","Plot of results with HybridCalculator",100);
  myHybridPlot->Draw();

  /// recover and display the results
  double clsb_data = myHybridResult->CLsplusb();
  double clb_data = myHybridResult->CLb();
  double cls_data = myHybridResult->CLs();
  double data_significance = myHybridResult->Significance();
  double min2lnQ_data = myHybridResult->GetTestStat_data();

  /// compute the mean expected significance from toys
  double mean_sb_toys_test_stat = myHybridPlot->GetSBmean();
  myHybridResult->SetDataTestStatistics(mean_sb_toys_test_stat);
  double toys_significance = myHybridResult->Significance();

  std::cout << "Completed HybridCalculator example:\n"; 
  std::cout << " - -2lnQ = " << min2lnQ_data << endl;
  std::cout << " - CL_sb = " << clsb_data << std::endl;
  std::cout << " - CL_b  = " << clb_data << std::endl;
  std::cout << " - CL_s  = " << cls_data << std::endl;
  std::cout << " - significance of data  = " << data_significance << std::endl;
  std::cout << " - mean significance of toys  = " << toys_significance << std::endl;
}


