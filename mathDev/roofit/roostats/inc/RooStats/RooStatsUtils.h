// @(#)root/roostats:$Id$
// Author: Kyle Cranmer   28/07/2008

/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_RooStatsUtils
#define ROOSTATS_RooStatsUtils

#ifndef ROOT_TMath
#include "TMath.h"
#endif

#ifndef ROOT_TTree
#include "TTree.h"
#endif

#ifndef ROOT_Math_DistFuncMathCore
#include"Math/DistFuncMathCore.h"
#endif

#include "RooArgSet.h"
#include "RooRealVar.h"
#include "RooAbsCollection.h"
#include "TIterator.h"
#include "RooStats/ModelConfig.h"
#include "RooProdPdf.h"
#include "RooDataSet.h"


namespace RooStats {


  // returns one-sided significance corresponding to a p-value
  inline Double_t PValueToSignificance(Double_t pvalue){
     return ::ROOT::Math::normal_quantile_c(pvalue,1); 
  }

  // returns p-value corresponding to a 1-sided significance
  inline Double_t SignificanceToPValue(Double_t Z){
    return ::ROOT::Math::normal_cdf_c(Z);
  }


  inline void SetParameters(const RooArgSet* desiredVals, RooArgSet* paramsToChange){
    *paramsToChange=*desiredVals ;
  }

  inline void RemoveConstantParameters(RooArgSet* set){
    RooArgSet constSet;
    RooLinkedListIter it = set->iterator();
    RooRealVar *myarg; 
    while ((myarg = (RooRealVar *)it.Next())) { 
      if(myarg->isConstant()) constSet.add(*myarg);
    }
    set->remove(constSet);
  }

   inline bool SetAllConstant(const RooAbsCollection &coll, bool constant = true) {
       // utility function to set all variable constant in a collection
       // (from G. Petrucciani)
       bool changed = false;
       RooLinkedListIter iter = coll.iterator();
       for (RooAbsArg *a = (RooAbsArg *) iter.Next(); a != 0; a = (RooAbsArg *) iter.Next()) {
          RooRealVar *v = dynamic_cast<RooRealVar *>(a);
          if (v && (v->isConstant() != constant)) {
             changed = true;
             v->setConstant(constant);
          }
       }
       return changed;
   }


  // assuming all values in set are RooRealVars, randomize their values
  inline void RandomizeCollection(RooAbsCollection& set,
                                  Bool_t randomizeConstants = kTRUE)
  {
    RooLinkedListIter it = set.iterator();
    RooRealVar* var;

    // repeat loop tpo avoid calling isConstant for nothing 
    if (randomizeConstants) { 
       while ((var = (RooRealVar*)it.Next()) != NULL)
         var->randomize();
    }
    else {
       // exclude constants variables
      while ((var = (RooRealVar*)it.Next()) != NULL)
      if (!var->isConstant() )
         var->randomize();
    }


  }

  

   void FactorizePdf(const RooArgSet &observables, RooAbsPdf &pdf, RooArgList &obsTerms, RooArgList &constraints);

   void FactorizePdf(RooStats::ModelConfig &model, RooAbsPdf &pdf, RooArgList &obsTerms, RooArgList &constraints);

   RooAbsPdf * MakeNuisancePdf(RooAbsPdf &pdf, const RooArgSet &observables, const char *name);

   RooAbsPdf * MakeNuisancePdf(const RooStats::ModelConfig &model, const char *name);
   
   // Create a TTree with the given name and description. All RooRealVars in the RooDataSet are represented as branches that contain values of type Double_t.
   TTree* GetAsTTree(TString name, TString desc, const RooDataSet& data);

   // others functions from Giovanni's code

   namespace Utils { 

    // Clone a pdf and all it's branch nodes. on request, clone also leaf nodes (i.e. RooRealVars)                                        
    RooAbsPdf *FullClonePdf(const RooAbsPdf *pdf, RooArgSet &holder, bool cloneLeafNodes=false) ;
    // Clone a function and all it's branch nodes. on request, clone also leaf nodes (i.e. RooRealVars)                                   
    RooAbsReal *FullCloneFunc(const RooAbsReal *pdf, RooArgSet &holder, bool cloneLeafNodes=false) ;

    /// factorize a RooAbsReal                                                                                                            
    void FactorizeFunc(const RooArgSet &observables, RooAbsReal &pdf, RooArgList &obsTerms, RooArgList &otherTerms, bool debug=false);

    /// Create a pdf which depends only on observables, and collect the other constraint terms                                            
    /// Will return 0 if it's all constraints, &pdf if it's all observables, or a new pdf if it's something mixed                         
    /// In the last case, you're the owner of the returned pdf.                                                                           
    RooAbsPdf *FactorizePdf(const RooArgSet &observables, RooAbsPdf &pdf, RooArgList &constraints);

      void copyAttributes(const RooAbsArg & from, RooAbsArg & to);

   }

}


#endif
