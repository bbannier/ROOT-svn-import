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

#include "TMVA/MethodBase.h"   
#include "TMVA/MethodBDT.h"   
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

   Double_t      bestFOM=-1000000, currentFOM;
   UInt_t        ibest=0;

   //let's somehow translate for each method individually (maybe we just 
   //define WHICH parameter(s) we want optimizer ourselfs (not the user) and
   //then do it here..
   //   too much freedom for the user just gives us bad publicity when being
   //   compared with Neurobayes!!
   
   std::vector<Int_t>  loopVariable;

   if (fFOM->GetMethod()->GetMethodType() == TMVA::Types::kBDT){
     for (UInt_t i=0;i<9;i++){loopVariable.push_back(i+1);}
   }

   for (UInt_t i=0;i<loopVariable.size();i++){    
     if(i!=0)fFOM->GetMethod()->Reset();
     ((MethodBDT*)(fFOM->GetMethod()))->SetMaxDepth(loopVariable[i]);     


     
     fFOM->GetMethod()->BaseDir()->cd();
     fFOM->GetMethod()->GetTransformationHandler().CalcTransformations(
                                                                       fFOM->GetMethod()->Data()->GetEventCollection());
     std::cout << "train in optimizer with " << fFOM->GetMethod()->Data()->GetNEvents() << std::endl;
     fFOM->GetMethod()->Train();
     std::cout << "Got back from train" << std::endl;
     currentFOM = fFOM->GetFOM(); 
     std::cout << "Got back from GetFOM with " << currentFOM << std::endl;
     if (currentFOM > bestFOM) {
       bestFOM = currentFOM;
       ibest   = i;
     }
   }

   fFOM->GetMethod()->Reset();
   ((MethodBDT*)(fFOM->GetMethod()))->SetMaxDepth(loopVariable[ibest]);
   std::cout << "And the winner is : MaxDepth = " << loopVariable[ibest] << std::endl;

}

