#ifndef ROOSTATS_GaussianPdf
#define ROOSTATS_GaussianPdf

#include "RooStats/AbstractPdf.h"
#include "RooAbsReal.h"
#include "RooAbsPdf.h"
#include "RooRealProxy.h"

namespace RooStats {
   class GaussianPdf : public RooAbsPdf, public AbstractPdf {
   public:
      GaussianPdf() {}
      GaussianPdf(const char* name, const char* title, RooAbsReal& obs, RooAbsReal& mean, RooAbsReal& sigma);
      GaussianPdf(const GaussianPdf& other, const char* name = NULL);
      virtual TObject* clone(const char* newName) const { return new GaussianPdf(*this, newName); }
      virtual ~GaussianPdf() {}

      virtual Double_t getLogVal(const RooArgSet* nset) const;

   // AbstractPdf interface
      virtual Double_t EvaluatePDF() const { return evaluate(); }
      virtual RooAbsData& EvaluatePDF(RooAbsData& data) const { return data; }
      virtual Double_t EvaluateCDF() const { return 0.0; }
      virtual std::vector<Double_t> EvaluateCDF(RooAbsData& data) const { return *(new std::vector<Double_t>()); }
      virtual Double_t Integrate() const { return 0.0; }
      virtual Double_t EvaluateNLL(RooAbsData& data) const { return 0.0; }
      virtual RooAbsPdf* createRooAbsPdf() const { return new GaussianPdf(*this); }
      virtual RooFitResult* FitTo() const { return NULL; }
      virtual Bool_t IsNormalized() const { return kFALSE; }


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

