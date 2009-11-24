#include "TMath.h"
#include "TTree.h"
#include "TRandom.h"
#include "TH1F.h""
#include "Riostream.h"
#include "TCanvas.h"

TRandom R;

TTree* CreateData( float& var, int nevents = 1000 )
{
   TTree* t = new TTree( "t", "" );
   t->Branch( "var", &var, "var/F" ); 
   
   for (int i=0; i<nevents; i++) {
//       var = R.Gaus();
//       if (var<0) var *= 2;
//       else       var /= 2.;

      float r = R.Rndm();
      float m = R.Rndm();
      var = 0.5*(r + m);

      t->Fill();
   }

   return t;
}


void Gaussianise()
{
   float v;
   TTree* t = CreateData( v, 100000 );
   
   // event loop
   float xmin = t->GetMinimum( "var" );
   float xmax = t->GetMaximum( "var" );
   cout << "Min / max: " << xmin << " / " << xmax << endl;

   int nbinH = 100000;
   int nbin  = 60;
   TH1F* hhigh = new TH1F( "hhigh", "hhigh", nbinH, xmin, xmax );
   TH1F* horig = new TH1F( "horig", "horig", nbin, xmin, xmax );
   TH1F* hflat = new TH1F( "hflat", "hflat", nbin, 0, 1 );
   TH1F* hgaus = new TH1F( "hgaus", "hgaus", nbin, -5, 5 );
   t->Draw("var>>hhigh","","0");
   t->Draw("var>>horig","","0");
   hhigh->Scale( 1./hhigh->GetEntries() );

   TTree* u = CreateData( v, 1000 );

   for (int i=0; i<u->GetEntries(); i++) {
      u->GetEntry(i);      
      int nmax = hhigh->FindBin(v);      

      // create flat distribution
      double sum = 0;
      for (int ibin=1; ibin<=nmax; ibin++) sum += hhigh->GetBinContent(ibin);
      hflat->Fill( sum, 1 );

      // create Gaussian
      double gauss = sqrt(2.)*TMath::ErfInverse(2*sum - 1);
      hgaus->Fill( gauss, 1 );
   }

   TCanvas* c = new TCanvas( "c", "c", 0, 0, 700, 700 );
   c->Divide( 2, 2 );

   c->cd(1);
   hhigh->Draw();

   c->cd(2);
   horig->Draw();

   c->cd(3);
   hflat->Sumw2();
   hflat->Scale( 1./hflat->GetEntries() );
   hflat->SetMinimum(0);
   hflat->Draw("e");
   hflat->Fit("pol0","e");

   c->cd(4);
   hgaus->Draw("e");   
   hgaus->Fit("gaus","e");

   delete t;
   delete u;
}
