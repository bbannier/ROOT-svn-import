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


ClassImp(RooStats::NumberCountingPdfFactory) ;


// A factory for building PDFs and data for a number counting combination.  
// The factory produces a PDF for N channels with uncorrelated background 
// uncertainty.  Correlations can be added by extending this PDF with additional terms.
// The factory relates the signal in each channel to a master signal strength times the 
// expected signal in each channel.  Thus, the final test is performed on the master signal strength.
// This yields a more powerful test than letting signal in each channel be independent.
// One can incorporate uncertainty on the expected signal by adding additional terms.
//
// For the future, perhaps this factory should be extended to include the efficiency terms automatically.

using namespace RooStats;


//_______________________________________________________
NumberCountingPdfFactory::NumberCountingPdfFactory() {
  // constructor

}

//_______________________________________________________
NumberCountingPdfFactory::~NumberCountingPdfFactory(){
  // destructor
}


//_______________________________________________________
RooWorkspace* NumberCountingPdfFactory::GetExpWS(Double_t* sig, 
					Double_t* back, 
					Double_t* back_syst, 
					Int_t nbins) {

  // Returns a workspace with data, pdf, and variables for a number counting combination

  using namespace RooFit;
  using std::vector;

  // make a new Workspace
  RooWorkspace* ws = new RooWorkspace("NumberCountingWS","Number Counting WS");

  // add the PDF to the WS (common to both usages) 
  AddPdf(sig, back, back_syst, nbins, ws) ;

  //add the data to the WS.  in this case arguments are an expected signal & bkg.
  AddExpData(sig, back, back_syst, nbins, ws) ;

  return ws;
}


//_______________________________________________________
void NumberCountingPdfFactory::AddPdf(Double_t* sig, 
				 Double_t* back, 
				 Double_t* back_syst, 
				 Int_t nbins, 
				 RooWorkspace* ws) {
  

// This method produces a PDF for N channels with uncorrelated background 
// uncertainty. It relates the signal in each channel to a master signal strength times the 
// expected signal in each channel.
//
// For the future, perhaps this method should be extended to include the efficiency terms automatically.

  using namespace RooFit;
  using std::vector;

  TList likelihoodFactors;

  Double_t MaxSigma = 8; // Needed to set ranges for varaibles.

  RooRealVar*   masterSignal = 
    new RooRealVar("masterSignal","masterSignal",1., 0., 3.);


  // loop over individual channels
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

    likelihoodFactors.Add(sigRegion);
    likelihoodFactors.Add(sideband);
    
  }

  RooArgSet likelihoodFactorSet(likelihoodFactors);
  RooProdPdf joint("joint","joint", likelihoodFactorSet );

  // add this PDF to workspace.  
  // Need to do import into workspace now to get all the structure imported as well.
  // Just returning the WS will loose the rest of the structure b/c it will go out of scope
  ws->import(joint);
}

//_______________________________________________________
void NumberCountingPdfFactory::AddExpData(Double_t* sig, 
					  Double_t* back, 
					  Double_t* back_syst, 
					  Int_t nbins, 
					  RooWorkspace* ws) {

  // A number counting combination for N channels with uncorrelated background 
  // uncertainty.  Correlations can be added by extending this PDF with additional terms.
  // Arguements are an array of expected signal, expected background, and relative 
  // background uncertainty (eg. 0.1 for 10% uncertainty), and the number of channels.

  using namespace RooFit;
  using std::vector;

  TList observablesCollection;

  TTree* tree = new TTree();
  Double_t* xForTree = new Double_t[nbins];
  Double_t* yForTree = new Double_t[nbins];

  // loop over channels
  for(Int_t i=0; i<nbins; ++i){
    std::stringstream str;
    str<<"_"<<i;

    Double_t _tau = 1./back[i]/back_syst[i]/back_syst[i];

    RooRealVar*   x = 
      new RooRealVar(("x"+str.str()).c_str(),("x"+str.str()).c_str(),  sig[i]+back[i], 0., 2.*(sig[i]+back[i]));
    RooRealVar*   y = 
      new RooRealVar(("y"+str.str()).c_str(),("y"+str.str()).c_str(),  back[i]*_tau,  0., 2.*back[i]*_tau);

    observablesCollection.Add(x);
    observablesCollection.Add(y);
    
    xForTree[i] = sig[i]+back[i];
    yForTree[i] = back[i]*_tau;
    tree->Branch(("x"+str.str()).c_str(), xForTree+i ,("x"+str.str()+"/D").c_str());
    tree->Branch(("y"+str.str()).c_str(), yForTree+i ,("y"+str.str()+"/D").c_str());
  }
  tree->Fill();
  //  tree->Print();
  //  tree->Scan();

  RooArgList* observableList = new RooArgList(observablesCollection);

  //  observableSet->Print();
  //  observableList->Print();

  RooDataSet* data = new RooDataSet("NumberCountingData","Number Counting Data", tree, *observableList); // one experiment
  data->Scan();


  // import hypothetical data
  ws->import(*data);

}

