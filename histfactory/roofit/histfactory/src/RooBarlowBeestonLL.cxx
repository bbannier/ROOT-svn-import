 /***************************************************************************** 
  * Project: RooFit                                                           * 
  *                                                                           * 
  * Copyright (c) 2000-2005, Regents of the University of California          * 
  *                          and Stanford University. All rights reserved.    * 
  *                                                                           * 
  * Redistribution and use in source and binary forms,                        * 
  * with or without modification, are permitted according to the terms        * 
  * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             * 
  *****************************************************************************/ 

//////////////////////////////////////////////////////////////////////////////
//
// BEGIN_HTML
// Class RooBarlowBeestonLL implements the profile likelihood estimator for
// a given likelihood and set of parameters of interest. The value return by 
// RooBarlowBeestonLL is the input likelihood nll minimized w.r.t all nuisance parameters
// (which are all parameters except for those listed in the constructor) minus
// the -log(L) of the best fit. Note that this function is slow to evaluate
// as a MIGRAD minimization step is executed for each function evaluation
// END_HTML
//

#include <stdexcept>

#include "Riostream.h" 

#include "RooFit.h"
#include "RooBarlowBeestonLL.h" 
#include "RooAbsReal.h" 
//#include "RooMinuit.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooMsgService.h"
#include "RooNLLVar.h"


#include "RooStats/RooStatsUtils.h"
#include "RooProdPdf.h"
#include "RooSimultaneous.h"
#include "RooArgList.h"
#include "RooAbsCategoryLValue.h"

#include "RooStats/HistFactory/ParamHistFunc.h"
#include "RooStats/HistFactory/HistFactoryModelUtils.h"

using namespace std ;

ClassImp(RooBarlowBeestonLL) 


//_____________________________________________________________________________ 
 RooBarlowBeestonLL::RooBarlowBeestonLL() : 
   RooAbsReal("RooBarlowBeestonLL","RooBarlowBeestonLL"), 
   _nll(), 
   _obs("paramOfInterest","Parameters of interest",this), 
   _par("nuisanceParam","Nuisance parameters",this,kFALSE,kFALSE)
{ 
  // Default constructor 
  // Should only be used by proof. 
  _piter = _par.createIterator() ; 
  _oiter = _obs.createIterator() ; 
} 


//_____________________________________________________________________________
RooBarlowBeestonLL::RooBarlowBeestonLL(const char *name, const char *title, 
			   RooAbsReal& nllIn, const RooArgSet& observables) :
  RooAbsReal(name,title), 
  _nll("input","-log(L) function",this,nllIn),
  _obs("paramOfInterest","Parameters of interest",this),
  _par("nuisanceParam","Nuisance parameters",this,kFALSE,kFALSE)
{ 
  // Constructor of profile likelihood given input likelihood nll w.r.t
  // the given set of variables. The input log likelihood is minimized w.r.t
  // to all other variables of the likelihood at each evaluation and the
  // value of the global log likelihood minimum is always subtracted.

  // Determine actual parameters and observables
  RooArgSet* actualObs = nllIn.getObservables(observables) ;
  RooArgSet* actualPars = nllIn.getParameters(observables) ;

  _obs.add(*actualObs) ;
  _par.add(*actualPars) ;

  delete actualObs ;
  delete actualPars ;

  _piter = _par.createIterator() ;
  _oiter = _obs.createIterator() ;
} 



//_____________________________________________________________________________
RooBarlowBeestonLL::RooBarlowBeestonLL(const RooBarlowBeestonLL& other, const char* name) :  
  RooAbsReal(other,name), 
  _nll("nll",this,other._nll),
  _obs("obs",this,other._obs),
  _par("par",this,other._par),
  _paramFixed(other._paramFixed)
{ 
  // Copy constructor

  _piter = _par.createIterator() ;
  _oiter = _obs.createIterator() ;

  // _paramAbsMin.addClone(other._paramAbsMin) ;
  // _obsAbsMin.addClone(other._obsAbsMin) ;
    
} 



//_____________________________________________________________________________
RooBarlowBeestonLL::~RooBarlowBeestonLL()
{
  // Destructor

  // Delete instance of minuit if it was ever instantiated
  // if (_minuit) {
  //   delete _minuit ;
  // }

  delete _piter ;
  delete _oiter ;
}


//_____________________________________________________________________________
RooArgSet* RooBarlowBeestonLL::getParameters(const RooArgSet* depList, Bool_t stripDisconnected) const {

  RooArgSet* allArgs = RooAbsArg::getParameters( depList, stripDisconnected );

  TIterator* iter_args = allArgs->createIterator();
  RooRealVar* arg;
  while((arg=(RooRealVar*)iter_args->Next())) {
    std::string arg_name = arg->GetName();

    // If there is a gamma in the name,
    // strip it from the list of dependencies

    if( arg_name.find("gamma_stat")!=string::npos ) {
      allArgs->remove( *arg, kTRUE );
    }

  }

  return allArgs;

}


/*
//_____________________________________________________________________________
const RooArgSet& RooBarlowBeestonLL::bestFitParams() const 
{
  validateAbsMin() ;
  return _paramAbsMin ;
}


//_____________________________________________________________________________
const RooArgSet& RooBarlowBeestonLL::bestFitObs() const 
{
  validateAbsMin() ;
  return _obsAbsMin ;
}
*/



//_____________________________________________________________________________
RooAbsReal* RooBarlowBeestonLL::createProfile(const RooArgSet& paramsOfInterest) 
{
  // Optimized implementation of createProfile for profile likelihoods.
  // Return profile of original function in terms of stated parameters 
  // of interest rather than profiling recursively.

  return nll().createProfile(paramsOfInterest) ;
}



/*
void RooBarlowBeestonLL::FactorizePdf(const RooArgSet &observables, RooAbsPdf &pdf, RooArgList &obsTerms, RooArgList &constraints) const {
  // utility function to factorize constraint terms from a pdf
  // (from G. Petrucciani)
  const std::type_info & id = typeid(pdf);
  if (id == typeid(RooProdPdf)) {
    RooProdPdf *prod = dynamic_cast<RooProdPdf *>(&pdf);
    RooArgList list(prod->pdfList());
    for (int i = 0, n = list.getSize(); i < n; ++i) {
      RooAbsPdf *pdfi = (RooAbsPdf *) list.at(i);
      FactorizePdf(observables, *pdfi, obsTerms, constraints);
    }
  } else if (id == typeid(RooSimultaneous) ) {    //|| id == typeid(RooSimultaneousOpt)) {
    RooSimultaneous *sim  = dynamic_cast<RooSimultaneous *>(&pdf);
    RooAbsCategoryLValue *cat = (RooAbsCategoryLValue *) sim->indexCat().Clone();
    for (int ic = 0, nc = cat->numBins((const char *)0); ic < nc; ++ic) {
      cat->setBin(ic);
      FactorizePdf(observables, *sim->getPdf(cat->getLabel()), obsTerms, constraints);
    }
    delete cat;
  } else if (pdf.dependsOn(observables)) {
    if (!obsTerms.contains(pdf)) obsTerms.add(pdf);
  } else {
    if (!constraints.contains(pdf)) constraints.add(pdf);
  }
}
*/                                         







//_____________________________________________________________________________
Double_t RooBarlowBeestonLL::evaluate() const 
{ 
  // Evaluate profile likelihood by first minimizing all
  // binwise parameters analytically.  Then, return the 
  // result of a profile likelihood.

  // Save current value of observables and params
  RooArgSet* paramSetOrig = (RooArgSet*) _par.snapshot() ;

  //
  // Set the gamma parameters to their minima
  //
  RooArgList obsTerms, constraints;

  // Get the data from the nll
  // Get the "nll", which is really a RooAddPdf
  // which also includes constraint terms:
  RooAbsReal* nll_with_constr = (RooAbsReal*) &_nll.arg();
  
  // Now, we loop over the servers and find the NLL itself
  TIterator* nll_itr = nll_with_constr->serverIterator();
  bool FoundNLLVar=false;
  RooNLLVar* nll_var=NULL;
  RooAbsArg* nll_server=NULL;
  while(( nll_server = (RooAbsArg*) nll_itr->Next() )) {

    if( strcmp(nll_server->ClassName(), "RooNLLVar") == 0 ) {
      nll_var = (RooNLLVar*) nll_server;
      FoundNLLVar=true;
      break;
    }
  }
  if( ! FoundNLLVar ) {
    std::cout << "Error: Couldn't find NLLVar" << std::endl;
    throw std::runtime_error("Failed to find NLLVar");
  }
  

  // RooNLLVar* nll_var = (RooNLLVar*) (&(_nll.arg()) );
  // RooNLLVar* nll_var = new RooNLLVar( * ((RooNLLVar*) (&_nll.arg())), "TempNLL" );
  //const RooNLLVar& nll_var = dynamic_cast<const RooNLLVar&>( _nll.arg() );
  if( !nll_var ) {
    std::cout << "Error: Failed to get pdf from nll" << std::endl;
  }
  else {
    std::cout << "nll_var->ClassName(): " << nll_var->ClassName() << std::endl;
  }
  RooAbsData& data = nll_var->data();
  std::cout << "Getting Name of Data" << std::endl;
  std::cout << "Data Name: " << data.GetName() << std::endl;
  std::cout << "Address of data: " << &data << std::endl;
  
  // Get the value of the data per bin, per channel
  std::map< std::string, std::vector<double> > ChannelBinDataMap;
  RooStats::getDataValuesForObservables( ChannelBinDataMap, &data, _pdf );

  // _obs
  /*
  std::cout << "About to get pdf from nll" << std::endl;
  std::cout << "nll_var->function().ClassName(): " << nll_var->function().ClassName() << std::endl;
  RooAbsReal* pdf_absreal = (RooAbsReal*) &(nll_var->function());
  std::cout << "pdf_absreal.ClassName(): " << pdf_absreal->ClassName() << std::endl;
  */

  /*
  RooAbsPdf* pdf = (RooAbsPdf*) (& pdf_absreal); // &(nll_var->function()) );
  if( !pdf ) {
    std::cout << "Error: Failed to get pdf from nll" << std::endl;
  }
  */
  //RooAbsPdf* pdf = (RooAbsPdf*) &(_nll.arg().function());
  RooStats::FactorizePdf(*(_pdf->getObservables(data)), *_pdf, obsTerms, constraints);
  //constraints.Print("V");


  // Start with the top-level pdf
  // Loop over channels
  // Get the Stat-Uncertainty ParamHistFunc
  // Loop over Gamma's


  // Take the SimPdf, get the different
  // channels (with constraint terms attached)
  // and get the terms with the constraint 
  // terms removed (RooRealSumPdf)
  RooArgSet* channels = new RooArgSet();
  RooArgSet* channelsWithConstraints = new RooArgSet();
  std::cout << "getChannelsFromModel()" << std::endl;
  RooStats::getChannelsFromModel( _pdf, channels, channelsWithConstraints );
  std::cout << "Successfully did: getChannelsFromModel()" << std::endl;

  // Loop over channels
  TIterator* iter_channels = channelsWithConstraints->createIterator();
  RooAbsPdf* channelPdf=NULL;
  while(( channelPdf=(RooAbsPdf*)iter_channels->Next()  )) {

    std::cout << "Checking Channel: " << channelPdf->GetName() << std::endl;

    // First, we check if this channel uses Stat Uncertainties:
    RooArgList* gammas = new RooArgList();
    ParamHistFunc* param_func=NULL;
    std::cout << "About to check for StatUncertainties" << std::endl;
    bool hasStatUncert = RooStats::getStatUncertaintyFromChannel( channelPdf, param_func, gammas );
    std::cout << "Successfully checked for StatUncertainties" << std::endl;

    if( ! hasStatUncert ) {
      std::cout << "Channel: " << channelPdf->GetName() 
		<< " doesn't have statistical uncertainties"
		<< std::endl;
      continue;
    }
    else {
      std::cout << "Channel " << channelPdf->GetName() 
		<< " has statistical uncertainties" 
		<< std::endl;
    }
    
    // Loop over the bins of this channel
    // using the ParamHistFunc
    // and get the gamma associated with each bin
    std::cout << "Got here 0" << std::endl;
    std::cout << "ParamHistFunc pointer: " << param_func << std::endl;
    std::cout << "Found ParamHistFunc: " << param_func->GetName() << std::endl;
    std::cout << "Got here 1" << std::endl;

    std::cout << "About to get the number of bins" << std::endl;
    int num_bins = param_func->numBins();
    std::cout << "Got the number of bins" << std::endl;

    for( Int_t bin_itr = 0; bin_itr < num_bins; ++bin_itr ) {
      
      // Convert to TH1 style index
      int bin_index = bin_itr; // + 1;

      std::cout << "Checking bin: " << bin_index << std::endl;

      RooRealVar* gamma_stat = &(param_func->getParameter(bin_index));
      std::cout << "Found gamma for bin: " << bin_index 
		<< ": " << gamma_stat->GetName() 
		<< std::endl;
      std::string paramName = gamma_stat->GetName();
		
      // Get the bin center for this bin
      std::cout << "Getting Bin Center:" << std::endl;
      const RooArgSet* bin_center = param_func->get( bin_index );
      bin_center->Print("V");
      std::cout << "Got Bin Center" << std::endl;
      
      /* Now, get the gamma by quering the ParamHistFunc

      // Now, loop over the gamma's for this channel
      TIterator* iter_gamma = gammas->createIterator() ;
      RooRealVar* gamma_stat;
      while(( gamma_stat = (RooRealVar*) iter_gamma->Next() )) {
	std::string paramName = gamma_stat->GetName();
      */

      /*
      // First, get the gamma's within the parameters
      TIterator* iter_gamma = _par.createIterator() ;
      RooRealVar* gamma_stat ;
      while((gamma_stat=(RooRealVar*)iter_gamma->Next())) {
    std::string paramName = gamma_stat->GetName();
    if( paramName.find("gamma_stat")==string::npos ) continue;
    if( paramName.find("nom_")!=string::npos ) continue;
      */
      std::cout << "Found gamma: " << gamma_stat->GetName() << std::endl;
    
      // If the gamma parameter is constant, we continue on
      // We don't need to do anything if it's constant
      if( gamma_stat->isConstant() ) {
	std::cout << "Ignoring constant gamma: " << gamma_stat->GetName() << std::endl;
	continue;
      }
    
      // Get the constraint term:
      std::string constraintTermName = paramName + "_constraint";
      RooAbsPdf* constraintTerm = (RooAbsPdf*) constraints.find( constraintTermName.c_str() );
      if( constraintTerm == NULL ) {
	std::cout << "Error: Couldn't find constraint term: " << constraintTermName
		  << " for parameter: " << paramName
		  << std::endl;
	throw std::runtime_error("Failed to find Gamma ConstraintTerm");
      }

      std::cout << "Found gamma parameter: " << paramName
		<< " with constraint term: " << constraintTermName
		<< std::endl;
      
      // Taking the constraint term (a Poisson), find
      // the "mean" which is the product: gamma*tau
      // Then, from that mean, find tau
      TIterator* iter_constr = constraintTerm->serverIterator(); //constraint_args
      RooRealVar* pois_mean ;
      bool FoundPoissonMean = false;
      while((pois_mean=(RooRealVar*)iter_constr->Next())) {
	std::string serverName = pois_mean->GetName();
	std::cout << "Poisson Server Found: " << serverName << std::endl;
	if( pois_mean->dependsOn( *gamma_stat ) ) {
	  FoundPoissonMean=true;
	  break;
	}
      }
      if( !FoundPoissonMean ) {
	std::cout << "Error: Did not find PoissonMean parameter in gamma constraint term: "
		  << constraintTermName << std::endl;
	throw std::runtime_error("Failed to find PoissonMean");
      }

      TIterator* iter_tau = pois_mean->serverIterator(); //constraint_args
      RooRealVar* var_tau ;
      bool FoundTau = false;
      while((var_tau=(RooRealVar*)iter_tau->Next())) {
	std::string serverName = var_tau->GetName();
	std::cout << "Tau Server Found: " << serverName << std::endl;
	if( serverName.find("_tau")!=string::npos ) {
	  FoundTau = true;  
	  break;
	}
      }
      if( !FoundTau ) {
	std::cout << "Error: Did not find Tau parameter in gamma constraint term PoissonMean: "
		  << pois_mean->GetName() << std::endl;
	throw std::runtime_error("Failed to find Tau");
      }
      
      // If we got here, we successfully found the 
      // PoissonMean and the tau
      
      std::cout << "Found Poisson Mean: " << pois_mean->GetName() << std::endl;
      std::cout << "Found tau: " << var_tau->GetName() << std::endl;


	/*  We do this commented block using the HistFactoryModelUtils

	// Get the ParamHistFunc that this gamma is attached to
	// Start with the pdf and loop over servers
	ParamHistFunc* param_func = NULL;
	TIterator* iter_pdf = _pdf->getComponents()->createIterator(); //serverIterator(); 
	RooAbsReal* pdf_node=NULL;
	bool FoundParamHistFunc=false;
	while((pdf_node=(RooAbsReal*)iter_pdf->Next())) {
	
	// Check if it is a ParamHistFunc
	  std::string NodeClassName = pdf_node->ClassName();
	  if( NodeClassName != std::string("ParamHistFunc") ) continue;
	  
	  // Check if it serves the current gamma
	  // (There should only be one paramhistfunc per stat gamma)
	  if( pdf_node->getVariables()->find( *gamma_stat ) ) {
	    param_func = (ParamHistFunc*) pdf_node;
	    FoundParamHistFunc=true;
	    break;
	  }
	}
	if( FoundParamHistFunc==false ) {
	  std::cout << "Failed to find ParamHistFunc" << std::endl;
	  throw std::runtime_error("Failed to find ParamHistFunc");
	}
	
	std::cout << "Found ParamHistFunc: " << param_func->GetName() 
		  << " " << param_func->ClassName() << std::endl;
	
	
	// Get the pdf for this channel out of the simultaneous pdf
	//RooArgSet* sim_components = _pdf->getComponents();
	RooAbsPdf* chan_pdf = NULL;
	TIterator* iter_chan = _pdf->serverIterator();
	bool FoundChannelPdf=false;
	RooAbsArg* chan_arg=NULL;
	while((chan_arg=(RooAbsArg*)iter_chan->Next())) {
	  if( chan_arg->dependsOn( *gamma_stat ) ) {
	    FoundChannelPdf=true;
	    chan_pdf = (RooAbsPdf*) chan_arg;
	    break;
	  }
	}
	if( ! FoundChannelPdf ) {
	  std::cout << "Failed to find PDF for channel" << std::endl;
	  throw std::runtime_error("Failed to find PDF for channel");
	}
	
	std::cout << "Channel Pdf: " << chan_pdf->GetName() << " " 
		  << chan_pdf->ClassName() << std::endl;
	

	// Now we want the pdf without constraints
	RooAbsPdf* sum_pdf = NULL;
	TIterator* iter_sum_pdf = chan_pdf->serverIterator();
	bool FoundSumPdf=false;
	RooAbsArg* sum_pdf_arg=NULL;
	while((sum_pdf_arg=(RooAbsArg*)iter_sum_pdf->Next())) {
	  
	  std::string NodeClassName = sum_pdf_arg->ClassName();
	  if( NodeClassName == std::string("RooRealSumPdf") ) {
	    FoundSumPdf=true;
	    sum_pdf = (RooAbsPdf*) sum_pdf_arg;
	    break;
	  }
	}
	if( ! FoundSumPdf ) {
	  std::cout << "Failed to find RooRealSumPdf for channel" << std::endl;
	  throw std::runtime_error("Failed to find RooRealSumPdf for channel");
	}
	*/
      
      // Now, get the RooRealSumPdf for this channel
      RooAbsPdf* sum_pdf = RooStats::getSumPdfFromChannel( channelPdf );

      // Get the value of the function without the gamma:
      std::cout << "Getting the dependence of pdf: " << sum_pdf->GetName()
		<< " on gamma: " << gamma_stat->GetName()
		<< std::endl;
      
      gamma_stat->setVal(0.0);
      double val_gamma_0 = sum_pdf->getVal();
      
      gamma_stat->setVal(1.0);
      double val_gamma_1 = sum_pdf->getVal();
      
      std::cout << "pdf(gamma=0) = " << val_gamma_0 << " "
		<< "pdf(gamma=1) = " << val_gamma_1 
		<< std::endl;
      
      // For now, just set them to 1
      gamma_stat->setVal( 1.0 );
      
      std::cout << "Done with this gamma" << std::endl;

    } // END: Loop over GAMMAS
    
      std::cout << "Done with this channel" << std::endl;

  } // END: Loop over CHANNELS

  delete paramSetOrig ;

  std::cout << "Done" << std::endl;

  return _nll;

} 


/*
//_____________________________________________________________________________
void RooBarlowBeestonLL::validateAbsMin() const 
{
  // Check that parameters and likelihood value for 'best fit' are still valid. If not,
  // because the best fit has never been calculated, or because constant parameters have
  // changed value or parameters have changed const/float status, the minimum is recalculated

  // Check if constant status of any of the parameters have changed
  if (_absMinValid) {
    _piter->Reset() ;
    RooAbsArg* par ;
    while((par=(RooAbsArg*)_piter->Next())) {
      if (_paramFixed[par->GetName()] != par->isConstant()) {
	cxcoutI(Minimization) << "RooBarlowBeestonLL::evaluate(" << GetName() << ") constant status of parameter " << par->GetName() << " has changed from " 
				<< (_paramFixed[par->GetName()]?"fixed":"floating") << " to " << (par->isConstant()?"fixed":"floating") 
				<< ", recalculating absolute minimum" << endl ;
	_absMinValid = kFALSE ;
	break ;
      }
    }
  }


  // If we don't have the absolute minimum w.r.t all observables, calculate that first
  if (!_absMinValid) {

    cxcoutI(Minimization) << "RooBarlowBeestonLL::evaluate(" << GetName() << ") determining minimum likelihood for current configurations w.r.t all observable" << endl ;


    // Save current values of non-marginalized parameters
    RooArgSet* obsStart = (RooArgSet*) _obs.snapshot(kFALSE) ;

    // Start from previous global minimum 
    if (_paramAbsMin.getSize()>0) {
      const_cast<RooSetProxy&>(_par).assignValueOnly(_paramAbsMin) ;
    }
    if (_obsAbsMin.getSize()>0) {
      const_cast<RooSetProxy&>(_obs).assignValueOnly(_obsAbsMin) ;
    }

    // Find minimum with all observables floating
    const_cast<RooSetProxy&>(_obs).setAttribAll("Constant",kFALSE) ;  
    _minuit->migrad() ;

    // Save value and remember
    _absMin = _nll ;
    _absMinValid = kTRUE ;

    // Save parameter values at abs minimum as well
    _paramAbsMin.removeAll() ;

    // Only store non-constant parameters here!
    RooArgSet* tmp = (RooArgSet*) _par.selectByAttrib("Constant",kFALSE) ;
    _paramAbsMin.addClone(*tmp) ;
    delete tmp ;

    _obsAbsMin.addClone(_obs) ;

    // Save constant status of all parameters
    _piter->Reset() ;
    RooAbsArg* par ;
    while((par=(RooAbsArg*)_piter->Next())) {
      _paramFixed[par->GetName()] = par->isConstant() ;
    }
    
    if (dologI(Minimization)) {
      cxcoutI(Minimization) << "RooBarlowBeestonLL::evaluate(" << GetName() << ") minimum found at (" ;

      RooAbsReal* arg ;
      Bool_t first=kTRUE ;
      _oiter->Reset() ;
      while ((arg=(RooAbsReal*)_oiter->Next())) {
	ccxcoutI(Minimization) << (first?"":", ") << arg->GetName() << "=" << arg->getVal() ;	
	first=kFALSE ;
      }      
      ccxcoutI(Minimization) << ")" << endl ;            
    }

    // Restore original parameter values
    const_cast<RooSetProxy&>(_obs) = *obsStart ;
    delete obsStart ;

  }
}
*/


//_____________________________________________________________________________
Bool_t RooBarlowBeestonLL::redirectServersHook(const RooAbsCollection& /*newServerList*/, Bool_t /*mustReplaceAll*/, 
					 Bool_t /*nameChange*/, Bool_t /*isRecursive*/) 
{ 
  /*
  if (_minuit) {
    delete _minuit ;
    _minuit = 0 ;
  }
  */
  return kFALSE ;
} 


