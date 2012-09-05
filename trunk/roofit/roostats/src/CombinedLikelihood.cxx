#include "RooStats/CombinedLikelihood.h"
#include "TIterator.h"
#include "TString.h"
#include <algorithm>

using namespace RooStats;

ClassImp(RooStats::CombinedLikelihood);


CombinedLikelihood::CombinedLikelihood(
   const char *name, 
   const char *title,
   RooArgList& pdfList
) :
   RooAbsPdf(name,title)
{
   fNumberOfChannels = pdfList.getSize();

   if(fNumberOfChannels == 0) {
      fCurrentPdf = NULL;
   }  else {
      TIterator *iter = pdfList.createIterator();
      
      RooAbsPdf *pdf; Int_t i = 0;
      while((pdf = (RooAbsPdf *)iter->Next()) != NULL) { 
         fChannels[i] = pdf;
         fChannelNames[i] = TString::Format("cat%d", i).Data(); 
         i++;
      }

      delete iter;
   }
}


//_____________________________________________________________________________
CombinedLikelihood::CombinedLikelihood(
   RooSimultaneous* simPdf,
   RooAbsData* data,
   const RooArgSet* nuis
) :
   RooAbsPdf(*simPdf)
{
   RooAbsCategoryLValue* index = (RooAbsCategoryLValue *)simPdf->indexCat().Clone();
   fNumberOfChannels = index->numBins((const char *)NULL);
   for(Int_t i = 0; i < fNumberOfChannels; ++i) {
      index->setBin(i); const char* crtLabel = index->getLabel();
      fChannels[i] = simPdf->getPdf(crtLabel);
      fChannelNames[i] = crtLabel;
   } 
   delete index;
}


CombinedLikelihood::~CombinedLikelihood()
{
}

RooAbsPdf* CombinedLikelihood::GetPdf(const std::string& catLabel) const
{
   std::vector<std::string>::const_iterator cIter = fChannelNames.begin();
   // we need the index too, that is why we don't use std::find
   for(Int_t i = 0; i < fNumberOfChannels; ++i, ++cIter) {
      if(*cIter == catLabel) return fChannels[i]; 
   }
   return NULL; // no pdf found for that category name
}

Bool_t CombinedLikelihood::AddPdf(RooAbsPdf& pdf, const std::string& catLabel)
{
   if(std::find(fChannelNames.begin(), fChannelNames.end(), catLabel) == fChannelNames.end()) {
      std::cout << "CombinedLikelihood::AddPdf - warning: cannot reassign pdf to category " + catLabel << std::endl;
      return kFALSE;
   }  
   fChannels.push_back(&pdf);
   fChannelNames.push_back(catLabel);
   return kTRUE;
}


RooDataSet* CombinedLikelihood::GenerateGlobalObs(const RooArgSet &vars, Int_t nEvents) {
   RooDataSet *data = new RooDataSet("gensimglobal", "gensimglobal", vars);

   std::vector<RooAbsPdf*>::const_iterator cIter;
   for(cIter = fChannels.begin(); cIter != fChannels.end(); cIter++) {
      RooAbsPdf *pdf = *cIter;
      RooArgSet *globObs = pdf->getObservables(vars);

      // NOTE: now generating all data for one channel at a time
      data->append(*pdf->generate(*globObs, nEvents));
   } 

   return data;
}

Double_t CombinedLikelihood::evaluate() const
{
   // Return the current value of the p.d.f. associated with the current category
   // TODO: Perhaps does not make sense

   if(fCurrentPdf == NULL) {
      coutE(Eval) << "CombinedLikelihood does not have any pdfs in its composition" << std::endl;
      return 0.0;
   }

   // TODO: change this
   Bool_t extendedPdf = kFALSE;
   Double_t catFraction = 1.0; catFraction += 1.0;

   if(extendedPdf == kTRUE) {
      return 1.0;
   }
  
   return fCurrentPdf->getVal(); // * catFrac
}


Double_t CombinedLikelihood::expectedEvents(const RooArgSet *nset) const
{
   return 0.0;
}


