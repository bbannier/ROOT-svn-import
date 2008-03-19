// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_CLASS
#define ROOT_CLASS

#include "TObject.h"

class CLASS : public TObject //, public TAttBBox
{
private:
   CLASS(const CLASS&);            // Not implemented
   CLASS& operator=(const CLASS&); // Not implemented

protected:

public:
   CLASS();
   virtual ~CLASS() {}

   // For TAttBBox:
   // virtual void ComputeBBox();
   // If painting is needed:
   // virtual void Paint(Option_t* option="");

   ClassDef(CLASS, 1); // Short description.
};

#endif
