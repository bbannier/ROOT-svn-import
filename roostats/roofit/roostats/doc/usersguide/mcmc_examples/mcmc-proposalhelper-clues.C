// assume bankOfClues is a RooDataSet with weighted "clues" as entries

RooArgSet vars(x,y);

ProposalHelper ph;
ph.SetVariables(vars);
ph.SetClues(bankOfClues);       // use bankOfClues to make a clues PDF
ph.SetCluesFraction(0.15);      // clues PDF accounts for 15% of PDF integral
ph.SetUpdateProposalParameters(kTRUE); // auto-create mean vars and add mappings
ph.SetCacheSize(100);
ProposalFunction* pf = ph.GetProposalFunction();
