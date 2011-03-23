namespace RooStats {
 class ModelConfig : public TNamed {
  /* getter methods */
   /// get model PDF (return NULL if pdf  does not exist)
   RooAbsPdf * GetPdf() const ;

   /// get parameters prior pdf  (return NULL if not existing) 
   RooAbsPdf * GetPriorPdf() const ;
   /// get Proto data set (return NULL if not existing) 
   RooAbsData * GetProtoData()  const ;
   /// get RooArgSet containing the parameter of interest (return NULL does not exist) 
   const RooArgSet * GetParametersOfInterest() const ;
   /// get RooArgSet containing the nuisance parameters (return NULL if not existing) 
   const RooArgSet * GetNuisanceParameters() const ;
   /// get RooArgSet containing the constraint parameters (return NULL if not existing) 
   const RooArgSet * GetConstraintParameters() const ;
   /// get RooArgSet for observales  (return NULL if not existing) 
   const RooArgSet * GetObservables() const ;
   /// get RooArgSet for conditional observales  (return NULL if not existing) 
   const RooArgSet * GetConditionalObservables() const ;
   /// get RooArgSet for parameters for a particular hypothesis  (return NULL if not existing) 
   const RooArgSet * GetSnapshot() const ;
   /// get the associated workspace
   const RooWorkspace * GetWS() const ;
    
   /* setter methods */
   // set a workspace that owns all the necessary components for the analysis
   virtual void SetWorkspace(RooWorkspace & ws);
   // Set the Pdf, add to the the workspace if not already there
   virtual void SetPdf(RooAbsPdf& pdf) ;
   // Set the Prior Pdf, add to the the workspace if not already there
   virtual void SetPriorPdf(RooAbsPdf& pdf) ;   
   // Set the proto DataSet, add to the the workspace if not already there
   virtual void SetProtoData(RooAbsData & data) ;
   // ... the rest of the SetXxx methods ...
   
  };
} 
