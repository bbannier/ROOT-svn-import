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
   //---- Container Traits for Type ----

   template <>
   inline std::string
   ContainerTraits::Key(const Type& type) const {
      return type.Name();
   }

   template <>
   inline const std::string&
   ContainerTraits::Key(const Type& type, std::string& buf) const {
      return type.Name(buf);
   }

   //---- Container Traits for Scope ----

   template <>
   inline std::string
   ContainerTraits::Key(const Scope& scope) const {
      return scope.Name();
   }

   template <>
   inline const std::string&
   ContainerTraits::Key(const Scope& scope, std::string& buf) const {
      return scope.Name(buf);
   }

   //---- Container Traits for Member ----

   template <>
   inline std::string
   ContainerTraits::Key(const Member& member) const {
      return member.Name();
   }

   template <>
   inline const std::string&
   ContainerTraits::Key(const Member& member, std::string& buf) const {
      return member.Name(buf);
   }

   //---- Container Traits for OwnedMember ----

   template <>
   inline std::string
   ContainerTraits::Key(const OwnedMember& member) const {
      return member.Name();
   }

   template <>
   inline const std::string&
   ContainerTraits::Key(const OwnedMember& member, std::string& buf) const {
      return member.Name(buf);
   }


   //---- Container Traits for MemberTemplate ----

   template <>
   inline std::string
   ContainerTraits::Key(const MemberTemplate& member) const {
      return member.Name();
   }

   template <>
   inline const std::string&
   ContainerTraits::Key(const MemberTemplate& member, std::string& buf) const {
      return member.Name(buf);
   }


   //---- Container Traits for TypeTemplate ----

   template <>
   inline std::string
   ContainerTraits::Key(const TypeTemplate& member) const {
      return member.Name();
   }

   template <>
   inline const std::string&
   ContainerTraits::Key(const TypeTemplate& member, std::string& buf) const {
      return member.Name(buf);
   }


   //---- Container Traits for OwnedMemberTemplate ----

   template <>
   inline std::string
   ContainerTraits::Key(const OwnedMemberTemplate& member) const {
      return member.Name();
   }

   template <>
   inline const std::string&
   ContainerTraits::Key(const OwnedMemberTemplate& member, std::string& buf) const {
      return member.Name(buf);
   }


   //---- Container Traits for TypeName* ----

   template <>
   inline std::string
   ContainerTraits::Key(TypeName* const & type) const {
      return type->Name();
   }

   template <>
   inline const std::string&
   ContainerTraits::Key(TypeName* const & type, std::string&) const {
      return type->Name();
   }

   //---- Container Traits for ScopeName* ----

   template <>
   inline std::string
   ContainerTraits::Key(ScopeName* const & scope) const {
      return scope->Name();
   }

   template <>
   inline const std::string&
   ContainerTraits::Key(ScopeName* const & scope, std::string&) const {
      return scope->Name();
   }


   //---- Container Traits for ByName(), ByTypeInfo() ----

   template <>
   void*
   ContainerTraits::ProxyByNameImpl<std::string>(const std::string& name, ContainerImplBase* coll) const {
      iterator ret = coll->Find(name);
      if (ret) return &(*ret);
      return 0;
   }

   template <>
   void*
   ContainerTraits::ProxyByTypeInfoImpl<const char*>(const std::type_info& ti, ContainerImplBase* coll) const {
      iterator ret = coll->Find(ti.name());
      if (ret) return &(*ret);
      return 0;
   }


} // namespace Internal
} // namespace Reflex

#endif
