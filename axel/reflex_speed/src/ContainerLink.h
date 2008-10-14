// @(#)root/reflex:$Name: merge_reflex $:$Id$
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_ContainerLink
#define Reflex_ContainerLink

#include <cstddef>
#include <string>
#include "AtomicCount.h"

namespace Reflex {
namespace Internal {
   namespace ContainerTools {

      //-------------------------------------------------------------------------------
      // Base class for contained elements. Provides interface for navigation,
      // creation, deletion.
      class Link1Base {
      public:

         // Default constructor, optionally setting next Link
         Link1Base(const Link1Base* next = 0):
            fNext(next) {}

         // Return next Link*
         const Link1Base* Next() const { return fNext; }

         void InsertAfter(Link1Base* newnext);

         const Link1Base* RemoveAfter();

         // Set previous and next
         void SetNext(const Link1Base* next) { fNext = next; }

      private:
         const Link1Base* fNext; // next link
      }; // class Link1

      //-------------------------------------------------------------------------------

      //-------------------------------------------------------------------------------
      // Doubly linked list element. Provides interface for navigation,
      // creation, deletion.
      class Link2Base {
      public:

         // Default constructor
         Link2Base(): fPrev(0), fNext(0) {}

         // Constructor setting previous and next Link2
         Link2Base(const Link2Base* prev, const Link2Base* next):
            fPrev(prev), fNext(next) {}

         // Return next Link2* given the previous one
         const Link2Base* Next() const { return fNext; }
         // Return previous Link2* given the next one
         const Link2Base* Prev() const { return fPrev; }

         void InsertAfter(Link2Base* newnext);
         Link2Base* RemoveAfter();

         // Set previous and next
         void Set(const Link2Base* prev, const Link2Base* next) {
            fPrev = prev; fNext = next; }
         void SetNext(const Link2Base* next) { fNext = next; }
         void SetPrev(const Link2Base* prev) { fPrev = prev; }

      private:
         const Link2Base* fPrev; // previous node
         const Link2Base* fNext; // next node
      }; // class Link2


      //-------------------------------------------------------------------------------

      class RefCounted {
      public:
         RefCounted(): fRefCount(0) {}
         // Reference Count
         void IncRef() const { ++fRefCount; }
         void DecRef() const { --fRefCount; }
         bool IsReferenced() const { return fRefCount; }

      private:
         mutable AtomicCount fRefCount; // number of iterator references
      };

      class NotRefCounted {
      public:
         void IncRef() const {}
         void DecRef() const {}
         bool IsReferenced() const { return false; }
      };

      template <class REFCOUNTER = RefCounted>
      class Link1: public Link1Base, public REFCOUNTER {
      public:
         typedef REFCOUNTER RefCounter_t;
         // Default constructor, optionally setting next Link
         Link1(const Link1* next = 0): Link1Base(next) {}
      };

      template <class REFCOUNTER = RefCounted>
      class Link2: public Link2Base, public REFCOUNTER {
      public:
         typedef REFCOUNTER RefCounter_t;
         // Default constructor, optionally setting next Link
         // Default constructor, optionally setting previous and next Link2
         Link2(const Link2Base* prev = 0, const Link2Base* next = 0): Link2Base(prev, next) {}
      };

   }; // namespace ContainerTools
}; // namespace Internal
}; // namespace Reflex

#endif // Reflex_ContainerLink
