// @(#)root/reflex:$Id$
// Author: Axel Naumann, 2009

// Copyright CERN, CH-1211 Geneva 23, 2004-2009, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_OnDemandBuilder
#define Reflex_OnDemandBuilder

#include "Reflex/Kernel.h"

namespace Reflex {
// not part of the interface
class BuilderContainer;

class RFLX_API OnDemandBuilder {
public:

   enum EBuilderKind {
      kBuildDataMembers,
      kBuildFunctionMembers,

      kNumBuilderKinds
   };

   OnDemandBuilder(): fContext(0), fRegisteredWhere(0), fNext(0) {}
   virtual ~OnDemandBuilder();

   // return whether the builder has changed reflection data
   virtual void Build() = 0;

   void Unregister();

   void UpdateRegistrationInfo(BuilderContainer* cont, void* context);
   void SetNext(OnDemandBuilder* next) { fNext = next; }

   OnDemandBuilder* Next() const { return fNext; }

   void* Context() const { return fContext; }

private:
   void* fContext; // registration context, e.g. ScopeBase*
   BuilderContainer* fRegisteredWhere; // where the builder is registered
   OnDemandBuilder* fNext; // next builder in chain
};
} // namespace Reflex

#endif // Reflex_OnDemandBuilder
