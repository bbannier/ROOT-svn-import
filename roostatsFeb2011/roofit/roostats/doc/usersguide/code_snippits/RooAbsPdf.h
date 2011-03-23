class RooAbsPdf : public RooAbsReal {
  // raw, unnormalized evaluation of the pdf.  
  // the user implements this when creating new pdfs
  virtual Double_t evaluate() const = 0 ;

  // Toy MC generation
  RooDataSet *generate(const RooArgSet &whatVars, Int_t nEvents, const RooCmdArg& arg1, ...);
  
  RooDataHist *generateBinned(const RooArgSet &whatVars, Double_t nEvents, const RooCmdArg& arg1, ...);

  // -log(L) fits to binned and unbinned data
  virtual RooFitResult* fitTo(RooAbsData& data, RooCmdArg arg1,...);

  // return object representing -log(L), same signature as fitTo
  virtual RooAbsReal* createNLL(RooAbsData& data, ...);
  
  // Project p.d.f into lower dimensional p.d.f
  virtual RooAbsPdf* createProjection(const RooArgSet& iset) ;  

  // Create cumulative density function from p.d.f
  RooAbsReal* createCdf(const RooArgSet& iset, const RooArgSet& nset=RooArgSet()) ;

  // Support for extended maximum likelihood, switched off by default
  enum ExtendMode { CanNotBeExtended, CanBeExtended, MustBeExtended } ;
  virtual ExtendMode extendMode() const ;

  // if extended, how many events are expected
  virtual Double_t expectedEvents(const RooArgSet* nset) const ; 
};
