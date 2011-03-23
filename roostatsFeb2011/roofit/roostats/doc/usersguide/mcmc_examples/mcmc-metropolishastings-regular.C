RooAbsReal* function = new RooGaussian("gauss", "gauss" x, mu, sigma);
RooArgSet vars(x);

// make our MetropolisHastings object
MetropolisHastings mh;
mh.SetFunction(*function);       // function to sample
mh.SetType(MetropolisHastings::kRegular);
mh.SetSign(MetropolisHastings::kPositive);
mh.SetProposalFunction(proposalFunction);
mh.SetParameters(vars);
mh.SetNumIters(10000);
MarkovChain* chain = mh.ConstructChain();
