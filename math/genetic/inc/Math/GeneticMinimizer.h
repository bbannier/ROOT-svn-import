// @(#)root/mathcore:$Id

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class GeneticMinimizer

#ifndef ROOT_Math_GeneticMinimizer
#define ROOT_Math_GeneticMinimizer

#include <vector>

#include "Math/Minimizer.h"

#include "TMVA/IFitterTarget.h"
#include "TMVA/Interval.h"

namespace ROOT { 
   namespace Math { 

//_______________________________________________________________________________
/** 
   GeneticMinimizer

   @ingroup MultiMin
*/

class GeneticMinimizer: public ROOT::Math::Minimizer {

public: 

   GeneticMinimizer (int = 0);
   virtual ~GeneticMinimizer ();

   virtual void Clear();
   using ROOT::Math::Minimizer::SetFunction;
   virtual void SetFunction(const ROOT::Math::IMultiGenFunction & func); 

   virtual bool SetLimitedVariable(unsigned int , const std::string& , double , double , double, double);
   virtual bool SetVariable(unsigned int ivar, const std::string & name, double val, double step); 

   virtual  bool Minimize(); 
   virtual double MinValue() const; 
   virtual double Edm() const; 
   virtual const double *  X() const; 
   virtual const double *  MinGradient() const;  
   virtual unsigned int NCalls() const;    

   virtual unsigned int NDim() const;  
   virtual unsigned int NFree() const;  

   virtual bool ProvidesError() const; 
   virtual const double * Errors() const;

   virtual double CovMatrix(unsigned int i, unsigned int j) const;  

   void SetParameters(Int_t nsteps=40, Int_t popSize=300, Int_t SC_steps=10, 
                      Int_t SC_rate=5, Double_t SC_factor=0.95, Double_t convCrit=0.001 );

protected:
   std::vector<TMVA::Interval*> fRanges;
   TMVA::IFitterTarget* fFitness;

   std::vector<double> fResult;


   Int_t fNsteps;
   Int_t fPopSize;
   Int_t fSC_steps;
   Int_t fSC_rate;
   Double_t fSC_factor;
   Double_t fConvCrit;
}; 

   } // end namespace Math
} // end namespace ROOT

#endif /* ROOT_Math_GeneticMinimizer */
