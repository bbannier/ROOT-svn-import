// @(#)root/reflex:$Id$
// Author: Axel Naumann, 2009

// Copyright CERN, CH-1211 Geneva 23, 2004-2009, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BUILD
# define REFLEX_BUILD
#endif

#include "Reflex/internal/BuilderContainer.h"
#include "Reflex/Builder/OnDemandBuilder.h"

//-------------------------------------------------------------------------------
Reflex::BuilderContainer::~BuilderContainer() {
//-------------------------------------------------------------------------------
   // Destructor. Remove all registered builders
   Clear();
}

//-------------------------------------------------------------------------------
void
Reflex::BuilderContainer::Insert(OnDemandBuilder* odb) {
//-------------------------------------------------------------------------------
   // Register a builder
   odb->SetNext(fFirst);
   fFirst = odb;
}

//-------------------------------------------------------------------------------
void
Reflex::BuilderContainer::Remove(OnDemandBuilder* odb) {
//-------------------------------------------------------------------------------
   // Unregister a builder
   if (odb == fFirst) {
      fFirst = odb->Next();
   } else {
      OnDemandBuilder* prev = fFirst;
      while (prev && prev->Next() != odb)
         prev = prev->Next();
      if (prev) {
         prev->SetNext(odb->Next());
      }
   }
   odb->SetNext(0);
   odb->UpdateRegistrationInfo(0, 0);
}

//-------------------------------------------------------------------------------
void
Reflex::BuilderContainer::Clear() {
//-------------------------------------------------------------------------------
   // Unregister all builders
   OnDemandBuilder* next = 0;
   for (OnDemandBuilder* odb = fFirst; odb; odb = next) {
      next = odb->Next();
      odb->UpdateRegistrationInfo(0, 0);
      odb->SetNext(0);
   }
   fFirst = 0;
}

//-------------------------------------------------------------------------------
void
Reflex::BuilderContainer::Build() {
//-------------------------------------------------------------------------------
   // Call Build() on all on demand builders and clears the container.
   // Returns true if the builders have changed the reflection data.
   OnDemandBuilder* oldFirst = fFirst;
   // prevent recursive invocation of builders
   fFirst = 0;
   for (OnDemandBuilder* odb = oldFirst; odb; odb = odb->Next())
      odb->Build();
   fFirst = oldFirst;
   Clear();
}
