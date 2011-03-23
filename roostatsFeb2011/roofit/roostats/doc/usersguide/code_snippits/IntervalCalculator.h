namespace RooStats {
   class IntervalCalculator { 

      // Main interface to get a ConfInterval, pure virtual
      virtual ConfInterval* GetInterval() const = 0; 

      // Get the size of the test (eg. rate of Type I error)
      virtual Double_t Size() const = 0;

      // Get the Confidence level for the test
      virtual Double_t ConfidenceLevel()  const = 0;  

      // Set the DataSet ( add to the the workspace if not already there ?)
      virtual void SetData(RooAbsData&) = 0;

      // Set the Model 
      virtual void SetModel(const ModelConfig & /* model */) = 0; 

      // set the size of the test (rate of Type I error) ( e.g. 0.05 for a 95% Confidence Interval)
      virtual void SetTestSize(Double_t size) = 0;

      // set the confidence level for the interval (e.g. 0.95 for a 95% Confidence Interval)
      virtual void SetConfidenceLevel(Double_t cl) = 0;
      
   };
}

