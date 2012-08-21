// @(#)root/roostats:$Id$
// Author: Ioan Gabriel Bucur   30/07/2012


#ifndef ROOSTATS_CombinedPdf
#define ROOSTATS_CombinedPdf

#include "TObject.h"
#include "RooAbsPdf.h"
#include "RooArgList.h"
#include "RooDataSet.h"
#include "RooSimultaneous.h"
#include <map>
#include <string>

namespace RooStats {

   class CombinedPdf : public RooAbsPdf {
   public:
      // Constructors, destructors, assignment
      CombinedPdf(const char *name, const char *title, RooArgList &pdfList);
      CombinedPdf(const char *name, const char *title, const std::map<std::string, RooAbsPdf*> &pdfMap);
      CombinedPdf(const CombinedPdf& other, const char *name = 0);
      virtual TObject *clone(const char *newname) const { return new CombinedPdf(*this, newname); }
      virtual ~CombinedPdf();

      RooAbsPdf* GetPdf(const std::string& catName) const; // considering deleting it
      // returns kTRUE if adding the pdf is succesful, kFALSE otherwise
      Bool_t AddPdf(RooAbsPdf& pdf, const std::string& catLabel);
      RooDataSet *GenerateGlobalObs(const RooArgSet& vars, Int_t nEvents);
      virtual Double_t evaluate() const;

   private:
      std::map<std::string, RooAbsPdf *> fChannels;
      RooAbsPdf *fCurrentPdf;
   protected:
      ClassDef(CombinedPdf,0)
   };
}

#endif

