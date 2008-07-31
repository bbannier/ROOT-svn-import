// @(#)root/reflex:$Name: merge_reflex $:$Id$
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_OrderedContainerImpl
#define Reflex_OrderedContainerImpl

#include <string>
#include "Reflex/Container.h"

namespace Reflex {
namespace Internal {

   template <typename VALUE>
   class LinkedObj: public ContainerTools::Link {
   public:
      LinkedObj(const VALUE& obj, LinkedObj* prev): fObj(obj), fPrev(prev)
      {
         // prev's old next is assumed to be 0, so prevprev is prev->Prev(0):
         if (prev) prev->InsertAfter(prev->Prev(0), this);
      }
      bool operator!= (const LinkedObj& rhs) const { return fObj != rhs.fObj; }

      VALUE      fObj;
      LinkedObj *fPrev;
   };

   // Container Adapter spacializations:
   // get the key for a value
   template < typename KEY, typename VALUE, typename TRAITS >
   struct ContainerTraitsLinkedObjT {
      static TRAITS fValueTraits;
      // get the key for a value
      KEY Key(const LinkedObj<VALUE>& value) const { return fValueTraits.Key(value.fObj); }
      // get the key for a value, using a pre-allocated key buffer
      const KEY& Key(const LinkedObj<VALUE>& value, KEY& buf) const { return fValueTraits.Key(value.fObj, buf); }
      // test whether the key for a value matches the given key
      bool KeyMatches(const KEY& key, const LinkedObj<VALUE>& value) const { return fValueTraits.KeyMatches(key, value.fObj); }
      // test whether the key for a value matches the given key, using a pre-allocated key buffer
      bool KeyMatches(const KEY& key, const LinkedObj<VALUE>& value, KEY& buf) const { return fValueTraits.KeyMatches(key, value.fObj, buf); }

      // get the hash for a key
      Hash_t Hash(const KEY& key) const { return fValueTraits.Hash(key); }
      // get the hash for a value
      Hash_t ValueHash(const LinkedObj<VALUE>& value) const { return fValueTraits.ValueHash(value.fObj); }
      // get a value that signals an invalidated value (e.g. for iterators pointing to removed nodes)
      void Invalidate(LinkedObj<VALUE>& value) const { fValueTraits.Invalidate(value.fObj); }
      // check whether a value is invalidated (e.g. for iterators pointing to removed nodes)
      bool IsInvalidated(const LinkedObj<VALUE>& value) const { return fValueTraits.IsInvalidated(value.fObj); }
   };

   template <typename KEY, typename VALUE, EUniqueness UNIQUENESS = kMany, class TRAITS = ContainerTraitsT<KEY, VALUE> >
   class OrderedContainerImpl: public IContainerImpl {
   protected:
      typedef ContainerTools::Link Link_t;
      typedef ContainerTools::LinkIter LinkIter_t;
      typedef LinkedObj<VALUE> LinkedObj_t;
      typedef typename ContainerImpl< KEY, LinkedObj_t, UNIQUENESS, ContainerTraitsLinkedObjT<KEY, VALUE, TRAITS > > Cont_t;
      typedef typename Cont_t::iterator ContIter_t;

   public:
      class iterator: public IConstIteratorImpl {
      public:
         iterator() {}
         iterator(const ContIter_t& icont): fLinkIter(icont ? icont->fPrev : 0, icont ? &(*icont) : 0) {}
         iterator(const LinkIter_t& ilink): fLinkIter(ilink) {}
         iterator(const iterator& other): fLinkIter(other.fLinkIter) {}

         bool operator == (const iterator& rhs) const
         { return rhs.fLinkIter == fLinkIter; }
         bool operator != (const iterator& rhs) const { return !operator==(rhs); }
         operator bool() const { return fLinkIter; }

         iterator& operator++() { ++fLinkIter; return *this; }

         const VALUE* operator->() const { return &static_cast<const LinkedObj<VALUE> &>(*fLinkIter.Curr()).fObj; }
         const VALUE& operator*() const { return static_cast<const LinkedObj<VALUE> &>(*fLinkIter.Curr()).fObj; }

         iterator& Reverse() {
            // Convert a forward iterator into a backward iterator and vice versa
            fLinkIter.Reverse();
            return *this;
         }

         // Collection Proxy / IConstIteratorImpl:
         bool ProxyIsEqual(const IConstIteratorImpl& other) const {
            return *this == ((iterator&)other);
         }

         void ProxyForward() { ++(*this); }

         const void* ProxyElement() const { return operator->(); }
         IConstIteratorImpl* ProxyClone() const { return new iterator(*this); }

      private:
         LinkIter_t fLinkIter;
      };

      OrderedContainerImpl(): fLast(0) {};
      OrderedContainerImpl(size_t size): fHashMap(size), fLast(0) {}
      virtual ~OrderedContainerImpl() {}

      iterator End() const { return iterator(); }
      iterator Begin() const { return iterator(LinkIter_t(fHashMap.GetNodeHelper(), fHashMap.Arena(),
         const_cast<Link_t*>(&fFirst), fFirst.Next(0))); }

      // NEED TO IMPLEMENT!!!
      iterator RBegin() const { return iterator(LinkIter_t(fHashMap.GetNodeHelper(), fHashMap.Arena(), 0, fLast)).Reverse(); }
      iterator REnd() const { return iterator(); }

      const VALUE* Insert(const VALUE& obj) {return Insert(obj, fHashMe(obj));}
      const VALUE* Insert(const VALUE& obj, Hash_t hash) {
         const LinkedObj* newLast = fHashMap.Insert(LinkedObj(obj, fLast), hash);
         if (newLast) {
            if (!fLast)
               fFirst.Set(0, newLast);
            fLast = const_cast<LinkedObj*>(newLast);
            return &newLast->fObj;
         }
         return 0;
      }
      void Remove(const VALUE& obj) {Remove(obj, fHashMe(obj));}
      void Remove(const VALUE& obj, Hash_t hash) {
         iterator iter(fHashMap.Find(LinkedObj(obj, 0), hash));
         Remove(iter);
      }
      void Remove(const iterator& it) {
         if (!it) return;
         LinkIter_t ic = it.fLinkIter;
         Link_t* prev = ic->fPrev;
         prev->RemoveAfter(prev->Prev(&(*ic)));
         fHashMap.Remove(ic);
      }

      iterator Find(const KEY& key) const { return fHashMap.Find(hs); }
      iterator Find(const KEY& key, Hash_t hash) const { return fHashMap.Find(hs, hash); }
      iterator Find(const KEY& key, const iterator& start) const { return fHashMap.Find(hs, start); }

      iterator FindValue(const VALUE& obj) const {
         return fHashMap.FindValue(obj); }
      iterator FindValue(const VALUE& obj, Hash_t hash) const { return Find(fTraits.Key(obj), hash); }

      bool Contains(const VALUE& obj) const {return Find(obj, fHashMe(obj));}
      bool Contains(const VALUE& obj, Hash_t hash) const { return fHashMap.Contains(obj, hash); }

      void   Clear() { fHashMap.Clear(); fLast = 0; fFirst.Set(0,0); }
      size_t Size() const { return fHashMap.Size(); }

      virtual void ProxyBegin(ConstIteratorBase& i) const { i.SetImpl(new iterator(Begin()), true); }
      virtual void ProxyEnd(ConstIteratorBase& i) const {
         static iterator sEnd = End();
         i.SetImpl(&sEnd, false);
      }

      virtual void ProxyRBegin(ConstIteratorBase& i) const { i.SetImpl(new iterator(RBegin()), true); }
      virtual void ProxyREnd(ConstIteratorBase& i) const {
         static iterator sREnd = REnd();
         i.SetImpl(&sREnd, false);
      }

      // return the size of the container via the API
      virtual size_t ProxySize() const { return Size(); }
      virtual bool ProxyEmpty() const {
         // return whether the container is empty via the API
         return !Size();
      }


   private:
      Cont_t           fHashMap;
      Link_t           fFirst;
      LinkedObj<VALUE> *fLast;
   };
} // namespace Internal
} // namespace Reflex

#endif // Reflex_OrderedContainerImpl
