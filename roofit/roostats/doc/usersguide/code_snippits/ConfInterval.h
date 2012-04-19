namespace RooStats {
   class ConfInterval : public TNamed {

      // check if given point is in the interval
      virtual Bool_t IsInInterval(const RooArgSet&) const = 0; 
    
      // used to set confidence level.  Keep pure virtual
      virtual void SetConfidenceLevel(Double_t cl) = 0;

      // return confidence level
      virtual Double_t ConfidenceLevel() const = 0;

      // return list of parameters of interest defining this interval (return a new cloned list)
      virtual RooArgSet* GetParameters() const = 0;
   }
}
