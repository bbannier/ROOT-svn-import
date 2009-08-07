// A simple ProposalFunction
ProposalFunction* pf = new UniformProposal();

MCMCCalculator mc;
mc.SetData(data);
mc.SetPdf(model);
mc.SetParameters(parametersOfInterest);
mc.SetProposalFunction(*pf);
mc.SetNumIters(100000);         // Metropolis-Hastings algorithm iterations
mc.SetNumBurnInSteps(50);       // first N steps to be ignored as burn-in
mc.SetNumBins(50);              // bins to use for RooRealVars in histograms
mc.SetTestSize(.1);             // 90% confidence level
mc.SetUseKeys(true);            // Use kernel estimation to determine interval
ConfInterval* mcmcInterval = mc.GetInterval();
