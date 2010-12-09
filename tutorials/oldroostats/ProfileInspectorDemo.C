
#include "RooStats/ProfileInspector.h"

using namespace RooStats;
ProfileInspectorDemo(){
  
  Int_t dim = 4;
  Int_t nPOI = 1;
   RooArgList xVec;
   RooArgList muVec;
   RooArgSet poi;

   // make the observable and means
   Int_t i,j;
   RooRealVar* x;
   RooRealVar* mu_x;
   for (i = 0; i < dim; i++) {
      char* name = Form("x%d", i);
      x = new RooRealVar(name, name, 0, -3,3);
      xVec.add(*x);

      char* mu_name = Form("mu_x%d",i);
      mu_x = new RooRealVar(mu_name, mu_name, 0, -2,2);
      muVec.add(*mu_x);
   }

   // put them into the list of parameters of interest
   for (i = 0; i < nPOI; i++) {
      poi.add(*muVec.at(i));
   }

   // make a covariance matrix that is all 1's
   TMatrixDSym cov(dim);
   for (i = 0; i < dim; i++) {
      for (j = 0; j < dim; j++) {
         if (i == j) cov(i,j) = 3.;
	 else        cov(i,j) = pow(-1,i+j)*1.0;
      }
   }
   
   // now make the multivariate Gaussian
   RooMultiVarGaussian mvg("mvg", "mvg", xVec, muVec, cov);

   ///////////////////////////////////////////
   // make a toy dataset
   RooDataSet* data = mvg.generate(xVec, 100);

   ///////////////////////////////////////////////
   // now create the model config for this problem
   RooWorkspace* w = new RooWorkspace("MVG");
   ModelConfig modelConfig(w);
   modelConfig.SetPdf(mvg);
   modelConfig.SetParametersOfInterest(poi);
   modelConfig.GuessObsAndNuisance(*data);
   
   //////////////////////////////////////////////
   // now use the profile inspector
   ProfileInspector p;
   TList* list = p.GetListOfProfilePlots(*data,&modelConfig);
   
   // now make plots
   TCanvas* c1 = new TCanvas("c1","ProfileInspectorDemo",800,200);
   c1->Divide(list->GetSize());
   for(int i=0; i<list->GetSize(); ++i){
     c1->cd(i+1);
     list->At(i)->Draw("al");
   }
}
