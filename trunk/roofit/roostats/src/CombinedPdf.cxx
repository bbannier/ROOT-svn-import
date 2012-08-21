#include "RooStats/CombinedPdf.h"
#include "TIterator.h"
#include "TString.h"


using namespace RooStats;

ClassImp(RooStats::CombinedPdf);


CombinedPdf::CombinedPdf(
   const char *name, 
   const char *title,
   RooArgList& pdfList
) :
   RooAbsPdf(name,title)
{
   if(pdfList.getSize() == 0) {
      fCurrentPdf = NULL;
   }  else {
      TIterator *iter = pdfList.createIterator();
      RooAbsPdf *pdf; Int_t i = 0;

      while((pdf = (RooAbsPdf *)iter->Next()) != NULL) { 
         fChannels[TString::Format("cat%d", i).Data()] = pdf;
         if(i == 0) fCurrentPdf = pdf; 
         i++;
      }

      delete iter;
   }
}

CombinedPdf::CombinedPdf(
   const char *name,
   const char *title,
   const std::map<std::string, RooAbsPdf*> &pdfMap
) :
   RooAbsPdf(name, title)
{
   fChannels = pdfMap;
   if(fChannels.empty()) {
      fCurrentPdf = NULL;
   } else {
      std::map<std::string, RooAbsPdf*>::const_iterator cIter = fChannels.begin();
      fCurrentPdf = (*cIter).second;
   }      
}

CombinedPdf::CombinedPdf(
   const CombinedPdf& other,
   const char *name
) :
   RooAbsPdf(other, name)
{
}


CombinedPdf::~CombinedPdf()
{
}

RooAbsPdf* CombinedPdf::GetPdf(const std::string& catName) const
{
   std::map<std::string, RooAbsPdf*>::const_iterator cIter = fChannels.find(catName);
   if(cIter != fChannels.end()) return (*cIter).second;
   return NULL;
}

Bool_t CombinedPdf::AddPdf(RooAbsPdf& pdf, const std::string& catLabel)
{
   if(fChannels.find(catLabel) != fChannels.end()) {
      std::cout << "CombinedPdf::AddPdf - warning: cannot reassign pdf to category " + catLabel << std::endl;
      return kFALSE;
   }  
   fChannels[catLabel] = &pdf;
   return kTRUE;
}


RooDataSet* CombinedPdf::GenerateGlobalObs(const RooArgSet &vars, Int_t nEvents) {
   RooDataSet *data = new RooDataSet("gensimglobal", "gensimglobal", vars);

   std::map<std::string, RooAbsPdf*>::const_iterator cIter;
   for(cIter = fChannels.begin(); cIter != fChannels.end(); cIter++) {
      RooAbsPdf *pdf = (*cIter).second;
      RooArgSet *globObs = pdf->getObservables(vars);

      // NOTE: now generating all data for one channel at a time
      data->append(*pdf->generate(*globObs, nEvents));
   } 

   return data;
}

Double_t CombinedPdf::evaluate() const
{
   // Return the current value of the p.d.f. associated with the current category

   if(fCurrentPdf == NULL) {
      coutE(Eval) << "CombinedPdf does not have any pdfs in its composition" << std::endl;
      return 0.0;
   }

   // TODO: change this
   Bool_t extendedPdf = kFALSE;
   Double_t catFraction = 1.0;



   if(extendedPdf == kTRUE) {
      return 1.0;
   }
  
   return fCurrentPdf->getVal(); // * catFrac
}


