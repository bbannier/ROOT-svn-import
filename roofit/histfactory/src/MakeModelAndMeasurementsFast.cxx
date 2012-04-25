// @(#)root/roostats:$Id:  cranmer $
// Author: Kyle Cranmer, Akira Shibata
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//_________________________________________________
/*
  BEGIN_HTML
  <p>
  This is a package that creates a RooFit probability density function from ROOT histograms 
  of expected distributions and histograms that represent the +/- 1 sigma variations 
  from systematic effects. The resulting probability density function can then be used
  with any of the statistical tools provided within RooStats, such as the profile 
  likelihood ratio, Feldman-Cousins, etc.  In this version, the model is directly
  fed to a likelihodo ratio test, but it needs to be further factorized.</p>

  <p>
  The user needs to provide histograms (in picobarns per bin) and configure the job
  with XML.  The configuration XML is defined in the file config/Config.dtd, but essentially
  it is organized as follows (see config/Combination.xml and config/ee.xml for examples)</p>

  <ul>
  <li> - a top level 'Combination' that is composed of:</li>
  <ul>
  <li>- several 'Channels' (eg. ee, emu, mumu), which are composed of:</li>
  <ul>
  <li>- several 'Samples' (eg. signal, bkg1, bkg2, ...), each of which has:</li>
  <ul>
  <li> - a name</li>
  <li> - if the sample is normalized by theory (eg N = L*sigma) or not (eg. data driven)</li>
  <li> - a nominal expectation histogram</li>
  <li> - a named 'Normalization Factor' (which can be fixed or allowed to float in a fit)</li>
  <li> - several 'Overall Systematics' in normalization with:</li>
  <ul>
  <li> - a name</li>
  <li> - +/- 1 sigma variations (eg. 1.05 and 0.95 for a 5% uncertainty)</li>
  </ul>
  <li>- several 'Histogram Systematics' in shape with:</li>
  <ul>
  <li>- a name (which can be shared with the OverallSyst if correlated)</li>
  <li>- +/- 1 sigma variational histograms</li>
  </ul>
  </ul>
  </ul>
  <li>- several 'Measurements' (corresponding to a full fit of the model) each of which specifies</li>
  <ul>
  <li>- a name for this fit to be used in tables and files</li>
  <ul>
  <li>      - what is the luminosity associated to the measurement in picobarns</li>
  <li>      - which bins of the histogram should be used</li>
  <li>      - what is the relative uncertainty on the luminosity </li>
  <li>      - what is (are) the parameter(s) of interest that will be measured</li>
  <li>      - which parameters should be fixed/floating (eg. nuisance parameters)</li>
  </ul>
  </ul>
  </ul>
  END_HTML
*/
//


// from std
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>

// from root
#include "TFile.h"
#include "TH1F.h"
#include "TDOMParser.h"
#include "TXMLAttr.h"
#include "TString.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLine.h"

// from roofit
#include "RooStats/ModelConfig.h"

// from this package
#include "Helper.h"
#include "RooStats/HistFactory/ConfigParser.h"
#include "RooStats/HistFactory/EstimateSummary.h"
#include "RooStats/HistFactory/Measurement.h"
#include "RooStats/HistFactory/HistoToWorkspaceFactoryFast.h"
//#include "RooStats/HistFactory/HistoToWorkspaceFactoryNew.h"
#include "RooStats/HistFactory/HistFactoryException.h"

#include "RooStats/HistFactory/MakeModelAndMeasurementsFast.h"

using namespace RooFit;
using namespace RooStats;
using namespace HistFactory;


/*
  void processMeasurement(string outputFileNamePrefix, 
  vector<string> xml_input, 
  TXMLNode* node,
  vector<string> preprocessFunctions);

  void processMeasurementXML(TXMLNode* node, 
  string& outputFileName, string outputFileNamePrefix, 
  Double_t& nominalLumi, Double_t& lumiRelError, Double_t& lumiError,
  Int_t& lowBin, Int_t& highBin,
  string& rowTitle, string& POI, string& mode,
  vector<string>& systToFix,
  map<string,double>& gammaSyst,
  map<string,double>& uniformSyst,
  map<string,double>& logNormSyst,
  map<string,double>& noSyst,
  bool& exportOnly
  );
*/


// Defined later in this file:



void fastDriver(string input){
  // TO DO:
  // would like to fully factorize the XML parsing.  
  // No clear need to have some here and some in ConfigParser


  // Make the list of measurements and channels
  std::vector< HistFactory::Measurement > measurement_list;
  std::vector< HistFactory::Channel >     channel_list;

  HistFactory::ConfigParser xmlParser;

  measurement_list = xmlParser.GetMeasurementsFromXML( input );

  // Fill them using the XML parser
  // xmlParser.FillMeasurementsAndChannelsFromXML( input, measurement_list, channel_list );

  // At this point, we have all the information we need
  // from the xml files.
  

  // We will make the measurements 1-by-1
  // This part will be migrated to the
  // MakeModelAndMeasurements function,
  // but is here for now.

  
  /* Now setup the measurement */
  // At this point, all we need
  // is the list of measurements
    
  for(unsigned int i = 0; i < measurement_list.size(); ++i) {

    HistFactory::Measurement measurement = measurement_list.at(i);

    measurement.CollectHistograms();

    MakeModelAndMeasurementFast( measurement );

  }

  return;

}

/*
std::vector<EstimateSummary> RooStats::HistFactory::GetChannelEstimateSummaries(RooStats::HistFactory::Measurement& measurement, RooStats::HistFactory::Channel& channel) {

  // Convert a "Channel" into a list of "Estimate Summaries"
  // This should only be a temporary function, as the
  // EstimateSummary class should be deprecated


  std::vector<EstimateSummary> channel_estimateSummary;

  std::cout << "Processing data: " << std::endl;

  // Add the data
  EstimateSummary data_es;
  data_es.name = "Data";
  data_es.channel = channel.GetName();
  data_es.nominal = (TH1*) channel.GetData().GetHisto()->Clone();
  channel_estimateSummary.push_back( data_es );

  // Add the samples
  for( unsigned int sampleItr = 0; sampleItr < channel.GetSamples().size(); ++sampleItr ) {

    EstimateSummary sample_es;
    RooStats::HistFactory::Sample& sample = channel.GetSamples().at( sampleItr );

    std::cout << "Processing sample: " << sample.GetName() << std::endl;

    // Define the mapping
    sample_es.name = sample.GetName();
    sample_es.channel = sample.GetChannelName();
    sample_es.nominal = (TH1*) sample.GetHisto()->Clone();

    std::cout << "Checking NormalizeByTheory" << std::endl;

    if( sample.GetNormalizeByTheory() ) {
      sample_es.normName = "" ; // Really bad, confusion convention
    }
    else {
      TString lumiStr;
      lumiStr += measurement.GetLumi();
      lumiStr.ReplaceAll(' ', TString());
      sample_es.normName = lumiStr ;
    }

    std::cout << "Setting the Histo Systs" << std::endl;

    // Set the Histo Systs:
    for( unsigned int histoItr = 0; histoItr < sample.GetHistoSysList().size(); ++histoItr ) {

      RooStats::HistFactory::HistoSys& histoSys = sample.GetHistoSysList().at( histoItr );

      sample_es.systSourceForHist.push_back( histoSys.GetName() );
      sample_es.lowHists.push_back( (TH1*) histoSys.GetHistoLow()->Clone()  );
      sample_es.highHists.push_back( (TH1*) histoSys.GetHistoHigh()->Clone() );

    }

    std::cout << "Setting the NormFactors" << std::endl;

    for( unsigned int normItr = 0; normItr < sample.GetNormFactorList().size(); ++normItr ) {

      RooStats::HistFactory::NormFactor& normFactor = sample.GetNormFactorList().at( normItr );

      EstimateSummary::NormFactor normFactor_es;
      normFactor_es.name = normFactor.GetName();
      normFactor_es.val  = normFactor.GetVal();
      normFactor_es.high = normFactor.GetHigh();
      normFactor_es.low  = normFactor.GetLow();
      normFactor_es.constant = normFactor.GetConst();
	  

      sample_es.normFactor.push_back( normFactor_es );

    }

    std::cout << "Setting the OverallSysList" << std::endl;

    for( unsigned int sysItr = 0; sysItr < sample.GetOverallSysList().size(); ++sysItr ) {

      RooStats::HistFactory::OverallSys& overallSys = sample.GetOverallSysList().at( sysItr );

      std::pair<double, double> DownUpPair( overallSys.GetLow(), overallSys.GetHigh() );
      sample_es.overallSyst[ overallSys.GetName() ]  = DownUpPair; //

    }

    std::cout << "Checking Stat Errors" << std::endl;

    // Do Stat Error
    sample_es.IncludeStatError  = sample.GetStatError().GetActivate();

    // Set the error and error threshold
    sample_es.RelErrorThreshold = channel.GetStatErrorConfig().GetRelErrorThreshold();
    if( sample.GetStatError().GetErrorHist() ) {
      sample_es.relStatError      = (TH1*) sample.GetStatError().GetErrorHist()->Clone();
    }
    else {
      sample_es.relStatError    = NULL;
    }


    // Set the constraint type;
    Constraint::Type type = channel.GetStatErrorConfig().GetConstraintType();

    // Set the default
    sample_es.StatConstraintType = EstimateSummary::Gaussian;

    if( type == Constraint::Gaussian) {
      std::cout << "Using Gaussian StatErrors" << std::endl;
      sample_es.StatConstraintType = EstimateSummary::Gaussian;
    }
    if( type == Constraint::Poisson ) {
      std::cout << "Using Poisson StatErrors" << std::endl;
      sample_es.StatConstraintType = EstimateSummary::Poisson;
    }


    std::cout << "Getting the shape Factor" << std::endl;

    // Get the shape factor
    if( sample.GetShapeFactorList().size() > 0 ) {
      sample_es.shapeFactorName = sample.GetShapeFactorList().at(0).GetName();
    }
    if( sample.GetShapeFactorList().size() > 1 ) {
      std::cout << "Error: Only One Shape Factor currently supported" << std::endl;
      throw bad_hf;
    }


    std::cout << "Setting the ShapeSysts" << std::endl;

    // Get the shape systs:
    for( unsigned int shapeItr=0; shapeItr < sample.GetShapeSysList().size(); ++shapeItr ) {

      RooStats::HistFactory::ShapeSys& shapeSys = sample.GetShapeSysList().at( shapeItr );

      EstimateSummary::ShapeSys shapeSys_es;
      shapeSys_es.name = shapeSys.GetName();
      shapeSys_es.hist = shapeSys.GetErrorHist();

      // Set the constraint type;
      Constraint::Type systype = shapeSys.GetConstraintType();

      // Set the default
      shapeSys_es.constraint = EstimateSummary::Gaussian;

      if( systype == Constraint::Gaussian) {
	shapeSys_es.constraint = EstimateSummary::Gaussian;
      }
      if( systype == Constraint::Poisson ) {
	shapeSys_es.constraint = EstimateSummary::Poisson;
      }

      sample_es.shapeSysts.push_back( shapeSys_es );

    }

    std::cout << "Adding this sample" << std::endl;

    // Push back
    channel_estimateSummary.push_back( sample_es );

  }

  return channel_estimateSummary;

}
*/

/*
void RooStats::HistFactory::ConfigureWorkspaceForMeasurement( const std::string& ModelName, RooWorkspace* ws_single, RooStats::HistFactory::Measurement& measurement ) {


  // First, turn the channel into a vector of estimate summaries
  //std::vector<EstimateSummary> channel_estimateSummary = GetChannelEstimateSummaries( measurement, channel );

  // Then, use HistFactory on that vector to create the workspace
  // RooWorkspace* ws_single = factory.MakeSingleChannelModel(channel_estimateSummary, measurement.GetConstantParams());


  // Create a Model config and do any necessary edits to the workspace

  // Make a ModelConfig and configure it
  ModelConfig * proto_config = (ModelConfig *) ws_single->obj("ModelConfig");
  cout << "Setting Parameter of Interest as :" << measurement.GetPOI() << endl;
  RooRealVar* poi = (RooRealVar*) ws_single->var( (measurement.GetPOI()).c_str() );
  RooArgSet * params= new RooArgSet;
  if(poi){
    params->add(*poi);
  }
  proto_config->SetParametersOfInterest(*params);

  // Activate Additional Constraint Terms
  if( measurement.GetGammaSyst().size()>0 || measurement.GetUniformSyst().size()>0 || measurement.GetLogNormSyst().size()>0 || measurement.GetNoSyst().size()>0) {
    //factory.EditSyst( ws_single, ("model_"+ch_name).c_str(), measurement.GetGammaSyst(), measurement.GetUniformSyst(), measurement.GetLogNormSyst(), measurement.GetNoSyst());
    HistoToWorkspaceFactoryFast::EditSyst( ws_single, (ModelName).c_str(), measurement.GetGammaSyst(), measurement.GetUniformSyst(), measurement.GetLogNormSyst(), measurement.GetNoSyst());
    std::string NewModelName = "newSimPdf"; // <- This name is hard-coded in HistoToWorkspaceFactoryFast::EditSyt.  Probably should be changed to : std::string("new") + ModelName;
    proto_config->SetPdf( *ws_single->pdf( "newSimPdf" ) );
  }
  
  // Set the ModelConfig's Params of Interest
  RooAbsData* expData = ws_single->data("asimovData");
  if(poi){
    proto_config->GuessObsAndNuisance(*expData);
  }

  // Cool, we're done
  return; // ws_single;
}
*/




RooWorkspace* RooStats::HistFactory::MakeModelAndMeasurementFast( RooStats::HistFactory::Measurement& measurement ) {
  
  /*
  // Add the channels to this measurement
  for( unsigned int chanItr = 0; chanItr < channel_list.size(); ++chanItr ) {
  measurement.channels.push_back( channel_list.at( chanItr ) );
  }
  */

  // This will be returned
  RooWorkspace* ws = NULL;

  try {


    std::cout << "Making Model and Measurements (Fast) for measurement: " << measurement.GetName() << std::endl;

    double lumiError = measurement.GetLumi()*measurement.GetLumiRelErr();

    std::cout << "using lumi = " << measurement.GetLumi() << " and lumiError = " << lumiError
	 << " including bins between " << measurement.GetBinLow() << " and " << measurement.GetBinHigh() << std::endl;
    std::cout << "fixing the following parameters:"  << std::endl;

    for(vector<string>::iterator itr=measurement.GetConstantParams().begin(); itr!=measurement.GetConstantParams().end(); ++itr){
      cout << "   " << *itr << endl;
    }

    std::string rowTitle = measurement.GetName();
    
    vector<RooWorkspace*> channel_workspaces;
    vector<string>        channel_names;


    // This holds the TGraphs that are created during the fit
    std::string outputFileName = measurement.GetOutputFilePrefix() + "_" + measurement.GetName() + ".root";
    std::cout << "Creating the output file: " << outputFileName << std::endl;
    TFile* outFile = new TFile(outputFileName.c_str(), "recreate");

    // This holds the table of fitted values and errors
    std::string tableFileName = measurement.GetOutputFilePrefix() + "_results.table";
    std::cout << "Creating the table file: " << tableFileName << std::endl;
    FILE*  tableFile =  fopen( tableFileName.c_str(), "a"); 

    std::cout << "Creating the HistoToWorkspaceFactoryFast factory" << std::endl;

    HistoToWorkspaceFactoryFast factory( measurement );

    // USING OLD VERSION...
    /*
    TFile* dummyFile = NULL; // TEMPORARY !!!!!
    HistoToWorkspaceFactoryFast factory(measurement.GetOutputFilePrefix(), rowTitle, measurement.GetConstantParams(), 
					measurement.GetLumi(), lumiError, 
					measurement.GetBinLow(), measurement.GetBinHigh(), dummyFile );
    */

    std::cout << "Setting preprocess functions" << std::endl;

    // Make the factory, and do some preprocessing
    // HistoToWorkspaceFactoryFast factory(measurement, rowTitle, outFile);
    factory.SetFunctionsToPreprocess( measurement.GetPreprocessFunctions() );

  
    // for results tables
    fprintf(tableFile, " %s &", rowTitle.c_str() );
  
    /***
	First: Loop to make the individual channels
    ***/


    for( unsigned int chanItr = 0; chanItr < measurement.GetChannels().size(); ++chanItr ) {
    
      HistFactory::Channel& channel = measurement.GetChannels().at( chanItr );

      if( ! channel.CheckHistograms() ) {
	std::cout << "MakeModelAndMeasurementsFast: Channel: " << channel.GetName()
		  << " has uninitialized histogram pointers" << std::endl;
	throw bad_hf;
	exit(-1);
      }

      string ch_name = channel.GetName();
      channel_names.push_back(ch_name);

      std::cout << "Starting to process channel: " << ch_name << std::endl;

      /*
      // First, turn the channel into a vector of estimate summaries
      std::vector<EstimateSummary> channel_estimateSummary = GetChannelEstimateSummaries( measurement, channel );
      
      // Then, use HistFactory on that vector to create the workspace
      RooWorkspace* ws_single = factory.MakeSingleChannelModel(channel_estimateSummary, measurement.GetConstantParams());

      // Finally, configure that workspace based on
      // properties of the measurement
      HistoToWorkspaceFactoryFast::ConfigureWorkspaceForMeasurement( "model_"+ch_name, ws_single, measurement );
      */

      RooWorkspace* ws_single = factory.MakeSingleChannelModel( measurement, channel );

      channel_workspaces.push_back(ws_single);

      // Get the Paramater of Interest as a RooRealVar
      RooRealVar* poi = (RooRealVar*) ws_single->var( (measurement.GetPOI()).c_str() );

      
      // Make the output
      std::string ChannelFileName = measurement.GetOutputFilePrefix() + "_" + ch_name + "_" + rowTitle + "_model.root";
      ws_single->writeToFile( ChannelFileName.c_str() );
    
      // Now, write the measurement to the file
      // Make a new measurement for only this channel
      RooStats::HistFactory::Measurement meas_chan( measurement );
      meas_chan.GetChannels().clear();
      meas_chan.GetChannels().push_back( channel );
      std::cout << "Opening File to hold channel: " << ChannelFileName << std::endl;
      TFile* chanFile = TFile::Open( ChannelFileName.c_str(), "UPDATE" );
      std::cout << "About to write channel measurement to file" << std::endl;
      meas_chan.writeToFile( chanFile );
      std::cout << "Successfully wrote channel to file" << std::endl;
      chanFile->Close();

      // do fit unless exportOnly requested
      if(! measurement.GetExportOnly()){
	if(!poi){
	  cout <<"can't do fit for this channel, no parameter of interest"<<endl;
	} else{
	  if(ws_single->data("obsData")){
	    FitModel(measurement.GetName(), measurement.GetOutputFilePrefix(), ws_single, ch_name, "obsData",    outFile, tableFile);
	  } else {
	    FitModel(measurement.GetName(), measurement.GetOutputFilePrefix(), ws_single, ch_name, "asimovData", outFile, tableFile);
	  }
	}
      }

      fprintf(tableFile, " & " );
    } // End loop over channels
  
    /***
	Second: Make the combined model:
	If you want output histograms in root format, create and pass it to the combine routine.
	"combine" : will do the individual cross-section measurements plus combination	
    ***/
  

    // Use HistFactory to combine the individual channel workspaces
    ws = factory.MakeCombinedModel(channel_names, channel_workspaces);

    // Configure that workspace
    HistoToWorkspaceFactoryFast::ConfigureWorkspaceForMeasurement( "simPdf", ws, measurement );

    // Get the Parameter of interest as a RooRealVar
    RooRealVar* poi = (RooRealVar*) ws->var( (measurement.GetPOI()).c_str() );

    //	  ws->writeToFile(("results/model_combined_edited.root").c_str());
    //ws->writeToFile((measurement.OutputFilePrefix+"_combined_"+rowTitle+"_model.root").c_str());
    std::string CombinedFileName = measurement.GetOutputFilePrefix()+"_combined_"+rowTitle+"_model.root";
    ws->writeToFile( CombinedFileName.c_str() );
    std::cout << "About to write combined measurement to file" << std::endl;
    TFile* combFile = TFile::Open( CombinedFileName.c_str(), "UPDATE" );
    measurement.writeToFile( combFile );
    combFile->Close();

    // TO DO:
    // Totally factorize the statistical test in "fit Model" to a different area
    if(! measurement.GetExportOnly()){
      if(!poi){
	cout <<"can't do fit for this channel, no parameter of interest"<<endl;
      } else{
	if(ws->data("obsData")){
	  FitModel(measurement.GetName(), measurement.GetOutputFilePrefix(), ws, "combined", "obsData",    outFile, tableFile);
	} else {
	  FitModel(measurement.GetName(), measurement.GetOutputFilePrefix(), ws, "combined", "asimovData", outFile, tableFile);
	}
      }
    }
  
    fprintf(tableFile, " \\\\ \n");

    outFile->Close();
    delete outFile;

    fclose( tableFile );

  }
  catch(exception& e)
    {
      std::cout << e.what() << std::endl;
      exit(-1);
    }

  return ws;


}


  ///////////////////////////////////////////////
void RooStats::HistFactory::FitModel(const std::string& MeasurementName, const std::string& FileNamePrefix, RooWorkspace * combined, string channel, string data_name, TFile* outFile, FILE* tableFile  )
  {

    cout << "In Fit Model"<<endl;
    ModelConfig * combined_config = (ModelConfig *) combined->obj("ModelConfig");
    if(!combined_config){
      cout << "no model config " << "ModelConfig" << " exiting" << endl;
      return;
    }
    //    RooDataSet * simData = (RooDataSet *) combined->obj(data_name.c_str());
    RooAbsData* simData = combined->data(data_name.c_str());
    if(!simData){
      cout << "no data " << data_name << " exiting" << endl;
      return;
    }
    //    const RooArgSet * constrainedParams=combined_config->GetNuisanceParameters();
    const RooArgSet * POIs=combined_config->GetParametersOfInterest();
    if(!POIs){
      cout << "no poi " << data_name << " exiting" << endl;
      return;
    }

    //RooAbsPdf* model=combined->pdf(model_name.c_str()); 
    RooAbsPdf* model=combined_config->GetPdf();
    //    RooArgSet* allParams = model->getParameters(*simData);

    ///////////////////////////////////////
    //Do combined fit
    //RooMsgService::instance().setGlobalKillBelow(RooMsgService::INFO) ;
    cout << "\n\n---------------" << endl;
    cout << "---------------- Doing "<< channel << " Fit" << endl;
    cout << "---------------\n\n" << endl;
    //    RooFitResult* result = model->fitTo(*simData, Minos(kTRUE), Save(kTRUE), PrintLevel(1));
    model->fitTo(*simData, Minos(kTRUE), PrintLevel(1));
    //    PrintCovarianceMatrix(result, allParams, "results/"+FilePrefixStr(channel)+"_corrMatrix.table" );

    if( outFile != NULL ) {

      //
      // assuming there is only on poi
      // 

      RooRealVar* poi = 0; // (RooRealVar*) POIs->first();
      // for results tables
      TIterator* params_itr=POIs->createIterator();
      TObject* params_obj=0;
      while((params_obj=params_itr->Next())){
	poi = (RooRealVar*) params_obj;
	cout << "printing results for " << poi->GetName() << " at " << poi->getVal()<< " high " << poi->getErrorLo() << " low " << poi->getErrorHi()<<endl;
      }
      fprintf(tableFile, " %.4f / %.4f  ", poi->getErrorLo(), poi->getErrorHi());

      RooAbsReal* nll = model->createNLL(*simData);
      RooAbsReal* profile = nll->createProfile(*poi);
      RooPlot* frame = poi->frame();
      FormatFrameForLikelihood(frame);
      TCanvas* c1 = new TCanvas( channel.c_str(), "",800,600);
      nll->plotOn(frame, ShiftToZero(), LineColor(kRed), LineStyle(kDashed));
      profile->plotOn(frame);
      frame->SetMinimum(0);
      frame->SetMaximum(2.);
      frame->Draw();
      //    c1->SaveAs( ("results/"+FilePrefixStr(channel)+"_profileLR.eps").c_str() );
      c1->SaveAs( (FileNamePrefix+"_"+channel+"_"+MeasurementName+"_profileLR.eps").c_str() );


      outFile->mkdir(channel.c_str())->mkdir("Summary")->cd();
    
      RooCurve* curve=frame->getCurve();
      Int_t curve_N=curve->GetN();
      Double_t* curve_x=curve->GetX();
      delete frame; delete c1;
    
      //
      // Verbose output from MINUIT
      //
      /*
	RooMsgService::instance().setGlobalKillBelow(RooFit::DEBUG) ;
	profile->getVal();
	RooMinuit* minuit = ((RooProfileLL*) profile)->minuit();
	minuit->setPrintLevel(5) ; // Print MINUIT messages
	minuit->setVerbose(5) ; // Print RooMinuit messages with parameter 
	// changes (corresponds to the Verbose() option of fitTo()
	*/
    
      Double_t * x_arr = new Double_t[curve_N];
      Double_t * y_arr_nll = new Double_t[curve_N];
      //     Double_t y_arr_prof_nll[curve_N];
      //     Double_t y_arr_prof[curve_N];
      
      for(int i=0; i<curve_N; i++){
	double f=curve_x[i];
	poi->setVal(f);
	x_arr[i]=f;
	y_arr_nll[i]=nll->getVal();
      }

      TGraph * g = new TGraph(curve_N, x_arr, y_arr_nll);
      //g->SetName( (FilePrefixStr(channel) +"_nll").c_str());
      g->SetName( (FileNamePrefix +"_nll").c_str() );
      g->Write(); 
      delete g;
      delete [] x_arr;
      delete [] y_arr_nll;

    }

    /** find out what's inside the workspace **/
    //combined->Print();

  }


void RooStats::HistFactory::FormatFrameForLikelihood(RooPlot* frame, string /*XTitle*/, string YTitle){

    gStyle->SetCanvasBorderMode(0);
    gStyle->SetPadBorderMode(0);
    gStyle->SetPadColor(0);
    gStyle->SetCanvasColor(255);
    gStyle->SetTitleFillColor(255);
    gStyle->SetFrameFillColor(0);  
    gStyle->SetStatColor(255);
    
    RooAbsRealLValue* var = frame->getPlotVar();
    double xmin = var->getMin();
    double xmax = var->getMax();
    
    frame->SetTitle("");
    //      frame->GetXaxis()->SetTitle(XTitle.c_str());
    frame->GetXaxis()->SetTitle(var->GetTitle());
    frame->GetYaxis()->SetTitle(YTitle.c_str());
    frame->SetMaximum(2.);
    frame->SetMinimum(0.);
    TLine * line = new TLine(xmin,.5,xmax,.5);
    line->SetLineColor(kGreen);
    TLine * line90 = new TLine(xmin,2.71/2.,xmax,2.71/2.);
    line90->SetLineColor(kGreen);
    TLine * line95 = new TLine(xmin,3.84/2.,xmax,3.84/2.);
    line95->SetLineColor(kGreen);
    frame->addObject(line);
    frame->addObject(line90);
    frame->addObject(line95);
}




/*
RooWorkspace* RooStats::HistFactory::MakeModelFast( RooStats::HistFactory::Measurement& measurement ) {
  // Take an input measurement object
  // Loop over its channels and create a
  // workspace for each channel
  // Then, combine those workspaces to form
  // the combined workspace and simultaneous pdf
  // Return that workspace (which contains the simPdf)
  

  // This will be returned
  RooWorkspace* ws = NULL;

  try {

    std::cout << "Making Model and Measurements (Fast) for measurement: " << measurement.GetName() << std::endl;

    double lumiError = measurement.GetLumi()*measurement.GetLumiRelErr();

    std::cout << "using lumi = " << measurement.GetLumi() << " and lumiError = " << lumiError
	 << " including bins between " << measurement.GetBinLow() << " and " << measurement.GetBinHigh() << std::endl;
    std::cout << "fixing the following parameters:"  << std::endl;

    for(vector<string>::iterator itr=measurement.GetConstantParams().begin(); itr!=measurement.GetConstantParams().end(); ++itr){
      cout << "   " << *itr << endl;
    }
  
    vector<string> preprocessFunctions;

    std::string rowTitle = measurement.GetName();
    
    vector<RooWorkspace*> channel_workspaces;
    vector<string>        channel_names;


    std::cout << "Creating the HistoToWorkspaceFactoryFast factory" << std::endl;
    TFile* dummyFile=NULL;     
    // TFile* outFile = new TFile( "dummy.root", "recreate");
    HistoToWorkspaceFactoryFast factory(measurement.GetOutputFilePrefix(), rowTitle, measurement.GetConstantParams(), 
					measurement.GetLumi(), lumiError, 
					measurement.GetBinLow(), measurement.GetBinHigh(), dummyFile);
    

    // Make the factory, and do some preprocessing
    // HistoToWorkspaceFactoryFast factory(measurement, rowTitle, outFile);
    std::cout << "Setting preprocess functions" << std::endl;
    factory.SetFunctionsToPreprocess( measurement.GetPreprocessFunctions() );


    // Make the individual channels
    for( unsigned int chanItr = 0; chanItr < measurement.GetChannels().size(); ++chanItr ) {
    
      HistFactory::Channel& channel = measurement.GetChannels().at( chanItr );

      if( ! channel.CheckHistograms() ) {
	std::cout << "MakeModelAndMeasurementsFast: Channel: " << channel.GetName()
		  << " has uninitialized histogram pointers" << std::endl;
	throw bad_hf;
	exit(-1);
      }

      string ch_name = channel.GetName();
      channel_names.push_back(ch_name);

      std::cout << "Starting to process channel: " << ch_name << std::endl;

      // First, turn the channel into a vector of estimate summaries
      std::vector<EstimateSummary> channel_estimateSummary = GetChannelEstimateSummaries( measurement, channel );
      
      // Then, use HistFactory on that vector to create the workspace
      RooWorkspace* ws_single = factory.MakeSingleChannelModel(channel_estimateSummary, measurement.GetConstantParams());
      channel_workspaces.push_back(ws_single);

      // Finally, configure that workspace based on
      // properties of the measurement
      HistoToWorkspaceFactoryFast::ConfigureWorkspaceForMeasurement( "model_"+ch_name, ws_single, measurement );


      // do fit unless exportOnly requested
      / *
      if(! measurement.GetExportOnly()){
	RooRealVar* poi = (RooRealVar*) ws_single->var( (measurement.GetPOI()).c_str() );
	if(!poi){
	  cout <<"can't do fit for this channel, no parameter of interest"<<endl;
	} else{
	  if(ws_single->data("obsData")){
	    bool dummy=false;
	    factory.FitModel(ws_single, ch_name, "", "obsData", dummy);
	  } else {
	    bool dummy=false;
	    factory.FitModel(ws_single, ch_name, "", "asimovData", dummy);
	  }
	}
      }
      * /

    } // End loop over channels
  

    // Use HistFactory to combine the individual channel workspaces
    ws = factory.MakeCombinedModel(channel_names, channel_workspaces);

    // Configure that workspace
    HistoToWorkspaceFactoryFast::ConfigureWorkspaceForMeasurement( "simPdf", ws, measurement );

    if(! measurement.GetExportOnly()){

      RooRealVar* poi = (RooRealVar*) ws->var( (measurement.GetPOI()).c_str() );
      if(!poi){
	cout <<"can't do fit for this channel, no parameter of interest"<<endl;
      } else{
	if(ws->data("obsData")){
	  factory.FitModel(measurement.GetOutputFilePrefix(), ws, "combined", "obsData",    NULL, NULL );
	} else {
	  factory.FitModel(measurement.GetOutputFilePrefix(), ws, "combined", "asimovData", NULL, NULL );
	}
      }
    }
    
  }
  catch(exception& e)
    {
      std::cout << e.what() << std::endl;
      exit(-1);
    }
  
  return ws;

}
*/

//

/*
// THIS IS THE OBSOLETE VERSION::
// main is int MakeModelAndMeasurements
void fastDriver(string input){
  // TO DO:
  // would like to fully factorize the XML parsing.  
  // No clear need to have some here and some in ConfigParser

  / *** read in the input xml *** /
  TDOMParser xmlparser;
  Int_t parseError = xmlparser.ParseFile( input.c_str() );
  if( parseError ) { 
    std::cerr << "Loading of xml document \"" << input
          << "\" failed" << std::endl;
  } 

  cout << "reading input : " << input << endl;
  TXMLDocument* xmldoc = xmlparser.GetXMLDocument();
  TXMLNode* rootNode = xmldoc->GetRootNode();

  // require combination 
  if( rootNode->GetNodeName() != TString( "Combination" ) ){ return; }

  string outputFileNamePrefix;
  vector<string> preprocessFunctions;
  vector<string> xml_input;
  
  TListIter attribIt = rootNode->GetAttributes();
  TXMLAttr* curAttr = 0;
  while( ( curAttr = dynamic_cast< TXMLAttr* >( attribIt() ) ) != 0 ) {
    if( curAttr->GetName() == TString( "OutputFilePrefix" ) ) {
      outputFileNamePrefix=string(curAttr->GetValue());
      cout << "output file prefix : " << outputFileNamePrefix << endl;
    }
  } 
  TXMLNode* node = rootNode->GetChildren();
  while( node != 0 ) {
    if( node->GetNodeName() == TString( "Input" ) ) { xml_input.push_back(node->GetText()); }
    if( node->GetNodeName() == TString( "Function" ) ) { 
      preprocessFunctions.push_back(ParseFunctionConfig(node ) ); 
    }
    node = node->GetNextNode();
  }
  
  / * process each xml node (= channel) * /
  node = rootNode->GetChildren();
  while( node != 0 ) {
    if( node->GetNodeName() != TString( "Measurement" ) ) {  node = node->GetNextNode(); continue; }
    processMeasurement(outputFileNamePrefix,xml_input,node,preprocessFunctions); // MB : I moved this to a separate function
    node = node->GetNextNode(); // next measurement
  } 
}
*/


/*
void processMeasurement(string outputFileNamePrefix, vector<string> xml_input, TXMLNode* node,
			vector<string> preprocessFunctions)
{  
  string outputFileName;
  Double_t nominalLumi=0, lumiRelError=0, lumiError=0;
  Int_t lowBin=0, highBin=0;
  string rowTitle, POI, mode;
  vector<string> systToFix;
  map<string,double> gammaSyst, uniformSyst, logNormSyst, noSyst;
  bool exportOnly = false;
  
  cout << "Now processing measurement " << endl;

  / * first interpret the specific measurement * /   // MB : I moved this to a separate function
  processMeasurementXML(node, 
			outputFileName, outputFileNamePrefix,
			nominalLumi, lumiRelError, lumiError,
			lowBin, highBin,
			rowTitle, POI, mode,
			systToFix,
			gammaSyst,
			uniformSyst,
			logNormSyst,
			noSyst,
			exportOnly
			);
  
  / * Now setup the measurement * /
  cout << "using lumi = " << nominalLumi << " and lumiError = " << lumiError
       << " including bins between " << lowBin << " and " << highBin << endl;
  cout << "fixing the following parameters:"  << endl;
  for(vector<string>::iterator itr=systToFix.begin(); itr!=systToFix.end(); ++itr){
    cout << "   " << *itr << endl;
  }
  
  / ***
      Construction of Model. Only requirement is that they return vector<vector<EstimateSummary> >
      This is where we use the factory.
  *** /
  
  vector<vector<EstimateSummary> > summaries;
  if(xml_input.empty()){
    cerr << "no input channels found" << endl;
    exit(1);
  }
  
  
  vector<RooWorkspace*> chs;
  vector<string> ch_names;
  TFile* outFile = new TFile(outputFileName.c_str(), "recreate");
  HistoToWorkspaceFactoryFast factory(outputFileNamePrefix, rowTitle, systToFix, nominalLumi, lumiError, lowBin, highBin , outFile);
  factory.SetFunctionsToPreprocess(preprocessFunctions);
  
  // for results tables
  fprintf(factory.pFile, " %s &", rowTitle.c_str() );
  
  / ***
      First: Loop to make the individual channels
  *** /

  for(vector<string>::iterator itr=xml_input.begin(); itr!=xml_input.end(); ++itr) {

    vector<EstimateSummary> oneChannel;
    // read xml
    ReadXmlConfig(*itr, oneChannel, nominalLumi);
    // not really needed anymore
    summaries.push_back(oneChannel);
    // use factory to create the workspace
    string ch_name=oneChannel[0].channel;
    ch_names.push_back(ch_name);
    RooWorkspace * ws = factory.MakeSingleChannelModel(oneChannel, systToFix);
    chs.push_back(ws);
    // set poi in ModelConfig
    ModelConfig * proto_config = (ModelConfig *) ws->obj("ModelConfig");
    cout << "Setting Parameter of Interest as :" << POI << endl;
    RooRealVar* poi = (RooRealVar*) ws->var(POI.c_str());
    RooArgSet * params= new RooArgSet;
    if(poi){
      params->add(*poi);
    }
    proto_config->SetParametersOfInterest(*params);
    
    
    // Gamma/Uniform Constraints:
    // turn some Gaussian constraints into Gamma/Uniform/LogNorm/NoConstraint constraints, rename model newSimPdf
    if(gammaSyst.size()>0 || uniformSyst.size()>0 || logNormSyst.size()>0 || noSyst.size()>0) {
      factory.EditSyst(ws,("model_"+oneChannel[0].channel).c_str(),gammaSyst,uniformSyst,logNormSyst,noSyst);
      proto_config->SetPdf(*ws->pdf("newSimPdf"));
    }
    
    // fill out ModelConfig and export
    RooAbsData* expData = ws->data("asimovData");
    if(poi){
      proto_config->GuessObsAndNuisance(*expData);
    }
    ws->writeToFile((outputFileNamePrefix+"_"+ch_name+"_"+rowTitle+"_model.root").c_str());
    
    // do fit unless exportOnly requested
    if(!exportOnly){
      if(!poi){
	cout <<"can't do fit for this channel, no parameter of interest"<<endl;
      } else{
	if(ws->data("obsData")){
	  factory.FitModel(ws, ch_name, "newSimPdf", "obsData", false);
	} else {
	  factory.FitModel(ws, ch_name, "newSimPdf", "asimovData", false);
	}
      }
      
    }
    fprintf(factory.pFile, " & " );
  } // end channel-creation loop
  
  / ***
      Second: Make the combined model:
      If you want output histograms in root format, create and pass it to the combine routine.
      "combine" : will do the individual cross-section measurements plus combination	
  *** /
  
  if ( true || mode.find("comb")!=string::npos) { // KC: always do comb. need to clean up code

    RooWorkspace* ws=factory.MakeCombinedModel(ch_names,chs);
    // Gamma/Uniform Constraints:
    // turn some Gaussian constraints into Gamma/Uniform/logNormal/noConstraint constraints, rename model newSimPdf
    if(gammaSyst.size()>0 || uniformSyst.size()>0 || logNormSyst.size()>0 || noSyst.size()) 
      factory.EditSyst(ws,"simPdf",gammaSyst,uniformSyst,logNormSyst,noSyst);
    //
    // set parameter of interest according to the configuration
    //
    ModelConfig * combined_config = (ModelConfig *) ws->obj("ModelConfig");
    cout << "Setting Parameter of Interest as :" << POI << endl;
    RooRealVar* poi = (RooRealVar*) ws->var((POI).c_str());
    //RooRealVar* poi = (RooRealVar*) ws->var((POI+"_comb").c_str());
    RooArgSet * params= new RooArgSet;
    //    cout << poi << endl;
    if(poi){
      params->add(*poi);
    }
    combined_config->SetParametersOfInterest(*params);
    ws->Print();
    
    // Set new PDF if there are gamma/uniform constraint terms
    if(gammaSyst.size()>0 || uniformSyst.size()>0 || logNormSyst.size()>0) 
      combined_config->SetPdf(*ws->pdf("newSimPdf"));
    
    RooAbsData* simData = ws->data("asimovData");
    combined_config->GuessObsAndNuisance(*simData);
    //	  ws->writeToFile(("results/model_combined_edited.root").c_str());
    ws->writeToFile((outputFileNamePrefix+"_combined_"+rowTitle+"_model.root").c_str());
    
    // TO DO:
    // Totally factorize the statistical test in "fit Model" to a different area
    if(!exportOnly){
      if(!poi){
	cout <<"can't do fit for this channel, no parameter of interest"<<endl;
      } else{
	if(ws->data("obsData")){
	  factory.FitModel(ws, "combined", "simPdf", "obsData", false);
	} else {
	  factory.FitModel(ws, "combined", "simPdf", "asimovData", false);
	}
      }
    }
  } // end combination of channels
  
  fprintf(factory.pFile, " \\\\ \n");
  
  outFile->Close();
  delete outFile;
}
*/

 /*
void processMeasurementXML(TXMLNode* node, 
			   string& outputFileName, string outputFileNamePrefix, 
			   Double_t& nominalLumi, Double_t& lumiRelError, Double_t& lumiError,
			   Int_t& lowBin, Int_t& highBin,
			   string& rowTitle, string& POI, string& mode,
			   vector<string>& systToFix,
			   map<string,double>& gammaSyst,
			   map<string,double>& uniformSyst,
			   map<string,double>& logNormSyst,
			   map<string,double>& noSyst,
			   bool& exportOnly
			  )
{
  //        TListIter attribIt = node->GetAttributes();
  //        TXMLAttr* curAttr = 0;
  TListIter attribIt = node->GetAttributes();
  TXMLAttr* curAttr = 0;

  while( ( curAttr = dynamic_cast< TXMLAttr* >( attribIt() ) ) != 0 ) {
    if( curAttr->GetName() == TString( "Lumi" ) ) {
      nominalLumi=atof(curAttr->GetValue());
    }
    if( curAttr->GetName() == TString( "LumiRelErr" ) ) {
      lumiRelError=atof(curAttr->GetValue());
    }
    if( curAttr->GetName() == TString( "BinLow" ) ) {
      cout <<"\n WARNING: In -standard_form ignore BinLow and BinHigh, just use all bins"<<endl;
      lowBin=atoi(curAttr->GetValue());
    }
    if( curAttr->GetName() == TString( "BinHigh" ) ) {
      cout <<"\n WARNING: In -standard_form ignore BinLow and BinHigh, just use all bins"<<endl;
      highBin=atoi(curAttr->GetValue());
    }
    if( curAttr->GetName() == TString( "Name" ) ) {
      rowTitle=curAttr->GetValue();
      outputFileName=outputFileNamePrefix+"_"+rowTitle+".root";
    }
    if( curAttr->GetName() == TString( "Mode" ) ) {
      cout <<"\n INFO: Mode attribute is deprecated, will ignore\n"<<endl;
      mode=curAttr->GetValue();
    }
    if( curAttr->GetName() == TString( "ExportOnly" ) ) {
      if(curAttr->GetValue() == TString( "True" ) )
	exportOnly = true;
      else
	exportOnly = false;
    }
  }
  lumiError=nominalLumi*lumiRelError;
  
  TXMLNode* mnode = node->GetChildren();
  while( mnode != 0 ) {
    if( mnode->GetNodeName() == TString( "POI" ) ) {
      POI=mnode->GetText();
    }
    if( mnode->GetNodeName() == TString( "ParamSetting" ) ) {
      //            TListIter attribIt = mnode->GetAttributes();
      //TXMLAttr* curAttr = 0;
      attribIt = mnode->GetAttributes();
      curAttr = 0;
      while( ( curAttr = dynamic_cast< TXMLAttr* >( attribIt() ) ) != 0 ) {
	if( curAttr->GetName() == TString( "Const" ) ) {
	  if(curAttr->GetValue()==TString("True")){
	    AddSubStrings(systToFix, mnode->GetText());
	  }
	}
      }
    }
    if( mnode->GetNodeName() == TString( "ConstraintTerm" ) ) {
      vector<string> syst; string type = ""; double rel = 0;
      AddSubStrings(syst,mnode->GetText());
      //            TListIter attribIt = mnode->GetAttributes();
      //            TXMLAttr* curAttr = 0;
      attribIt = mnode->GetAttributes();
      curAttr = 0;
      while( ( curAttr = dynamic_cast< TXMLAttr* >( attribIt() ) ) != 0 ) {
	if( curAttr->GetName() == TString( "Type" ) ) {
	  type = curAttr->GetValue();
	}
	if( curAttr->GetName() == TString( "RelativeUncertainty" ) ) {
	  rel = atof(curAttr->GetValue());
	}
      }
      if (type=="Gamma" && rel!=0) {
	for (vector<string>::const_iterator it=syst.begin(); it!=syst.end(); it++) gammaSyst[(*it).c_str()] = rel;
      }
      if (type=="Uniform" && rel!=0) {
	for (vector<string>::const_iterator it=syst.begin(); it!=syst.end(); it++) uniformSyst[(*it).c_str()] = rel;
      }
      if (type=="LogNormal" && rel!=0) {
	for (vector<string>::const_iterator it=syst.begin(); it!=syst.end(); it++) logNormSyst[(*it).c_str()] = rel;
      }
      if (type=="NoConstraint") {
	for (vector<string>::const_iterator it=syst.begin(); it!=syst.end(); it++) noSyst[(*it).c_str()] = 1.0; // MB : dummy value
      }
    }
    mnode = mnode->GetNextNode();
  } 

  //cout << "processMeasurementXML() : " << gammaSyst.size() << " " << uniformSyst.size() << " " << logNormSyst.size() << " " << noSyst.size() << endl;
  
  // end of xml processing
}

*/
