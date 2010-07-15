#include "tmvaglob.C"
#include "TControlBar.h"
#include <sstream>
#include <string>

void BPDEFoamControlPlots( TString fin = "TMVA.root", 
			   bool useTMVAStyle=kTRUE )
{
   // set style and remove existing canvas'
   TMVAGlob::Initialize( useTMVAStyle );
   TH1::AddDirectory(kFALSE);

   cout << "read file: " << fin << endl;
   TFile *file = TFile::Open(fin);

   // get all titles of the method Boost
   TList titles;
   UInt_t ninst = TMVAGlob::GetListOfTitles("Method_BPDEFoam",titles);
   if (ninst==0) {
      cout << "Could not locate directory 'Method_BPDEFoam' in file " << fin << endl;
      return;
   }
   // loop over all titles
   TIter keyIter(&titles);
   TDirectory *boostdir;
   TKey *key;
   while ((key = TMVAGlob::NextKey(keyIter,"TDirectory"))) {
      boostdir = (TDirectory *)key->ReadObj();
      const TString titName = boostdir->GetName();
      cout << "Found directory: " << titName << endl;
      if (titName == "BPDEFoam"){

	 cout << "entered: " << titName.Data() << endl;

	 // Draw control plots
	 const Int_t nPlots = 4;
	 TString hname[nPlots]={"Booster_ROCIntegral","Booster_Separation",
				"Booster_MisclassRate","Booster_OrigMisclassRate"};

	 cout << "create canvas" << endl;

	 TCanvas *c = new TCanvas( TString("cv_")+titName,  
				   Form( "%s Control Plots", titName.Data() ),
				   900, 900 ); 
	 c->Divide(2,2);

	 cout << "draw boosting control plots" << endl;

	 for (Int_t i=0; i<nPlots; i++){
	    Int_t color = 4; 
	    TPad * cPad = (TPad*)c->cd(i+1);
	    TH1 *h = (TH1*) boostdir->Get(hname[i]);
	    if (h) {
	       TString plotname = h->GetName();
	       h->SetMaximum( TMath::Min(1.0, h->GetMaximum()*1.05 ) );
	       h->SetMinimum( TMath::Max(0.0, h->GetMinimum()*0.95 ) );
	       h->SetMarkerColor(color);
	       h->SetMarkerSize( 0.7 );
	       h->SetMarkerStyle( 24 );
	       h->SetLineWidth(2);
	       h->SetLineColor(color);
	       h->Draw();
	       c->Update();
	    }
	    else
	       cout << "Error: histogram " << hname[i] << " not found!" << endl;
	 }
	 
	 cout << "draw MVA plots" << endl;
	 
	 // ------ Draw all MVA plots (classifier response)

	 // loop over all titles
	 TObjString *fBoostNum_os = (TObjString*) boostdir->Get("fBoostNum");
	 Int_t fBoostNum = 0;
	 if (fBoostNum_os)
	    fBoostNum = fBoostNum_os->String().Atoi();
	 else {
	    cout << "Number of boosts not found!" << endl;
	    return;
	 }
	 cout << "Number of boosts: " << fBoostNum << endl;

	 // loop over all boosts
	 for (Int_t i=0; i<=fBoostNum; i++){
	    // draw only every 10th histogram (plus 1st and last one)
	    if ((i!=1) && (i!=fBoostNum) && (i % 10 != 0)) 
	       continue;

	    PlotSigBg(boostdir,
		      Form("%s_MVA_testing_S_%i",titName.Data(),i), 
		      Form("%s_MVA_testing_B_%i",titName.Data(),i), 
		      Form("cv_Boost_%i",i),
		      Form("%s MVA distr. boost %i",titName.Data(),i));

	    PlotSigBg(boostdir,
		      Form("Booster_ReweightedTrainingSample_var1_S_%i",i), 
		      Form("Booster_ReweightedTrainingSample_var1_B_%i",i), 
		      Form("cv_RW_TrainingSample_%i",i),
		      Form("Reweighted training sample boost %i",i),
		      Form("BoostWeights_var1_%i",i) );
	 }

      } // if (titName == "BPDEFoam")
   } // loop over all keys
}


void PlotSigBg(
   TDirectory *boostdir,
   TString name_s, 
   TString name_b, 
   TString name, 
   TString title,
   TString gname = "" )
{
   cout << "Draw histogram: " <<  name_s << ", " << name_b << endl;

   // read histograms from file
   TH1F *hS = dynamic_cast<TH1F*>( boostdir->Get(name_s) );
   TH1F *hB = dynamic_cast<TH1F*>( boostdir->Get(name_b) );

   // check if histograms were found
   if ( !(hS && hB) ) {
      cout << "Error: Histograms not found: "
	   << name_s << "=" << hS << " "
	   << name_b << "=" << hB << endl;
      return;
   }

   // create canvas
   TCanvas *cSB = new TCanvas( name, title, 500, gname == "" ? 500 : 1000 ); 
   if (gname != "") {
      cSB->Divide(1,2); // to draw the graph below
      cSB->cd(1);
   }

   // normalise both signal and background
   TMVAGlob::NormalizeHists( hS, hB );
   TMVAGlob::SetSignalAndBackgroundStyle( hS, hB );

   // Draw histograms
   hS->Rebin(2);
   hS->Draw("hist");
   hB->Rebin(2);
   hB->Draw("SAME HIST");

   // Draw legend               
   TLegend *legend= new TLegend( cSB->GetLeftMargin() + 0.2, 
				 1 - cSB->GetTopMargin() - 0.12, 
				 cSB->GetLeftMargin() + 0.6, 
				 1 - cSB->GetTopMargin() );
   legend->SetFillStyle( 1 );
   legend->AddEntry(hS,TString("Signal"),     "F");
   legend->AddEntry(hB,TString("Background"), "F");
   legend->SetBorderSize(1);
   legend->SetMargin( 0.3 );
   legend->Draw("same");
   
   if (gname != "") {
      cSB->cd(2);
      // read graph from file
      TGraph *g = dynamic_cast<TGraph*>( boostdir->Get(gname) );

      // check if graph was found
      if ( !g ) {
	 cout << "Error: Graph not found: "
	      << gname << "=" << g << endl;
	 return;
      }

      // dummy histogram for axis range
      TH2F *hpx = new TH2F("hpx",gname+";var1;boost weight",10,
			   hS->GetXaxis()->GetXmin(),
			   hS->GetXaxis()->GetXmax(),
			   10,0.0,1.5);
      hpx->SetStats(kFALSE);  // no statistics
      hpx->Draw();
      g->GetXaxis()->SetTitle("X-Axis");
      g->GetYaxis()->SetTitle("Y-Axis");
      g->Draw("*");


      // // Draw graph
      // //cSB->GetPad()->RangeAxis();
      // g->Draw("A*");
   }

   cSB->Update();
}
