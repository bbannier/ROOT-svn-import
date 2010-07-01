/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Class  : OptimizerFOM                                                          *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description: Return different figures of merit (FOM) used in the               *
 *              automatic optimization of the Classifiers/Regressor parameters    *
 *                                                                                *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      Andreas Hoecker <Andreas.Hocker@cern.ch> - CERN, Switzerland              *
 *      Helge Voss      <Helge.Voss@cern.ch>     - MPI-K Heidelberg, Germany      *
 *                                                                                *
 * Copyright (c) 2005:                                                            *
 *      CERN, Switzerland                                                         * 
 *      MPI-K Heidelberg, Germany                                                 * 
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://ttmva.sourceforge.net/LICENSE)                                         *
 **********************************************************************************/

#include "TMVA/OptimizerFOM.h"
#include "TMVA/Tools.h"

ClassImp(TMVA::OptimizerFOM)

#include <limits>
#include "TMath.h"

#include "TMVA/PDF.h"   
//_______________________________________________________________________
TMVA::OptimizerFOM::OptimizerFOM(MethodBase* const method, TString fomType):
   fMethod(method),
   fFOMType(fomType),
   fMvaSig(NULL),
   fMvaBkg(NULL),
   fMvaSigFineBin(NULL),
   fMvaBkgFineBin(NULL)
{
   // Constructor which sets either "Classification or Regression"
   // and the type of Figure of Merit that you've chosen
   if (fMethod->DoRegression()){
      std::cout << " ERROR: Sorry, Regression is not yet implement for automatic parameter optimisation"
                << " --> exit" << std::endl;
      exit(1);
   }
}
//_______________________________________________________________________
Double_t TMVA::OptimizerFOM::GetFOM()
{
  // Return the Figure of Merit (FOM) used in the parameter 
  //  optimization process
  
   Double_t fom=0;
   if (fMethod->DoRegression()){
   }else{
      if      (fFOMType == "Separation")  fom = GetSeparation();
      else if (fFOMType == "ROCIntegral") fom = GetROCIntegral();
      else if (fFOMType == "SigEffAt01")  fom = GetSigEffAt(0.1);
      else if (fFOMType == "SigEffAt001") fom = GetSigEffAt(0.01);
      else {
         std::cout << " ERROR, you've specified as Figure of Merit in the \n"
                   << " parameter optimisation " << fFOMType << " which has not\n"
                   << " been implemented yet!! ---> exit " << std::endl;
         exit(1);
      }
   }
   return fom;
}

//_______________________________________________________________________
void TMVA::OptimizerFOM::GetMVADists()
{
   // fill the private histograms with the mva distributinos for sig/bkg

   if (fMvaSig) fMvaSig->Delete();
   if (fMvaBkg) fMvaBkg->Delete();
   if (fMvaSigFineBin) fMvaSigFineBin->Delete();
   if (fMvaBkgFineBin) fMvaBkgFineBin->Delete();
 
   // maybe later on this should be done a bit more clever (time consuming) by
   // first determining proper ranges, removing outliers, as we do in the 
   // MVA output calculation in MethodBase::TestClassifier...
   // --> then it might be possible also to use the splined PDF's which currently
   // doesn't seem to work

   fMvaSig        = new TH1D("fMvaSig","",100,-1.5,1.5); //used for spline fit
   fMvaBkg        = new TH1D("fMvaBkg","",100,-1.5,1.5); //used for spline fit
   fMvaSigFineBin = new TH1D("fMvaSigFineBin","",100000,-1.5,1.5);
   fMvaBkgFineBin = new TH1D("fMvaBkgFineBin","",100000,-1.5,1.5);

   const std::vector<Event*> events=fMethod->Data()->GetEventCollection(Types::kTesting);
   
   UInt_t signalClassNr = fMethod->DataInfo().GetClassInfo("Signal")->GetNumber();

   //   fMethod->GetTransformationHandler().CalcTransformations(fMethod->Data()->GetEventCollection(Types::kTesting));

   for (UInt_t iev=0; iev < events.size() ; iev++){
      //      std::cout << " GetMVADists event " << iev << std::endl;
      //      std::cout << " Class  = " << events[iev]->GetClass() << std::endl;
      //         std::cout << " MVA Value = " << fMethod->GetMvaValue(events[iev]) << std::endl;
      if (events[iev]->GetClass() == signalClassNr) {
         fMvaSig->Fill(fMethod->GetMvaValue(events[iev]),events[iev]->GetWeight());
      } else {
         fMvaBkg->Fill(fMethod->GetMvaValue(events[iev]),events[iev]->GetWeight());
      }
   }
}
//_______________________________________________________________________
Double_t TMVA::OptimizerFOM::GetSeparation()
{
   // return the searation between the signal and background 
   // MVA ouput distribution
   GetMVADists();
   if (1){
      PDF *splS = new PDF( " PDF Sig", fMvaSig, PDF::kSpline2 );
      PDF *splB = new PDF( " PDF Bkg", fMvaBkg, PDF::kSpline2 );
      return gTools().GetSeparation(*splS,*splB);
   }else{
      std::cout << "Separation caclulcaton via histograms (not PDFs) seems to give still strange results!! Don't do that, check!!"<<std::endl;
      return gTools().GetSeparation(fMvaSigFineBin,fMvaBkgFineBin); // somehow sitll gives strange results!!!! Check!!!
   }
}


//_______________________________________________________________________
Double_t TMVA::OptimizerFOM::GetROCIntegral() 
{
   // calculate the area (integral) under the ROC curve as a
   // overall quality measure of the classification
   //
   // makeing pdfs out of the MVA-ouput distributions doesn't work
   // reliably for cases where the MVA-ouput isn't a smooth distribution.
   // this happens "frequently" in BDTs for example when the number of
   // trees is small resulting in only some discrete possible MVA ouput values.
   // (I still leave the code here, but use this with care!!! The default
   // however is to use the distributions!!!

   GetMVADists();

   Double_t integral = 0;
   if (0){
      PDF *pdfS = new PDF( " PDF Sig", fMvaSig, PDF::kSpline2 );
      PDF *pdfB = new PDF( " PDF Bkg", fMvaBkg, PDF::kSpline2 );

      Double_t xmin = TMath::Min(pdfS->GetXmin(), pdfB->GetXmin());
      Double_t xmax = TMath::Max(pdfS->GetXmax(), pdfB->GetXmax());
      
      UInt_t   nsteps = 1000;
      Double_t step = (xmax-xmin)/Double_t(nsteps);
      Double_t cut = xmin;
      for (UInt_t i=0; i<nsteps; i++){
         integral += (1-pdfB->GetIntegral(cut,xmax)) * pdfS->GetVal(cut);
         cut+=step;
      } 
      integral*=step;
   }else{
      // sanity checks
      if ( (fMvaSigFineBin->GetXaxis()->GetXmin() !=  fMvaBkgFineBin->GetXaxis()->GetXmin()) ||
           (fMvaSigFineBin->GetNbinsX() !=  fMvaBkgFineBin->GetNbinsX()) ){
         std::cout << " Error in OptimizerFOM, unequal histograms for sig and bkg.." << std::endl;
         exit(1);
      }else{
          
         Double_t *cumulator  = fMvaBkgFineBin->GetIntegral();
         Int_t    nbins       = fMvaSigFineBin->GetNbinsX();
         // get the true signal integral (CompuetIntegral just return 1 as they 
         // automatically normalize. IN ADDITION, they do not account for variable
         // bin sizes (which you migh perhaps use later on for the fMvaSig/Bkg histograms)
         Double_t sigIntegral = 0;
         for (Int_t ibin=1; ibin<=nbins; ibin++){
            sigIntegral += fMvaSigFineBin->GetBinContent(ibin) * fMvaSigFineBin->GetBinWidth(ibin);
         }
         //gTools().NormHist( fMvaSigFineBin  ); // also doesn't  use variable bin width. And callse TH1::Scale, which oddly enough does not change the SumOfWeights !!!

         for (Int_t ibin=1; ibin <= nbins; ibin++){ // don't include under- and overflow bin
            integral += (cumulator[ibin]) * fMvaSigFineBin->GetBinContent(ibin)/sigIntegral * fMvaSigFineBin->GetBinWidth(ibin) ;
         }
      }
   }


   return integral;
         

}


//_______________________________________________________________________
Double_t TMVA::OptimizerFOM::GetSigEffAt(Double_t bkgEff) 
{
   // calculate the signal efficiency for a given background efficiency 

   GetMVADists();
   Double_t sigEff=0;

   // sanity checks
   if ( (fMvaSigFineBin->GetXaxis()->GetXmin() !=  fMvaBkgFineBin->GetXaxis()->GetXmin()) ||
        (fMvaSigFineBin->GetNbinsX() !=  fMvaBkgFineBin->GetNbinsX()) ){
      std::cout << " Error in OptimizerFOM, unequal histograms for sig and bkg.." << std::endl;
      exit(1);
   }else{
      
      //Double_t bkgIntegral     = fMvaBkgFineBin->ComputeIntegral();
      Double_t *bkgCumulator   = fMvaBkgFineBin->GetIntegral();
      //Double_t sigIntegral     = fMvaSigFineBin->ComputeIntegral();
      Double_t *sigCumulator   = fMvaSigFineBin->GetIntegral();

      Int_t nbins=fMvaBkgFineBin->GetNbinsX();
      Int_t ibin=0;
   
      // std::cout << " bkgIntegral="<<bkgIntegral
      //           << " sigIntegral="<<sigIntegral
      //           << " bkgCumulator[nbins]="<<bkgCumulator[nbins]
      //           << " sigCumulator[nbins]="<<sigCumulator[nbins]
      //           << std::endl;

      while (bkgCumulator[nbins-ibin] > (1-bkgEff)) {
         sigEff = sigCumulator[nbins]-sigCumulator[nbins-ibin];
         ibin++;
      }
   } 

   return sigEff;
}


