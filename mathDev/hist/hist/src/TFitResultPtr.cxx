// @(#)root/hist:$Id: TFitResultPtr.cxx 31127 2009-11-12 14:16:09Z dmgonzal $
// Author: David Gonzalez Maline   12/11/09

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TFitResultPtr.h"
#include "TFitResult.h"

ClassImp(TFitResultPtr)

TFitResultPtr::TFitResultPtr(const TFitResultPtr& p)
{
   fPointer = new TFitResult(*p);
}

TFitResultPtr::~TFitResultPtr()
{
   if ( fPointer )
      delete fPointer;
}

TFitResultPtr::operator int() const 
{
   if ( fPointer != 0 )
      return fStatus;
   else
      return fPointer->Status();
}

TFitResult& TFitResultPtr::operator*() const
{
   return *fPointer;
}

TFitResult* TFitResultPtr::operator->() const
{
   return fPointer;
}

TFitResult* TFitResultPtr::Get() const
{
   return fPointer;
}

TFitResultPtr& TFitResultPtr::operator=(const TFitResultPtr& p)
{
   if ( fPointer )
      delete fPointer;
   fPointer = new TFitResult(*p);
   return *this;
}
