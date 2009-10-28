// @(#)root/html:$Id$
// Author: Axel Naumann 2009-10-26

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TDocumented.h"

#include "TClass.h"
#include "TLibraryDoc.h"
#include "TModuleDoc.h"
#include "TFileDoc.h"
#include "TClassDoc.h"
#include "TTypedefDoc.h"
#include "TFunctionDoc.h"
#include "TDataDoc.h"

//______________________________________________________________________________
Doc::TDocumented::~TDocumented()
{
   // Destructor.
}

//______________________________________________________________________________
Int_t Doc::TDocumented::Compare(const TObject* obj) const {
   // Compare this element with another TDocumented. The order is:
   // classes, typedefs, function members, data members;
   // within each type, the Compare() function of the
   // derived class should not have forwarded to us.
   // Returns 0 when equal, -1 when this is
   // smaller and +1 when bigger (like strcmp).
   if (!obj->InheritsFrom(TDocumented::Class())) {
      Error("Compare", "Cannot compare with object of class %s",
            obj->IsA()->GetName());
      return 0;
   }

   static const TClass* sClasses[] = {
      TLibraryDoc::Class(),
      TModuleDoc::Class(),
      TFileDoc::Class(),
      TClassDoc::Class(),
      TTypedefDoc::Class(),
      TFunctionDoc::Class(),
      TDataDoc::Class()
   };
   TClass* me = IsA();
   TClass* it = obj->IsA();
   for (size_t i = 0; i < sizeof(sClasses) / sizeof(TClass*); ++i) {
      if (me == sClasses[i]) {
         return -1;
      }
      if (it == sClasses[i]) {
         return 1;
      }
   }

   Error("Compare", "Logic error - we cannot end up here");
   return 0;
}
