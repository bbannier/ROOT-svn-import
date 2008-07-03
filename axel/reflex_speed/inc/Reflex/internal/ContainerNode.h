// @(#)root/reflex:$Name: merge_reflex $:$Id$
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_ContainerNode
#define Reflex_ContainerNode

#include <stddef.h>
#include "Reflex/internal/AtomicCount.h"

namespace Reflex {
   namespace Internal {
      namespace ContainerTools {

         //-------------------------------------------------------------------------------
         // Base class for contained elements. Provides interface for navigation,
         // creation, deletion.
         class Link {
         public:

            // Default constructor, optionally setting previous and next Link
            Link(const Link* prev = 0, const Link* next = 0): fLink((ptrdiff_t)prev ^ (ptrdiff_t)next), fRefCount(0), fDebugPrev(prev), fDebugNext(next) {}

            // Return next Link* given the previous one
            Link* Next(const Link* prev) const { return (Link*)(fLink ^ (XorLink_t)prev); }
            // Return previous Link* given the next one
            Link* Prev(const Link* next) const { return Next(next); }

            Link* Progress(Link* &prev) {
               // Move prev to this and return the next one.
               // curr = curr->Progress(prev);
               Link* next = Next(prev);
               prev = this;
               return next;
            }
            const Link* Progress(const Link* &prev) const {
               // Move prev to this and return the next one.
               // curr = curr->Progress(prev);
               const Link* next = Next(prev);
               prev = this;
               return next;
            }

            void InsertAfter(const Link* prev, Link* newnext);

            Link* RemoveAfter(const Link* prev);

            // Reference Count
            void IncRef() const { ++fRefCount; }
            void DecRef() const { --fRefCount; }
            bool IsReferenced() const { return fRefCount; }

            // Set previous and next
            void Set(const Link* prev, const Link* next) { fLink = (XorLink_t)prev ^ (XorLink_t)next; fDebugPrev = prev; fDebugNext = next; }

         private:
            // type for XOR'ed pointers
            typedef ptrdiff_t XorLink_t;

         private:
            XorLink_t fLink; // XOR of prev, next

            const Link* fDebugPrev;
            const Link* fDebugNext;

            mutable AtomicCount fRefCount; // number of iterator references
         }; // class Link

         //-------------------------------------------------------------------------------

         class NodeArena;

         //-------------------------------------------------------------------------------
         // Interface definition of a node helper class
         class INodeHelper {
         public:
            virtual bool IsInvalidated(const Link* link) const = 0;
         };

         //-------------------------------------------------------------------------------
         // handles both iterator and reverse_iterator due to XOR of prev/next; the
         // initialization decides whether its reverse or not
         class LinkIter {
            public:
               LinkIter(): fPrev(0), fCurr(0) {} // == End(), should be as efficient as possible

               LinkIter(const INodeHelper* helper, NodeArena* arena, Link* prev = 0, Link* curr = 0):
                  fHelper(helper), fArena(arena), fPrev(prev), fCurr(curr) {
                  IncRef(fPrev);
                  IncRef(fCurr);
               }

               LinkIter(const LinkIter& rhs): fHelper(rhs.fHelper), fArena(rhs.fArena), fPrev(rhs.fPrev), fCurr(rhs.fCurr) {
                  IncRef(fPrev);
                  IncRef(fCurr);
               }

               ~LinkIter() {
                  DecRef(fPrev);
                  DecRef(fCurr);
               }

               operator bool() const { return fCurr; }

               LinkIter& operator=(const LinkIter& rhs) {
                  fHelper = rhs.fHelper;
                  fArena = rhs.fArena;
                  fPrev = rhs.fPrev;
                  fCurr = rhs.fCurr;

                  IncRef(fPrev);
                  IncRef(fCurr);

                  return *this;
               }

               LinkIter& operator++() {
               // Iterate over Link objects.

                  // End() is unchanged
                  if (!fCurr) return *this;

                  // Leaving prev:
                  DecRef(fPrev);
                  fCurr = fCurr->Progress(fPrev);
                  IncRef(fCurr);
                  return *this;
               }

               bool operator == (const LinkIter& rhs) const {
                  return (rhs.fCurr == fCurr);
               }
               bool operator != (const LinkIter& rhs) const {
                  return (rhs.fCurr != fCurr);
               }

               Link* Prev() const { return fPrev; }
               Link* Curr() const { return fCurr; }

               const INodeHelper* Helper() const { return fHelper; }
               NodeArena* Arena() const { return fArena; }

            protected:
               void IncRef(const Link* link) const { if (link) link->IncRef(); }
               void DecRef(const Link* link) const {
                  if (link) {
                     link->DecRef();
                     if (fHelper && !link->IsReferenced() && fHelper->IsInvalidated(link)) {
                        ArenaDelete(const_cast<Link*>(link));
                     }
                  }
               }
               void ArenaDelete(Link* link) const;

            private:
               const INodeHelper* fHelper;
               NodeArena*  fArena;
               Link* fPrev;
               Link* fCurr;
            }; // class Link_Iterator
      }; // namespace ContainerTools
   }; // namespace Internal
}; // namespace Reflex

#endif
