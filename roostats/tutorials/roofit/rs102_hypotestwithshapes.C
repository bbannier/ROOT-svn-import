/////////////////////////////////////////////////////////////////
//
// rs102_hypotestwithshapes for RooStats project
// Author: Kyle Cranmer <cranmer@cern.ch>
// 
// Modified from version of February 29, 2008
//
//
/////////////////////////////////////////////////////////////////

#include "RooGlobalFunc.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooAddPdf.h"
#include "RooProdPdf.h"
#include "RooAddition.h"
#include "RooProduct.h"
#include "TCanvas.h"
#include "RooChebychev.h"
#include "RooAbsPdf.h"
#include "RooFit.h"
#include "RooFitResult.h"
#include "RooPlot.h"
#include "RooAbsArg.h"
#include "RooWorkspace.h"
#include "RooStats/ProfileLikelihoodCalculator.h"
#include "RooStats/HypoTestResult.h"
#include <string>


using namespace RooFit;
using namespace RooStats;
 
//____________________________________
void AddModel(RooWorkspace* ws){

  Double_t lowRange = 60, highRange = 200;

  // make a RooRealVar for the observable
  RooRealVar mTauTau("mTauTau", "M_{#tau#tau} (GeV)", lowRange, highRange);
 
  ////////////////////////////////////////////
  // Make models
  // This part requires an intellegent modeling of signal and background.
  // This is only an example.  
  // One may need to separately fit the shape of the backgrounds, or 
  // introduce a control sample and do a simultaneous fit.
  // Other examples will be provided that treat the background shape
  // uncertainty in a more realistic way.

  /////////////////////////////////////////////
  // make signal model.  A double Gaussian
  RooRealVar mH("mH","Higgs Mass",130,90,160) ; 

  RooRealVar sigma1("sigma1","Width of Gaussian",8,2,100)  ;
  RooRealVar sigma2("sigma2","Width of Gaussian",12,2,100)  ;  
  RooRealVar fgauss1("fgauss1","fraction of 2nd Gaussian",.635,0,1)  ;
  RooGaussian sigGauss1("sigGauss1", "core Gaussian for signal", mTauTau, mH, sigma1);
  RooGaussian sigGauss2("sigGauss2", "wide Gaussian for signal", mTauTau, mH, sigma2);
  
  RooAddPdf sigModel("sigModel", "signal model", RooArgList(sigGauss1,sigGauss2), RooArgList(fgauss1));


  /////////////////////////////////////////////
  // make zjj model.  Just like signal model
  RooRealVar mZ("mZ", "Z Mass", 91.2, 0, 100);
  mZ.setConstant();
  RooRealVar sigma1_z("sigma1_z","Width of Gaussian",8,6,100)  ;
  RooRealVar sigma2_z("sigma2_z","Width of Gaussian",13,6,100)  ;  
  RooRealVar fgauss1_z("fgauss1_z","fraction of 2nd Gaussian",.635,0,1)  ;
  RooGaussian zjjGauss1("zjjGauss1", "core Gaussian for signal", mTauTau, mZ, sigma1_z);
  RooGaussian zjjGauss2("zjjGauss2", "wide Gaussian for signal", mTauTau, mZ, sigma2_z);
  
  RooAddPdf zjjModel("zjjModel", "Z+jets model", RooArgList(zjjGauss1,zjjGauss2), RooArgList(fgauss1_z));


  //////////////////////////////////////////////
  // make QCD model
  RooRealVar a0("a0","a0",0.26,-1,1) ; 
  RooRealVar a1("a1","a1",-0.17596,-1,1) ; 
  RooRealVar a2("a2","a2",0.018437,-1,1) ; 
  RooRealVar a3("a3","a3",0.02,-1,1) ; 
  RooChebychev qcdModel("qcdModel","A  Polynomail for QCD",mTauTau,RooArgList(a0,a1,a2)) ; 

  // let's assume this shape is known, but the normalization is not
  a0.setConstant();
  a1.setConstant();
  a2.setConstant();

  //////////////////////////////////////////////
  // combined model

  // mu is the signal strength in units of the expectation.  
  // eg. mu = 1 is the SM, mu = 0 is no signal, mu=2 is 2x the SM
  RooRealVar mu("mu","signal strength in units of SM expectation",1,0.,2) ; 
  // rho is the ratio of the MSSM/SM in the cross-section*BR
  Double_t ratioMSSMtoSM = 1;
  RooRealVar rho("rho","ratio of the MSSM/SM",ratioMSSMtoSM,0.,10) ; 
  rho.setConstant();
  // ratio of signal efficiency to nominal signal efficiency. rho=1 is nominal situation
  RooRealVar ratioSigEff("ratioSigEff","ratio of signal efficiency to nominal signal efficiency",1,0.,2) ; 
  ratioSigEff.setConstant(kTRUE);
  RooRealVar fsigExpected("fsigExpected","expected fraction of signal events",.2,0.,1) ; 
  fsigExpected.setConstant();
  RooProduct fsig("fsig","fraction of signal events",RooArgSet(mu,rho,ratioSigEff,fsigExpected)) ; 
  RooRealVar fzjj("fzjj","fraction of zjj background events",.4,0.,1) ; 

  // full model
  RooAddPdf model("model","sig+zjj+qcd background shapes",RooArgList(sigModel,zjjModel, qcdModel),RooArgList(fsig,fzjj)) ; 


  // interesting for debugging, the model
  model.printCompactTree("","fullModel.txt");
  model.graphVizTree("fullModel.dot");

  ws->import(model);
}

//____________________________________
void AddData(RooWorkspace* ws){
  cout << "da add data" << endl;

  RooAbsPdf* model = ws->pdf("model");
  RooRealVar* mTauTau = ws->var("mTauTau");
 
  RooDataSet* data = model->generate(*mTauTau,100);
  
  ws->import(*data, RenameDataset("data"));

}

//____________________________________
void DoHypothesisTest(RooWorkspace* ws){
  cout << "do hypothesis test" << endl;



  ProfileLikelihoodCalculator plc;
  plc.SetWorkspace(ws);
  plc.SetCommonPdf("model");

  plc.SetData("data"); 

  RooRealVar* mu = ws->var("mu");
  RooRealVar* mH = ws->var("mH");
  RooArgSet* nullParams = new RooArgSet("nullParams");
  nullParams->addClone(*mu);
  nullParams->addClone(*mH);
  // here we explicitly set the value of the parameters for the null
  nullParams->setRealValue("mu",0); 
  nullParams->setRealValue("mH",130.); 
  plc.SetNullParameters(nullParams);

  // Step 7, Use the Calculator to get a HypoTestResult
  HypoTestResult* htr = plc.GetHypoTest();
  cout << "-------------------------------------------------" << endl;
  cout << "The p-value for the null is " << htr->NullPValue() << endl;
  cout << "Corresponding to a signifcance of " << htr->Significance() << endl;
  cout << "-------------------------------------------------\n\n" << endl;


}

//____________________________________
void MakePlots(RooWorkspace* ws) {

  RooAbsPdf* model = ws->pdf("model");
  RooAbsPdf* sigModel = ws->pdf("sigModel");
  RooAbsPdf* zjjModel = ws->pdf("zjjModel");
  RooAbsPdf* qcdModel = ws->pdf("qcdModel");

  RooRealVar* mu = ws->var("mu");
  RooRealVar* mH = ws->var("mH");
  RooRealVar* mTauTau = ws->var("mTauTau");
  RooAbsData* data = ws->data("data");

   RooFitResult* fitResult = model->fitTo(*data,Save(kTRUE),Minos(kFALSE), Hesse(kFALSE));
  

  //plot sig candidates, full model, and individual componenets
  TCanvas* cdata = new TCanvas();
  RooPlot* frame = mTauTau->frame() ; 
  data->plotOn(frame ) ; 
  model->plotOn(frame) ;   
  //  model->plotOn(frame,Components(*sigModel),LineStyle(kDashed), LineColor(kRed)) ;   
  //  model->plotOn(frame,Components(*zjjModel),LineStyle(kDashed),LineColor(kBlack)) ;   
  //  model->plotOn(frame,Components(*qcdModel),LineStyle(kDashed),LineColor(kGreen)) ;   
  cdata->SaveAs("alternateFit.gif");
  
  
  frame->SetTitle("An example fit to the signal + background model");
  frame->Draw() ;

  //////////////////////////////////////////////////////////
  // Do Fit to the Null hypothesis.  Eg. the \hat\hat variables
  cout << "--------------- Fit to bkg-only -------------------" << endl;

  mu->setVal(0); // set signal fraction to 0
  mu->setConstant(kTRUE); // set constant 
  mH->setConstant(kTRUE); // parameters of signal model don't make any difference, so fix them to speed up convergence

  RooFitResult* fitResult2 = model->fitTo(*data, Save(kTRUE), Minos(kFALSE), Hesse(kFALSE));
  fitResult2->Print();

  // plot signal candidates with background model and components
  TCanvas* cbkgonly = new TCanvas();
  RooPlot* xframe2 = mTauTau->frame() ; 
  data->plotOn(xframe2, DataError(RooAbsData::SumW2)) ; 
  model->plotOn(xframe2) ; 
  model->plotOn(xframe2, Components(*zjjModel),LineStyle(kDashed),LineColor(kBlack)) ;   
  model->plotOn(xframe2, Components(*qcdModel),LineStyle(kDashed),LineColor(kGreen)) ;   
  
  xframe2->SetTitle("An example fit to the background-only model");
  xframe2->Draw() ;
  cbkgonly->SaveAs("nullFit.gif");

}


//____________________________________
void rs102_hypotestwithshapes() {

  RooWorkspace* wspace = new RooWorkspace("myWS");

  AddModel(wspace);

  AddData(wspace);

  wspace->Print();

  DoHypothesisTest(wspace);

  //  MakePlots(wspace);

  delete wspace;
}
