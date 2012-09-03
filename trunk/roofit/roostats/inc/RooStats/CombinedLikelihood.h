// @(#)root/roostats:$Id$
// Author: Ioan Gabriel Bucur   30/07/2012


#ifndef ROOSTATS_CombinedLikelihood
#define ROOSTATS_CombinedLikelihood

#include "TObject.h"
#include "RooAbsPdf.h"
#include "RooArgList.h"
#include "RooDataSet.h"
#include "RooSimultaneous.h"
#include <map>
#include <string>

namespace RooStats {

   class CombinedLikelihood : public RooAbsPdf {
   public:
      // Constructors, destructors, assignment
      CombinedLikelihood(const char* name, const char* title, RooArgList& pdfList);
      CombinedLikelihood(const char* name, const char* title, const std::map<std::string, RooAbsPdf*> &pdfMap);
      CombinedLikelihood(RooSimultaneous* simPdf, RooAbsData* data, const RooArgSet* nuis = NULL);
      CombinedLikelihood(const CombinedLikelihood& other, const char* name = NULL);
      virtual TObject *clone(const char *newname) const { return new CombinedLikelihood(*this, newname); }
      virtual ~CombinedLikelihood();

      RooAbsPdf* GetPdf(const std::string& catName) const; // considering deleting it
      // returns kTRUE if adding the pdf is succesful, kFALSE otherwise
      Bool_t AddPdf(RooAbsPdf& pdf, const std::string& catLabel);
      RooDataSet *GenerateGlobalObs(const RooArgSet& vars, Int_t nEvents);
      virtual Double_t evaluate() const;
      virtual Double_t expectedEvents(const RooArgSet* nset) const; // TODO: remove dependence on nset parameter
   private:
      std::map<std::string, RooAbsPdf*> fChannels;
      RooAbsPdf *fCurrentPdf;
   protected:
      ClassDef(CombinedLikelihood,1)
   };
}

#endif // ROOSTATS_CombinedLikelihood

