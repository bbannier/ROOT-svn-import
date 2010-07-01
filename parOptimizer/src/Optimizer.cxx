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
#include "TMVA/GeneticFitter.h"
#include "TMVA/Interval.h"
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
void TMVA::Optimizer::optimize(TString optimizationType)
{
   if      (optimizationType == "Scan") this->optimizeScan();
   else if (optimizationType == "GA"  ) this->optimizeGA();
   else {
      std::cout << "You have chosen as optimization type " << optimizationType
                << " that is not (yet) coded --> exit()" << std::endl;
      exit(1);
   }
   return;

}//_______________________________________________________________________
void TMVA::Optimizer::optimizeScan()
{
   // do the actual optimization using a simple scan method, 
   // i.e. calcualte the FOM for 
   // different tuning paraemters and remember which one is
   // gave the best FOM

   Double_t      bestFOM=-1000000, currentFOM;
   UInt_t        ibest=0;

   //let's somehow translate for each method individually (maybe we just 
   //define WHICH parameter(s) we want optimizer ourselfs (not the user) and
   //then do it here..
   //   too much freedom for the user just gives us bad publicity when being
   //   compared with Neurobayes!!
   
   std::vector< std::pair<Int_t,Int_t > >  loopVariable;
   
   if (fFOM->GetMethod()->GetMethodType() == TMVA::Types::kBDT){
      //      for (UInt_t i=0;i<9;i++){loopVariable.push_back(i+1);}
      //      for (UInt_t i=0;i<9;i++){loopVariable.push_back(9-i);}
      loopVariable.push_back(std::make_pair(2,10));
      loopVariable.push_back(std::make_pair(3,10));
      loopVariable.push_back(std::make_pair(4,10));
      loopVariable.push_back(std::make_pair(5,10));
      loopVariable.push_back(std::make_pair(10,10));
      loopVariable.push_back(std::make_pair(15,10));
      loopVariable.push_back(std::make_pair(20,10));

      loopVariable.push_back(std::make_pair(2,50));
      loopVariable.push_back(std::make_pair(3,50));
      loopVariable.push_back(std::make_pair(4,50));
      loopVariable.push_back(std::make_pair(5,50));
      loopVariable.push_back(std::make_pair(10,50));
      loopVariable.push_back(std::make_pair(15,50));
      loopVariable.push_back(std::make_pair(20,50));

      loopVariable.push_back(std::make_pair(2,100));
      loopVariable.push_back(std::make_pair(3,100));
      loopVariable.push_back(std::make_pair(4,100));
      loopVariable.push_back(std::make_pair(5,100));
      loopVariable.push_back(std::make_pair(10,100));
      loopVariable.push_back(std::make_pair(15,100));
      loopVariable.push_back(std::make_pair(20,100));

      loopVariable.push_back(std::make_pair(2,300));
      loopVariable.push_back(std::make_pair(3,300));
      loopVariable.push_back(std::make_pair(4,300));
      loopVariable.push_back(std::make_pair(5,300));
      loopVariable.push_back(std::make_pair(10,300));
      loopVariable.push_back(std::make_pair(15,300));
      loopVariable.push_back(std::make_pair(20,300));

      loopVariable.push_back(std::make_pair(2,400));
      loopVariable.push_back(std::make_pair(3,400));
      loopVariable.push_back(std::make_pair(4,400));
      loopVariable.push_back(std::make_pair(5,400));
      loopVariable.push_back(std::make_pair(10,400));
      loopVariable.push_back(std::make_pair(15,400));
      loopVariable.push_back(std::make_pair(20,400));

      loopVariable.clear();
      
      loopVariable.push_back(std::make_pair(3,400));
      loopVariable.push_back(std::make_pair(2,50));
      loopVariable.push_back(std::make_pair(4,300));


   }
   
   for (UInt_t i=0;i<loopVariable.size();i++){    
      
      
      if(i!=0)fFOM->GetMethod()->Reset();
      std::cout << "Set new parameter " << loopVariable[i].first << " / " << loopVariable[i].second << std::endl;
      ((MethodBDT*)(fFOM->GetMethod()))->SetMaxDepth(loopVariable[i].first);     
      ((MethodBDT*)(fFOM->GetMethod()))->SetNodeMinEvents(loopVariable[i].second);     
      
      std::cout << "Set BaseDir " << std::endl;
      
      
      fFOM->GetMethod()->BaseDir()->cd();
      std::cout << "Get Transformation events " << std::endl;
      fFOM->GetMethod()->GetTransformationHandler().CalcTransformations(
                                                                        fFOM->GetMethod()->Data()->GetEventCollection());
      std::cout << "train in optimizer with " << fFOM->GetMethod()->Data()->GetNEvents() << std::endl;
      fFOM->GetMethod()->Train();
      currentFOM = fFOM->GetFOM(); 
      std::cout << "With variable:"  << loopVariable[i].first << " / " << loopVariable[i].second << " we get GetFOM = " << currentFOM << std::endl;
      if (currentFOM > bestFOM) {
         bestFOM = currentFOM;
         ibest   = i;
      }
   }
   
   fFOM->GetMethod()->Reset();
   ((MethodBDT*)(fFOM->GetMethod()))->SetMaxDepth(loopVariable[ibest].first);
   ((MethodBDT*)(fFOM->GetMethod()))->SetNodeMinEvents(loopVariable[ibest].second);
   
   std::cout << "And the winner is : MaxDepth = " << loopVariable[ibest].first << " MinEv="<<  loopVariable[ibest].second  << std::endl;

}

void TMVA::Optimizer::optimizeGA()
{
   // ranges in which the fit varies the parameters (currently hardcoded for BDTs only)
   std::vector<Interval*> ranges;

   ranges.push_back(new Interval(2,20,20-2+1)); // MaxDepth
   ranges.push_back(new Interval(50,500,(500-50)/10+1)); // NNodeMinEvents


   // create the fitter
   TString opt="PopSize=20:Steps=30:Cycles=3:ConvCrit=0.01:SaveBestCycle=5";

   FitterBase* fitter = new GeneticFitter( *this, 
                                           "FitterGA_BDTOptimize", 
                                           ranges, opt );


   std::vector<Double_t> pars;
   for (std::vector<Interval*>::const_iterator parIt = ranges.begin(); parIt != ranges.end(); parIt++) {
      pars.push_back( (*parIt)->GetMean() );
   }

   fitter->CheckForUnusedOptions();

   // perform the fit
   fitter->Run(pars);      
   
   // clean up
   for (UInt_t ipar=0; ipar<ranges.size(); ipar++) delete ranges[ipar];


   std::cout << "And the winner is : MaxDepth = " << pars[0] 
             << " and NNodeMinEvents = " << pars[1] << std::endl;

   fFOM->GetMethod()->Reset();
   ((MethodBDT*)(fFOM->GetMethod()))->SetMaxDepth(Int_t(pars[0]));
   ((MethodBDT*)(fFOM->GetMethod()))->SetNodeMinEvents(Int_t(pars[1]));

}
//_______________________________________________________________________
Double_t TMVA::Optimizer::EstimatorFunction( std::vector<Double_t> & pars)
{
   // return the estimator (from current OptimizerFOM) for the fitting interface



   std::map< std::vector<Double_t> , Double_t>::const_iterator iter;
   iter = fAlreadyTrainedParCombination.find(pars);

   if (iter != fAlreadyTrainedParCombination.end()) {
      // std::cout << "I  had trained  Depth=" <<Int_t(pars[0])
      //           <<" MinEv=" <<Int_t(pars[1])
      //           <<" already --> FOM="<< iter->second <<std::endl; 
      return iter->second;
   }else{

      fFOM->GetMethod()->Reset();
      ((MethodBDT*)(fFOM->GetMethod()))->SetMaxDepth(Int_t(pars[0]));     
      ((MethodBDT*)(fFOM->GetMethod()))->SetNodeMinEvents(Int_t(pars[1]));     
      
      fFOM->GetMethod()->BaseDir()->cd();
      
      fFOM->GetMethod()->GetTransformationHandler().CalcTransformations(
                                                                        fFOM->GetMethod()->Data()->GetEventCollection());
      
      fFOM->GetMethod()->Train();
      
      Double_t currentFOM = fFOM->GetFOM(); 
      
      // std::cout << "currently: Depth=" <<Int_t(pars[0])
      //           <<" MinEv=" <<Int_t(pars[1])
      //           <<" --> FOM="<< currentFOM<<std::endl;

      fAlreadyTrainedParCombination.insert(std::make_pair(pars,-currentFOM));
      return  -currentFOM;
   }
}
