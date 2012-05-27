
// A set of utils for navegating HistFactory models
#include <stdexcept>    

#include "RooStats/HistFactory/ParamHistFunc.h"
#include "TIterator.h"
#include "RooAbsArg.h"
#include "RooAbsPdf.h"
#include "RooArgSet.h"
#include "RooArgList.h"
#include "RooSimultaneous.h"
#include "RooCategory.h"
#include "RooRealVar.h"
#include "TH1.h"

#include "RooStats/HistFactory/HistFactoryModelUtils.h"

namespace RooStats{


  std::string channelNameFromPdf( RooAbsPdf* channelPdf ) {
    std::string channelPdfName = channelPdf->GetName();
    std::string ChannelName = channelPdfName.substr(6, channelPdfName.size() );
    return ChannelName;
  }

  RooAbsPdf* getSumPdfFromChannel( RooAbsPdf* sim_channel ) {

    std::cout << "Getting the RooRealSumPdf for the channel: " 
	      << sim_channel->GetName() << std::endl;

    std::string channelPdfName = sim_channel->GetName();
    std::string ChannelName = channelPdfName.substr(6, channelPdfName.size() );

    // Now, get the RooRealSumPdf
    // ie the channel WITHOUT constraints
    std::string realSumPdfName = ChannelName + "_model";

    RooAbsPdf* sum_pdf = NULL;        
    TIterator* iter_sum_pdf = sim_channel->getComponents()->createIterator(); //serverIterator();
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
      std::cout << "Failed to find RooRealSumPdf for channel: " << sim_channel->GetName() << std::endl;
      sim_channel->getComponents()->Print("V");
      throw std::runtime_error("Failed to find RooRealSumPdf for channel");
    } 
    else {
      std::cout << "Found RooRealSumPdf: " << sum_pdf->GetName() << std::endl;
    }
    delete iter_sum_pdf;
    iter_sum_pdf = NULL;

    return sum_pdf;

  }

  void getChannelsFromModel( RooAbsPdf* model, RooArgSet* channels, RooArgSet* channelsWithConstraints ) {

    // Loop through the model
    // Find all channels

    std::string modelClassName = model->ClassName();

    if( modelClassName == std::string("RooSimultaneous") ) {

      TIterator* simServerItr = model->serverIterator();

      // Loop through the child nodes of the sim pdf
      // and find the channel nodes
      RooAbsArg* sim_channel = NULL;
      while(( sim_channel = (RooAbsArg*) simServerItr->Next() )) {

	// Ignore the Channel Cat
	std::string channelPdfName = sim_channel->GetName();
	std::string channelClassName = sim_channel->ClassName();
	if( channelClassName == std::string("RooCategory") ) continue;

	// If we got here, we found a channel.
	// Format is model_<ChannelName>

	std::string ChannelName = channelPdfName.substr(6, channelPdfName.size() );

	// Now, get the RooRealSumPdf
	// ie the channel WITHOUT constraints

	std::string realSumPdfName = ChannelName + "_model";

	RooAbsPdf* sum_pdf = NULL;        
	TIterator* iter_sum_pdf = sim_channel->getComponents()->createIterator(); //serverIterator();
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
	  std::cout << "Failed to find RooRealSumPdf for channel: " << sim_channel->GetName() << std::endl;
	  sim_channel->getComponents()->Print("V");
	  throw std::runtime_error("Failed to find RooRealSumPdf for channel");
	}                                                  
	delete iter_sum_pdf;
	iter_sum_pdf = NULL;

	// Okay, now add to the arg sets
	channels->add( *sum_pdf );
	channelsWithConstraints->add( *sim_channel );

      }

      delete simServerItr;

    }
    else {
      std::cout << "Not Yet Implemented" << std::endl;
    }

  }


  bool getStatUncertaintyFromChannel( RooAbsPdf* channel, ParamHistFunc*& paramfunc, RooArgList* gammaList ) {

    // Find the servers of this channel
    //TIterator* iter = channel->serverIterator();
    TIterator* iter = channel->getComponents()->createIterator(); //serverIterator();
    bool FoundParamHistFunc=false;
    RooAbsArg* paramfunc_arg = NULL;        
    while(( paramfunc_arg = (RooAbsArg*) iter->Next() )) {
   
 
      std::string NodeName = paramfunc_arg->GetName();
      std::string NodeClassName = paramfunc_arg->ClassName();
      if( NodeClassName != std::string("ParamHistFunc") ) continue;
      if( NodeName.find("mc_stat_") != std::string::npos ) {
	FoundParamHistFunc=true;
	paramfunc = (ParamHistFunc*) paramfunc_arg;
	break;
      }
    }
    if( ! FoundParamHistFunc || !paramfunc ) {
      std::cout << "Failed to find ParamHistFunc for channel: " << channel->GetName() << std::endl;
      return false;
    }                                               
    
    delete iter;
    iter = NULL;

    // Now, get the set of gamma's
    gammaList = (RooArgList*) &( paramfunc->paramList());
    gammaList->Print("V");

    return true;

  }
  

  void getDataValuesForObservables( std::map< std::string, std::vector<double> >& ChannelBinDataMap,
				    RooAbsData* data, RooAbsPdf* pdf ) {

    //std::map< std::string, std::vector<int>  ChannelBinDataMap;

    RooSimultaneous* simPdf = (RooSimultaneous*) pdf;

    // get category label
    RooArgSet* allobs = (RooArgSet*) data->get();
    TIterator* obsIter = allobs->createIterator();
    RooCategory* cat = NULL;
    RooAbsArg* temp = NULL;
    while( (temp=(RooAbsArg*) obsIter->Next())) {
      // use dynamic cast here instead
      if( strcmp(temp->ClassName(),"RooCategory")==0){
	cat = (RooCategory*) temp;
      }
    }

    if(!cat) 
      std::cout <<"didn't find category"<< std::endl;
    else 
      std::cout <<"found category"<< std::endl;

    // split dataset
    TList* dataByCategory = data->split(*cat);
    dataByCategory->Print();
    // note :
    // RooAbsData* dataForChan = (RooAbsData*) dataByCategory->FindObject("");

    // loop over channels
    RooCategory* channelCat = (RooCategory*) (&simPdf->indexCat());
    TIterator* iter = channelCat->typeIterator() ;
    RooCatType* tt = NULL;
    while((tt=(RooCatType*) iter->Next())) {

      // Get pdf associated with state from simpdf
      RooAbsPdf* pdftmp = simPdf->getPdf(tt->GetName()) ;

      std::string ChannelName = pdftmp->GetName(); //tt->GetName();
      std::cout << "Getting data for channel: " << ChannelName << std::endl;
      ChannelBinDataMap[ ChannelName ] = std::vector<double>();

      RooAbsData* dataForChan = (RooAbsData*) dataByCategory->FindObject(tt->GetName());
      dataForChan->Print();

      // Generate observables defined by the pdf associated with this state
      RooArgSet* obstmp = pdftmp->getObservables(*dataForChan->get()) ;   
      RooRealVar* obs = ((RooRealVar*)obstmp->first());
      obs->Print();

      //double expected = pdftmp->expectedEvents(*obstmp);

      // set value to desired value (this is just an example)
      // double obsVal = obs->getVal();
      // set obs to desired value of observable
      // obs->setVal( obsVal );
      //double fracAtObsValue = pdftmp->getVal(*obstmp);

      // get num events expected in bin for obsVal
      // double nu = expected * fracAtObsValue;

      // an easier way to get n
      TH1* histForN = dataForChan->createHistogram("HhstForN",*obs);
      for(int i=1; i<=histForN->GetNbinsX(); ++i){
	double n = histForN->GetBinContent(i);
	cout << "n" <<  i << " = " << n  << std::endl;
	ChannelBinDataMap[ ChannelName ].push_back( n ); 
      }

    
    } // End Loop Over Categories

    return;

  }


  int getStatUncertaintyConstraintTerm( RooArgList* constraints, RooRealVar* gamma_stat, 
					RooAbsReal*& pois_nom, RooRealVar*& tau ) {
    // Given a set of constraint terms, 
    // find the poisson constraint for the 
    // given gamma and return the mean
    // as well as the 'tau' parameter


    // To get the constraint term, loop over all constraint terms
    // and look for the gamma_stat name as well as '_constraint'
    std::string constraintTermName = std::string(gamma_stat->GetName()) + "_constraint";
    TIterator* iter_list = constraints->createIterator();
    RooAbsArg* term_constr=NULL;
    bool FoundConstraintTerm=false;
    RooAbsPdf* constraintTerm=NULL;
    while((term_constr=(RooAbsArg*)iter_list->Next())) {
      std::string TermName = term_constr->GetName();
      // std::cout << "Checking if is a constraint term: " << TermName << std::endl;
      
      //if( TermName.find(gamma_stat->GetName())!=string::npos ) {
      if( term_constr->dependsOn( *gamma_stat) ) {
	if( TermName.find("_constraint")!=string::npos ) {
	  FoundConstraintTerm=true;
	  constraintTerm = (RooAbsPdf*) term_constr;
	  break;
	}
      }
    }
    if( FoundConstraintTerm==false ) {
      std::cout << "Error: Couldn't find constraint term for parameter: " << gamma_stat->GetName() << std::endl;
      throw std::runtime_error("Failed to find Gamma ConstraintTerm");
      return -1;
    }

    /*
    RooAbsPdf* constraintTerm = (RooAbsPdf*) constraints->find( constraintTermName.c_str() );
    if( constraintTerm == NULL ) {
      std::cout << "Error: Couldn't find constraint term: " << constraintTermName
		<< " for parameter: " << gamma_stat->GetName()
		<< std::endl;
      throw std::runtime_error("Failed to find Gamma ConstraintTerm");
      return -1;
    }
    */

    // Find the "data" of the poisson term
    // This is the nominal value
    bool FoundNomMean=false;
    TIterator* iter_pois = constraintTerm->serverIterator(); //constraint_args
    RooAbsArg* term_pois ;
    while((term_pois=(RooAbsArg*)iter_pois->Next())) {
      std::string serverName = term_pois->GetName();
      //std::cout << "Checking Server: " << serverName << std::endl;
      if( serverName.find("nom_")!=string::npos ) {
	FoundNomMean = true;  
	pois_nom = (RooRealVar*) term_pois;
      }
    }
    if( !FoundNomMean || !pois_nom ) {
      std::cout << "Error: Did not find Nominal Pois Mean parameter in gamma constraint term PoissonMean: "
		<< constraintTerm->GetName() << std::endl;
      throw std::runtime_error("Failed to find Nom Pois Mean");
    }
    else {
      std::cout << "Found Poisson 'data' term: " << pois_nom->GetName() << std::endl;
    }

    // Taking the constraint term (a Poisson), find
    // the "mean" which is the product: gamma*tau
    // Then, from that mean, find tau
    TIterator* iter_constr = constraintTerm->serverIterator(); //constraint_args
    RooAbsArg* pois_mean_arg=NULL;
    bool FoundPoissonMean = false;
    while(( pois_mean_arg = (RooAbsArg*) iter_constr->Next() )) {
      std::string serverName = pois_mean_arg->GetName();
      if( pois_mean_arg->dependsOn( *gamma_stat ) ) {
	FoundPoissonMean=true;
	// pois_mean = (RooAbsReal*) pois_mean_arg;
	break;
      }
    }
    if( !FoundPoissonMean || !pois_mean_arg ) {
      std::cout << "Error: Did not find PoissonMean parameter in gamma constraint term: "
		<< constraintTermName << std::endl;
      throw std::runtime_error("Failed to find PoissonMean");
      return -1;
    }
    else {
      std::cout << "Found Poisson 'mean' term: " << pois_mean_arg->GetName() << std::endl;
    }

    TIterator* iter_product = pois_mean_arg->serverIterator(); //constraint_args
    RooAbsArg* term_in_product ;
    bool FoundTau=false;
    while((term_in_product=(RooAbsArg*)iter_product->Next())) {
      std::string serverName = term_in_product->GetName();
      //std::cout << "Checking Server: " << serverName << std::endl;
      if( serverName.find("_tau")!=string::npos ) {
	FoundTau = true;  
	tau = (RooRealVar*) term_in_product;
      }
    }
    if( !FoundTau || !tau ) {
      std::cout << "Error: Did not find Tau parameter in gamma constraint term PoissonMean: "
		<< pois_mean_arg->GetName() << std::endl;
      throw std::runtime_error("Failed to find Tau");
    }
    else {
      std::cout << "Found Poisson 'tau' term: " << tau->GetName() << std::endl;
    }
    

    return 0;

  }



} // close RooStats namespace
