#include "RooRandom.h"
#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooPolynomial.h"
#include "RooArgSet.h"
#include "RooAddPdf.h"
#include "RooDataSet.h"
#include "RooExtendPdf.h"
#include "RooWorkspace.h"

#ifndef __CINT__  // problem including this file with CINT
#include "RooGlobalFunc.h"
#endif

#include "RooStats/HybridCalculator.h"
#include "RooStats/HybridResult.h"
#include "RooStats/HybridPlot.h"

#define USE_WS

void rs202_hybridcalculator()
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
  RooAddPdf tot_pdf("tot_pdf","",RooArgList(sig_pdf,bkg_pdf),RooArgList(sig_yield,bkg_yield));

  /// build the prior PDF on the parameters to be integrated
  // gaussian contraint on the background yield ( N_B = 40 +/- 10  ie. 25% )
  RooGaussian bkg_yield_prior("bkg_yield_prior","",bkg_yield,RooConst(bkg_yield.getVal()),RooConst(10.));
  RooArgSet nuisance_parameters(bkg_yield); // variables to be integrated

#ifdef USE_WS
  RooWorkspace wspace;
  wspace.import(tot_pdf);
  wspace.import(bkg_ext_pdf);
  wspace.import(bkg_yield_prior);
  RooRealVar * obsdata = wspace.var("x");
  RooDataSet* data = tot_pdf.generate(*obsdata,RooFit::Extended());
  wspace.import(*data, RenameDataset("dataset") );
#else
  /// generate a data sample
  RooArgList observables(x); // variables to be generated
  RooDataSet* data = tot_pdf.generate(observables,RooFit::Extended());
#endif
  //***********************************************************************//

  /// run HybridCalculator on those inputs
  HybridCalculator myHybridCalc("myHybridCalc","HybridCalculator example"); 
  //HybridCalculator myHybridCalc("myHybridCalc","HybridCalculator example",tot_pdf,bkg_ext_pdf,observables,nuisance_parameters,bkg_yield_prior);

  // method needed for hybrid calculator
  myHybridCalc.SetNumberOfToys(3000); 
  myHybridCalc.UseNuisancePriorPdf(true); 
  myHybridCalc.SetNuisancePriorPdf(bkg_yield_prior); 
  // here I use the default test statistics: 2*lnQ (optional)
  myHybridCalc.SetTestStatistics(1);

  // use HypoTest interface
  HypoTestCalculator & hypoCalc = myHybridCalc;  

#ifdef USE_WS
  hypoCalc.SetWorkspace(wspace);
  hypoCalc.SetCommonPdf(tot_pdf.GetName() );
  hypoCalc.SetCommonPdf(bkg_ext_pdf.GetName() );
  hypoCalc.SetData("dataset" );
#else 
  hypoCalc.SetCommonPdf(tot_pdf);
  hypoCalc.SetNullPdf(bkg_ext_pdf);
  hypoCalc.SetData(*data); 
  hypoCalc.SetNullParameters(nuisance_parameters);
  // observables are not needed
#endif  
  
                             

  /// run 3000 toys with gaussian prior on the background yield
  //HybridResult* myHybridResult = myHybridCalc.Calculate(*data,3000,true);
  HypoTestResult * result = hypoCalc.GetHypoTest(); 
  if (!result) { 
     std::cerr << "\nError returned from Hypothesis test" << std::endl;
     return;
  }

  /// run 1000 toys without gaussian prior on the background yield
  //HybridResult* myHybridResult = myHybridCalc.Calculate(*data,1000,false);

  /// save the toy-MC results to file, this way splitting into sub-batch jobs is possible
  //TFile fileOut("some_hybridresult.root","RECREATE");
  //fileOut.cd();
  //myHybridResult.Write();
  //fileOut.Close();

  /// read the results from a file
  //TFile fileIn("some_hybridresult.root");
  //HybridResult* myOtherHybridResult = (HybridResult*) fileIn.Get("myHybridCalc");

  /// example on how to merge with toy-MC results obtained in another job
  //HybridResult* mergedHybridResult = new HybridResult("mergedHybridResult","this object holds merged results");
  //mergedHybridResult->Add(myHybridResult);
  //mergedHybridResult->Add(myOtherHybridResult);
  /// or
  //myHybridResult->Add(myOtherHybridResult);

  /// nice plot of the results
  HybridResult* myHybridResult = dynamic_cast<HybridResult *>(result); 
  HybridPlot* myHybridPlot = myHybridResult->GetPlot("myHybridPlot","Plot of results with HybridCalculator",100);
  myHybridPlot->Draw();

  /// recover and display the results
  double clsb_data = myHybridResult->CLsplusb();
  double clb_data = myHybridResult->CLb();
  double cls_data = myHybridResult->CLs();
  double data_significance = myHybridResult->Significance();

  /// compute the mean expected significance from toys
  double mean_sb_toys_test_stat = myHybridPlot->GetSBmean();
  myHybridResult->SetDataTestStatistics(mean_sb_toys_test_stat);
  double toys_significance = myHybridResult->Significance();

  std::cout << "Completed HybridCalculator example:\n"; 
  std::cout << " - -2lnQ = " << myHybridResult->GetTestStat_data() << endl;
  std::cout << " - CL_sb = " << clsb_data << std::endl;
  std::cout << " - CL_b  = " << clb_data << std::endl;
  std::cout << " - CL_s  = " << cls_data << std::endl;
  std::cout << " - significance of data  = " << data_significance << std::endl;
  std::cout << " - mean significance of toys  = " << toys_significance << std::endl;
}


