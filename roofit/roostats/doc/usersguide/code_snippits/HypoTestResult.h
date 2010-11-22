namespace RooStats {
   class HypoTestResult : public TNamed {

      // Return p-value for null hypothesis
      virtual Double_t NullPValue() const {return fNullPValue;}

      // Return p-value for alternate hypothesis
      virtual Double_t AlternatePValue() const {return fAlternatePValue;}

      // Convert  NullPValue into a "confidence level"
      virtual Double_t CLb() const {return 1.-NullPValue();}

      // Convert  AlternatePValue into a "confidence level"
      virtual Double_t CLsplusb() const {return AlternatePValue();}

      // CLs is simply CLs+b/CLb (not a method, but a quantity)
      virtual Double_t CLs() const ;

      // familiar name for the Null p-value in terms of 1-sided Gaussian significance
      virtual Double_t Significance() const;
   };
}


#endif
