namespace RooStats {
   class HypoTestCalculator {

      // main interface to get a HypoTestResult, pure virtual
      virtual HypoTestResult* GetHypoTest() const = 0;

      // Set the model for the null hypothesis
      virtual void SetNullModel(const ModelConfig& model) = 0;

      // Set the model for the alternate hypothesis
      virtual void SetAlternateModel(const ModelConfig& model) = 0;

      // Set the DataSet
      virtual void SetData(RooAbsData& data) = 0;

      // Set a common model for both the null and alternate
      virtual void SetCommonModel(const ModelConfig& model) ;
   }
}
