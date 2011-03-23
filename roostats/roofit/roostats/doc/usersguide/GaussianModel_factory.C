{
  // use the workspace factory to do the same thing
  RooWorkspace wspace("wspace");
  wspace.factory("Gaussian::G(x[150,100,200],mu[150,130,170], sigma[5,0,20])");
  wspace.pdf("G")->graphVizTree("GaussianModel_factory.dot", ":",true,true);
}
