//////////////////////////////////////////////////////////////////////////
//
// RooStats tutorial macro #500a
// 2009/08 - Nils Ruthmann, Gregory Schott
//
// Prepare a workspace (stored in a ROOT file) containing a models,
// data and other objects needed to run statistical classes in
// RooStats.
//
// In this macro a PDF model is built for a counting analysis.  A
// certain number of events are observed (this can be enforced or left
// free) while a number of background events is expected.  In this
// macro, no systematic uncertainty is considered (see
// rs500b_PrepareWorkspace_Poisson_withSystematics.C) The parameter of
// interest is the signal yield and we assume for it a flat prior.
// All needed objects are stored in a ROOT file (within a RooWorkspace
// container); this ROOT file can then be fed as input to various
// statistical methods.
//
// root -q -x -l 'rs500a_PrepareWorkspace_Poisson.C()'
//
//////////////////////////////////////////////////////////////////////////


#include "RooAbsPdf.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooPlot.h"
#include "RooWorkspace.h"

#include "TFile.h"

using namespace RooFit;
using namespace RooStats;

// prepare the workspace
// type = 0 : binned data with fixed number of total events
// type = 1 : binned data with total events fluctuationg with Poisson statistics
// type = 2 : binned data without any bin-by bin fluctuations (Asimov data)

void rs500a_PrepareWorkspace_Poisson( TString fileName = "WS_Poisson.root", int type = 1 )
{

   // use a RooWorkspace to store the pdf models, prior informations, list of parameters,...
   RooWorkspace* myWS = new RooWorkspace("myWS");

   // Observable
   myWS->factory("x[0,0,1]") ;

   // Pdf in observable, 
   myWS->factory("Uniform::sigPdf(x)") ;
   myWS->factory("Uniform::bkgPdf(x)") ;
   myWS->factory("SUM::model(S[50,0,100]*sigPdf,B[100]*bkgPdf") ;
   // Background only pdf
   myWS->factory("ExtendPdf::modelBkg(bkgPdf,B)") ;

   // Prior
   myWS->factory("Uniform::priorPOI(S)") ;

   // Definition of observables and parameters of interest
   myWS->defineSet("observables","x");
   myWS->defineSet("POI","S");

   // save snapshots of parameters for different hypotheses
   myWS->var("S")->setVal(50.);
   myWS->saveSnapshot("alternate", "S");
   myWS->var("S")->setVal(0.);
   myWS->saveSnapshot("null", "S");

   // create model config for alternate
   ModelConfig* alternateModel = new ModelConfig("alternate");
   alternateModel->SetWorkspace( *myWS );
   alternateModel->SetPdf( *myWS->pdf("model") );
   alternateModel->SetPriorPdf( *myWS->pdf("priorPOI") );
   alternateModel->SetParametersOfInterest( *myWS->set("POI") );
   alternateModel->SetObservables( *myWS->set("observables") );
   myWS->loadSnapshot( "alternate" );
   alternateModel->SetSnapshot( *myWS->set("POI") );
   
   // create model for null
   ModelConfig* nullModel = new ModelConfig(*alternateModel);
   myWS->loadSnapshot( "null" );
   nullModel->SetSnapshot( *myWS->set("POI") );

   // import the model config into the workspace
   myWS->import(*alternateModel,"alternateModel");
   myWS->import(*nullModel,"nullModel");
   cout << "check shapshots" << endl;
   alternateModel->GetSnapshot()->Print("v");
   nullModel->GetSnapshot()->Print("v");


   // Generate data
   RooAbsData* data = 0;
   // binned data with fixed number of events
   if (type ==0) data = myWS->pdf("model")->generateBinned(*myWS->set("observables"),myWS->var("S")->getVal(),Name("data"));
   // binned data with Poisson fluctuations
   if (type ==1) data = myWS->pdf("model")->generateBinned(*myWS->set("observables"),Extended(),Name("data"));
   // Asimov data: binned data without any fluctuations (average case) 
   if (type ==2) data = myWS->pdf("model")->generateBinned(*myWS->set("observables"),Name("data"),ExpectedData());
   myWS->import(*data) ;

   // control plot of the generated data
   RooPlot* plot = myWS->var("x")->frame();
   data->plotOn(plot);
   plot->DrawClone();

   myWS->writeToFile(fileName);  
   std::cout << "\nRooFit model initialized and stored in " << fileName << std::endl;
}
