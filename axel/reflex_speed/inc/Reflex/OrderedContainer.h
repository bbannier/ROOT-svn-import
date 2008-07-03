// @(#)root/reflex:$Name: merge_reflex $:$Id$
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Container
#define Reflex_Container

#include <string>
#include "Reflex/Container.h"

namespace Reflex {
   template <typename KEY, typename VALUE, class ADAPTOR = ContainerAdaptor<KEY, VALUE> >
   class OrderedContainer {
   protected:
      typedef Internal::ContainerBase::Link Link_t;

      class LinkedObj: public Link_t {
      public:
         LinkedObj(const T& obj, LinkedObj* prev): fObj(obj), fPrev(prev)
         {
            // prev's old next is assumed to be 0, so prevprev is prev->Prev(0):
            if (prev) prev->InsertAfter(prev->Prev(0), this);
         }
         const std::string& Name(std::string& name) const { return fObj.Name(name); }
         std::string Name() const { return fObj.Name(); }
         bool operator!= (const LinkedObj& rhs) const { return fObj != rhs.fObj; }

         T          fObj;
         LinkedObj *fPrev;
      };

      typedef typename Container<LinkedObj>::const_iterator ContIter_t;

   public:
      class const_iterator {
      public:
         const_iterator() {}
         const_iterator(const ContIter_t& icont): fLinkIter(icont->fPrev, &(*icont)) {}
         const_iterator(const Link_t::const_iterator& ilink): fLinkIter(ilink) {}

         bool operator == (const const_iterator& rhs) const
         { return rhs.fLinkIter == fLinkIter; }
         bool operator != (const const_iterator& rhs) const { return !operator==(rhs); }

         const_iterator& operator++() { ++fLinkIter; return *this; }

         const T* operator->() const { return &static_cast<const LinkedObj&>(*fLinkIter).fObj; }
         const T& operator*() const { return static_cast<const LinkedObj&>(*fLinkIter).fObj; }

      private:
         Link_t::const_iterator fLinkIter;
      };

      OrderedContainer(): fLast(0) {};
      OrderedContainer(size_t size): fHashMap(size), fLast(0) {}
      virtual ~OrderedContainer() {}

      const T* Insert(const T& obj) {return Insert(obj, fHashMe(obj));}
      const T* Insert(const T& obj, Hash_t hash) {
         const LinkedObj* newLast = fHashMap.Insert(LinkedObj(obj, fLast), hash);
         if (newLast) {
            fLast = const_cast<LinkedObj*>(newLast);
            return &newLast->fObj;
         }
         return 0;
      }
      void Remove(const T& obj) {Remove(obj, fHashMe(obj);}
      void Remove(const T& obj, Hash_t hash) {
         ContIter_t ic = fHashMap.Find(LinkedObj(obj, 0), hash);
         Link_t* prev = ic->fPrev;
         prev->RemoveAfter(prev->Prev(&(*ic)));
         fHashMap.Remove(ic);
      }

      const_iterator Find(const HashedString& hs) const {
         ContIter_t ic = fHashMap.Find(hs);
         if (ic == fHashMap.End()) return End();
         return const_iterator(ic);
      }
      const_iterator Find(const HashedString& hs, const const_iterator& start) const {
         ContIter_t ic = fHashMap.Find(hs, start);
         if (ic == fHashMap.End()) return End();
         return const_iterator(ic);
      }

      bool Contains(const T& obj) const {return Find(obj, fHashMe(obj);}
      bool Contains(const T& obj, Hash_t hash) const {
         return fHashMap.Contains(obj, hash);
      }

      const_iterator End() const { return const_iterator(); }
      const_iterator Begin() const { return const_iterator(fFirst.Next(0)); }

      void   Clear() { fHashMap.Clear(); fLast = 0; fFirst.
         (0,0); }
      size_t Entries() const { return fHashMap.Entries(); }
      size_t Size() const { return fHashMap.Entries(); }

   private:
      Container<LinkedObj> fHashMap;
      Link_t               fFirst;
      LinkedObj           *fLast;
   };

