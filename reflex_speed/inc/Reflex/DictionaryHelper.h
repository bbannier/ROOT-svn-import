// @(#)root/reflex:$Id$
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_DictionaryHelper
#define Reflex_DictionaryHelper

namespace Reflex {
   class DictionaryHelper {
   public:
      template <typename T>
      static T* New() { return new T(); }

      template <typename T>
      static T* New(void* addr) { return new (addr) T(); }

      template <typename T>
      static T* New(const T& t) { return new T(t); }

      template <typename T>
      static T* New(void* addr, const T& t) { return new (addr) T(t); }

      template <typename T>
      static void Delete(T* t) { delete t; }

      template <typename T>
      static void* AddressOf(const T& t) { return (void*)(&t); }
   };
}

#endif // Reflex_DictionaryHelper

