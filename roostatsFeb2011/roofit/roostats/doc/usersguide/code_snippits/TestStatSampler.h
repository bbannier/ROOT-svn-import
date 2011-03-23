namespace RooStats {
   class TestStatSampler {

      // Main interface to get a SamplingDistribution, pure virtual
      virtual SamplingDistribution* GetSamplingDistribution(RooArgSet& paramsOfInterest); 

      // Main interface to evaluate the test statistic on a dataset
      virtual Double_t EvaluateTestStatistic(RooAbsData& data, RooArgSet& paramsOfInterest) ;

      // Get the TestStatistic
      virtual const TestStatistic* GetTestStatistic()  const = 0;  
    
      // Set the Pdf, etc. so we know how to sample and evaluate test stat
      virtual void SetModel(ModelConfig&) = 0;

      // Set the TestStatistic
      virtual void SetTestStatistic(TestStatistic& )  const = 0;  
      
   };
}
