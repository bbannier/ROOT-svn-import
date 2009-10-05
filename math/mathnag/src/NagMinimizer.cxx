// @(#)root/mathmore:$Id$
// Author: L. Moneta Tue Dec 19 15:41:39 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Implementation file for class NagMinimizer

#include "Math/NagMinimizer.h"

#include "NagMinFunctionAdapter.h"
#include "NagMinWorkSpace.h"

//#include "GSLMultiMinimizer.h"

#include "Math/MultiNumGradFunction.h"

#include <cassert>

#include <iostream>
#include <cmath>

#include <nag.h>
#include <nag_stdlib.h>
#include <nage04.h>
#include <nagx04.h>
#include <limits>

namespace ROOT { 

   namespace Math { 



NagMinimizer::NagMinimizer(unsigned int dim ) : 
   fDim(dim),
   fStatus(false),
   fWorkSpace(0), 
   fObjFunc(0)
{
   // Constructor implementation 

   fValues.reserve(10); 
   fNames.reserve(10); 
   fSteps.reserve(10); 

   //fLSTolerance = 0.1; // use 10**-4 
   SetMaxIterations(1000);
   SetPrintLevel(3);

   // if I have the dimension, I can initialize by allocating the arrays
   if (fDim != 0) DoInitialization(); 
}


NagMinimizer::~NagMinimizer () { 
   if (fObjFunc) delete fObjFunc; 
   if (fWorkSpace) delete fWorkSpace; 
}

bool NagMinimizer::SetVariable(unsigned int ivar, const std::string & name, double val, double step) { 
   // set free variable in minimizer 
   // no transformation implemented - so far
   if (ivar > fValues.size() ) return false; 
   if (ivar == fValues.size() ) { 
      fValues.push_back(val); 
      fNames.push_back(name);
      fSteps.push_back(step); 
   }
   else { 
      fValues[ivar] = val; 
      fNames[ivar] = name;
      fSteps[ivar] = step; 
   }
   return true; 
}
      
void NagMinimizer::SetFunction(const ROOT::Math::IMultiGenFunction & func) { 
   // set the function to minimizer 
   // need to calculate numerical the derivatives since are not supported
   //fObjFunc = new MultiNumGradFunction( func); 
   fObjFunc = func.Clone(); 
   if (fDim != fObjFunc->NDim() ) { 
     // re-initialize
     fDim = fObjFunc->NDim(); 
     DoInitialization();
   }

   // set the option to calculate internally derivatives
   nag_opt_nlp_option_set_integer("Derivative Level",0, fWorkSpace->State(), fWorkSpace->ErrState());
}

void NagMinimizer::SetFunction(const ROOT::Math::IMultiGradFunction & func) { 
   // set the function to minimizer (need to clone ??)
   fObjFunc = dynamic_cast< const ROOT::Math::IMultiGradFunction *>(func.Clone() ); 
   fDim = func.NDim(); 

   DoInitialization();
   // set the option to use provided derivatives
   nag_opt_nlp_option_set_integer("Derivative Level",3, fWorkSpace->State(), fWorkSpace->ErrState());
}


void NagMinimizer::DoInitialization() { 

   // in case of a previous initialization
   if (fWorkSpace) delete fWorkSpace; 

   // allocate the required memory 
   fWorkSpace = new NagMinWorkSpace(fDim); // forget now about the constraints  
   if (fWorkSpace->a == 0) {
      fStatus = false;
      delete fWorkSpace;
   }
   else 
      fStatus = true; 
 }


bool NagMinimizer::Minimize() { 

   if (fObjFunc == 0) fStatus = false; 
   if (fWorkSpace == 0) fStatus = false; 

   // set initial parameters of the minimizer
   if (!fStatus) {       
      MATH_ERROR_MSG("NagMinimizer","wrong minimizer status");
      return false;
   } 


   int debugLevel = PrintLevel(); 

   if (debugLevel >=1 ) std::cout <<"Minimize using NagMinimizer " << std::endl; 

   // set print level (to do ) 


   // set initial values in workspace
   std::copy(fValues.begin(),fValues.end(),fWorkSpace->x);
   //  variables are free, set lower/upper bounds
   for (unsigned int i = 0; i < fDim; ++i) { 
     fWorkSpace->bl[i] = -std::numeric_limits<double>::max();
     fWorkSpace->bu[i] = std::numeric_limits<double>::max();
   }

   // call minimization routine
   // no contraints for the moment: use e04wdp to replace confun
   Integer n = fDim; 
   Integer majits = 0; // major number of iterations

   // set the function object in the Nag comm state pointer  
   Nag_Comm comm; 
   comm.p = const_cast<ROOT::Math::IMultiGenFunction *>(fObjFunc); 

   Nag_E04State * state = fWorkSpace->State();
   //NagError     * nagerr = fWorkSpace->ErrState();

   // initialize the solver first 
   nag_opt_nlp_init( state, fWorkSpace->ErrState()); 
   if (fWorkSpace->ErrState()->code != NE_NOERROR ) { 
       MATH_ERROR_MSG("NagMinimizer","initialization of nag_opt_nlp_init failed");
       return false;
   }


   // check if function provides gradient 
   typedef void ( *NagConfun)(Integer *, Integer, Integer, Integer, const Integer  needc[], const double x[], double ccon[], double cjac[], Integer nstate, Nag_Comm *comm); 
   NagConfun dummyConfun = 0;

   if (fUseGradFunc)  
      
      nag_opt_nlp_solve(n, 0, 0, n, n, n, fWorkSpace->a, fWorkSpace->bl, fWorkSpace->bu,  
                        dummyConfun , NagMinFunctionAdapter<ROOT::Math::IMultiGradFunction>::ObjFun, 
                        &majits, fWorkSpace->istate, fWorkSpace->ccon, fWorkSpace->cjac, 
                        fWorkSpace->clamda, &fMinVal, fWorkSpace->grad, fWorkSpace->hess, 
                        fWorkSpace->x, state, &comm, fWorkSpace->ErrState() );

   else 

      nag_opt_nlp_solve(n, 0, 0, n, n, n, fWorkSpace->a, fWorkSpace->bl, fWorkSpace->bu,  
                        dummyConfun , NagMinFunctionAdapter<ROOT::Math::IMultiGenFunction>::ObjFun, 
                        &majits, fWorkSpace->istate, fWorkSpace->ccon, fWorkSpace->cjac, 
                        fWorkSpace->clamda, &fMinVal, fWorkSpace->grad, fWorkSpace->hess, 
                        fWorkSpace->x, state, &comm, fWorkSpace->ErrState() );
     



   //std::cout <<"print Level " << debugLevel << std::endl; 
   //debugLevel = 3; 


   // retrieve x values 
   double * x = fWorkSpace->x; 
   if (x == 0) return false; 
   std::copy(x, x +fDim, fValues.begin() ); 
      
   if (fWorkSpace->ErrState()->code == NE_NOERROR) { 
      if (debugLevel >=1 ) { 
         std::cout << "NagMinimizer: Minimum Found" << std::endl;  
         int pr = std::cout.precision(18);
         std::cout << "FVAL         = " << fMinVal << std::endl;
         std::cout.precision(pr);
//      std::cout << "Edm   = " << fState.Edm() << std::endl;
         std::cout << "Niterations  = " << majits << std::endl;
         for (unsigned int i = 0; i < fDim; ++i) 
            std::cout << fNames[i] << "\t  = " << fValues[i] << std::endl; 
      }
      return true; 
   }
   else { 
      if (debugLevel >= -1 ) { 
         std::cout << "NagMinimizer: Minimization did not converge" << std::endl;  
         std::cout << "Error message is " << fWorkSpace->ErrState()->message << std::endl;
         std::cout << "FVAL         = " << fMinVal << std::endl;
//      std::cout << "Edm   = " << fState.Edm() << std::endl;
         std::cout << "Niterations  = " << majits << std::endl;
      }
      return false; 
   }
   return false; 
}

const double * NagMinimizer::MinGradient() const {
   return fWorkSpace->grad; 
}

   } // end namespace Math

} // end namespace ROOT

