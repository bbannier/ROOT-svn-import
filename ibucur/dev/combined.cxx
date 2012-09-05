#include "RooSimultaneous.h"
#include "RooStats/CombinedLikelihood.h"
#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooWorkspace.h"
#include "RooAbsCategoryLValue.h"
#include "TIterator.h"

#include "RooStats/ModelConfig.h"
#include "RooStats/ProfileLikelihoodCalculator.h"
#include "RooStats/HypoTestResult.h"

int main()
{

   RooWorkspace w("w");

   w.factory("Gaussian::constr1(obs1[20,0,100], mean[0,100], 3)");
   w.factory("Gaussian::constr2(obs2[30,0,100], mean[0,100], 2)");

   w.factory("SIMUL::sim_pdf(index[cat1,cat2],cat1=constr1,cat2=constr2)");

   RooStats::ModelConfig* model = new RooStats::ModelConfig("ModelConfig", &w);
   model->SetObservables("obs1,obs2,index");
   model->SetParametersOfInterest("mean");
   model->SetPdf("sim_pdf");

   RooStats::ModelConfig* bModel = new RooStats::ModelConfig(*model);
   bModel->SetName("BModelConfig");
   

   RooSimultaneous* sim = (RooSimultaneous *)w.pdf("sim_pdf");
   RooDataSet* data = sim->generate(*model->GetObservables());

   
   w.var("mean")->setVal(0);
   model->SetSnapshot(*model->GetParametersOfInterest());
   w.var("mean")->setVal(40);
   bModel->SetSnapshot(*model->GetParametersOfInterest());
   //std::cout << "Data size " << data->numEntries() << std::endl;


   RooStats::ProfileLikelihoodCalculator plc(*data, *model);
   plc.SetNullParameters(*model->GetSnapshot());
   plc.SetAlternateParameters(*bModel->GetSnapshot());

   std::cout << plc.GetHypoTest()->Significance() << std::endl;

   RooStats::CombinedLikelihood* cl = new RooStats::CombinedLikelihood(sim, data, NULL);

   return 0;
}

