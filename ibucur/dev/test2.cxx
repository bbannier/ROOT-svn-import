// ROOT headers
#include "TBenchmark.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TSystem.h"
#include "TGraph.h"

// RooFit headers
#include "RooWorkspace.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooSimultaneous.h"
#include "RooAbsCategoryLValue.h"
#include "RooMinuit.h"
#include "RooMinimizer.h"
#include "RooFitResult.h"
#include "RooAbsReal.h"
#include "RooArgSet.h"
#include "RooCmdArg.h"
#include "RooLinkedList.h"
#include "RooAddPdf.h"
#include "RooNumIntConfig.h"

// RooStats headers
#include "RooStats/ModelConfig.h"
#include "RooStats/RooStatsUtils.h"
#include "RooStats/ProfileLikelihoodTestStat.h"
#include "RooStats/HistFactory/FlexibleInterpVar.h"

const int RUNS = 1;

using namespace RooFit;
using namespace RooStats;

void buildSumModel(RooWorkspace *w);
void buildAddModel(RooWorkspace *w);
void buildSimultaneousModel(RooWorkspace *w);

void test2(const char* file = "comb_hgg_125.root", const char* ws = "w", const char* data = "data_obs") {

   RooAbsReal::defaultIntegratorConfig()->method1D().setLabel("RooAdaptiveGaussKronrodIntegrator1D");

   RooStats::HistFactory::FlexibleInterpVar fiv;
   gSystem->Load("libHistFactory");   
   TBenchmark myBenchmark;

   // XXX never forget workspace name
   // Build Higgs model
//   TFile f(file);
//   RooWorkspace* w = (RooWorkspace *)f.Get(ws);
//   ModelConfig* model = (ModelConfig*)w->obj("ModelConfig");

//   w->var("MH")->setVal(125);

   RooWorkspace *w = new RooWorkspace("w", kTRUE);
//   buildSimultaneousModel(w);   
   buildAddModel(w);
//   buildSumModel(w);
   ModelConfig* model = (ModelConfig*)w->obj("S+B");

//   *((RooArgSet *)model->GetObservables()) = *w->data(data)->get(0);
   
   RooLinkedList commands;
   RooCmdArg arg1(RooFit::CloneData(kFALSE));
   RooCmdArg arg2(RooFit::Constrain(*model->GetNuisanceParameters()));
   commands.Add(&arg1);
   commands.Add(&arg2);
   
//   std::cout << "Observables before createNLL " << std::endl;
//   RooArgSet *getObs = (RooArgSet *)model->GetObservables();
//   getObs->setRealValue("obs1", 3);
//   *getObs = *w->data("data_obs")->get(9);
//   getObs->Print("v");

   // XXX never forget data set name
   RooAbsReal* nll = model->GetPdf()->createNLL(*w->data(data), commands);
//   RooAbsReal* nll = RooStats::CreateNLL(*model->GetPdf(), *w->data(data), commands);

  
 //  *((RooArgSet *)nll->getObservables(*w->data(data))) = *model->GetObservables();

//   w->var("sig")->setVal(0.1);
//   double lastVal = nll->getVal();
//   double *values = new double[10];
//   double *x = new double[10];
//   std::cout << "Observables after createNLL " << std::endl;
//   RooArgSet obs;
  // model->GetObservables()->snapshot(obs);
/*
   int j = 0;
   Int_t numPdfs = add->pdfList().getSize();
   for(double d = 0.3; j < 10; d += 0.2, j++) {
      TIterator *itPdf = add->pdfList().createIterator();
      TIterator *itCoef = add->coefList().createIterator();

      w->var("sig")->setVal(d);
      getObs->setRealValue("obs1", 5);
    //  values[j] = nll->getVal() - lastVal; 
  //    x[j] = d; lastVal = nll->getVal();
    //  std::cout << "nll diff " << d << " " << values[j] << std::endl;
   //   std::cout << "obs " << obs->getRealValue("obs1") << " "
     //           << obs->getRealValue("obs2") << " " 
       //         << obs->getRealValue("obs3") << std::endl;

      for(Int_t i = 0; i < numPdfs; ++i) {
         RooAbsReal *coef = (RooAbsReal *)(itCoef->Next());
         RooAbsPdf  *pdf  = (RooAbsPdf * )(itPdf->Next() );
         RooArgSet vars(*pdf->getVariables());
    //     vars = obs;
        // std::cout << "getObs";
        // getObs->Print("v");
        // std::cout << "Vars";
        // vars.Print("v");
         std::cout << "\n" << pdf->getObservables(w->data("data_obs"))->find("obs1") << std::endl;
        // if (pdf->getVariables()->find("obs1")) std::cout << "Vars: " << pdf->getVariables()->getRealValue("obs1") << std::endl;
         std::cout << "\n" << getObs->find("obs1") << std::endl; 
         //std::cout << "Obs: " << getObs->getRealValue("obs1") << std::endl;
         //std::cout << "pdf " << i << pdf->ClassName() << " value " << pdf->getVal(obs) << std::endl;
      std::cout << "\nObs: "; getObs->Print("v");
      std::cout << "\nPdf: "; pdf->getObservables(w->data("data_obs"))->Print("v");
//       std::cout << "coef " << i << " value " << coef->getVal() << std::endl;
      }
   }

   w->data("data_obs")->Print("v");
//   delete values; delete x;
   return;
*/

   RooMinimizer m(*nll);
   m.setMinimizerType("Minuit2");
   m.optimizeConst(2);
   m.setErrorLevel(0.5);
   m.setEps(1);
//   m.setPrintLevel(-1);
   m.setStrategy(0);

   myBenchmark.Start("CombinedLikelihood");

   for(Int_t i = 0; i < RUNS; i++) {
      m.migrad();
   }

   myBenchmark.Stop("CombinedLikelihood");

   myBenchmark.Print("CombinedLikelihood");

   
//   model->GetPdf()->getParameters(w->data("data"))->Print("s");
//   m.setVerbose(kFALSE);

   // Run HESSE to calculate errors from d2L/dp2
   // m.hesse();
   
   // Run MINOS on only one parameter (but which one?)
   // m.minos(/* param */);

  // m.save()->Print("v");

  // RooFitResult *r = m.save();
  // r->Print("v");
   // RooPlot* frame = m.contour(/* */);
   // frame->SetTitle("RooMinuit contour plot");
}

//__________________________________________________________________________________
void buildSimultaneousModel(RooWorkspace *w)
{
   // Build model
   w->factory("sig[2,0,10]");
   w->factory("Poisson::u1(x1[15,0,100], bkg1[30,0,100])");
   w->factory("Exponential::u2(x2[25,0,100], bkg2[-0.1,-10,0])");
   w->factory("Landau::u3(x3[10,0,100], 15, bkg3[4, 0, 10])");
   w->factory("Gaussian::constr1(gbkg1[50,0,100], bkg1, 3)");
   w->factory("Gaussian::constr2(gbkg2[-10,0], bkg2, 2)");
   w->factory("Gaussian::constr3(gbkg3[3, 1, 15], bkg3, 1)");

   w->factory("ExtendPdf::ext_pdf1(PROD::p1(u1,constr1), expr::n1('sig+bkg1', sig, bkg1))");
   w->factory("ExtendPdf::ext_pdf2(PROD::p2(u2,constr2), expr::n2('sig+bkg2', sig, bkg2))");
   w->factory("ExtendPdf::ext_pdf3(PROD::p3(u3,constr3), expr::n3('sig+bkg3', sig, bkg3))");
//   w->factory("ExtendPdf::ext_0pdf1(u1, expr::n1('sig+bkg1', sig, bkg1))");
//   w->factory("ExtendPdf::ext_0pdf2(u2, expr::n2('sig+bkg2', sig, bkg2))");
//   w->factory("ExtendPdf::ext_0pdf3(u3, expr::n3('sig+bkg3', sig, bkg3))");
//   w->factory("PROD:ext_pdf1(ext_0pdf1,constr1)");
//   w->factory("PROD:ext_pdf2(ext_0pdf2,constr2)");
//   w->factory("PROD:ext_pdf3(ext_0pdf3,constr3)");

   w->factory("SIMUL::sim_pdf(index[cat1,cat2,cat3],cat1=ext_pdf1,cat2=ext_pdf2,cat3=ext_pdf3)");


   // create combined signal + background model configuration
   ModelConfig *sbModel = new ModelConfig("S+B", w);
   sbModel->SetObservables("x1,x2,x3,index");
   sbModel->SetParametersOfInterest("sig");
   sbModel->SetGlobalObservables("gbkg1,gbkg2,gbkg3");
   sbModel->SetNuisanceParameters("bkg1,bkg2,bkg3");
   sbModel->SetPdf("sim_pdf");
   w->import(*sbModel);

   // create combined background model configuration
   ModelConfig *bModel = new ModelConfig(*sbModel);
   bModel->SetName("B");
   w->import(*bModel);

   // define data set
   RooDataSet *data = w->pdf("sim_pdf")->generate(*sbModel->GetObservables(), Extended(), Name("data_obs"));
   w->import(*data);

//   RooArgList constraints;
//   RooAbsPdf *simplePdf = StripConstraints(*w->pdf("sim_pdf"), *sbModel->GetObservables(), constraints);
}





void buildAddModel(RooWorkspace *w)
{
   // Build model
   w->factory("Gaussian::s1(obs1[10,-1000,1000], sig[10,-1000,1000], bkg1[1,0,1000])");
   w->factory("Gaussian::s2(obs2[10,-1000,1000], 40, sig)");
   w->factory("Gaussian::s3(obs3[20,0,1000], sig, 1)"); 
   w->factory("SUM::sum_pdf(0.2*s1,0.3*s2,0.5*s3)");

   // create combined signal + background model configuration
   ModelConfig *sbModel = new ModelConfig("S+B", w);
   sbModel->SetObservables("obs1,obs2,obs3");
   sbModel->SetParametersOfInterest("sig");
//   sbModel->SetGlobalObservables("gbkg1,gbkg2,gbkg3");
   sbModel->SetNuisanceParameters("bkg1");
   sbModel->SetPdf("sum_pdf");
   w->import(*sbModel);

   // create combined background model configuration
   ModelConfig *bModel = new ModelConfig(*sbModel);
   bModel->SetName("B");
   w->import(*bModel);

   // define data set
   RooDataSet *data = w->pdf("sum_pdf")->generate(*sbModel->GetObservables(), 10);
   data->SetName("data_obs");
   w->import(*data);

   w->pdf("sum_pdf")->Print("");
}


void buildSumModel(RooWorkspace *w)
{
   w->factory("Poisson::p1(obs1[3,1,5],sig[2,1,10])");
   w->factory("prod::sig2(2,sig)");
   w->factory("Poisson::p2(obs2[2,1,5],sig2)");
   w->factory("prod::sig3(3,sig)");
   w->factory("Poisson::p3(obs3[1,1,10],sig3)");
   w->factory("f1[1,0,2]");
   w->factory("f2[1.5,0,2]");
   w->factory("f3[1,0,2]");
   w->factory("SUM::sum_pdf(f1*p1,f2*p2, f3*p3)");

   ModelConfig* sbModel = new ModelConfig("S+B", w);
   sbModel->SetObservables("obs1,obs2,obs3");
   sbModel->SetParametersOfInterest("sig");
   sbModel->SetNuisanceParameters("f1,f2,f3");
   sbModel->SetPdf("sum_pdf");
   w->import(*sbModel);


   RooDataSet *data = w->pdf("sum_pdf")->generate(*sbModel->GetObservables(), 10);
   data->SetName("data_obs");
   w->import(*data);

   w->pdf("sum_pdf")->Print("");
}


int main(int argc, char* argv[]) {
   if(argc == 4) {
      std::cout << "Using command line args" << std::endl;
      test2(argv[1], argv[2], argv[3]);
   } else {
      std::cout << "Using default args" << std::endl;
      test2();
   }
   return 0;
}


