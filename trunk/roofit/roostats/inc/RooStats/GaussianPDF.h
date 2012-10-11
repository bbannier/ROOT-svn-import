#ifndef ROOSTATS_GaussianPDF
#define ROOSTATS_GaussianPDF

#include "RooStats/AbstractPDF.h"
#include "RooAbsReal.h"
#include "RooRealProxy.h"
#include "RooGaussian.h"

namespace RooStats {
   class GaussianPDF : public RooGaussian, virtual public AbstractPDF {
   public:
      // RooAbsPdf interface
      GaussianPDF() {}
      GaussianPDF(const char* name, const char* title, RooAbsReal& obs, RooAbsReal& mean, RooAbsReal& sigma);
      GaussianPDF(const GaussianPDF& other, const char* name = NULL);
      GaussianPDF(const RooGaussian& gauss, const char* name = NULL);
      virtual TObject* clone(const char* newName) const { return new GaussianPDF(*this, newName); }
      virtual ~GaussianPDF() {}

      virtual void generateEvent(Int_t code);
      virtual Double_t getLogVal(const RooArgSet* nset) const;
      Double_t analyticalIntegral(Int_t code, const char* rangeName = NULL) const;

      // AbstractPDF interface
      virtual Double_t EvaluatePDF() const { return evaluate(); } // TODO: eventually replace evaluate with this function
      virtual RooAbsData& EvaluatePDF(RooAbsData& data) const { return data; }
      virtual Double_t EvaluateCDF() const { return analyticalIntegral(0); }
      virtual std::vector<Double_t> EvaluateCDF(RooAbsData&) const { return *(new std::vector<Double_t>()); }
      virtual Double_t Integrate() const { return 0.0; }
      virtual Double_t EvaluateNLL(RooAbsData&) const { return 0.0; }
      virtual RooAbsPdf* createRooAbsPdf() const { return new GaussianPDF(*this); }
      virtual RooFitResult* FitTo() const { return NULL; }
      virtual Bool_t IsNormalized() const { return kFALSE; }
      virtual Double_t GenerateEvent() { generateEvent(0); return fObs; } // returns current value of observable as well

   protected:
   
      virtual Double_t evaluate() const;

   private:
      // TODO: The goal is to eventually replace data structures and sever all ties between
         // GaussianPDF and RooGaussian; as such, the RooRealProxies in RooGaussian will become redundant
      RooRealProxy fObs;
      RooRealProxy fMean;
      RooRealProxy fSigma;

      ClassDef(GaussianPDF, 1)
   };
}


#endif // ROOSTATS_GaussianPDF

