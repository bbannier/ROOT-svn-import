//////////////////////////////////////////////////////////////////////////
//
// RooStats tutorial macro #501
// 2009/08 - Nils Ruthmann, Gregory Schott
//
// Show how to run the RooStats classes to perform specific tasks. The
// ROOT file containing a workspace holding the models, data and other
// objects needed to run can be prepared with any of the rs500*.C
// tutorial macros.
//
// Compute with ProfileLikelihoodCalculator a 95% CL upper limit on
// the parameter of interest for the given data.
//
//////////////////////////////////////////////////////////////////////////

#include "RooRealVar.h"
#include "RooProdPdf.h"
#include "RooWorkspace.h"
#include "RooStats/ProfileLikelihoodCalculator.h"
#include "RooStats/FeldmanCousins.h"
#include "RooStats/LikelihoodIntervalPlot.h"
#include "TH2F.h"
#include "RooDataHist.h"
#include "TFile.h"

using namespace RooFit;
using namespace RooStats;


void rs501_ProfileLikelihoodCalculator_limit( const char* fileName="WS_GaussOverFlat_withSystematics_floatingMass.root" )
{
  // Open the ROOT file and import from the workspace the objects needed for this tutorial
  TFile* file = new TFile(fileName);
  RooWorkspace* myWS = (RooWorkspace*) file->Get("myWS");
  RooAbsPdf* modelTmp = myWS->pdf("model");
  RooAbsData* data = myWS->data("data");
  RooAbsPdf* nuisanceTerm = myWS->pdf("nuisanceTerm");
  const RooArgSet* POI = myWS->set("POI");
  POI->Print("v");
  RooRealVar* parameterOfInterest = dynamic_cast<RooRealVar*>(POI->first());
  assert(parameterOfInterest);

  // If there are nuisance parameters, multiply their term to the full model
  RooAbsPdf* model = modelTmp;
  if( nuisanceTerm!=0 ) model = new RooProdPdf("constrainedModel","Model with nuisance parameters",*modelTmp,*nuisanceTerm);

  // Set up the ProfileLikelihoodCalculator
  ProfileLikelihoodCalculator plc(*data, *model, *POI);
  // ProfileLikelihoodCalculator usually make intervals: the 95% CL one-sided upper-limit is the same as the two-sided upper-limit of a 90% CL interval  
  plc.SetTestSize(0.10);


  // Pointer to the confidence interval
  //  model->fitTo(*data,SumW2Error(kFALSE)); // <-- problem
  LikelihoodInterval* interval = plc.GetInterval();

  // Compute the upper limit: a fit is needed first in order to locate the minimum of the -log(likelihood) and ease the upper limit computation
  //  model->fitTo(*data,SumW2Error(kFALSE)); // <-- problem
  const double upperLimit = interval->UpperLimit(*parameterOfInterest); // <-- to simplify

  // Make a plot of the profile-likelihood and confidence interval
  LikelihoodIntervalPlot plot(interval);
  plot.Draw();

  
  FeldmanCousins fc;
  fc.SetPdf(*model);
  fc.SetData(*data); 
  fc.SetParameters( *POI );
  fc.UseAdaptiveSampling(true);
  fc.SetNBins(10); // number of points to test per parameter
  fc.SetTestSize(.1);
  //  fc.SaveBeltToFile(true); // optional
  ConfInterval* fcint = NULL;
  fcint = fc.GetInterval();  // that was easy.

  // first plot a small dot for every point tested
  RooDataHist* parameterScan = (RooDataHist*) fc.GetPointsToScan();
  TH2F* hist = (TH2F*) parameterScan->createHistogram("trueMass:S",10,10);
  //  hist->Draw();
  TH2F* forContour = (TH2F*)hist->Clone();

  // now loop through the points and put a marker if it's in the interval
  RooArgSet* tmpPoint;
  // loop over points to test
  for(Int_t i=0; i<parameterScan->numEntries(); ++i){
     // get a parameter point from the list of points to test.
    tmpPoint = (RooArgSet*) parameterScan->get(i)->clone("temp");
    //    tmpPoint->Print("v");
    if (fcint){
      if (fcint->IsInInterval( *tmpPoint ) ) {
	forContour->SetBinContent( hist->FindBin(tmpPoint->getRealValue("trueMass"), 
						  tmpPoint->getRealValue("S")),	 1);
      }else{
	forContour->SetBinContent( hist->FindBin(tmpPoint->getRealValue("trueMass"), 
						  tmpPoint->getRealValue("S")),	 0);
      }
    }


    delete tmpPoint;
  }
  cout << "so far so good" << endl;
  forContour->Draw("box,same");
  


  std::cout << "One sided upper limit at 95% CL: "<< upperLimit << std::endl;
  //file->Close();

  delete model;
}
