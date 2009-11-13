// @(#)root/mathcore:$Id: TFitResultPtr.h 28961 2009-06-12 15:13:46Z dmgonzal $
// Author: David Gonzalez Maline Tue Nov 10 15:01:24 2009

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

#ifndef ROOT_TFitResultPtr
#define ROOT_TFitResultPtr

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TFitResultPtr                                                        //
//                                                                      //
// Provides a way to view the fit result and to store them.             //
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TNamed.h"

class TFitResult;

class TFitResultPtr {
public:

   TFitResultPtr(int status): fStatus(status), fPointer(0) {};
   TFitResultPtr(TFitResult* p): fStatus(0), fPointer(p) {};
   TFitResultPtr(const TFitResultPtr& p);

   operator int() const;
   
   TFitResult& operator*() const;
   TFitResult* operator->() const;
   TFitResult* Get() const;

   TFitResultPtr& operator= (const TFitResultPtr&);

   virtual ~TFitResultPtr();

private:
   int fStatus;
   TFitResult* fPointer;

   ClassDef(TFitResultPtr,1)
};

#endif
