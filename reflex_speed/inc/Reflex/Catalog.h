// @(#)root/reflex:$Name:  $:$Id$
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Catalog
#define Reflex_Catalog

#include <string>
#include <typeinfo>
#include "Reflex/Kernel.h"
#include "Reflex/Type.h"
#include "Reflex/Scope.h"
#include "Reflex/Tools.h"

namespace Reflex {

   namespace Internal {
      class CatalogImpl;
   }

   template <typename T> class Container;

   class RFLX_API Catalog {
   public:
      Catalog(Internal::CatalogImpl* catalog = 0): fImpl(catalog) {}
      ~Catalog() {}


      // static instance
      static const Catalog& Instance();

      Type ByName(const std::string& name) const;
      Type ByTypeInfo(const std::type_info & ti) const;
      Scope ScopeByName(const std::string& name) const;

      // iterator access
      const Container<Reflex::Type>  Types() const;
      const Container<Reflex::Scope> Scopes() const;

      // shortcut access:
      Scope GlobalScope() const;

      // for internal use:
      Internal::CatalogImpl* Impl() const { return fImpl; }

      static Type Get(EFUNDAMENTALTYPE);

      static Type Get_char() {return Get(kChar);}
      static Type Get_signed_char() {return Get(kSignedChar);}
      static Type Get_uchar() {return Get(kUnsignedChar);}
      static Type Get_short() {return Get(kShortInt);}
      static Type Get_ushort() {return Get(kUnsignedShortInt);}
      static Type Get_int() {return Get(kInt);}
      static Type Get_uint() {return Get(kUnsignedInt);}
      static Type Get_long() {return Get(kLongInt);}
      static Type Get_ulong() {return Get(kUnsignedLongInt);}
      static Type Get_longlong() {return Get(kLongLong);}
      static Type Get_ulonglong() {return Get(kULongLong);}

      static Type Get_bool() {return Get(kBool);}
      static Type Get_void() {return Get(kVoid);}

      static Type Get_float() {return Get(kFloat);}
      static Type Get_double() {return Get(kDouble);}
      static Type Get_long_double() {return Get(kLongDouble);}

   private:
      Internal::CatalogImpl* fImpl; // non-const so it can be used internally
   };

} // namespace Reflex

#endif
