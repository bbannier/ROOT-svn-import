RooAbsReal* nll = pdf->createNLL(*data);
RooArgSet* vars = nll->getParameters(*data);
RemoveConstantParameters(vars); // to be safe

MetropolisHastings mh;
mh.SetFunction(*nll);       // function to sample
mh.SetType(MetropolisHastings::kLog);
mh.SetSign(MetropolisHastings::kNegative);
mh.SetProposalFunction(proposalFunction);
mh.SetParameters(*vars);
mh.SetNumIters(10000);
MarkovChain* chain = mh.ConstructChain();
