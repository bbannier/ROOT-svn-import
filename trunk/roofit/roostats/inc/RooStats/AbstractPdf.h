#ifndef ROOSTATS_AbstractPdf
#define ROOSTATS_AbstractPdf

#include "RooAbsPdf.h"
#include "RooAbsData.h"

namespace RooStats {

   class AbstractPdf {
   public:
      virtual ~AbstractPdf() = 0;

      virtual Double_t EvaluatePDF() const = 0;
      virtual RooAbsData& EvaluatePDF(RooAbsData& data) const = 0;
      virtual Double_t EvaluateCDF() const = 0;
      virtual std::vector<Double_t> EvaluateCDF(RooAbsData& data) const = 0;
      virtual Double_t Integrate() const = 0; // need to pass integration variable somehow
      virtual Double_t EvaluateNLL(RooAbsData& data) const = 0;
      virtual RooAbsPdf* createRooAbsPdf() const = 0;
      virtual RooFitResult* FitTo() const = 0; 
      virtual Bool_t IsNormalized() const = 0;
      virtual Double_t GenerateEvent() = 0;

      ClassDef(AbstractPdf, 1)

   };

}


#endif // ROOSTATS_AbstractPdf


