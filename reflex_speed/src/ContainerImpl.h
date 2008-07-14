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

   template <typename VALUE, typename NODE> class RFLX_API Container_iterator;
   template <typename VALUE, typename NODE> class RFLX_API Container_const_iterator;

   // Base class of adaptor; allows function-wise specialization
   struct RFLX_API ContainerAdaptor {
      // get the key for a value
      template <typename KEY, typename VALUE>
      KEY Key(const VALUE& value) const { return (KEY) value; }

      // get the key for a value, using a pre-allocated key buffer
      template <typename KEY, typename VALUE>
      const KEY& Key(const VALUE& value, KEY& buf) const { return (buf = Key<KEY, VALUE>(value)); }

      // test whether the key for a value matches the given key
      template <typename KEY, typename VALUE>
      bool KeyMatches(const KEY& key, const VALUE& value) const { return (key == Key<KEY, VALUE>(value)); }

      // test whether the key for a value matches the given key, using a pre-allocated key buffer
      template <typename KEY, typename VALUE>
      bool KeyMatches(const KEY& key, const VALUE& value, KEY& buf) const { return (key == Key<KEY, VALUE>(value, buf)); }

      // get the hash for a key
      template <typename KEY>
      Hash_t Hash(const KEY& key) const { return (Hash_t) key; }

      // get the hash for a value
      template <typename KEY, typename VALUE>
      Hash_t ValueHash(const VALUE& value) const { return Hash<KEY>( Key<KEY, VALUE>(value) ); }

      // set a value to invalid (e.g. for iterators pointing to removed nodes)
      template <typename VALUE>
      void Invalidate(VALUE& value) const { value = VALUE(); }

      // check whether a value is invalidated (e.g. for iterators pointing to removed nodes)
      template <typename VALUE>
      bool IsInvalidated(const VALUE& value) const { return value == VALUE(); }
   
      // specialization for pointer values: invalid means NULL pointer
      template <typename U>
      void Invalidate(U* &u) const { u = 0; }
      template <typename U>
      void IsInvalidated(const U* &u) const { return !u; }
   };

   // Adaptor class used by Container
   template <typename KEY, typename VALUE>
   struct RFLX_API ContainerAdaptorT: public ContainerAdaptor {
      // get the key for a value
      KEY Key(const VALUE& value) const { return ContainerAdaptor::Key<KEY, VALUE>(value); }
      // get the key for a value, using a pre-allocated key buffer
      const KEY& Key(const VALUE& value, KEY& buf) const { return ContainerAdaptor::Key<KEY, VALUE>(value, buf); }
      // test whether the key for a value matches the given key
      bool KeyMatches(const KEY& key, const VALUE& value) const { return ContainerAdaptor::KeyMatches<KEY, VALUE>(key, value); }
      // test whether the key for a value matches the given key, using a pre-allocated key buffer
      bool KeyMatches(const KEY& key, const VALUE& value, KEY& buf) const { return ContainerAdaptor::KeyMatches<KEY, VALUE>(key, value, buf); }

      // get the hash for a key
      Hash_t Hash(const KEY& key) const { return ContainerAdaptor::Hash<KEY>(key); }
      // get the hash for a value
      Hash_t ValueHash(const VALUE& value) const { return ContainerAdaptor::ValueHash<KEY, VALUE>(value); }
      // get a value that signals an invalidated value (e.g. for iterators pointing to removed nodes)
      void Invalidate(VALUE& value) const { ContainerAdaptor::Invalidate<VALUE>(value); }
      // check whether a value is invalidated (e.g. for iterators pointing to removed nodes)
      bool IsInvalidated(const VALUE& value) const { return ContainerAdaptor::IsInvalidated<VALUE>(value); }
   };

   
   // specialization for std::string key: hash uses Reflex's Hash()
   template <>
   Hash_t ContainerAdaptor::Hash(const std::string& key) const { return StringHash(key); }

   // specialization for std::string key: hash uses Reflex's Hash()
   template <>
   Hash_t ContainerAdaptor::Hash(const char* const & key) const { return StringHash(key); }

   enum RFLX_API EUniqueness {
      kMANY   = 0, // for UNIQUENESS: allow multiple instances of the same object in the container
      kUNIQUE = 1 // for UNIQUENESS: allow container to hold only one instance of each object
   };

   template <typename KEY, typename VALUE, EUniqueness UNIQUENESS = kUNIQUE, class ADAPTOR = ContainerAdaptorT<KEY, VALUE> >
   class RFLX_API ContainerImpl: public ContainerBase, public IContainerImpl {
   private:
      class Node: public ContainerImplBase::Link {
      public:
         Node(const VALUE& obj): fObj(obj) {}
         VALUE  fObj;
      }; // class Node

      class NodeHelper: public ContainerTools::INodeHelper {
      public:
         bool IsInvalidated(const Link* link) const {
            return fAdaptor.IsInvalidated(static_cast<const Node*>(link)->fObj);
         }
      private:
         ADAPTOR fAdaptor;
      };
   public:

      //-------------------------------------------------------------------------------

      typedef Container_iterator<VALUE, Node> iterator;
      typedef Container_const_iterator<VALUE, Node> const_iterator;

      //-------------------------------------------------------------------------------

      // Initialize a default container holding VALUE objects retrievable by KEY.
      // Allocate 17 chunks for now.
      ContainerImpl(/*size_t size = 17*/): ContainerImplBase(sizeof(Node)) {};

      // Initialize a default container holding VALUE objects retrievable by KEY.
      // Allocate psize chunks for now; psize the next element of fgPrimeArraySqrt3
      // greater or equal than the size parameter.
      ContainerImpl(size_t size): ContainerImplBase(sizeof(Node), size) {}
      // Destruct a container
      virtual ~ContainerImpl() {
         REFLEX_RWLOCK_W(fLock);
         RemoveAllNodes();
      }

      Hash_t Hash(const KEY& key) const { return fAdaptor.Hash(key); }
      Hash_t ValueHash(const VALUE& obj) const { return fAdaptor.ValueHash(obj); }

      // Insert VALUE obj into container; return &obj or 0 if not inserted
      const VALUE* Insert(const VALUE& obj) { return Insert(obj, fAdaptor.ValueHash(obj)); }
      // Insert VALUE obj with hash into container; return & obj or 0 if not inserted
      const VALUE* Insert(const VALUE& obj, Hash_t hash) {
         bool canInsert = (UNIQUENESS == kMANY);
         if (!canInsert) {
            canInsert = !ContainsValue(obj, hash);
         }
         if (canInsert) {
            Node* n = new (fNodeArena->New()) Node(obj);
            InsertNode(n, hash);
            return &n->fObj;
         }
         return 0;
      }

      void Remove(const VALUE& obj) {Remove(obj, fAdaptor.ValueHash(obj));}
      void Remove(const VALUE& obj, Hash_t hash) {
         iterator it = FindValue(obj, hash);
         if (it) Remove(it);
      }

      void Remove(const iterator& it) {
         REFLEX_RWLOCK_W(fLock);
         Node* prev     = it.Prev();
         Node* curr     = it.Curr();
         Link* prevprev = prev->Prev(curr);
         prev->RemoveAfter(prevprev);
         DeleteNode(curr);
         --fSize;
      }

      void DeleteNode(Node* node) {
         // Delete the node if it is not referenced anymore, by telling the node
         // arena to delete it. Invalidate the node otherwise.
         // Locking is done by the caller.
         if (!node->IsReferenced()) {
            // depends on the fact that invalidated links are not reachable
            // anymore (as they are removed from the bucket chain), thus
            // fRefCount cannot change between the line above and the deletion.
            fNodeArena->Delete(node);
         } else {
            fAdaptor.Invalidate(node->fObj);
         }
      }

      // leave out const_iterator overloads as these are not part of the API anyway.
      iterator Begin() const { return iterator(*this, *GetNodeHelper(), End()); }
      iterator End() const { return iterator(); }

      // NEED TO IMPLEMENT!!!
      iterator RBegin() const { return iterator(*this, *GetNodeHelper(), End()); }
      iterator REnd() const { return iterator(); }

      virtual void ProxyBegin(ConstIteratorBase& i) const { i.SetImpl(new iterator(Begin(), true)); }
      virtual void ProxyEnd(ConstIteratorBase& i) const {
         static iterator sEnd = End();
         i.SetImpl(new iterator(&sEnd, false));
      }

      virtual void ProxyRBegin(ConstIteratorBase& i) const { i.SetImpl(new iterator(RBegin(), true)); }
      virtual void ProxyREnd(ConstIteratorBase& i) const {
         static iterator sREnd = REnd();
         i.SetImpl(new iterator(&sREnd, false));
      }

      // return the size of the container via the API
      virtual size_t ProxySize() const { return Size(); }
      virtual bool ProxyEmpty() const {
         // return whether the container is empty via the API
         return !Size();
      }



      iterator Find(const KEY& key) const { return Find(key, fAdaptor.Hash(key)); }
      iterator Find(const KEY& key, Hash_t hash) const {
         // Find first entry matching key with given hash
         REFLEX_RWLOCK_R(fLock);
         int posBuckets = hash % fBuckets.size();
         std::string name;
         for (ContainerTools::LinkIter i = fBuckets[posBuckets].Begin(GetNodeHelper(), fNodeArena); i; ++i) {
            if (fAdaptor.KeyMatches(key, static_cast<const Node*>(i.Curr())->fObj))
               return iterator(i, fBuckets.IterAt(posBuckets), End());
         }
         return End();
      }

      iterator Find(const KEY& key, const const_iterator& start) const {
         // Find next match, starting the search after a first match at start.
         // Used as
         //   while (start = Find(key, start) && !check(*start));
         using namespace ContainerTools;
         if (!start) return End();
         KEY buf;
         LinkIter startbucket = start.CurrentBucket();
         iterator ret(start.CurrentLink(), startbucket, End());
         while (++ret && ret.CurrentBucket() == startbucket)
            if (fAdaptor.KeyMatches(key, *ret, buf))
               return ret;
         if (start.NextContainerBegin()) {
            LinkIter iBucket(start.NextContainerBegin().CurrentBucket());
            if (iBucket) {
               const Bucket* bucket0 = static_cast<const Bucket*>(iBucket.Curr());
               const int numBuckets = bucket0->fIndex;
               int posBuckets = fAdaptor.Hash(key) % numBuckets;
               while(posBuckets--)
                  ++iBucket;
               if (iBucket) {
                  // this is the bucket we need to start searching at
                  Bucket* bucket = static_cast<Bucket*>(iBucket.Curr());
                  const iterator substart(bucket->Begin(GetNodeHelper(), fNodeArena),
                                          iBucket,
                                          start.NextContainerBegin().NextContainerBegin());
                  if (substart.CurrentLink()) {
                     // Already the substart could match; Find(key, substart) would skip it, so test here:
                     const Node* subfirstnode = static_cast<const Node*>(substart.CurrentLink().Curr());
                     if (fAdaptor.KeyMatches(key, subfirstnode->fObj, buf))
                        return substart;
                     return Find(key, substart);
                  }
               }
            }
         }
         return End();
      }

      iterator FindValue(const VALUE& obj) const { return Find(fAdaptor.Key(obj), fAdaptor.ValueHash(obj)); }
      iterator FindValue(const VALUE& obj, Hash_t hash) const { return Find(fAdaptor.Key(obj), hash); }

      bool Contains(const KEY& key) const { return Contains(key, fAdaptor.Hash(key)); }
      bool Contains(const KEY& key, Hash_t hash) const { return Find(key, hash); }

      bool ContainsValue(const VALUE& obj) const { return ContainsValue(obj, fAdaptor.ValueHash(obj)); }
      bool ContainsValue(const VALUE& obj, Hash_t hash) const { return FindValue(obj, hash); }

      void SetNext(const ContainerImpl<KEY, VALUE, UNIQUENESS, ADAPTOR>* next) { fNext = next; }
      const ContainerImpl<KEY, VALUE, UNIQUENESS, ADAPTOR>* GetNext() const { return fNext; }

   protected:
      void RemoveAllNodes() {
         for (iterator iNode = Begin(); iNode; ++iNode)
            DeleteNode(static_cast<Node*>(iNode.CurrentLink().Curr()));
      }

      virtual Hash_t GetHash(const Link* node) const
      { return fAdaptor.ValueHash(static_cast< const Node* >(node)->fObj); }

      virtual bool IsNodeInvalidated(const Link* node) const {
         return fAdaptor.IsInvalidated(static_cast<const Node*>(node)->fObj);
      }

      static NodeHelper* GetNodeHelper() {
         static NodeHelper helper;
         return &helper;
      }

   private:
      ADAPTOR fAdaptor;
      const ContainerImpl<KEY, VALUE, UNIQUENESS, ADAPTOR>* fNext; // next container for chained iteration

   }; // class ContainerImpl

   //-------------------------------------------------------------------------------

   namespace Internal {
      template <typename VALUE, class NODE>
      class RFLX_API Container_const_iterator: public ContainerImplBase_iterator {
      public:
         typedef ContainerImplBase_iterator CBIter;
         typedef ContainerTools::LinkIter LinkIter;
         typedef ContainerTools::LinkIter BucketIter;
         typedef ContainerTools::INodeHelper INodeHelper;

         Container_const_iterator() {}

         Container_const_iterator(const LinkIter& linkiter, const BucketIter& bucketiter,
            const CBIter& nextContainer):
         CBIter(linkiter, bucketiter, nextContainer) {}

         Container_const_iterator(const ContainerImplBase& container, const INodeHelper& helper,
            const CBIter& nextContainer):
         CBIter(container, helper, nextContainer) {}

         Container_const_iterator& operator++() {
            CBIter::operator++();
            return *this;
         }

         Container_const_iterator operator++(int) {
            Container_const_iterator ret = *this;
            CBIter::operator++();
            return ret;
         }

         const NODE* Prev() const { return static_cast<NODE*>(CBIter::CurrentLink().Prev()); }
         const NODE* Curr() const { return static_cast<NODE*>(CBIter::CurrentLink().Curr()); }

         const VALUE* operator->() const { return &(Curr()->fObj); }
         const VALUE& operator*() const  { return Curr()->fObj; }
      }; // class Container_const_iterator

      //-------------------------------------------------------------------------------

      template <typename VALUE, class NODE>
      class RFLX_API Container_iterator: public Container_const_iterator<VALUE, NODE> {
      public:
         typedef ContainerImplBase_iterator CBIter;
         typedef ContainerTools::LinkIter LinkIter;
         typedef ContainerTools::LinkIter BucketIter;
         typedef ContainerTools::INodeHelper INodeHelper;
         typedef Container_const_iterator<VALUE, NODE> ConstIter;

         Container_iterator() {}

         Container_iterator(const LinkIter& linkiter, const BucketIter& bucketiter,
            const CBIter& nextContainer):
         ConstIter(linkiter, bucketiter, nextContainer) {}

         Container_iterator(const ContainerImplBase& container, const INodeHelper& helper,
            const CBIter& nextContainer):
         ConstIter(container, helper, nextContainer) {}

         Container_iterator& operator++() {
            CBIter::operator++();
            return *this;
         }

         Container_iterator operator++(int) {
            Container_iterator ret = *this;
            CBIter::operator++();
            return ret;
         }

         NODE* Prev() const { return static_cast<NODE*>(CBIter::CurrentLink().Prev()); }
         NODE* Curr() const { return static_cast<NODE*>(CBIter::CurrentLink().Curr()); }

         VALUE* operator->() const { return &(Curr()->fObj); }
         VALUE& operator*() const  { return Curr()->fObj; }
      }; // class Container_iterator
   } // namespace Internal
} // namespace Reflex

#endif
