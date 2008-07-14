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

#include <iterator>
#include "Kernel.h"

namespace Reflex {

   //-------------------------------------------------------------------------------
   // Opaque layer around Reflex internal collections
   //
   // Cannot copy, cannot dereference, cannot reference - can only be used
   // to call members immediately from Reflex API, e.g.
   //   scope.GetMembers().Begin()

   namespace Internal {
      class RFLX_API IConstIteratorImpl {
      public:
         virtual ~IConstIteratorImpl() {}

         virtual bool operator==(const IConstIteratorImpl& other) const = 0;
         virtual bool operator!=(const IConstIteratorImpl& other) const = 0;

         virtual IConstIteratorImpl& operator++() = 0;
         virtual IConstIteratorImpl& operator--() = 0;

         virtual void* operator->() const = 0;
      };

      typedef IConstIteratorImpl IConstReverseIteratorImpl;

      class RFLX_API ConstIteratorBase {
      public:
         ConstIteratorBase(IConstIteratorImpl* ii): fIter(ii) {}
         ~ConstIteratorBase(); // deletes fIter

      protected:
         IConstIteratorImpl* fIter;
      };
   }

   template <typename T>
   class RFLX_API ConstIterator: public Internal::ConstIteratorBase, public std::iterator<std::bidirectional_iterator_tag, T> {
   public:
      ConstIterator(Internal::IConstIteratorImpl* ii): Internal::ConstIteratorBase(ii) {}

      bool operator==(const ConstIterator& other) const { return *fIter == *other.fIter; }
      bool operator!=(const ConstIterator& other) const { return *fIter != *other.fIter; }

      ConstIterator& operator++() { ++(*fIter); return *this; }
      ConstIterator& operator--() { --(*fIter); return *this; }
      ConstIterator  operator++(int) { ConstIterator ret(*this); ++(*this); return ret; }
      ConstIterator  operator--(int) { ConstIterator ret(*this); --(*this); return ret; }

      const T& operator* () const { return *(T*) fIter->operator->(); }
      const T* operator->() const { return  (T*) fIter->operator->(); }
   };

   template <class T>
   class RFLX_API ConstReverseIterator: public ConstIterator<T> {
   public:
      ConstReverseIterator(Internal::IConstIteratorImpl* ii): ConstIterator<T>(ii) {}

      bool operator==(const ConstReverseIterator& other) const { return ConstIterator<T>::operator==(other); }
      bool operator!=(const ConstReverseIterator& other) const { return ConstIterator<T>::operator!=(other); }

      ConstReverseIterator& operator++() { ConstIterator<T>::operator++(); return *this; }
      ConstReverseIterator& operator--() { ConstIterator<T>::operator--(); return *this; }
      ConstReverseIterator  operator++(int) { ConstReverseIterator ret(*this); ++(*this); return ret; }
      ConstReverseIterator  operator--(int) { ConstReverseIterator ret(*this); --(*this); return ret; }
   };


   namespace Internal {
      class RFLX_API IContainerImpl {
      public:
         virtual ~IContainerImpl() {};

         // gets copied into a ConstReverseIterator<T> by Container<T>
         virtual const IConstIteratorImpl& ProxyBegin() const = 0;
         virtual const IConstIteratorImpl& ProxyEnd() const = 0;

         virtual const IConstReverseIteratorImpl& ProxyRBegin() const = 0;
         virtual const IConstReverseIteratorImpl& ProxyREnd() const = 0;

         virtual size_t Size() const = 0;
         virtual bool   Empty() const = 0;
      };

      class RFLX_API ContainerBase {
      public:
         ContainerBase(IContainerImpl* coll): fCont(coll) {}

      protected:
         Internal::IContainerImpl* fCont; // actual collection wrapper
      };
   }

   template <typename T>
   class RFLX_API Container: public Internal::ContainerBase {
   private:
      const Container* operator&() const;  // intentionally not implemented
      Container* operator&();  // intentionally not implemented
      Container(const Container&); // intentionally not implemented
      Container& operator=(const Container& rhs); // intentionally not implemented

   public:
      Container(Internal::IContainerImpl* coll): Internal::ContainerBase(coll) {}
         
      ConstIterator<T> Begin() const { return (ConstIterator<T>&) fCont->ProxyBegin(); }
      ConstIterator<T> End() const   { return (ConstIterator<T>&) fCont->ProxyEnd(); }

      ConstReverseIterator<T> RBegin() const { return (ConstReverseIterator<T>&) fCont->ProxyRBegin(); }
      ConstReverseIterator<T> REnd() const { return (ConstReverseIterator<T>&) fCont->ProxyREnd(); }

      size_t Size() const  { return fCont->Size(); }
      bool   Empty() const { return fCont->Empty(); }
   };

} // namespace Reflex

#endif
