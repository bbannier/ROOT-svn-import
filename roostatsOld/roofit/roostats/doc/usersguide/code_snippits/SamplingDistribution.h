namespace RooStats {
 class SamplingDistribution : public TNamed {

    // Constructor for SamplingDistribution
    SamplingDistribution(const char *name,const char *title, 
			 std::vector<Double_t>& samplingDist, 
			 const TString varName = 0);

    // Constructor with weighted samples
    SamplingDistribution(const char *name,const char *title,
			 std::vector<Double_t>& samplingDist, 
			 std::vector<Double_t>& sampleWeights, 
			 const TString varName = 0);

    // get the inverse of the Cumulative distribution function
    Double_t InverseCDF(Double_t pvalue);

    // get the inverse of the Cumulative distribution function
    Double_t InverseCDFInterpolate(Double_t pvalue);
  
    // merge two sampling distributions
    void Add(SamplingDistribution* other);
    
    // get values of test statistic
    const std::vector<Double_t> & GetSamplingDistribution() const;

    // get the sampling weights 
    const std::vector<Double_t> & GetSampleWeights() const;

    // for plotting
    const TString GetVarName() const {return fVarName;}
    
  };
}

#endif
