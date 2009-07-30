{
  // Make observable and parameters
  RooRealVar x("x","x", 150,100,200);
  RooRealVar mu("mu","#mu", 150,130,170);
  RooRealVar sigma("sigma","#sigma", 5,0,20);

  // make a simple model
  RooGaussian G("G","G",x, mu, sigma);

  // make graph to represent model (using GraphViz and latex formatting)
  G.graphVizTree("GaussianModel.dot", ":",true, true);
}
