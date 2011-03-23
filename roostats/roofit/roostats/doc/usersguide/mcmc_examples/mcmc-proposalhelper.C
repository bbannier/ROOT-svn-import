RooFitResult* fit = model->fitTo(*data);

// Easy ProposalFunction construction with ProposalHelper
ProposalHelper ph;
ph.SetVariables(fit->floatParsFinal());
ph.SetCovMatrix(fit->covarianceMatrix());
ph.SetUpdateProposalParameters(kTRUE); // auto-create mean vars and add mappings
ph.SetCacheSize(100);
ProposalFunction* pf = ph.GetProposalFunction();
