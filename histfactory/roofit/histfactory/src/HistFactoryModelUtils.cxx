
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


  RooAbsPdf* getSumPdfFromChannel( RooAbsPdf* sim_channel ) {

    std::string channelPdfName = sim_channel->GetName();
    std::string ChannelName = channelPdfName.substr(6, channelPdfName.size() );
    std::cout << sim_channel->GetName() << " " << ChannelName << " " << sim_channel->ClassName() << std::endl;

    // Now, get the RooRealSumPdf
    // ie the channel WITHOUT constraints
    std::string realSumPdfName = ChannelName + "_model";

    RooAbsPdf* sum_pdf = NULL;        
    TIterator* iter_sum_pdf = sim_channel->serverIterator();
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
    delete iter_sum_pdf;
    iter_sum_pdf = NULL;

    return sum_pdf;

  }

  void getChannelsFromModel( RooAbsPdf* model, RooArgSet* channels, RooArgSet* channelsWithConstraints ) {

    // Loop through the model
    // Find all channels

    std::cout << "Getting channels from Model: " << model->GetName() << std::endl;

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
	std::cout << sim_channel->GetName() << " " << ChannelName << " " << sim_channel->ClassName() << std::endl;

	// Now, get the RooRealSumPdf
	// ie the channel WITHOUT constraints

	std::string realSumPdfName = ChannelName + "_model";

	RooAbsPdf* sum_pdf = NULL;        
	TIterator* iter_sum_pdf = sim_channel->serverIterator();
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
	delete iter_sum_pdf;
	iter_sum_pdf = NULL;

	std::cout << sum_pdf->GetName() << std::endl;


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
    
      std::string NodeClassName = paramfunc_arg->ClassName();
      if( NodeClassName == std::string("ParamHistFunc") ) {
	FoundParamHistFunc=true;
	paramfunc = (ParamHistFunc*) paramfunc_arg;
	break;
      }
    }
    if( ! FoundParamHistFunc || !paramfunc ) {
      std::cout << "Failed to find ParamHistFunc for channel: " << channel->GetName() << std::endl;
      return false;
      //throw std::runtime_error("Failed to find RooRealSumPdf for channel");
    }                                               
    
    delete iter;
    iter = NULL;

    std::cout << "ParamHistFunc name: " << paramfunc->GetName() << std::endl;

    // Now, get the set of gamma's
    gammaList = (RooArgList*) &( paramfunc->paramList());

    gammaList->Print("V");

    return true;

  }
  

  void getDataValuesForObservables( std::map< std::string, std::vector<double> >& ChannelBinDataMap,
				    RooAbsData* data, RooAbsPdf* pdf ) {

    //std::map< std::string, std::vector<int>  ChannelBinDataMap;

    std::cout << "Entering: getDataValuesForObservables" << std::endl;

    RooSimultaneous* simPdf = (RooSimultaneous*) pdf;

    std::cout << "Getting Category Labels" << std::endl;

    // get category label
    std::cout << "Got here 0" << std::endl;
    std::cout << "Data: " << data << std::endl;
    RooArgSet* allobs = (RooArgSet*) data->get();
    std::cout << "Got here 1" << std::endl;
    TIterator* obsIter = allobs->createIterator();
    std::cout << "Got here 2" << std::endl;
    RooCategory* cat = NULL;
    RooAbsArg* temp = NULL;
    std::cout << "Got here 3" << std::endl;
    while( (temp=(RooAbsArg*) obsIter->Next())) {
      // use dynamic cast here instead
      if( strcmp(temp->ClassName(),"RooCategory")==0){
	cat = (RooCategory*) temp;
      }
    }
    std::cout << "Got here 4" << std::endl;

    if(!cat) 
      std::cout <<"didn't find category"<< std::endl;
    else 
      std::cout <<"found category"<< std::endl;

    std::cout << "Splitting Dataset" << std::endl;

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

      std::string ChannelName = tt->GetName();
      std::cout <<"processing channel " << tt->GetName() << std::endl;
      ChannelBinDataMap[ ChannelName ] = std::vector<double>();

      // Get pdf associated with state from simpdf
      RooAbsPdf* pdftmp = simPdf->getPdf(tt->GetName()) ;

      RooAbsData* dataForChan = (RooAbsData*) dataByCategory->FindObject(tt->GetName());
      dataForChan->Print();

      // Generate observables defined by the pdf associated with this state
      RooArgSet* obstmp = pdftmp->getObservables(*dataForChan->get()) ;   
      RooRealVar* obs = ((RooRealVar*)obstmp->first());
      obs->Print();

      double expected = pdftmp->expectedEvents(*obstmp);
      std::cout <<"expected = " <<expected<< std::endl;

      // set value to desired value (this is just an example)
      double obsVal = obs->getVal();
      std::cout <<"desired x = " << obsVal << std::endl;
      // set obs to desired value of observable
      obs->setVal( obsVal );
      double fracAtObsValue = pdftmp->getVal(*obstmp);
      std::cout <<"fracAtObsValue = " << fracAtObsValue << endl;

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


} // close RooStats namespace
