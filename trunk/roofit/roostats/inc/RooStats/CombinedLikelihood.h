// @(#)root/roostats:$Id$
// Author: Ioan Gabriel Bucur   30/07/2012


#ifndef ROOSTATS_CombinedLikelihood
#define ROOSTATS_CombinedLikelihood

#include "TObject.h"
#include "RooAbsPdf.h"
#include "RooArgList.h"
#include "RooDataSet.h"
#include "RooSimultaneous.h"
#include "RooAbsCategoryLValue.h"
#include <vector>
#include <string>
#include <memory>

namespace RooStats {

   class CombinedLikelihood : public RooAbsReal {
   public:
      // Constructors, destructors, assignment
      CombinedLikelihood(const char* name, const char* title, RooArgList& pdfList);
      CombinedLikelihood(const CombinedLikelihood& rhs, const char *newName = NULL);
      CombinedLikelihood(const RooSimultaneous& simPdf, const RooAbsData& data, const RooLinkedList& cmdList);
      virtual TObject *clone(const char *newName) const { return new CombinedLikelihood(*this, newName); }
      virtual ~CombinedLikelihood();

      RooAbsPdf* GetPdf(const std::string& catName) const; // considering deleting it
      // returns kTRUE if adding the pdf is succesful, kFALSE otherwise
      Bool_t AddPdf(RooAbsPdf& pdf, const std::string& catLabel);
      RooDataSet *GenerateGlobalObs(const RooArgSet& vars, Int_t nEvents);
      virtual Double_t evaluate() const;
      virtual Double_t expectedEvents(const RooArgSet* nset) const; // TODO: remove dependence on nset parameter
   private:
      CombinedLikelihood& operator=(const CombinedLikelihood& rhs); // disallow default assignment operator

      Int_t fNumberOfChannels;
      std::vector<RooAbsPdf*> fChannels;
      std::vector<std::string> fChannelNames;
      RooAbsPdf *fCurrentPdf;
   protected:
      ClassDef(CombinedLikelihood,1)
   };
}

#endif // ROOSTATS_CombinedLikelihood

