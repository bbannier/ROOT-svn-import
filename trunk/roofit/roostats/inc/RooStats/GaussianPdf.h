#ifndef ROOSTATS_GaussianPdf
#define ROOSTATS_GaussianPdf

#include "RooStats/AbstractPdf.h"
#include "RooAbsReal.h"
#include "RooAbsPdf.h"
#include "RooRealProxy.h"

namespace RooStats {
   class GaussianPdf : public RooAbsPdf, public AbstractPdf {
   public:
      // RooAbsPdf interface
      GaussianPdf() {}
      GaussianPdf(const char* name, const char* title, RooAbsReal& obs, RooAbsReal& mean, RooAbsReal& sigma);
      GaussianPdf(const GaussianPdf& other, const char* name = NULL);
      virtual TObject* clone(const char* newName) const { return new GaussianPdf(*this, newName); }
      virtual ~GaussianPdf() {}

      virtual void generateEvent(Int_t code);
      virtual Double_t getLogVal(const RooArgSet* nset) const;
      Double_t analyticalIntegral(Int_t code, const char* rangeName = NULL) const;

      // AbstractPdf interface
      virtual Double_t EvaluatePDF() const { return evaluate(); } // TODO: eventually replace evaluate with this function
      virtual RooAbsData& EvaluatePDF(RooAbsData& data) const { return data; }
      virtual Double_t EvaluateCDF() const { return analyticalIntegral(0); }
      virtual std::vector<Double_t> EvaluateCDF(RooAbsData&) const { return *(new std::vector<Double_t>()); }
      virtual Double_t Integrate() const { return 0.0; }
      virtual Double_t EvaluateNLL(RooAbsData&) const { return 0.0; }
      virtual RooAbsPdf* createRooAbsPdf() const { return new GaussianPdf(*this); }
      virtual RooFitResult* FitTo() const { return NULL; }
      virtual Bool_t IsNormalized() const { return kFALSE; }
      virtual Double_t GenerateEvent() { generateEvent(0); return fObs; } // returns current value of observable as well

   protected:
   
      virtual Double_t evaluate() const;

   private:
      RooRealProxy fObs;
      RooRealProxy fMean;
      RooRealProxy fSigma;

      ClassDef(GaussianPdf, 1)
   };
}


#endif // ROOSTATS_GaussianPdf

