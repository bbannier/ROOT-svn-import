/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Class  : OptimizerFOM                                                          *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description: Return different figures of merit (FOM) used in the               *
 *              automatic optimization of the Classifiers/Regressor parameters    *
 *                                                                                *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      Helge Voss      <Helge.Voss@cern.ch>     - MPI-K Heidelberg, Germany      *
 *                                                                                *
 * Copyright (c) 2005:                                                            *
 *      CERN, Switzerland                                                         * 
 *      MPI-K Heidelberg, Germany                                                 * 
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://ttmva.sourceforge.net/LICENSE)                                         *
 **********************************************************************************/

#ifndef ROOT_TMVA_OptimizerFOM
#define ROOT_TMVA_OptimizerFOM


#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

#ifndef ROOT_TH1
#include "TH1.h"
#endif

#ifndef ROOT_TMVA_MethodBase
#include "TMVA/MethodBase.h"
#endif

#ifndef ROOT_TMVA_DataSet
#include "TMVA/DataSet.h"
#endif

namespace TMVA {

   class OptimizerFOM {
      
   public:
      
      //default constructor
      OptimizerFOM(const MethodBase* method, const DataSet* data, TString fomType="Separation");
      
      // destructor
      virtual ~OptimizerFOM(){}
      
      // return the Figure of Merit (used in the parameter optimisation)
      Double_t GetFOM();
      
      
   private:
      
      void GetMVADists();
      Double_t GetSeparation();
      
      
      const MethodBase *fMethod; // The MVA method to be evaluated
      const DataSet    *fData;   // The dataset to be used (test to start with, later crossvalidation etc..
      TString           fFOMType;    // the FOM type (Separation, ROC integra.. whaeter you implemented..
      
      TH1D             *fMvaSig; // MVA distrituion for signal events
      TH1D             *fMvaBkg; // MVA distrituion for bakgr. events
      
      ClassDef(OptimizerFOM,0) // Interface to different separation critiera used in training algorithms
  };
  
  
} // namespace TMVA

#endif
