TestPoisson(){
  

  double a = 5;
  double b = 10;
  TRandom3 r;

  TH2F* hist = new TH2F("hist","",100,0,.01,100,0,1E-6);
  TH2F* histL12 = new TH2F("histL12","",100,0,20,100,0,20);
  TH2F* histLT = new TH2F("histLT","",100,0,20,100,0,20);
  for(int i=0; i<100; ++i){
    int n1 = r.Poisson(a);
    int n2 = r.Poisson(b);
    
    double p12 = TMath::Poisson(n1,a)*TMath::Poisson(n2,b);
    double pT = TMath::Poisson(n1+n2,a+b)*pow(a/(a+b),n1)*pow(b/(a+b),n2);

    //    cout << p12 << " " << pT << endl;
    hist->Fill(p12,pT);
  }

  int n1 = r.Poisson(a);
  int n2 = r.Poisson(b);
  double min12 = -1E6;
  double minT = -1E6;
  cout << "n1,n2 " << n1 << " " << n2 << endl;
  for(double a=0.1; a<20; a+=.2){
    for(double b=0.1; b<20; b+=.2){
      
      double p12 = TMath::Poisson(n1,a)*TMath::Poisson(n2,b);
      double pT = TMath::Poisson(n1+n2,a+b)*pow(a/(a+b),n1)*pow(b/(a+b),n2);

      cout << p12 << " " << pT << endl;
      if(p12>min12) min12=p12;
      if(pT>minT)   minT=pT;
    }
  }
  cout << "min12, minT " << min12 << " " << minT << endl;

  for(double a=0.1; a<20; a+=.2){
    for(double b=0.1; b<20; b+=.2){
      
      double p12 = TMath::Poisson(n1,a)*TMath::Poisson(n2,b);
      double pT  = TMath::Poisson(n1+n2,a+b)*pow(a/(a+b),n1)*pow(b/(a+b),n2);

      //cout << p12 << " " << pT << endl;
      histL12->Fill(a,b, -log(p12/min12));
      histLT->Fill(a,b, -log(pT/minT));
    }
  }

  histL12->SetMaximum(5);
  histLT->SetMaximum(5);

  TCanvas* c1 = new TCanvas();
  c1->Divide(2,2);
  c1->cd(1);
  histL12->SetLineColor(kRed);
  histL12->Draw("cont3");
  histLT->SetLineColor(kBlue);
  histLT->SetLineStyle(kDashed);
  histLT->Draw("cont3,same");
  c1->cd(2);
  histLT->Draw("cont3");
  c1->cd(3);
  histLT->Draw("surf");
  c1->cd(4);
  histL12->Draw("surf");
}



RooTestPoisson(){
  
  RooRealVar n1("n1","",0,100);
  RooRealVar n2("n2","",0,100);

  RooRealVar a("a","",5,0,100);
  RooRealVar b("b","",10,0,100);

  RooPoisson p1("p1","",n1,a);
  RooPoisson p2("p2","",n2,b);

  RooAddition
  RooPoisson pT("pT","",n1,a);


}
