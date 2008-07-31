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
#include <typeinfo>
#include <string>
#include "Reflex/Kernel.h"

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

         virtual bool ProxyIsEqual(const IConstIteratorImpl& other) const = 0;
         virtual void ProxyForward() = 0;
         virtual const void* ProxyElement() const = 0;
         virtual IConstIteratorImpl* ProxyClone() const = 0;
      };

      typedef IConstIteratorImpl IConstReverseIteratorImpl;

      class RFLX_API ConstIteratorBase {
      public:
         ConstIteratorBase(IConstIteratorImpl* impl, bool owned = true):
            fDeleteImpl(owned), fImpl(impl) {}
         ConstIteratorBase(const ConstIteratorBase& other):
            fDeleteImpl(other.fDeleteImpl),
            fImpl(other.fDeleteImpl ? other.fImpl->ProxyClone() : other.fImpl) {}
         ~ConstIteratorBase();

         ConstIteratorBase& operator=(const ConstIteratorBase& other) {
            fDeleteImpl = other.fDeleteImpl;
            if (fDeleteImpl)
               fImpl = other.fImpl->ProxyClone();
            else
               fImpl = other.fImpl;
            return *this;
         }

         void SetImpl(IConstIteratorImpl* impl, bool owned = true) {
            fImpl = impl; fDeleteImpl = owned;
         }

         bool operator==(const ConstIteratorBase& other) const { return  fImpl->ProxyIsEqual(*other.fImpl); }
         bool operator!=(const ConstIteratorBase& other) const { return !fImpl->ProxyIsEqual(*other.fImpl); }

      protected:
         bool fDeleteImpl; // whether to delete fImpl in the destructor usually false for End()
         IConstIteratorImpl* fImpl; // collection's implementation of the interface
      };
   }

   template <typename T>
   class RFLX_API ConstIterator: public Internal::ConstIteratorBase,
                                 public std::iterator<std::forward_iterator_tag, T> {
   public:
      ConstIterator(): Internal::ConstIteratorBase(0, true) {}
      ConstIterator(Internal::IConstIteratorImpl* ii, bool owned = true):
         Internal::ConstIteratorBase(ii, owned) {}

      ConstIterator& operator++() { fImpl->ProxyForward(); return *this; }
      ConstIterator  operator++(int) { ConstIterator ret(*this); ++(*this); return ret; }

      const T* operator->() const { return (const T*) fImpl->ProxyElement(); }
      const T& operator* () const { return *operator->(); }
   };

   template <class T>
   class RFLX_API ConstReverseIterator: public ConstIterator<T> {
   public:
      ConstReverseIterator(Internal::IConstIteratorImpl* ii): ConstIterator<T>(ii) {}

      bool operator==(const ConstReverseIterator& other) const { return ConstIterator<T>::operator==(other); }
      bool operator!=(const ConstReverseIterator& other) const { return ConstIterator<T>::operator!=(other); }

      ConstReverseIterator& operator++() { ConstIterator<T>::operator++(); return *this; }
      ConstReverseIterator  operator++(int) { ConstReverseIterator ret(*this); ++(*this); return ret; }
   };


   namespace Internal {
      class RFLX_API IContainerImpl {
      public:
         virtual ~IContainerImpl() {};

         virtual void ProxyBegin(ConstIteratorBase& i) const = 0;
         virtual void ProxyEnd(ConstIteratorBase& i) const = 0;

         // empty implementation for unordered container
         virtual void ProxyRBegin(ConstIteratorBase&) const {};
         virtual void ProxyREnd(ConstIteratorBase&) const {};

         virtual size_t ProxySize() const = 0;
         virtual bool   ProxyEmpty() const = 0;

         virtual void*  ProxyByName(const std::string& /*name*/) const { return 0; }
         virtual void*  ProxyByTypeInfo(const std::type_info& /*ti*/) const { return 0; }
      };

      class RFLX_API ContainerBase {
      public:
         ContainerBase(const IContainerImpl* coll): fCont(coll) {}

      protected:
         const Internal::IContainerImpl* fCont; // actual collection wrapper
      };
   }

   template <typename T>
   class RFLX_API Container: public Internal::ContainerBase {
   protected:
      const Container* operator&() const;  // intentionally not implemented
      Container* operator&();  // intentionally not implemented
      Container(const Container&); // intentionally not implemented
      Container& operator=(const Container& rhs); // intentionally not implemented

   public:
      typedef ConstIterator<T> const_iterator;

      Container(const Internal::IContainerImpl* coll): Internal::ContainerBase(coll) {}
         
      const_iterator Begin() const { const_iterator ret; fCont->ProxyBegin(ret); return ret; }
      const_iterator End() const   { const_iterator ret; fCont->ProxyEnd(ret);   return ret; }

      size_t Size() const  { return fCont->ProxySize(); }
      bool   Empty() const { return fCont->ProxyEmpty(); }

      T ByName(const std::string& name) const {
         const T* ret = (const T*)fCont->ProxyByName(name);
         if (ret) return *ret;
         return T();
      }
      T ByTypeInfo(const std::type_info& ti) const {
         const T* ret = (const T*)fCont->ProxyByTypeInfo(ti);
         if (ret) return *ret;
         return T();
      }

      template <typename CONT>
      friend const CONT&
         Dummy::Get();
      friend class Scope;
      friend class Type;
   };


   // OrderedContainer is used as a tag (mainly for the user)
   // to signal that the elements keep their order. Because
   // the containers are read-only this is irrelevant for
   // their interface.
   template <typename T>
   class RFLX_API OrderedContainer: public Container<T>
   {
   protected:
      const OrderedContainer* operator&() const;  // intentionally not implemented
      OrderedContainer* operator&();  // intentionally not implemented
      OrderedContainer(const OrderedContainer&); // intentionally not implemented
      OrderedContainer& operator=(const OrderedContainer& rhs); // intentionally not implemented

   public:
      typedef ConstReverseIterator<T> const_reverse_iterator;

      OrderedContainer(const Internal::IContainerImpl* coll): Container<T>(coll) {}

      // reverse iteration only possible for ordered container
      const_reverse_iterator RBegin() const { const_reverse_iterator ret; fCont->ProxyRBegin(ret); return ret; }
      const_reverse_iterator REnd() const   { const_reverse_iterator ret; fCont->ProxyREnd(ret);   return ret; }
   };

} // namespace Reflex

#endif
