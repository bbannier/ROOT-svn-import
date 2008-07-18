// @(#)root/reflex:$Name: merge_reflex $:$Id$
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_ContainerAdaptorImpl
#define Reflex_ContainerAdaptorImpl

#include <string>
#include "ContainerImpl.h"
#include "Reflex/Type.h"
#include "Reflex/Scope.h"
#include "Reflex/Member.h"

namespace Reflex {
namespace Internal {
   //---- Container Adaptor for Type ----

   template <>
   std::string
   ContainerAdaptor::Key(const Type& type) const {
      return type.Name();
   }

   template <>
   const std::string&
   ContainerAdaptor::Key(const Type& type, std::string& buf) const {
      return type.Name(buf);
   }

   //---- Container Adaptor for Scope ----

   template <>
   std::string
   ContainerAdaptor::Key(const Scope& scope) const {
      return scope.Name();
   }

   template <>
   const std::string&
   ContainerAdaptor::Key(const Scope& scope, std::string& buf) const {
      return scope.Name(buf);
   }

   //---- Container Adaptor for Member ----

   template <>
   std::string
   ContainerAdaptor::Key(const Member& member) const {
      return member.Name();
   }

   template <>
   const std::string&
   ContainerAdaptor::Key(const Member& member, std::string& buf) const {
      return member.Name(buf);
   }

} // namespace Internal
} // namespace Reflex

#endif
