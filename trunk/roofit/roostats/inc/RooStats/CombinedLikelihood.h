// @(#)root/roostats:$Id$
// Author: Ioan Gabriel Bucur   30/07/2012


#ifndef ROOSTATS_CombinedLikelihood
#define ROOSTATS_CombinedLikelihood

#include "RooSimultaneous.h"
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <cmath>

class RooArgList;
class RooArgSet;
class RooDataSet;
class RooSetProxy;

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

      //void SetData(const RooAbsData& data) {}
      
      // TODO: implement properly
      virtual RooArgSet* getParameters(const RooArgSet*, Bool_t) const {
        // std::cout << "CombinedLikelihood::getParameters" << std::endl;
       //  fPdf->getParameters(fData)->Print("v");
         return fPdf->getParameters(fData);
      }

   private:
      CombinedLikelihood& operator=(const CombinedLikelihood& rhs); // disallow default assignment operator

      const RooSimultaneous* fPdf;
      const RooAbsData* fData;
      const RooArgSet* fNuisanceParameters;

      Int_t fNumberOfChannels;
      Int_t fNumberOfConstraints;

      std::vector<RooAbsPdf*> fChannels;
      std::vector<RooAbsData*> fDataSets;
    
      std::vector<RooAbsPdf*> fConstraints;
      RooSetProxy fConstraintParameters;
      std::vector<Double_t> fConstraintZeroPoints;
      std::vector<std::string> fChannelNames;
      std::vector<RooAbsReal*> fChannelLikelihoods;

   protected:
      ClassDef(CombinedLikelihood,1)
   };
}

#endif // ROOSTATS_CombinedLikelihood

