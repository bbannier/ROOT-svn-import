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

#include "TMVA/Optimizer.h"

ClassImp(TMVA::Optimizer)

#include <limits>
#include "TMath.h"
   
//_______________________________________________________________________
TMVA::Optimizer::Optimizer(MethodBase * const method, TString fomType) 
{
   // Constructor which sets either "Classification or Regression"
   // and the type of Figure of Merit that you've chosen


   //Somehow loop over the different paremters (ListOfOptions) and
   //find the one where

   if ( !(fFOM = new TMVA::OptimizerFOM(method,fomType))){
      std::cout << "ERROR in TMVA::Optimizer, you've probably chosen an FOM not yet implemented"
                << std::endl;
      exit(1);
   }
}

//_______________________________________________________________________
void TMVA::Optimizer::optimize()
{
   // do the actual optimization, i.e. calcualte the FOM for 
   // different tuning paraemters and remember which one is
   // gave the best FOM

   Double_t      currentFOM;
   

   fFOM->GetMethod()->Train();
   currentFOM = fFOM->GetFOM(); 
  

}

