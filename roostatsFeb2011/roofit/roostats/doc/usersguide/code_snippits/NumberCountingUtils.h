namespace RooStats{
   namespace  NumberCountingUtils {
 
  // Expected P-value for s=0 in a ratio of Poisson means.  
  // Here the background and its uncertainty are provided directly and 
  // assumed to be from the double Poisson counting setup described in the 
  // BinomialWithTau functions.  
  // Normally one would know tau directly, but here it is determiend from
  // the background uncertainty.  This is not strictly correct, but a useful 
  // approximation.
     Double_t BinomialExpZ(Double_t sExp, Double_t bExp, Double_t fractionalBUncertainty);

  // See BinomialWithTauExpP
     Double_t BinomialWithTauExpZ(Double_t sExp, Double_t bExp, Double_t tau);   
  // See BinomialObsP
     Double_t BinomialObsZ(Double_t nObs, Double_t bExp, Double_t fractionalBUncertainty);
  // See BinomialWithTauObsP
     Double_t BinomialWithTauObsZ(Double_t nObs, Double_t bExp, Double_t tau);
  // See BinomialExpP
     Double_t BinomialExpP(Double_t sExp, Double_t bExp, Double_t fractionalBUncertainty);

  // Expected P-value for s=0 in a ratio of Poisson means.  
  // Based on two expectations, a main measurement that might have signal
  // and an auxiliarly measurement for the background that is signal free.
  // The expected background in the auxiliary measurement is a factor
  // tau larger than in the main measurement.
     Double_t BinomialWithTauExpP(Double_t sExp, Double_t bExp, Double_t tau);

  // P-value for s=0 in a ratio of Poisson means.  
  // Here the background and its uncertainty are provided directly and 
  // assumed to be from the double Poisson counting setup.  
  // Normally one would know tau directly, but here it is determiend from
  // the background uncertainty.  This is not strictly correct, but a useful 
  // approximation.
     Double_t BinomialObsP(Double_t nObs, Double_t, Double_t fractionalBUncertainty);

  // P-value for s=0 in a ratio of Poisson means.  
  // Based on two observations, a main measurement that might have signal
  // and an auxiliarly measurement for the background that is signal free.
  // The expected background in the auxiliary measurement is a factor
  // tau larger than in the main measurement.
     Double_t BinomialWithTauObsP(Double_t nObs, Double_t bExp, Double_t tau);
   }
}
