#include "tmvaglob.C"
#include "TControlBar.h"
#include "TMap.h"

#include <sstream>
#include <string>
#include <cfloat>

#include "TMVA/PDEFoam.h"

void PlotFoams( TString fin = "weights/TMVAClassification_PDEFoam.weights_foams.root", 
                bool useTMVAStyle=kTRUE )
{
   cout << "read file: " << fin << endl;
   TFile *file = TFile::Open(fin);

   // set style and remove existing canvas'
   TMVAGlob::Initialize( useTMVAStyle );

   // create control bar
   TControlBar* cbar = new TControlBar( "vertical", "Choose cell value for plot:", 50, 50 );
   if ((gDirectory->Get("SignalFoam") && gDirectory->Get("BgFoam")) || 
       gDirectory->Get("MultiTargetRegressionFoam")) {
      TString macro = Form( "Plot(\"%s\", TMVA::kValueDensity, \"Event density\", %s)", 
			    fin.Data(), (useTMVAStyle ? "kTRUE" : "kFALSE") );
      cbar->AddButton( "Event density", macro, "Plot event density", "button" );
   } else if (gDirectory->Get("DiscrFoam") || gDirectory->Get("MultiClassFoam0")){
      TString macro = Form( "Plot(\"%s\", TMVA::kValue, \"Discriminator\", %s)", 
			    fin.Data(), (useTMVAStyle ? "kTRUE" : "kFALSE") );
      cbar->AddButton( "Discriminator", macro, "Plot discriminator", "button" );
   } else if (gDirectory->Get("MonoTargetRegressionFoam")){
      TString macro = Form( "Plot(\"%s\", TMVA::kValue, \"Target\", %s)", 
			    fin.Data(), (useTMVAStyle ? "kTRUE" : "kFALSE") );
      cbar->AddButton( "Target", macro, "Plot target", "button" );
   } else {
      cout << "Error: no foams found in file: " << fin << endl;
      return;
   }
   
   TString macro_rms = Form( "Plot(\"%s\", TMVA::kRms, \"RMS\", %s)", 
			     fin.Data(), (useTMVAStyle ? "kTRUE" : "kFALSE") );
   cbar->AddButton( "RMS", macro_rms, "Plot RMS (Variance)", "button" );
   TString macro_rms_ov_mean = Form( "Plot(\"%s\", TMVA::kRmsOvMean, \"RMS/Mean\", %s)", 
				     fin.Data(), (useTMVAStyle ? "kTRUE" : "kFALSE") );
   cbar->AddButton( "RMS over Mean", macro_rms_ov_mean, "Plot RMS over Mean", "button" );

   cbar->Show();
   file->Close();
}

// foam plotting macro
void Plot( TString fin = "weights/TMVAClassification_PDEFoam.weights_foams.root", 
	   TMVA::ECellValue cv, TString cv_long, bool useTMVAStyle=kTRUE )
{
   cout << "read file: " << fin << endl;
   TFile *file = TFile::Open(fin);

   gStyle->SetNumberContours(999);
   if (useTMVAStyle)  TMVAGlob::SetTMVAStyle();

   // find foams and foam type
   TList foam_list; // the foams and their captions
   if (gDirectory->Get("SignalFoam") && gDirectory->Get("BgFoam")){
      foam_list.Add(new TPair(SignalFoam, new TObjString("Signal Foam")));
      foam_list.Add(new TPair(BgFoam, new TObjString("Background Foam")));
   } else if (gDirectory->Get("DiscrFoam")){
      foam_list.Add(new TPair(DiscrFoam, new TObjString("Discriminator Foam")));
   } else if (gDirectory->Get("MultiClassFoam0")){
      UInt_t cls = 0;
      TMVA::PDEFoam *fm = NULL;
      while (fm = (TMVA::PDEFoam*) gDirectory->Get(Form("MultiClassFoam%u", cls))) {
	 foam_list.Add(new TPair(fm, new TObjString(Form("Discriminator Foam %u",cls))));
	 cls++;
      }
   } else if (gDirectory->Get("MonoTargetRegressionFoam")){
      foam_list.Add(new TPair(MonoTargetRegressionFoam, 
			      new TObjString("MonoTargetRegression Foam")));
   } else if (gDirectory->Get("MultiTargetRegressionFoam")){
      foam_list.Add(new TPair(MultiTargetRegressionFoam, 
			      new TObjString("MultiTargetRegression Foam")));
   } else {
      cout << "ERROR: no Foams found in file: " << fin << endl;
      return;
   }

   // loop over all foams and print out a debug message
   TListIter foamIter(&foam_list);
   TPair *fm_pair = NULL;
   Int_t kDim; // foam dimensions
   while (fm_pair = (TPair*) foamIter()) {
      kDim = ((TMVA::PDEFoam*) fm_pair->Key())->GetTotDim();
      cout << "Foam loaded: " << ((TObjString*) fm_pair->Value())->String()
	   << " (dimension = " << kDim << ")" << endl;
   }

   // kernel to use for the projection
   TMVA::PDEFoamKernel *kernel = new TMVA::PDEFoamKernel();

   // ********** plot foams ********** //
   if (kDim == 1){
      // draw histogram
      TCanvas *canv = NULL; // the canvas
      TH1D *proj = NULL;    // the foam projection

      // loop over all foams and draw the projection
      TListIter it(&foam_list); // the iterator
      TPair *fm_pair = NULL;    // the (foam, caption) pair
      while (fm_pair = (TPair*) it()) {
	 TMVA::PDEFoam *foam = (TMVA::PDEFoam*) fm_pair->Key();
	 TString foam_capt(((TObjString*) fm_pair->Value())->String());

	 canv = new TCanvas(Form("canvas_%u",foam), "1-dimensional PDEFoam", 400, 400);

	 TString var_name = foam->GetVariableName(0)->String();
	 proj = foam->Draw1Dim(cv, 100, kernel);
	 proj->SetTitle(cv_long+" of "+foam_capt+";"+var_name);
	 proj->Draw();
	 proj->SetDirectory(0);

	 canv->Update();
      } // loop over foams
   } else { 
      // if dimension of foam > 1, draw foam projections
      TCanvas *canv = NULL; // the canvas
      TH2D *proj = NULL;    // the foam projection

      // loop over all foams and draw the projection
      TListIter it(&foam_list); // the iterator
      TPair *fm_pair = NULL;    // the (foam, caption) pair
      while (fm_pair = (TPair*) it()) {
	 TMVA::PDEFoam *foam = (TMVA::PDEFoam*) fm_pair->Key();
	 TString foam_capt(((TObjString*) fm_pair->Value())->String());

	 // draw all possible projections (kDim*(kDim-1)/2)
	 for(Int_t i=0; i<kDim; i++){
	    for (Int_t k=i+1; k<kDim; k++){

	       // create canvas
	       canv = new TCanvas(Form("canvas_%u_%i:%i",foam,i,k),
				  Form("Foam projections %i:%i",i,k),
				  (Int_t)(400/(1.-0.2)), 400);
	       canv->SetRightMargin(0.2);

	       TString title_proj = Form("%s of %s: Projection %s:%s;%s;%s",
					 cv_long.Data(),
					 foam_capt.Data(),
					 foam->GetVariableName(i)->String().Data(),
					 foam->GetVariableName(k)->String().Data(),
					 foam->GetVariableName(i)->String().Data(),
					 foam->GetVariableName(k)->String().Data() );

	       // do projections
	       proj = foam->Project2(i, k, cv, kernel);
	       proj->SetTitle(title_proj);
	       proj->Draw("COLZ"); // CONT4Z
	       proj->SetDirectory(0);

	       canv->Update();
	    } // loop over all possible projections
	 } // loop over all possible projections
      } // loop over foams
   } // if dimension > 1

   file->Close();
}
