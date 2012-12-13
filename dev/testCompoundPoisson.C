// test of a compound Poisson distribution

void testCompoundPoisson(double mu = 6.0, double a = 0.0, double b = 1.0, int nexp = 100000) { 



   TH1D * h1 = new TH1D("h1","h1",200,-300,300);

   for (int i = 0; i < nexp; ++i) { 

      double k = gRandom->PoissonD(mu); 
      double y = 0;

      for (int j = 0; j < k; j++)
         y += gRandom->Uniform(a, b); 

      h1->Fill(y); 
   }

//   h1->Draw();
//   double mean = (a + b) * mu / 2.0;
   double mean = (b - a) * mu / 2.0;
   double variance = (a * b + a * a + b * b) * mu / 3.0;
   double gamma = mean * mean / variance;
   double beta = variance / mean;
   std::cout << "gamma " << gamma << " beta " << beta << std::endl; 
   std::cout << "mean " << gamma * beta << " variance " << gamma * beta * beta << std::endl;
   std::cout << "mean estimate " << h1->GetMean() << " variance estimate " << h1->GetMeanError() * h1->GetMeanError() * nexp << std::endl;


   RooWorkspace w; 
   w.factory("x[-300,300]");
   w.var("x")->setVal((a + b) * mu / 2.0); // set to mean
   w.factory("gamma[0.01,1000]");
   w.var("gamma")->setVal(gamma);
   w.factory("beta[0.01,1000]");
   w.var("beta")->setVal(beta);
   w.factory("a[-300,300]");
   w.var("a")->setVal(-a);
   w.factory("Gamma:g(x,gamma,beta,a)");
   

   RooPlot * pl = w.var("x")->frame(); 
   w.pdf("g")->Print();
//   w.function("a")->Print();

   w.pdf("g")->getVariables()->Print("v");
   w.pdf("g")->plotOn(pl);

   new TCanvas(); 
   pl->Draw();

   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");

   TF1 * f1 = new TF1("f1","[0]*ROOT::Math::gamma_pdf(x, [1], [2], [3])");
   f1->SetParameters(nexp, gamma, beta, a);
   f1->SetLineColor(kRed);
   h1->Fit(f1,"+");



/*   
   RooDataSet * d = w.pdf("g")->generate(*w.var("x"), nexp);

   TH1D * h2 = new TH1D("h2","h2",200,0,1000);
   d->fillHistogram(h2,*w.var("x"));
   h2->Draw();

   TF1 * f0 = new TF1("f0","[0]*ROOT::Math::normal_pdf(x, [2]*sqrt([1]),[1]*[2])");
   f0->SetParameters(1000,10,30);   
   f0->SetLineColor(kBlue);
   h2->Fit(f0,"+");
   TF1 * f2 = new TF1("f2","[0]*ROOT::Math::negative_binomial_pdf(x, [1],[2])");
   f2->SetParameters(1000,0.03,30);
   f2->SetLineColor(kGreen);
   h2->Fit(f2,"+");

   TLegend * l = new TLegend(0.6,0.8,0.92,0.92);
   l->AddEntry(h2,"scaled Poisson toys");
   l->AddEntry(f1,"Gamma(x | neff, scale)");
   l->AddEntry(f2,"NegBin(x | p, r)");

   l->Draw();
*/
}
