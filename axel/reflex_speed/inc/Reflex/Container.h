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

         virtual IConstIteratorImpl& operator++() = 0;
         virtual IConstIteratorImpl& operator--() = 0;

         virtual void* operator->() const = 0;
      };

      typedef IConstIteratorImpl IConstReverseIteratorImpl;

      class RFLX_API ConstIteratorBase {
         ConstIteratorBase(IConstIteratorImpl* ii, bool owned = true):
            fDeleteImpl(owned), fImpl(ii) {}
         ~ConstIteratorBase();

         void SetImpl(IConstIteratorImpl* impl, bool owned = true) {
            fImpl = impl; fDeleteImpl = owned;
         }

         bool operator==(const ConstIteratorBase& other) const { return  fImpl->operator==(*other.fImpl); }
         bool operator!=(const ConstIteratorBase& other) const { return !fImpl->operator==(*other.fImpl); }

      protected:
         bool fDeleteImpl; // whether to delete fImpl in the destructor usually false for End()
         IConstIteratorImpl* fImpl; // collection's implementation of the interface
      };
   }

   template <typename T>
   class RFLX_API ConstIterator: public Internal::ConstIteratorBase,
                                 public std::iterator<std::bidirectional_iterator_tag, T> {
   public:
      ConstIterator(Internal::IConstIteratorImpl* ii, bool owned = true):
         Internal::ConstIteratorBase(ii, owned) {}

      ConstIterator& operator++() { fImpl->operator++(); return *this; }
      ConstIterator& operator--() { fImpl->operator--(); return *this; }
      ConstIterator  operator++(int) { ConstIterator ret(*this); ++(*this); return ret; }
      ConstIterator  operator--(int) { ConstIterator ret(*this); --(*this); return ret; }

      const T* operator->() const { return  (T*) fImpl->operator->(); }
      const T& operator* () const { return *operator->(); }
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
         virtual void ProxyBegin(ConstIteratorBase& i) const = 0;
         virtual void ProxyEnd(ConstIteratorBase& i) const = 0;

         virtual void ProxyRBegin(ConstIteratorBase& ) const = 0;
         virtual void ProxyREnd(ConstIteratorBase& ) const = 0;

         virtual size_t ProxySize() const = 0;
         virtual bool   ProxyEmpty() const = 0;
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
         
      ConstIterator<T> Begin() const { ConstIterator<T> ret; fCont->ProxyBegin(ret); return ret; }
      ConstIterator<T> End() const   { ConstIterator<T> ret; fCont->ProxyEnd(ret);   return ret; }

      ConstReverseIterator<T> RBegin() const { ConstReverseIterator<T> ret; fCont->ProxyRBegin(ret); return ret; }
      ConstReverseIterator<T> REnd() const   { ConstReverseIterator<T> ret; fCont->ProxyREnd(ret);   return ret; }

      size_t Size() const  { return fCont->ProxySize(); }
      bool   Empty() const { return fCont->ProxyEmpty(); }
   };

} // namespace Reflex

#endif
