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

#include "Reflex/Builder/OnDemandBuilder.h"
#include "Reflex/internal/BuilderContainer.h"

//-------------------------------------------------------------------------------
Reflex::OnDemandBuilder::~OnDemandBuilder() {
//-------------------------------------------------------------------------------
   // Unregister the builder upon destruction if it didn't happen before
   Unregister();
}

//-------------------------------------------------------------------------------
void
Reflex::OnDemandBuilder::Unregister() {
//-------------------------------------------------------------------------------
   // Unregister the builder
   if (fRegisteredWhere) {
      fRegisteredWhere->Remove(this);
   }
}

//-------------------------------------------------------------------------------
void
Reflex::OnDemandBuilder::UpdateRegistrationInfo(BuilderContainer* h) {
//-------------------------------------------------------------------------------
   // Called from the registrar.
   if (fRegisteredWhere && fRegisteredWhere != h) {
      throw RuntimeError("Attempt to register OnDemandBuilder twice!");
   }
   fRegisteredWhere = h;
}

