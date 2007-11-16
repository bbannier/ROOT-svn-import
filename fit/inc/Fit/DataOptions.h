// @(#)root/fit:$Id: inc/Fit/DataOptions.h,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Wed Aug 30 11:04:59 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class DataOptions

#ifndef ROOT_Fit_DataOptions
#define ROOT_Fit_DataOptions


namespace ROOT { 

   namespace Fit { 


/** 
   DataOptions : simple structure holding the options on how the data are filled 
*/ 
struct DataOptions {


   /** 
      Default constructor: have default options
   */ 
   DataOptions () : 
      fIntegral(false), 
      fUseEmpty(false), 
      fError1(false),
      fUseRange(false)
   {}


   bool fIntegral; 
   bool fUseEmpty; 
   bool fError1; 
   bool fUseRange; 


}; 

   } // end namespace Fit

} // end namespace ROOT


#endif /* ROOT_Fit_DataOptions */
