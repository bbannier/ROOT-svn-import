class RooAbsReal : public RooAbsArg {
  // Return value and unit accessors
  virtual Double_t getVal(const RooArgSet* set=0) const ;

  // Analytical integration support
  virtual Double_t analyticalIntegral(Int_t code,const char* rangeName=0) const;

  // create a function of fewer variables, eliminating others via profiling
  virtual RooAbsReal* createProfile(const RooArgSet& paramsOfInterest) ;

  // create a new RooAbsReal that is the integral of this function
  RooAbsReal* createIntegral(const RooArgSet& iset, 
			     const RooCmdArg arg1, ...) const;

  // User entry point for plotting
  virtual RooPlot* plotOn(RooPlot* frame, ...) const ;

  // Create 1,2, and 3D histograms from and fill it
  TH1 *createHistogram(const char *name, const RooAbsRealLValue& xvar,
                       const RooCmdArg& arg1=RooCmdArg::none(), ...) ;

  // ...
};
