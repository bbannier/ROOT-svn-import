//////////////////////////////////////////////////////////////////////////
//
// RooStats tutorial macro #501
// 2009/08 - Nils Ruthmann, Gregory Schott
//
// Shows how to run the RooStats classes to perform specific
// tasks. The ROOT file containing a workspace holding the models,
// data and other objects needed to run can be prepared with any of
// the rs500*.C tutorial macros.
//
// 
//
//////////////////////////////////////////////////////////////////////////

using namespace RooFit;
using namespace RooStats;


void rs501_ProfileLikelihoodCalculator_limit(const char* fname="WS_GaussOverFlat.root",const char* outputplot="pnll.ps")
{

  TFile* file =new TFile(fname);
  RooWorkspace* my_WS = (RooWorkspace*) file->Get("myWS");
  //Import the objects needed
  RooAbsPdf* modeltmp=my_WS->pdf("model");
  RooAbsData* data=my_WS->data("data");
  RooAbsPdf* priorNuisance=my_WS->pdf("priorNuisance");
  RooArgSet* paramInterestSet=my_WS->set("POI");
  RooRealVar* paramInterest=paramInterestSet->first();
  RooArgSet* nuisanceParam=my_WS->set("parameters");

  //If there are nuisance parameters, multiply their prior to the full model
  if(priorNuisance!=0) {
    RooProdPdf* model=new RooProdPdf("constrainedModel","Model with nuisance parameters",*modeltmp,*priorNuisance);
  }
  else{
    RooProdPdf* model=modeltmp;
  }

  //Set up the nll
  ProfileLikelihoodCalculator plc;
  plc.SetPdf(*model);
  plc.SetData(*data);
  RooArgSet POI(*paramInterest);
  plc.SetParameters(POI);
  plc.SetTestSize(0.1); //ProfileLikelihoodCalculator usually make intervals. 95% Upperlimit is the same as the 90% upper limit of an interval  
  ConfInterval* pllint=plc.GetInterval();
  model->fitTo(*data);
  
  TCanvas* c1=new TCanvas();
  c1->cd();
  
  //Receive the function
  RooAbsReal* newProfile = ((LikelihoodInterval*)pllint)->GetLikelihoodRatio();
  //Get the limit
  const Double_t upperlimit = ((LikelihoodInterval*)pllint)->UpperLimit(*paramInterest);
  
  //Make a control plot
  RooPlot *frame = paramInterest->frame();
  newProfile->plotOn(frame);
  TLine *upperlimitline = new TLine(upperlimit,0.,upperlimit,1.5);
  upperlimitline->SetLineColor(2);
  upperlimitline->SetLineWidth(6);
  frame->addObject(upperlimitline);
  frame->SetMinimum(0.);
  frame->SetMaximum(1.36+2);
  frame->GetYaxis()->SetTitle("- log #lambda");
  frame->Draw();
  c1->Print(outputplot);
  std::cout<<"A plot was saved at"<<outputplot <<std::endl;
  std::cout<<"One sided upper limit at 95\% CL:"<<upperlimit<<std::endl; 
  file->Close();
  
}
