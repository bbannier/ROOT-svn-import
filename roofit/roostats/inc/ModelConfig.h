// @(#)root/roostats:$Id: ModelConfig.h 27519 2009-02-19 13:31:41Z pellicci $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_ModelConfig
#define ROOSTATS_ModelConfig


#ifndef ROO_ABS_PDF
#include "RooAbsPdf.h"
#endif

#ifndef ROO_ABS_DATA
#include "RooAbsData.h"
#endif

#ifndef ROO_ARG_SET
#include "RooArgSet.h"
#endif

#ifndef ROO_WORKSPACE
#include "RooWorkspace.h"
#endif

#ifndef ROO_MSG_SERVICE
#include "RooMsgService.h"
#endif


//_________________________________________________
/*
BEGIN_HTML
<p>
ModelConfig is a simple class that holds configuration information specifying how a model
should be used in the context of various RooStats tools.  A single model can be used
in different ways, and this class should carry all that is needed to specify how it should be used.
</p>
END_HTML
*/
//

namespace RooStats {

   class ModelConfig : public TNamed {

  public:
  ModelConfig() : TNamed(){
      fWS = 0;
      fOwnsWorkspace = false;
    }
    
  ModelConfig(const char* name) : TNamed(name, name){
      fWS = 0;
      fOwnsWorkspace = false;
    }
    
  ModelConfig(const char* name, const char* title) : TNamed(name, title){
      fWS = 0;
      fOwnsWorkspace = false;
    }
    
    virtual ~ModelConfig() {
      // destructor.
      if( fOwnsWorkspace && fWS) delete fWS;
    }
    
    // set a workspace that owns all the necessary components for the analysis
    virtual void SetWorkspace(RooWorkspace & ws) {
      if (!fWS)
	fWS = &ws;
      else{
	RooMsgService::instance().setGlobalKillBelow(RooMsgService::ERROR) ;
	fWS->merge(ws);
	RooMsgService::instance().setGlobalKillBelow(RooMsgService::DEBUG) ;
      }
      
    }

    // helper function to avoid code duplication
    void DefineSet(const char* name, RooArgSet& set){
      if (!fWS) {
	fWS = new RooWorkspace();
	fOwnsWorkspace = true; 
      }
      if (! fWS->set( name )){
	RooMsgService::instance().setGlobalKillBelow(RooMsgService::ERROR) ;
	fWS->defineSet(name, set);
	RooMsgService::instance().setGlobalKillBelow(RooMsgService::DEBUG) ;
      }
    }
    

    // Set the DataSet, add to the the workspace if not already there
    virtual void SetData(RooAbsData & data) {      
      if (!fWS) {
	fWS = new RooWorkspace();
	fOwnsWorkspace = true; 
      }
      if (! fWS->data( data.GetName() ) ){
	RooMsgService::instance().setGlobalKillBelow(RooMsgService::ERROR) ;
	fWS->import(data);
	RooMsgService::instance().setGlobalKillBelow(RooMsgService::DEBUG) ;
      }
      SetData( data.GetName() );
      
    }

    // Set the proto DataSet, add to the the workspace if not already there
    virtual void SetProtoData(RooAbsData & data) {      
      if (!fWS) {
	fWS = new RooWorkspace();
	fOwnsWorkspace = true; 
      }
      if (! fWS->data( data.GetName() ) ){
	RooMsgService::instance().setGlobalKillBelow(RooMsgService::ERROR) ;
	fWS->import(data);
	RooMsgService::instance().setGlobalKillBelow(RooMsgService::DEBUG) ;
      }
      SetProtoData( data.GetName() );
    }
    
    // Set the Pdf, add to the the workspace if not already there
    virtual void SetPdf(RooAbsPdf& pdf) {
      if (!fWS) 
	fWS = new RooWorkspace();
      if (! fWS->pdf( pdf.GetName() ) ){
	RooMsgService::instance().setGlobalKillBelow(RooMsgService::ERROR) ;
	fWS->import(pdf);
	RooMsgService::instance().setGlobalKillBelow(RooMsgService::DEBUG) ;
      }
      SetPdf( pdf.GetName() );
    }
    
    // specify the parameters of interest in the interval
    virtual void SetParameters(RooArgSet& set) {
      string temp = GetName();
      temp+="_POI";
      fPOIName=temp.c_str();
      DefineSet(fPOIName, set);
    }
    
    // specify the nuisance parameters (eg. the rest of the parameters)
    virtual void SetNuisanceParameters(RooArgSet& set) {
      string temp = GetName();
      temp+="_NuisParams";
      fNuisParamsName=temp.c_str();
      DefineSet(fNuisParamsName, set);
    }

    // set parameter values for the null if using a common PDF
    virtual void SetSnapshot(RooArgSet& set) {
      string temp = GetName();
      temp+="_Snapshot";
      fSnapshotName=temp.c_str();
      DefineSet(fSnapshotName, set);
    }    
    
    // specify the name of the PDF in the workspace to be used
    virtual void SetPdf(const char* name) {
      if(!fWS){
	coutE(ObjectHandling) << "workspace not set" << endl;
	return;
      }
      if(fWS->pdf(name))
	fPdfName = name;
      else
	coutE(ObjectHandling) << "pdf "<<name<< " does not exist in workspace"<<endl;
    }

    // specify the name of the PDF in the workspace to be used
    virtual void SetPriorPdf(const char* name) {
      if(!fWS){
	coutE(ObjectHandling) << "workspace not set" << endl;
	return;
      }
      if(fWS->pdf(name))
	fPriorPdfName = name;
      else
	coutE(ObjectHandling) << "pdf "<<name<< " does not exist in workspace"<<endl;
    }

    // specify the name of the dataset in the workspace to be used
    virtual void SetData(const char* name){
      if(!fWS){
	coutE(ObjectHandling) << "workspace not set" << endl;
	return;
      }
      if(fWS->data(name))
	fDataName = name;
      else
	coutE(ObjectHandling) << "dataset "<<name<< " does not exist in workspace"<<endl;
    }

    // specify the name of the dataset in the workspace to be used
    virtual void SetProtoData(const char* name){
      if(!fWS){
	coutE(ObjectHandling) << "workspace not set" << endl;
	return;
      }
      if(fWS->data(name))
	fProtoDataName = name;
      else
	coutE(ObjectHandling) << "dataset "<<name<< " does not exist in workspace"<<endl;
    }

    
    
  protected:
    
    RooWorkspace* fWS; // a workspace that owns all the components to be used by the calculator
    Bool_t fOwnsWorkspace;
    const char* fPdfName; // name of  PDF in workspace
    const char* fDataName; // name of data set in workspace
    const char* fPOIName; // name for RooArgSet specifying parameters of interest
    
    const char* fNuisParamsName; // name for RooArgSet specifying nuisance parameters
    const char* fConstrainedParamName; // name for RooArgSet specifying constrained parameters
    const char* fPriorPdfName; // name for RooAbsPdf specifying a prior on the parameters
    
    const char* fConditionalObservablesName; // name for RooArgSet specifying conditional observables
    const char* fProtoDataName; // name for RooArgSet specifying dataset that should be used as protodata
    
    const char* fSnapshotName; // name for RooArgSet that specifies a particular hypothesis
    
    const char* fObservablesName; // name for RooArgSet specifying observable parameters. 
    
    ClassDef(ModelConfig,1) // A class that holds configuration information for a model using a workspace as a store
      
 };
}


#endif
