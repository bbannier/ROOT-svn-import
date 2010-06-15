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
 *      Andreas Hoecker <Andreas.Hocker@cern.ch> - CERN, Switzerland              *
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

#include "TMVA/OptimizerFOM.h"
#include "TMVA/Tools.h"

ClassImp(TMVA::OptimizerFOM)

#include <limits>
#include "TMath.h"
   
//_______________________________________________________________________
TMVA::OptimizerFOM::OptimizerFOM(MethodBase* const method, TString fomType):
   fMethod(method),
   fFOMType(fomType),
   fMvaSig(NULL),
   fMvaBkg(NULL)
{
   // Constructor which sets either "Classification or Regression"
   // and the type of Figure of Merit that you've chosen
   if (fMethod->DoRegression()){
      if ( fFOMType == "Separation"){
         // that's ok... 
      } else {
         std::cout << " ERROR, you've specified as Figure of Merit in the \n"
                   << " parameter optimisation " << fFOMType << " which has not\n"
                   << " been implemented yet!! ---> exit " << std::endl;
         exit(1);
      }
   } else {
      std::cout << " ERROR: Sorry, Regression is not yet implement for automatic parameter optimisation"
                << " --> exit" << std::endl;
      exit(1);
   }
}
//_______________________________________________________________________
Double_t TMVA::OptimizerFOM::GetFOM()
{
  // Return the Figure of Merit (FOM) used in the parameter 
  //  optimization process
  
   Double_t fom=0;
   if (fMethod->DoRegression()){
   }else{
      if (fFOMType == "Separation") fom = GetSeparation();
   }
   return fom;
}

//_______________________________________________________________________
void TMVA::OptimizerFOM::GetMVADists()
{
   // fill the private histograms with the mva distributinos for sig/bkg

   const std::vector<Event*> events=fMethod->Data()->GetEventCollection(Types::kTesting);
   
   UInt_t signalClassNr = fMethod->DataInfo().GetClassInfo("Signal")->GetNumber();

   for (UInt_t iev=0; iev < events.size() ; iev++){
      if (events[iev]->GetClass() == signalClassNr) {
         fMvaSig->Fill(fMethod->GetMvaValue(events[iev]));
      } else {
         fMvaBkg->Fill(fMethod->GetMvaValue(events[iev]));
      }
   }
}
//_______________________________________________________________________
Double_t TMVA::OptimizerFOM::GetSeparation()
{
   // return the searation between the signal and background 
   // MVA ouput distribution
   GetMVADists();
   return gTools().GetSeparation(fMvaSig,fMvaBkg);
}

