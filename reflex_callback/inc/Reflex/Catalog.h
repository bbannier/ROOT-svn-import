// @(#)root/reflex:$Id$
// Author: Axel Naumann, 2009

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Catalog
#define Reflex_Catalog

#include "Kernel.h"
#include "NotifyInfo.h"

namespace Reflex {

   class Callback;
   class CallbackInterface;

   namespace Internal {
      class CatalogImpl;
   }

   class RFLX_API Catalog {
   public:
      typedef int (*FreeCallbackFunc_t)(const NotifyInfo& ni, void* userData);

      Catalog();
      Catalog(const std::string& name);
      Catalog(Internal::CatalogImpl*);
      ~Catalog() {}

      // static instance, == Catalog()
      static Catalog Instance();

      bool operator==(Catalog other) const { return fImpl == other.fImpl; }
      bool operator!=(Catalog other) const { return fImpl != other.fImpl; }

      Type TypeByName(const std::string& name) const;
      Type TypeByTypeInfo(const std::type_info & ti) const;
      Scope ScopeByName(const std::string& name) const;

      // iterator access
      Type_Iterator Type_Begin() const;
      Type_Iterator Type_End() const;
      Reverse_Type_Iterator Type_RBegin() const;
      Reverse_Type_Iterator Type_REnd() const;
      size_t TypeSize() const;
      Type TypeAt(size_t nth) const;

      Scope_Iterator Scope_Begin() const;
      Scope_Iterator Scope_End() const;
      Reverse_Scope_Iterator Scope_RBegin() const;
      Reverse_Scope_Iterator Scope_REnd() const;
      size_t ScopeSize() const;
      Scope ScopeAt(size_t nth) const;

      // shortcut access:
      Scope GlobalScope() const;
      Scope __NIRVANA__() const;

      // Callbacks
      const Callback& RegisterCallback(const CallbackInterface* ci, const NotifySelection& ns) const;
      const Callback& RegisterCallback(FreeCallbackFunc_t callback, const NotifySelection& ns, void* userData = 0) const;
      void UnregisterCallback(const Callback&) const;

      void Unload();

      Internal::CatalogImpl* ToImpl() const { return fImpl; }

   private:
      Internal::CatalogImpl* fImpl; // non-const so it can be used internally
   };
}

#endif // Reflex_Catalog

