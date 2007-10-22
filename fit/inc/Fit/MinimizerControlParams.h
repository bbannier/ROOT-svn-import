// @(#)root/fit:$Id: inc/Fit/MinimizerControlParams.h,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Mon Oct 23 15:26:20 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class MinimizerControlParams

#ifndef ROOT_Fit_MinimizerControlParams
#define ROOT_Fit_MinimizerControlParams


namespace ROOT { 

   namespace Fit { 


/** 
   Minimizer Control Parameters  
*/ 
class MinimizerControlParams {

public: 

   /** 
      Default constructor setting the default values 
   */ 
   MinimizerControlParams () : 
      fDebug(0), 
      fMaxCalls(0),  // 0 means leave to the minimizer to decide  
      fMaxIter(0), 
      fTol(0.001)
   {}

   /** 
      Destructor (no operations)
   */ 
   ~MinimizerControlParams () {}

   /** minimizer configuration parameters **/

   /// set print level
   int PrintLevel() const { return fDebug; }

   ///  max number of function calls
   unsigned int MaxFunctionCalls() { return fMaxCalls; } 

   /// max iterations
   unsigned int MaxIterations() { return fMaxIter; } 

   /// absolute tolerance 
   double Tolerance() const { return  fTol; }

   /// set print level
   void SetPrintLevel(int level) { fDebug = level; }

   ///set maximum of function calls 
   void SetMaxFunctionCalls(unsigned int maxfcn) { fMaxCalls = maxfcn; }

   /// set maximum iterations (one iteration can have many function calls) 
   void SetMaxIterations(unsigned int maxiter) { fMaxIter = maxiter; } 

   /// set the tolerance
   void SetTolerance(double tol) { fTol = tol; }



private: 

   int fDebug; 
   unsigned int fMaxCalls; 
   unsigned int fMaxIter; 
   double fTol; 
  

}; 

   } // end namespace Fit

} // end namespace ROOT


#endif /* ROOT_Fit_MinimizerControlParams */
