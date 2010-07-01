/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Class  : Optimizer                                                             *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description: The Optimizer takes care of "scanning" the different tuning       *
 *              parameters in order to find the best set of tuning paraemters     *
 *              which will be used in the end                                     *
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

#ifndef ROOT_TMVA_Optimizer
#define ROOT_TMVA_Optimizer


#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

#ifndef ROOT_TMVA_MethodBase
#include "TMVA/MethodBase.h"
#endif

#ifndef ROOT_TMVA_DataSet
#include "TMVA/DataSet.h"
#endif

#ifndef ROOT_TMVA_OptimizerFOM
#include "TMVA/OptimizerFOM.h"
#endif

#ifndef ROOT_TMVA_IFitterTarget
#ifndef ROOT_IFitterTarget
#include "IFitterTarget.h"
#endif
#endif

namespace TMVA {

   class Optimizer : public IFitterTarget  {
      
   public:
      
      //default constructor
      Optimizer(MethodBase * const method, TString fomType="Separation");
      
      // destructor
      virtual ~Optimizer(){}
      // could later be changed to be set via option string... 
      // but for now it's impler like this
      void optimize(TString optimizationType = "GA"); 
      
      void optimizeScan();
      void optimizeGA();

      Double_t EstimatorFunction( std::vector<Double_t> & );

      
   private:

      OptimizerFOM *fFOM;

      std::map< std::vector<Double_t> , Double_t>  fAlreadyTrainedParCombination;
      
      ClassDef(Optimizer,0) // Interface to different separation critiera used in training algorithms
   };


} // namespace TMVA

#endif
