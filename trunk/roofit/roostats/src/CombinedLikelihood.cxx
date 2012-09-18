#include "RooStats/CombinedLikelihood.h"
#include "RooStats/RooStatsUtils.h"
#include "TIterator.h"
#include "TString.h"
#include "RooArgSet.h"
#include "RooAbsPdf.h"
#include "RooAddPdf.h"
#include "RooRealSumPdf.h"
#include "RooRealVar.h"
#include "RooAbsData.h"
#include <algorithm>
#include <stdexcept>

using namespace RooStats;

ClassImp(RooStats::CombinedLikelihood);


class CachedArgSet {
public:
   CachedArgSet(const RooAbsCollection& set) { 
      TIterator *iter = set.createIterator();
      for(TObject *obj = iter->Next(); obj != NULL; obj = iter->Next()) {
         RooRealVar *var = dynamic_cast<RooRealVar *>(obj);
         if(var) {
            fVariables.push_back(var);
            fValues.push_back(var->getVal());
         }
      }
      delete iter;
   }
   Bool_t IsDirty(Bool_t updateIfChanged = kFALSE) {
      std::vector<RooRealVar*>::const_iterator varIter = fVariables.begin(), varEnd = fVariables.end();
      std::vector<Double_t>::iterator valIter = fValues.begin();
      Bool_t hasChanged = kFALSE;
      for( ; varIter != varEnd; ++varIter, ++valIter) {
         Double_t value = (*varIter)->getVal();
         if (value != *valIter) {
            hasChanged = kTRUE;
            if (updateIfChanged) { *valIter = value; }
            else break;
         }
      }
      return hasChanged;
   }

private:
   std::vector<RooRealVar*> fVariables;
   std::vector<Double_t> fValues;
};

class CachedPdf {
public:
   // TODO: RooAbsPdf instead of RooAbsReal
   CachedPdf(RooAbsReal *pdf, const RooArgSet* obs) : fPdf(pdf), fObs(obs), fCacheObs(*obs) { }
   void SetDataDirty() { fLastData = NULL; }
   const std::vector<Double_t>& Eval(const RooAbsData& data) {
      bool newData = (fLastData != &data);
      if (newData) {
         fLastData = &data;
         fPdf->optimizeCacheMode(*data.get()); 
         fPdf->attachDataSet(data); // TODO: is it necessary?
         const_cast<RooAbsData*>(fLastData)->setDirtyProp(kFALSE);
         Fill(data); // TODO: see if it is worth to have multiple caches
      }

      return fValues;
   }

private:
   RooAbsReal* fPdf;
   const RooArgSet* fObs;
   CachedArgSet fCacheObs;
   std::vector<RooRealVar*> fVariables;
   std::vector<Double_t> fValues;
   const RooAbsData* fLastData;   

   void Fill(const RooAbsData& data) {
      Int_t numEntries = data.numEntries();
      fValues.reserve(numEntries);

      std::vector<Double_t>::iterator iter = fValues.begin();
      for(Int_t i = 0; i < numEntries; ++i, ++iter) {
         data.get(i);
         *iter = fPdf->getVal(fObs);
      }
   }
};




class CachedAddPdf : public RooAbsReal{
public:
   CachedAddPdf(RooAbsPdf* pdf, RooAbsData* data) :
      RooAbsReal("CachedAddPdf", "Cached version of adding pdfs"),
      fPdf(pdf),
      fZeroPoint(0)   
   {
      if(!pdf) throw std::invalid_argument("Pdf passed is null");
      SetData(*data);
      Init();
   }

   virtual Double_t evaluate() const {
      std::fill( fPartialSums.begin(), fPartialSums.end(), 0.0);
      
      std::vector<RooAbsReal*>::const_iterator itCoef, begCoef = fCoefficients.begin(), endCoef = fCoefficients.end();
      std::vector<CachedPdf>::iterator itPdf = fCachedPdfs.begin();
      std::vector<Double_t>::const_iterator itWgt, begWgt = fWeights.begin();
      std::vector<Double_t>::iterator itSum, begSum = fPartialSums.begin(), endSum = fPartialSums.end();

      Double_t sumCoef = 0.0;
      for(itCoef = begCoef ; itCoef != endCoef; ++itPdf, ++itCoef) {
         Double_t coef = (*itCoef)->getVal();
         if (fIsRooAddPdf) {
            sumCoef += coef;
         } else { // RooRealSum 
            sumCoef += coef * fIntegrals[itCoef - begCoef]->getVal();
         }

         // get pdf values
         const std::vector<Double_t>& pdfVals = itPdf->Eval(*fData);
         std::vector<Double_t>::const_iterator itVal = pdfVals.begin();
         for(itSum = begSum; itSum != endSum; ++itSum, ++itVal) {
            *itSum += coef * (*itVal);
         }
      }

      // get final NLL
      Double_t result = 0.0;
      for(itSum = begSum, itWgt = begWgt; itSum != endSum; ++itSum, ++itWgt) {

         if(*itWgt == 0) continue;
         if(!std::isnormal(*itSum) || *itSum <= 0) {
            std::cerr << "WARNING: underflow to " << *itSum << " in " << GetName() << std::endl;
            // TODO: CachingSimNLL::noDeepLEE_
         }

         Double_t term = (*itWgt) * (*itSum <= 0 ? -9e9 : log( ((*itSum) / sumCoef))); 
         // TODO: Determine the reason for -9e9
         // TODO: KAHAN_SUM
         result += term;
      }
      
      result = -result;

      Double_t expectedEvents = fIsRooAddPdf ? sumCoef : fPdf->getNorm(fData->get());
      if (expectedEvents <= 0) {
         // TODO: CachingSimNLL::noDeepLEE_ logEvalError
         expectedEvents = 1e-6;
      }

      result += expectedEvents - fSumWeights * log(expectedEvents);
      result += fZeroPoint;

      return result;
   }

   virtual Double_t defaultErrorLevel() const { return 0.5; }
   Double_t SumWeights() const { return fSumWeights; }
   void SetZeroPoint() { fZeroPoint = -this->getVal(); setValueDirty(); }
   void ClearZeroPoint() { fZeroPoint = 0.0; setValueDirty(); } 


private:
   void Init() {
      const RooArgSet *observables = fData->get();
      // FIXME: clear integrals (do not think it is necessary)
      RooAddPdf* addPdf = NULL; RooRealSumPdf* sumPdf = NULL;

      if( (addPdf = dynamic_cast<RooAddPdf*>(fPdf)) != NULL) {

         fIsRooAddPdf = kTRUE;

         Int_t numPdfs = addPdf->pdfList().getSize();
         fCoefficients.reserve(numPdfs);
         fCachedPdfs.reserve(numPdfs);

         TIterator *itPdf  = addPdf->pdfList().createIterator();
         TIterator *itCoef = addPdf->coefList().createIterator();
         
         for(Int_t i = 0; i < numPdfs; ++i) {
            RooAbsReal* coef = dynamic_cast<RooAbsReal*>(itCoef->Next());
            RooAbsPdf*  pdf  = dynamic_cast<RooAbsPdf* >(itPdf->Next() );
            fCoefficients.push_back(coef);
            fCachedPdfs.push_back(CachedPdf(pdf, observables));
         }

      } else if ( (sumPdf = dynamic_cast<RooRealSumPdf*>(fPdf)) != NULL) {

         fIsRooAddPdf = kFALSE;

         Int_t numFuncs = sumPdf->funcList().getSize();
         fCoefficients.reserve(numFuncs);
         fCachedPdfs.reserve(numFuncs);
           
         TIterator *itFunc = sumPdf->funcList().createIterator();
         TIterator *itCoef = sumPdf->coefList().createIterator();
 
         for(Int_t i = 0; i < numFuncs; ++i) {
            RooAbsReal* coef = dynamic_cast<RooAbsReal*>(itCoef->Next());
            RooAbsReal* func = dynamic_cast<RooAbsReal*>(itFunc->Next() );
            fCoefficients.push_back(coef);
            fCachedPdfs.push_back(CachedPdf(func, observables));
            fIntegrals.push_back(func->createIntegral(*observables));
         }
      } else {
         throw std::invalid_argument("ERROR: CachedAddPdf");
      }
   }
 

   void SetData(const RooAbsData& data) {
      fData = &data;
      setValueDirty();
      fSumWeights = 0.0;

      Double_t numEntries = data.numEntries();
      fWeights.reserve(numEntries);
      fPartialSums.reserve(numEntries);
      
      std::vector<Double_t>::iterator itWgt = fWeights.begin();
      for(Int_t i = 0, n = numEntries; i < n; ++i, ++itWgt) {
         data.get(i);
         *itWgt = data.weight();
         fSumWeights += *itWgt;
      }

      for(std::vector<CachedPdf>::iterator itPdf = fCachedPdfs.begin(), endPdf = fCachedPdfs.end();
         itPdf != endPdf; ++itPdf) (*itPdf).SetDataDirty();
      
   }
   
   RooAbsPdf* fPdf;
   const RooAbsData *fData;

   std::vector<Double_t> fWeights;
   mutable std::vector<CachedPdf> fCachedPdfs;
   std::vector<RooAbsReal*> fIntegrals;
   mutable std::vector<Double_t> fPartialSums;
   std::vector<RooAbsReal*> fCoefficients;
   Double_t fZeroPoint;
   Double_t fSumWeights;
   Bool_t   fIsRooAddPdf;
};





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
   )
{
   RooAbsCategoryLValue* index = (RooAbsCategoryLValue *)simPdf.indexCat().Clone();
   fNumberOfChannels = index->numBins((const char *)NULL);
   for(Int_t i = 0; i < fNumberOfChannels; ++i) {
      index->setBin(i); const char* crtLabel = index->getLabel();
      fChannels.push_back(simPdf.getPdf(crtLabel));
      fChannelNames.push_back(std::string(crtLabel));
   } 
   delete index;

   // Setup part
//   RooArgList constraints;
//   RooSimultaneous *unconstrainedPdf = (RooSimultaneous *)RooStats::StripConstraints(simPdf, *data.get(), constraints);

//   if(constraints.getSize()) {
      // TODO: Add constraints to constraint list
//   } else {
//      std::cerr << "PDF didn't factorize! No constraint terms!" << std::endl;
      // TODO: Add more detail
//   }

}


//_____________________________________________________________________________
CombinedLikelihood::CombinedLikelihood(const CombinedLikelihood& rhs, const char* newName) :
   RooAbsReal(rhs, newName),
   fNumberOfChannels(rhs.fNumberOfChannels),
   fChannels(rhs.fChannels),
   fChannelNames(rhs.fChannelNames)
{
   // TODO: see how to remove, not a real copy constructor
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
   
   return 0.0;
}


Double_t CombinedLikelihood::expectedEvents(const RooArgSet *nset) const
{
   return 0.0;
}


