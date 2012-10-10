#include "RooStats/CombinedLikelihood.h"
#include "RooStats/RooStatsUtils.h"
#include "TIterator.h"
#include "TString.h"
#include "RooAbsData.h"
#include <algorithm>
#include "RooCategory.h"
#include <memory>

using namespace RooStats;

ClassImp(RooStats::CombinedLikelihood);


CombinedLikelihood::CombinedLikelihood(
   const char *name, 
   const char *title,
   RooArgList& pdfList
) :
   RooAbsReal(name, title),
   fNumberOfChannels(pdfList.getSize())
{
   TIterator *iter = pdfList.createIterator();
   
   RooAbsPdf *pdf; Int_t i = 0; 
   while((pdf = (RooAbsPdf *)iter->Next()) != NULL) { 
      fChannels.push_back(pdf);
      fChannelNames.push_back(TString::Format("cat%d", ++i).Data()); 
   }

   delete iter;
}


//_____________________________________________________________________________
CombinedLikelihood::CombinedLikelihood(
   const RooSimultaneous& simPdf,
   const RooAbsData& data,
   const RooLinkedList& cmdList
) :
   RooAbsReal(
      TString::Format("comb_nll_%s_%s", simPdf.GetName(), data.GetName()).Data(),
      "combined -log(likelihood)"
   ),
   fData(&data),
   fNuisanceParameters(NULL),
   fConstraintParameters("constr_params", "constraint parameters", this)
{
   // XXX: maybe need Init method ?
   setOperMode(RooAbsArg::ADirty);

   // Constraint Setup part
   RooArgList constraints;
   fPdf = (RooSimultaneous *) RooStats::StripConstraints(*((RooAbsPdf*)&simPdf), *data.get(), constraints);
   fNumberOfConstraints = constraints.getSize();

   // check that pdf and data have the same category and save a pointer to it
   RooCategory* pdfCatClone = dynamic_cast<RooCategory*>(fPdf->indexCat().Clone());
   assert(pdfCatClone != NULL);
   // TODO: see if it is possible to avoid second call to indexCat()
   RooCategory* dataCat = dynamic_cast<RooCategory*>(data.get()->find(fPdf->indexCat().GetName()));
   assert(dataCat != NULL);

   fNumberOfChannels = pdfCatClone->numBins(NULL); 


   // Data splitting part
   // TODO: explore SetData necessity
   Int_t numEntries = data.numEntries();
   RooArgSet observables(*data.get()); observables.remove(*dataCat, kTRUE, kTRUE);
   fDataSets.reserve(fNumberOfChannels);

   // FIXME: see if / when we need this
   RooRealVar weight("comb_nll_weight", "CombinedLikelihood data set weight", 1.0);
   for(Int_t i = 0; i < fNumberOfChannels; ++i) {
      // assume fDataSets[i] == NULL
      fDataSets[i] = new RooDataSet(TString::Format("comb_nll_dataset_%d", i), 
         "CombinedLikelihood channel data set", RooArgSet(observables, weight), "comb_nll_weight"
      );
   }

   for(Int_t i = 0; i < numEntries; ++i) {
      data.get(i); 
      if(data.weight() == 0.0) continue; 
      else if(data.weight() > 0.0) {
         Int_t bin = dataCat->getBin();
         fDataSets[bin]->add(observables, data.weight());
      }
      else throw std::logic_error("CombinedLikelihood::CombinedLikelihood - negative weights in input data set");
   }
   
 
   for(Int_t i = 0; i < fNumberOfChannels; ++i) {
      pdfCatClone->setBin(i); const char* crtLabel = pdfCatClone->getLabel();

      // Set Channels
      fChannels.push_back(fPdf->getPdf(crtLabel)); 
      fChannelNames.push_back(std::string(crtLabel));
     
      // TODO: eliminate logL variable if not needed in the end 
      RooAbsReal* logL = RooStats::CreateNLL(*fPdf->getPdf(crtLabel), *fDataSets[i], cmdList);
      fChannelLikelihoods.push_back(logL);
   } 


   // TODO: decide on LEE (logEvalErrors)
//   std::cout << "--START--simPdf" << std::endl;
//   simPdf.getParameters(data)->Print("v");
//   std::cout << "---END---simPdf" << std::endl;
   
   if(fNumberOfConstraints > 0) {

      fConstraints.reserve(fNumberOfConstraints);
      fConstraintZeroPoints.reserve(fNumberOfConstraints);

      // TODO: Add constraints to constraint list
      TIterator *itConstr = constraints.createIterator();
      for(Int_t i = 0; i < fNumberOfConstraints; i++) {
         RooAbsPdf *constr = dynamic_cast<RooAbsPdf*>(itConstr->Next()); assert(constr != NULL);
         fConstraints.push_back(constr);
         fConstraintZeroPoints.push_back(0.0); // TODO: analyse the necessity of this

         assert(constr->getParameters(data) != NULL);
         fConstraintParameters.add(*constr->getParameters(data));
      }
   } else {
      std::cerr << "PDF didn't factorize! No constraint terms!" << std::endl;
      std::cout << "Parameters: " << std::endl;
      simPdf.getParameters(data)->Print("v");
      std::cout << "Observables: " << std::endl;
      data.get()->Print("v");
   }

   delete pdfCatClone;

}


//_____________________________________________________________________________
CombinedLikelihood::CombinedLikelihood(const CombinedLikelihood& rhs, const char* newName) :
   RooAbsReal(rhs, newName),
   fPdf(rhs.fPdf),
   fNuisanceParameters(rhs.fNuisanceParameters),
   fConstraintParameters("constr_params", "constraint parameters", this)
{
   assert(0); // should not be called yet
}


//_____________________________________________________________________________
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

   Double_t result = 0.0;

   std::vector<RooAbsReal*>::const_iterator citCLL = fChannelLikelihoods.begin(), cendCLL = fChannelLikelihoods.end();
   for ( ; citCLL != cendCLL; ++citCLL) {
      result += (*citCLL)->getVal();
   }

   if (fNumberOfConstraints > 0) {
      std::vector<RooAbsPdf*>::const_iterator citPdf = fConstraints.begin(), cendPdf = fConstraints.end(); 
      std::vector<Double_t>::const_iterator citZP  = fConstraintZeroPoints.begin();
      for ( ; citPdf != cendPdf; ++citPdf, ++citZP) {
         Double_t logPdfValue = (*citPdf)->getLogVal(fNuisanceParameters); 
         // XXX: are Log Eval errors checked in getLogVal
         result -= (logPdfValue + *citZP); // XXX FastConstraints implemented in new RooGaussian
      }
          
   }

   return result;
}


