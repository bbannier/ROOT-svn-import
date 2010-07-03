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
#include "TGraph.h"

#include "TMVA/MethodBase.h"   
#include "TMVA/MethodBDT.h"   
#include "TMVA/GeneticFitter.h"
#include "TMVA/MinuitFitter.h"
#include "TMVA/Interval.h"
//_______________________________________________________________________
TMVA::Optimizer::Optimizer(MethodBase * const method, TString fomType) 
{
   // Constructor which sets either "Classification or Regression"
   // and the type of Figure of Merit that you've chosen


   //Somehow loop over the different paremters (ListOfOptions) and
   //find the one where

   if ( !(fFOM = new TMVA::OptimizerFOM(method,fomType,fFOMvsIter))){
      std::cout << "ERROR in TMVA::Optimizer, you've probably chosen an FOM not yet implemented"
                << std::endl;
      exit(1);
   }
}

//_______________________________________________________________________
TMVA::Optimizer::~Optimizer() 
{
   // the destructor (delete the OptimizerFOM, store the graph and .. delete it)
   
   fFOM->GetMethod()->BaseDir()->cd();
   Int_t n=Int_t(fFOMvsIter.size());
   Float_t *x = new Float_t[n];
   Float_t *y = new Float_t[n];
   Float_t  ymin=+999999999;
   Float_t  ymax=-999999999;
   std::cout << "make graph with " << n << " points" << std::endl;
   for (Int_t i=0;i<n;i++){
      x[i] = Float_t(i);
      y[i] = fFOMvsIter[i];
      if (ymin>y[i]) ymin=y[i];
      if (ymax<y[i]) ymax=y[i];

      std::cout << "for "<<i<<" fom = " << y[i] << std::endl;
   }

   TH2D   *h=new TH2D(fFOM->GetMethod()->GetMethodName()+"_FOMvsIterHist","",2,0,n,2,ymin*0.95,ymax*1.05);
   TGraph *gFOMvsIter = new TGraph(n,x,y);
   gFOMvsIter->SetName((fFOM->GetMethod()->GetMethodName()+"_FOMvsIter").Data());
   gFOMvsIter->Write();
   h->Write();



   //delete fFOM;

   // delete fFOMvsIter;
} 
//_______________________________________________________________________
void TMVA::Optimizer::optimize(TString optimizationFitType)
{
   if      (optimizationFitType == "Scan"    ) this->optimizeScan();
   else if (optimizationFitType == "GA" || optimizationFitType == "Minuit" ) this->optimizeFit(optimizationFitType);
   else {
      std::cout << "You have chosen as optimization type " << optimizationFitType
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
   
   std::vector< std::vector<Int_t>* >  loopVariable;
   
   if (fFOM->GetMethod()->GetMethodType() == TMVA::Types::kBDT){
      loopVariable.clear();
      for (Int_t idepth=2; idepth<=20; idepth++)
        for (Int_t nEv=50; nEv<=500; nEv+=50)
          for (Int_t nTrees=50; nTrees<=800; nTrees+=50){
            std::vector<Int_t> *tmp = new std::vector<Int_t>; 
            tmp->push_back(idepth); tmp->push_back(nEv); tmp->push_back(nTrees); loopVariable.push_back(tmp);
          }
   }
      
   for (UInt_t i=0;i<loopVariable.size();i++){    
     if(i!=0)fFOM->GetMethod()->Reset();
     ((MethodBDT*)(fFOM->GetMethod()))->SetMaxDepth((*(loopVariable[i]))[0]);     
     ((MethodBDT*)(fFOM->GetMethod()))->SetNodeMinEvents((*(loopVariable[i]))[1]);     
     ((MethodBDT*)(fFOM->GetMethod()))->SetNTrees((*(loopVariable[i]))[2]);     
     
     fFOM->GetMethod()->BaseDir()->cd();
     fFOM->GetMethod()->GetTransformationHandler().CalcTransformations(
                                                                       fFOM->GetMethod()->Data()->GetEventCollection());
     fFOM->GetMethod()->Train();
     currentFOM = fFOM->GetFOM(); 
     
     std::cout << "With variables:";
     for (UInt_t iv=0; iv<(*(loopVariable[i])).size(); iv++) std::cout << (*(loopVariable[i]))[iv]<< "  ";
     std::cout << " we get GetFOM = " << currentFOM << std::endl;
      if (currentFOM > bestFOM) {
         bestFOM = currentFOM;
         ibest   = i;
      }

   }
   
   std::cout << "And the winner is : MaxDepth = " << (*(loopVariable[ibest]))[0] 
             << " and NNodeMinEvents = " << (*(loopVariable[ibest]))[1]
             << " and NTrees = " << (*(loopVariable[ibest]))[2]
             << std::endl;

   fFOM->GetMethod()->Reset();
   ((MethodBDT*)(fFOM->GetMethod()))->SetMaxDepth((*(loopVariable[ibest]))[0]);
   ((MethodBDT*)(fFOM->GetMethod()))->SetNodeMinEvents((*(loopVariable[ibest]))[1]);
   ((MethodBDT*)(fFOM->GetMethod()))->SetNTrees((*(loopVariable[ibest]))[2]);

   
}

void TMVA::Optimizer::optimizeFit(TString optimizationFitType)
{
   // ranges in which the fit varies the parameters (currently hardcoded for BDTs only)
   std::vector<Interval*> ranges;

   ranges.push_back(new Interval(2,20,20-2+1)); // MaxDepth
   ranges.push_back(new Interval(50,500,50)); // NNodeMinEvents
   ranges.push_back(new Interval(50,800,50)); // NTrees

   // create the fitter

   FitterBase* fitter = NULL;

   if ( optimizationFitType == "Minuit"  ) {
     TString opt="";
     fitter = new MinuitFitter(  *this, 
                                 "FitterMinuit_BDTOptimize", 
                                 ranges, opt );
   }else if ( optimizationFitType == "GA"  ) {
     TString opt="PopSize=20:Steps=30:Cycles=3:ConvCrit=0.01:SaveBestCycle=5";
     fitter = new GeneticFitter( *this, 
                                 "FitterGA_BDTOptimize", 
                                 ranges, opt );
   }

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
             << " and NNodeMinEvents = " << pars[1] 
             << " and NTrees = " << pars[2] 
             << std::endl;

   fFOM->GetMethod()->Reset();
   ((MethodBDT*)(fFOM->GetMethod()))->SetMaxDepth(Int_t(pars[0]));
   ((MethodBDT*)(fFOM->GetMethod()))->SetNodeMinEvents(Int_t(pars[1]));
   ((MethodBDT*)(fFOM->GetMethod()))->SetNTrees(Int_t(pars[2]));
   
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
