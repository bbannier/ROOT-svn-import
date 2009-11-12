// @(#)root/mathcore:$Id: TFitResult.h 28961 2009-06-12 15:13:46Z dmgonzal $
// Author: David Gonzalez Maline Tue Nov 10 15:01:24 2009

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

#ifndef ROOT_TFitResult
#define ROOT_TFitResult

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TFitResult                                                           //
//                                                                      //
// Provides a way to view the fit result and to store them.             //
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TNamed.h"
#include "Fit/FitResult.h"

class TFitResult:public TNamed, public ROOT::Fit::FitResult {
public:

   // Default constructor for I/O
   TFitResult(int status = 0): TNamed("TFitResult","Title"), 
                           ROOT::Fit::FitResult() {
      fStatus = status;
   };

   TFitResult(const ROOT::Fit::FitResult& f): TNamed("TFitResult","Title"),
                                              ROOT::Fit::FitResult(f) {};

   // conversor operator to int, so that backward compatibility is not
   // broken.
   operator int()
   {
      return fStatus;
   }

   operator int() const
   {
      return fStatus;
   }

   virtual ~TFitResult() {};

   ClassDef(TFitResult,1)
};

#endif
