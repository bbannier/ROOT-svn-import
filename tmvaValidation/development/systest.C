#include "tmvaglob.C"

enum HistType { MVAType = 0, ProbaType = 1, RarityType = 2 };

 void systest( TString finRef = "sys/TMVApp_ref_ref.root", TString finSys = "sys/TMVApp_sysm_ref.root", 
               TString plotsdir = "plotsSys_ref",
               HistType htype = MVAType, bool useTMVAStyle=kTRUE )

// void systest( TString finRef = "sys/TMVApp_ref_sysm.root", TString finSys = "sys/TMVApp_sysm_sysm.root", 
//               TString plotsdir = "plotsSys_sysm",
//               HistType htype = MVAType, bool useTMVAStyle=kTRUE )
{
   // set style and remove existing canvas'
   TMVAGlob::Initialize( useTMVAStyle );

   // switches
   const Bool_t Draw_CFANN_Logy = kFALSE;
   const Bool_t Save_Images     = kTRUE;

   cout << "--- Opening file: " << finRef << endl;
   TFile* fileRef = new TFile( finRef );
   cout << "--- Opening file: " << finSys << endl;
   TFile* fileSys = new TFile( finSys );

   // define Canvas layout here!
   const Int_t width = 600;   // size of canvas

   // counter variables
   Int_t countCanvas = 0;
   char    fname[200];

   Int_t nvar = 4, countCanvas = 0;
   TString fnames[] = { "MVA_Fisher", "MVA_Likelihood", "MVA_LikelihoodD", "MVA_BDT" };

   for (Int_t ivar=0; ivar<nvar; ivar++) {

      cout << "--- Plotting method: " << fnames[ivar] << endl;

      TString title = fnames[ivar];
      title.ReplaceAll("MVA_","");
      TString ctitle = Form("TMVA output for classifier: %s", title.Data());
      TString atitle = Form("%s output", title.Data());
      TCanvas* c = new TCanvas( Form("canvas%d", countCanvas+1), ctitle, 
                                countCanvas*50+200, countCanvas*20, width, width*0.78 ); 

      TH1* refHist = (TH1*)fileRef->Get( fnames[ivar] );
      TH1* sysHist = (TH1*)fileSys->Get( fnames[ivar] );

      // compute normalisation factor
      
      refHist->GetXaxis()->SetTitle( atitle );
      refHist->GetYaxis()->SetTitle( "Events" );
      refHist->GetXaxis()->SetTitleOffset( 1.25 );
      refHist->GetYaxis()->SetTitleOffset( 1.22 );
      refHist->SetLineWidth( 3 );
      refHist->SetLineColor( 4 );
      refHist->Draw();

      sysHist->SetLineWidth( 3 );
      sysHist->SetLineColor( 2 );
      sysHist->Draw("samehist");

      // Draw legend               
      TLegend *legend= new TLegend( c->GetLeftMargin(), 1 - c->GetTopMargin() - 0.12, 
                                    c->GetLeftMargin() + 0.3, 1 - c->GetTopMargin() );
      legend->SetFillStyle( 1 );
      legend->AddEntry(refHist,"Reference","L");
      legend->AddEntry(sysHist,"Systematic shift","L");
      legend->SetBorderSize(1);
      legend->SetMargin( 0.3 );
      legend->Draw("same");
      
      // save canvas to file
      c->Update();
      //TMVAGlob::plot_logo();
      if (Save_Images) {
         TMVAGlob::imgconv( c, Form("%s/mva_%s", plotsdir.Data(), title.Data()) );
      }
      countCanvas++;
   }
}
