// @(#)root/reflex:$Name: merge_reflex $:$Id$
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_ContainerImpl
#define Reflex_ContainerImpl

#include <string>
#include "Reflex/Container.h"

#include "ContainerImplBase.h"
#include "ContainerArena.h"
#include "ContainerNode.h"

// re-hash level
#ifndef REFLEX_CONTAINER_REHASH_LEVEL
#define REFLEX_CONTAINER_REHASH_LEVEL 3
#endif

namespace Reflex {
namespace Internal {
   //-------------------------------------------------------------------------------
   //-------------------------------------------------------------------------------
   //-------------------------------------------------------------------------------
   // A hash map container (its type-safe layer)
   //
   //-------------------------------------------------------------------------------

   enum EUniqueness {
      kMany   = 0, // for UNIQUENESS: allow multiple instances of the same object in the container
      kUnique = 1 // for UNIQUENESS: allow container to hold only one instance of each object
   };

   //-------------------------------------------------------------------------------


   template <class NODE>
   class Container_iterator: public IConstIteratorImpl {
   public:
      typedef ContainerTools::NodeArena Arena_t;
      typedef typename NODE::Value_t Value_t;

      Container_iterator(): fCurr(0, 0) {} // == End(), should be as efficient as possible

      Container_iterator(Arena_t* arena, NODE* node):
      fCurr(node, arena), fNext(0) {}

      Container_iterator(Arena_t* arena, NODE* node, const Container_iterator& nextContainer):
      fCurr(node, arena), fNext(CreateNext(nextContainer)) {}

      Container_iterator(const Container_iterator& iter, const Container_iterator& nextContainer):
      fCurr(iter.Curr(), iter.Arena()), fNext(CreateNext(nextContainer)) {}

      operator bool() const { return fCurr; }

      bool operator == (const Container_iterator& rhs) const { return fCurr == rhs.fCurr; }
      bool operator != (const Container_iterator& rhs) const { return fCurr != rhs.fCurr; }

      Arena_t* Arena() const { return fCurr.Arena(); }
      NODE* Curr() const { return fCurr; }
      const Value_t* operator->() const { return &(Curr()->fObj); }
      const Value_t& operator*() const  { return Curr()->fObj; }

      Container_iterator& operator++() {
         // End() is unchanged
         if (!fCurr) return *this;
         fCurr = (NODE*)fCurr->Next();
         while (!fCurr && fNext) {
            // out of buckets, try next container
            Container_iterator* tobedeleted = fNext;
            *this = *fNext;
            delete tobedeleted;
         }
         // else: 
         //    no next node, no next collection: the End().
         return *this;
      }

      // Collection Proxy / IConstIteratorImpl:
      bool ProxyIsEqual(const IConstIteratorImpl& other) const {
         return *this == ((Container_iterator&)other);
      }

      void ProxyForward() { ++(*this); }

      const void* ProxyElement() const { return operator->(); }
      IConstIteratorImpl* ProxyClone() const { return new Container_iterator(*this); }

      Container_iterator NextContainerBegin() const {
         if (fNext)
            return *fNext;
         return Container_iterator();
      }

   private:
      Container_iterator* CreateNext(const Container_iterator& next) {
         if (next) return new Container_iterator(next);
         else return 0;
      }

      NodeRef<NODE> fCurr; // current node pointed to by the iterator
      Container_iterator<NODE>* fNext; // next contaienr used for iteration beyond End()
   }; // class Container_iterator


   //-------------------------------------------------------------------------------

   template <typename NODE> class Container_iterator;
   class TypeInfoType; // for "ProxyByTypeInfo()"

   //-------------------------------------------------------------------------------

   template <typename KEY, typename VALUE, EUniqueness UNIQUENESS = Reflex::Internal::kMany,
      class REFCOUNTER = Reflex::Internal::ContainerTools::RefCounted,
      class NODE = Reflex::Internal::ContainerNode<KEY, VALUE, REFCOUNTER> >
   class ContainerImpl: public ContainerImplBase, public IContainerImpl {
   public:
      typedef Container_iterator<NODE> iterator;
      typedef Container_iterator<NODE> const_iterator;

      //-------------------------------------------------------------------------------

      // Initialize a default container holding VALUE objects retrievable by KEY.
      // Allocate 17 chunks.
      ContainerImpl(const IContainerImpl* other = 0): ContainerImplBase(sizeof(NODE), other) {};

      // Initialize a default container holding VALUE objects retrievable by KEY.
      // Allocate psize chunks for now; psize the next element of fgPrimeArraySqrt3
      // greater or equal than the size parameter.
      ContainerImpl(size_t size, const IContainerImpl* other = 0): ContainerImplBase(sizeof(NODE), size, other) {}
      // Destruct a container
      virtual ~ContainerImpl() {
         REFLEX_RWLOCK_W(fLock);
         DeleteAllNodes();
      }


      iterator Begin() const { return iterator(Arena(), (NODE*)First()); }
      iterator Begin(const iterator& nextContainer) const { return iterator(Arena(), (NODE*)First(), nextContainer); }
      iterator End() const { return iterator(); }

      const typename NODE::Traits_t& Traits() const { return NODE::fgTraits; }
      Hash_t Hash(const KEY& key) const { return Traits().Hash(key); }
      Hash_t ValueHash(const VALUE& value) const { return Traits().ValueHash(value); }

      // Insert VALUE obj into container; return &obj or 0 if not inserted
      const VALUE* Insert(const VALUE& obj) { return Insert(obj, ValueHash(obj)); }
      // Insert VALUE obj with hash into container; return & obj or 0 if not inserted
      const VALUE* Insert(const VALUE& obj, Hash_t hash) {
         bool canInsert = (UNIQUENESS == kMany);
         if (!canInsert) {
            canInsert = !ContainsValue(obj, hash);
         }
         if (canInsert) {
            NODE* n = new (fNodeArena->New()) NODE(obj);
            InsertNode(n, hash);
            return &n->fObj;
         }
         return 0;
      }

      void Remove(const VALUE& obj) {Remove(obj, ValueHash(obj));}
      void Remove(const VALUE& obj, Hash_t hash) {
         iterator it = FindValue(obj, hash);
         Remove(it, hash);
      }

      void Remove(const iterator& it) {Remove(it, ValueHash(*it));}
      void Remove(const iterator& it, Hash_t hash) {
         if (!it) return;
         NODE* curr = it.Curr();
         ContainerTools::Link1Base* prev = 0;
         {
            REFLEX_RWLOCK_R(fLock);
            // locate previous node:
            size_t prevbucket = BucketIndex(hash);
            prev = fBuckets[prevbucket];
            if (prev == curr) {
               prev = 0;

               // The current bucket starts with the current node.
               // We need to update the bucket so it starts with curr->Next()...
               // ...unless curr->Next() is the head of the next bucket
               // ...or curr->Next() is the last node in the buckets
               size_t nextbucket = prevbucket + 1;
               if (nextbucket == fBuckets.size()) {
                  fBuckets[prevbucket] = 0;
               } else {
                  ContainerTools::Link1Base* next = fBuckets[nextbucket];
                  while (!next && nextbucket + 1 < fBuckets.size())
                     next = fBuckets[++nextbucket];
                  if (!next || curr->Next() == next)
                     fBuckets[prevbucket] = 0;
                  else
                     fBuckets[prevbucket] = next;
               }
            }
            while (!prev && prevbucket > 0) {
               // this is not the previous node; we need to move to the previous bucket.
               prev = fBuckets[--prevbucket];
            }
            while (prev && prev->Next() != curr) {
               prev = const_cast<ContainerTools::Link1Base*>(prev->Next());
            }
         }
         if (prev) {
            REFLEX_RWLOCK_W(fLock);
            prev->RemoveAfter();
            curr->Invalidate(Arena());
            --fSize;
         } else {
            std::cerr << "ERROR Reflex::Internal::ContainerImpl::Remove(): cannot find location to remove!" << std::endl;
         }
      }

      iterator Find(const KEY& key) const { return Find(key, Hash(key)); }
      iterator Find(const KEY& key, Hash_t hash) const {
         REFLEX_RWLOCK_R(fLock);
         return Find(key, iterator(Arena(), (NODE*)fBuckets[BucketIndex(hash)]), hash);
      }

      iterator Find(const KEY& key, const iterator& start) const {
         // Find next match, starting the search after a first match at start.
         // Used as
         //   while (start = Find(key, start) && !check(*start));
         Hash_t hash = Hash(key);
         return Find(key, start, hash);
      }

      iterator Find(const KEY& key, const iterator& start, Hash_t hash) const {
         // Find next match, starting the search after a first match at start.
         // Used as
         //   while (start = Find(key, start) && !check(*start));
         if (!start) return End();

         REFLEX_RWLOCK_R(fLock);
         size_t posBuckets = BucketIndex(hash);
         const ContainerTools::Link1Base* endnode = 0;
         if (posBuckets + 1 < fBuckets.size())
            endnode = fBuckets[posBuckets + 1];
         KEY buf;
         for (NODE* c = static_cast<NODE*>(start.Curr()); c && c != endnode; c = (NODE*)c->Next()) {
            if (Traits().KeyMatches(key, c->fObj, buf))
               return iterator(Arena(), c);
            buf = KEY();
         }
         // should now iterate over next container (see start.NextContainerBegin())
         return End();
      }

      iterator FindValue(const VALUE& obj) const { return Find(Traits().Key(obj), ValueHash(obj)); }
      iterator FindValue(const VALUE& obj, Hash_t hash) const { return Find(Traits().Key(obj), hash); }

      bool Contains(const KEY& key) const { return Contains(key, Hash(key)); }
      bool Contains(const KEY& key, Hash_t hash) const { return Find(key, hash); }

      bool ContainsValue(const VALUE& obj) const { return ContainsValue(obj, ValueHash(obj)); }
      bool ContainsValue(const VALUE& obj, Hash_t hash) const { return FindValue(obj, hash); }

      virtual void ProxyBegin(ConstIteratorBase& i) const { i.SetImpl(new iterator(Begin()), true); }
      virtual void ProxyEnd(ConstIteratorBase& i) const {
         static iterator sEnd = End();
         i.SetImpl(&sEnd, false);
      }

      // return the size of the container via the API
      virtual size_t ProxySize() const { return Size(); }
      virtual bool ProxyEmpty() const {
         // return whether the container is empty via the API
         return !Size();
      }


      virtual const void* ProxyByName(const std::string& name) const {
         return ProxyByNameImpl(name);
      }
      const void* ProxyByNameImpl(const KEY& name) const {
         // overload for the Containers with KEY == std::string
         iterator ret = Find(name);
         if (ret) return &(*ret);
         return 0;
      }
      template <typename STRING>
      const void* ProxyByNameImpl(STRING name) const {
         // templated version; only instantiated if ProxyByName(const KEY& name)
         // doesn't match.
         if (fOther) return fOther->ProxyByName(name);
         return 0;
      }

      virtual const void* ProxyByTypeInfo(const std::type_info& ti) const {
         return ProxyByTypeInfoImpl(ti, (const char*)0, (const TypeInfoType*)0);
      }
      const void* ProxyByTypeInfoImpl(const std::type_info& ti, KEY, const VALUE*) const {
         // overload for the Containers with KEY == std::type_info
         iterator ret = Find(ti.name());
         if (ret) return &(*ret);
         return 0;
      }
      template <typename NOTHANDLED>
      const void* ProxyByTypeInfoImpl(const std::type_info& ti, const char*, NOTHANDLED) const {
         // templated version; only instantiated if
         // ProxyByTypeInfoImpl(const std::type_info& ti, const VALUE*) doesn't match.
         if (fOther) return fOther->ProxyByTypeInfo(ti);
         return 0;
      }

      void InsertNode(NODE* node, Hash_t hash) {
         //-------------------------------------------------------------------------------
         // Insert node with hash into the container. The hash defines
         // the container's bucket to store the node in.
         if (InsertNodeBase(node, hash) && !fRehashPaused && NeedRehash())
            Rehash();
      }

      // Reset the elements
      void Clear() {
         REFLEX_RWLOCK_W(fLock);
         DeleteAllNodes();
         // NO! We need to keep our nodearena, we cannot have it deleted!
         // fNodeArena->ReleaseInstance();
         fCollisions = 0;
         fSize = 0;
         fRehashPaused = false;
      }

      void PauseRehash(bool pause = true) {
         // Prevent the container from rehashing.
         // PauseRehash() should be called when inserting a large number of elements;
         // afterwards, rehashing should be turned on again by calling PausRehash(false).
         REFLEX_RWLOCK_R(fLock);
         if (fRehashPaused == pause)
               return;
         REFLEX_RWLOCK_R_RELEASE(fLock);
         {
            REFLEX_RWLOCK_W(fLock);
            fRehashPaused = pause;
         }
         if (!pause && NeedRehash())
            Rehash();
      }

   protected:
      void DeleteAllNodes() {
         // Empty the buckets, deleting all nodes
         for (iterator iNode = Begin(); iNode; ++iNode)
            iNode.Curr()->Invalidate(Arena());
         fBuckets.clear();
      }

   private:

      // Rehash the nodes
      void Rehash();

   }; // class ContainerImpl


} // namespace Internal
} // namespace Reflex


//-------------------------------------------------------------------------------
template <typename KEY, typename VALUE, Reflex::Internal::EUniqueness UNIQUENESS, class REFCOUNTER, class NODE>
inline
void
Reflex::Internal::ContainerImpl<KEY, VALUE, UNIQUENESS, REFCOUNTER, NODE>::Rehash() {
//-------------------------------------------------------------------------------
// Resize fNodesList to reduce collisions. Check with NeedRehash() before
// calling this function. We assume that we have reached the amoutn of collisions
// defined by the Rehash level; to not Rehash that soon again we extend the list
// by at least (Rehash level * Rehash level); the new size will be the next
// prime larger than the current size * (Rehash level * Rehash level).
   REFLEX_RWLOCK_W(fLock);
   size_t newSize = fBuckets.size() * REFLEX_CONTAINER_REHASH_LEVEL;
   int i = 0;
   while (i < 19 && fgPrimeArraySqrt3[i] < newSize) ++i;
   if (i == 19) // exceeds max supported entries...
      return;
   newSize = fgPrimeArraySqrt3[i];
   BucketVector_t oldList = fBuckets;

   fBuckets.resize(newSize);
   fCollisions = 0;
   bool hadRehashPaused = fRehashPaused;
   fRehashPaused = true;
   NODE* c = (NODE*) First();
   NODE* n = 0;
   for (; c; c = n) {
      n = (NODE*) c->Next();
      InsertNode(c, c->Hash());
   }
   fRehashPaused = hadRehashPaused;

   std::cout << "Rehashing from " << oldList.size() << " to " << newSize << " for " << fSize << " entries." << std::endl; // AND REMOVE IOSTREAM, TOO!
}

// reflex-specific specializations etc:
#include "ContainerTraitsImpl.h"

#endif
