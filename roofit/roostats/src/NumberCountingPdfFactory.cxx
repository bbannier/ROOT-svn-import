// @(#)root/roostats:$Id: NumberCountingPdfFactory.cxx 25835 2008-10-15 19:37:19Z moneta $
// Author: Kyle Cranmer   28/07/2008

/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/////////////////////////////////////////
// NumberCountingPdfFactory
//
// Encapsulates common number counting utilities
/////////////////////////////////////////
  ///////////////////////////////////
  // Standalone Functions.
  // Naming conventions:
  //  Exp = Expected
  //  Obs = Observed
  //  P   = p-value
  //  Z   = Z-value or significance in Sigma (one-sided convention)
  //////////////////////////////////

#ifndef RooStats_NumberCountingPdfFactory
#include "RooStats/NumberCountingPdfFactory.h"
#endif

#ifndef RooStats_RooStatsUtils
#include "RooStats/RooStatsUtils.h"
#endif

#include "RooRealVar.h"
#include "RooAddition.h"
#include "RooProduct.h"
#include "RooDataSet.h"
#include "RooProdPdf.h"
#include "RooFitResult.h"
#include "RooPoisson.h"
#include "RooGlobalFunc.h"
#include "RooCmdArg.h"
#include "TTree.h"
#include <sstream>


// Without this macro the THtml doc  can not be generated
#if !defined(R__ALPHA) && !defined(R__SOLARIS) && !defined(R__ACC) && !defined(R__FBSD)
NamespaceImp(RooStats)
//NamespaceImp(NumberCountingUtils)
#endif




using namespace RooStats;


//_______________________________________________________
NumberCountingPdfFactory::NumberCountingPdfFactory(Double_t* sig, 
						     Double_t* back, 
						     Double_t* back_syst, 
							    Int_t nbins) {

  

  // A number counting combination for N channels with uncorrelated background 
  //  uncertainty.  
  // Background uncertainty taken into account via the profile likelihood ratio.
  // Arguements are an array of expected signal, expected background, and relative 
  // background uncertainty (eg. 0.1 for 10% uncertainty), and the number of channels.

  using namespace RooFit;
  using std::vector;

  // make a new workspace
  fWS = new RooWorkspace();

  // actually fill the workspace
  FillWorkspace(sig, back, back_syst, nbins) ;

}

//_______________________________________________________
NumberCountingPdfFactory::~NumberCountingPdfFactory(){
  delete fWS;
}

//_______________________________________________________
void NumberCountingPdfFactory::FillWorkspace(Double_t* sig, 
					Double_t* back, 
					Double_t* back_syst, 
					Int_t nbins) {

  // This method actually fills the workspace

  vector<RooRealVar*> backVec, tauVec, xVec, yVec;
  vector<RooProduct*> sigVec;
  vector<RooFormulaVar*> splusbVec;
  vector<RooPoisson*> sigRegions, sidebands;
  TList likelihoodFactors;
  TList observablesCollection;

  TTree* tree = new TTree();
  Double_t* xForTree = new Double_t[nbins];
  Double_t* yForTree = new Double_t[nbins];

  Double_t MaxSigma = 8; // Needed to set ranges for varaibles.

  RooRealVar*   masterSignal = 
    new RooRealVar("masterSignal","masterSignal",1., 0., 3.);
  for(Int_t i=0; i<nbins; ++i){
    std::stringstream str;
    str<<"_"<<i;
    RooRealVar*   expectedSignal = 
      new RooRealVar(("expected_s"+str.str()).c_str(),("expected_s"+str.str()).c_str(),sig[i], 0., 2*sig[i]);
    expectedSignal->setConstant(kTRUE);

    RooProduct*   s = 
      new RooProduct(("s"+str.str()).c_str(),("s"+str.str()).c_str(), RooArgSet(*masterSignal, *expectedSignal)); 

    RooRealVar*   b = 
      new RooRealVar(("b"+str.str()).c_str(),("b"+str.str()).c_str(),back[i],  0., 1.2*back[i]+MaxSigma*(sqrt(back[i])+back[i]*back_syst[i]));
    b->Print();
    Double_t _tau = 1./back[i]/back_syst[i]/back_syst[i];
    RooRealVar*  tau = 
      new RooRealVar(("tau"+str.str()).c_str(),("tau"+str.str()).c_str(),_tau,0,2*_tau); 
    tau->setConstant(kTRUE);

    RooAddition*  splusb = 
      new RooAddition(("splusb"+str.str()).c_str(),("s"+str.str()+"+"+"b"+str.str()).c_str(),   
		      RooArgSet(*s,*b)); 
    RooProduct*   bTau = 
      new RooProduct(("bTau"+str.str()).c_str(),("b*tau"+str.str()).c_str(),   RooArgSet(*b, *tau)); 
    RooRealVar*   x = 
      new RooRealVar(("x"+str.str()).c_str(),("x"+str.str()).c_str(),  sig[i]+back[i], 0., 1.2*sig[i]+back[i]+MaxSigma*sqrt(sig[i]+back[i]));
    RooRealVar*   y = 
      new RooRealVar(("y"+str.str()).c_str(),("y"+str.str()).c_str(),  back[i]*_tau,  0., 1.2*back[i]*_tau+MaxSigma*sqrt(back[i]*_tau));


    RooPoisson* sigRegion = 
      new RooPoisson(("sigRegion"+str.str()).c_str(),("sigRegion"+str.str()).c_str(), *x,*splusb);
    RooPoisson* sideband = 
      new RooPoisson(("sideband"+str.str()).c_str(),("sideband"+str.str()).c_str(), *y,*bTau);

    sigVec.push_back(s);
    backVec.push_back(b);
    tauVec.push_back(tau);
    xVec.push_back(x);
    yVec.push_back(y);
    sigRegions.push_back(sigRegion);
    sidebands.push_back(sideband);

    likelihoodFactors.Add(sigRegion);
    likelihoodFactors.Add(sideband);
    observablesCollection.Add(x);
    observablesCollection.Add(y);
    
    // print to see range on variables
    //    x->Print();
    //    y->Print();
    //    b->Print();


    xForTree[i] = sig[i]+back[i];
    yForTree[i] = back[i]*_tau;
    tree->Branch(("x"+str.str()).c_str(), xForTree+i ,("x"+str.str()+"/D").c_str());
    tree->Branch(("y"+str.str()).c_str(), yForTree+i ,("y"+str.str()+"/D").c_str());
  }
  tree->Fill();
  //  tree->Print();
  //  tree->Scan();

  RooArgSet likelihoodFactorSet(likelihoodFactors);
  RooProdPdf joint("joint","joint", likelihoodFactorSet );
  //  likelihoodFactorSet.Print();

  //  cout << "\n print model" << endl;
  //  joint.Print();
  //  joint.printCompactTree();

  //  RooArgSet* observableSet = new RooArgSet(observablesCollection);
  RooArgList* observableList = new RooArgList(observablesCollection);

  //  observableSet->Print();
  //  observableList->Print();

  //  cout << "Make hypothetical dataset:" << endl;
  RooDataSet* toyMC = new RooDataSet("data","data", tree, *observableList); // one experiment
  toyMC->Scan();


  // import hypothetical data
  fWS->import(*toyMC);
  // import joint PDF
  fWS->import(joint);


}

//_______________________________________________________
RooWorkspace* NumberCountingPdfFactory::GetWorkspace() const {
  // returns workspace
  return fWS;
}

//_______________________________________________________
RooAbsPdf* NumberCountingPdfFactory::GetPdf() const {
  // returns pdf
  return fWS->pdf("joint");
}

//_______________________________________________________
RooAbsData* NumberCountingPdfFactory::GetData() const {
  // returns dataset
  return fWS->data("data");
}

