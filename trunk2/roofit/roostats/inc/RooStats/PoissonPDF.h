#ifndef ROOSTATS_PoissonPDF
#define ROOSTATS_PoissonPDF

#include "RooStats/AbstractPDF.h"
#include "RooAbsReal.h"
#include "RooAbsPdf.h"
#include "RooRealProxy.h"

namespace RooStats {
   class PoissonPDF : public RooAbsPdf, virtual public AbstractPDF {
   public:
      // RooAbsPdf interface
      PoissonPDF() {}
      PoissonPDF(const char* name, const char* title, RooAbsReal& obs, RooAbsReal& mean);
      PoissonPDF(const PoissonPDF& other, const char* newName = NULL);
      virtual TObject* clone(const char *newName) const { return new PoissonPDF(*this, newName); }
      virtual ~PoissonPDF() {}

      virtual void generateEvent(Int_t code);
      virtual Double_t getLogVal(const RooArgSet* nset) const;
      Double_t analyticalIntegral(Int_t code, const char* rangeName = NULL) const;

      // AbstractPDF interface
      virtual Double_t EvaluatePDF() const { return evaluate(); } //
      virtual RooAbsData& EvaluatePDF(RooAbsData& data) const { return data; }
      virtual Double_t EvaluateCDF() const { return analyticalIntegral(0); }
      virtual std::vector<Double_t> EvaluateCDF(RooAbsData&) const { return *(new std::vector<Double_t>()); }
      virtual Double_t Integrate() const { return 0.0; }
      virtual Double_t EvaluateNLL(RooAbsData&) const { return 0.0; }
      virtual RooAbsPdf* createRooAbsPdf() const { return new PoissonPDF(*this); }
      virtual RooFitResult* FitTo() const { return NULL; }
      virtual Bool_t IsNormalized() const { return kFALSE; }
      virtual Double_t GenerateEvent() { generateEvent(0); return fObs; } // returns current value of observable as well

   protected: // XXX: ? protected
      virtual Double_t evaluate() const;

   private:
      // eventually replace data structures
      RooRealProxy fObs;
      RooRealProxy fMean;

      // XXX: possibly useful values
      Double_t fLogMean;

      ClassDef(PoissonPDF, 1)
   };
}


#endif // ROOSTATS_PoissonPDF


