namespace RooStats {

  // returns one-sided significance corresponding to a p-value
  inline Double_t PValueToSignificance(Double_t pvalue);

  // returns p-value corresponding to a 1-sided significance
  inline Double_t SignificanceToPValue(Double_t Z);

  // remove from a set all parameters that are set to constant (ie. "fixed" )
  inline void RemoveConstantParameters(RooArgSet* set);

  // Assuming all values in set are RooRealVars, randomize their values.
  inline void RandomizeCollection(RooAbsCollection& set,
                                  Bool_t randomizeConstants = kTRUE);
}
