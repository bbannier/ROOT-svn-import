using namespace RooFit;
PlotGammaConstraint(){

  TFile *_file0 = TFile::Open("results/example_channel1_LogNormExample_model.root");
  proto->exportToCint("proto");

  //  TCanvas* c1 = new TCanvas;
  //  c1->Divide(2);
  //  c1->cd(1);

  plot = proto::beta_syst2.frame();
  proto::beta_syst2Constraint.plotOn(plot);
  proto::alphaOfBeta_syst2.plotOn(plot);

  TFile *_file0 = TFile::Open("results/example_channel1_GammaExample_model.root");
  proto->exportToCint("proto");
  proto::beta_syst2Constraint.plotOn(plot,LineColor(kRed));
  proto::alphaOfBeta_syst2.plotOn(plot,LineColor(kRed));
  
  plot->Draw();

  //  c1->cd(1);

}
