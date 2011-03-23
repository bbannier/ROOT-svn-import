namespace RooStats {
  class TestStatistic {
    // Main interface to evaluate the test statistic on a dataset
    virtual Double_t Evaluate(RooAbsData& data, RooArgSet& paramsOfInterest);

    // what is this called (for plotting)
    virtual const TString GetVarName() const = 0;
  }
}
