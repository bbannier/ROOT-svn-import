// @(#)root/mathmore:$Id$
// Author: L. Moneta Wed Oct 18 11:48:00 2006

 /**********************************************************************
  *                                                                    *
  * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
  *                                                                    *
  *                                                                    *
  **********************************************************************/


// Header file for class NagMinimizer

#ifndef ROOT_Math_NagMinimizer
#define ROOT_Math_NagMinimizer

#ifndef ROOT_Math_Minimizer
#include "Math/Minimizer.h"
#endif


#ifndef ROOT_Math_IFunctionfwd
#include "Math/IFunctionfwd.h"
#endif

#ifndef ROOT_Math_IParamFunctionfwd
#include "Math/IParamFunctionfwd.h"
#endif


#include <vector>
#include <string> 


/**
   @defgroup MultiMin Multi-dimensional Minimization
   @ingroup NumAlgo

   Classes implementing algorithn for multi-dimensional minimization 
   based on Nag
 */



namespace ROOT { 

   namespace Math { 


      class NagMinWorkSpace; 

   }

   namespace Math { 

/** 
   NagMinimizer class
   Implementation of ROOT::Math::Minimizer based on the GSL multi-dimensional 
   minimization algorithms
   See <A HREF="http://www.gnu.org/software/gsl/manual/html_node/Multidimensional-Minimization.html>"GSL doc</A> 
   from more info on the algorithms. 

   @ingroup Min1D
*/ 
class NagMinimizer : public ROOT::Math::Minimizer {

public: 

   /** 
      Default constructor
   */ 
   NagMinimizer (unsigned int fDim = 0); 


   /** 
      Destructor 
   */ 
   virtual ~NagMinimizer (); 

private:
   // usually copying is non trivial, so we make this unaccessible

   /** 
      Copy constructor
   */ 
   NagMinimizer(const NagMinimizer &) : Minimizer() {}

   /** 
      Assignment operator
   */ 
   NagMinimizer & operator = (const NagMinimizer & rhs) { 
      if (this == &rhs) return *this;  // time saving self-test
      return *this;
   }

public: 

   /// set the function to minimize
   virtual void SetFunction(const ROOT::Math::IMultiGenFunction & func); 

   /// set gradient the function to minimize
   virtual void SetFunction(const ROOT::Math::IMultiGradFunction & func); 

   /// set free variable 
   virtual bool SetVariable(unsigned int ivar, const std::string & name, double val, double step); 

#ifdef LATER
   /// set lower limit variable  (override if minimizer supports them )
   virtual bool SetLowerLimitedVariable(unsigned int  ivar , const std::string & name , double val , double step , double lower );
   /// set upper limit variable (override if minimizer supports them )
   virtual bool SetUpperLimitedVariable(unsigned int ivar , const std::string & name , double val , double step , double upper ); 
   /// set upper/lower limited variable (override if minimizer supports them )
   virtual bool SetLimitedVariable(unsigned int ivar , const std::string & name , double val , double step , double /* lower */, double /* upper */); 
   /// set fixed variable (override if minimizer supports them )
   virtual bool SetFixedVariable(unsigned int /* ivar */, const std::string & /* name */, double /* val */);  
#endif

   /// method to perform the minimization
   virtual  bool Minimize(); 

   /// return minimum function value
   virtual double MinValue() const { return fMinVal; } 

   /// return expected distance reached from the minimum
   virtual double Edm() const { return 0; } // not impl. }

   /// return  pointer to X values at the minimum 
   virtual const double *  X() const { return &fValues.front(); } 

   /// return pointer to gradient values at the minimum 
   virtual const double *  MinGradient() const; 

   /// number of function calls to reach the minimum 
   virtual unsigned int NCalls() const { return 0; } // not yet ipl.  

   /// this is <= Function().NDim() which is the total 
   /// number of variables (free+ constrained ones) 
   virtual unsigned int NDim() const { return fDim; }   

   /// number of free variables (real dimension of the problem) 
   /// this is <= Function().NDim() which is the total 
   virtual unsigned int NFree() const { return fDim; }  

   /// minimizer provides error and error matrix
   virtual bool ProvidesError() const { return true; } 

   /// return errors at the minimum 
   virtual const double * Errors() const { 
      static std::vector<double> err; 
      err.resize(fDim);
      return &err.front(); 
   }

   /** return covariance matrices elements 
       if the variable is fixed the matrix is zero
       The ordering of the variables is the same as in errors
   */ 
   virtual double CovMatrix(unsigned int , unsigned int ) const { return 0; }

   /// minos error for variable i, return false if Minos failed
   virtual bool GetMinosError(unsigned int , double & /* errLow */ , double & /* errUp */ ) { return false; }

   /// return reference to the objective function
   ///virtual const ROOT::Math::IGenFunction & Function() const; 


protected: 

   /// initialize workspace
   void DoInitialization(); 

private: 
   
   
   unsigned int fDim;      // dimension of the function to be minimized 
   bool fStatus;           // flag indicating status (if false an error occured and minimization will fail)  
   bool fUseGradFunc;       // use gradient provided by funciton (otherwise calculate internally) 

   ROOT::Math::NagMinWorkSpace * fWorkSpace;  // pointer to min workspace containing nag state and all info
 
   const ROOT::Math::IMultiGenFunction * fObjFunc; 
   
   double fMinVal; 

   mutable std::vector<double> fValues;
   //mutable std::vector<double> fErrors;
   std::vector<double> fSteps;
   std::vector<std::string> fNames;


}; 

   } // end namespace Fit

} // end namespace ROOT



#endif /* ROOT_Math_NagMinimizer */
