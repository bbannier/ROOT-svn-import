// @(#)root/reflex:$Name: merge_reflex $:$Id$
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_ContainerTraitsImpl
#define Reflex_ContainerTraitsImpl

#include <string>
#include "ContainerImpl.h"
#include "Reflex/Type.h"
#include "Reflex/TypeTemplate.h"
#include "Reflex/Scope.h"
#include "Reflex/Member.h"
#include "Reflex/MemberTemplate.h"

#include "OwnedMember.h"
#include "OwnedMemberTemplate.h"
#include "TypeName.h"
#include "ScopeName.h"

namespace Reflex {
namespace Internal {

   template <typename VALUE>
   struct ContainerTraits_KeyExtractor<std::string, VALUE> {
      std::string Get(const VALUE& v) { return v.Name(); }
      const std::string& Get(const VALUE& v, std::string& buf) { return v.Name(buf); }
   };

   template <typename VALUE>
   struct ContainerTraits_KeyExtractor<std::string, VALUE*> {
      std::string Get(const VALUE* v) { return v->Name(); }
      const std::string& Get(const VALUE* v, std::string& buf) { return v->Name(buf); }
   };

   template<>
   struct ContainerTraits_KeyExtractor<std::string, Reflex::Internal::ScopeName*> {
      std::string Get(const Reflex::Internal::ScopeName* v) { return v->Name(); }
      const std::string& Get(const Reflex::Internal::ScopeName* v, std::string&) {
         return v->Name(); }
   };

} // namespace Internal
} // namespace Reflex

#endif
