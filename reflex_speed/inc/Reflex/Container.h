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


namespace Reflex {

   //-------------------------------------------------------------------------------
   // Opaque layer around Reflex internal collections
   //
   // Cannot copy, cannot dereference, cannot reference - can only be used
   // to call members immediately from Reflex API, e.g.
   //   scope.GetMembers().Begin()

   namespace Internal {
      class IConstIteratorImpl {
      public:
         virtual ~IConstIteratorImpl() {}

         virtual bool operator==(const IConstIteratorImpl& other) const = 0;
         virtual bool operator!=(const IConstIteratorImpl& other) const = 0;

         virtual IConstIteratorImpl& operator++() = 0;
         virtual IConstIteratorImpl& operator--() = 0;

         virtual void* operator->() const = 0;
      };

      typedef IConstIteratorImpl IConstReverseIteratorImpl;

      class ConstIteratorBase {
      public:
         ConstIteratorBase(IConstIteratorImpl* ii): fIter(ii) {}
         ~ConstIteratorBase(); // deletes fIter

      protected:
         IConstIteratorImpl* fIter;
      };
   }

   template <typename T>
   class ConstIterator: public ConstIteratorBase, public std::iterator<bidirectional_iterator_tag, T> {
   public:
      ConstIterator(Internal::IConstIteratorImpl* ii): ConstIteratorBase(ii) {}

      bool operator==(const ConstIterator& other) const { return *fIter == *other.fIter; }
      bool operator!=(const ConstIterator& other) const { return *fIter != *other.fIter; }

      ConstIterator& operator++() { ++(*fIter); return *this; }
      ConstIterator& operator--() { --(*fIter); return *this; }
      ConstIterator  operator++(int) { ConstIterator ret(*this); ++(*this); return ret; }
      ConstIterator  operator--(int) { ConstIterator ret(*this); --(*this); return ret; }

      const T& operator* () const { return *(T*) fIter->operator->(); }
      const T* operator->() const { return  (T*) fIter->operator->(); }
   };

   typedef ConstIterator ConstReverseIterator;


   namespace Internal {
      class IContainerImpl {
      public:
         virtual ~IContainerImpl() {};

         virtual IConstIteratorImpl& Begin() const = 0
         virtual IConstIteratorImpl& End() const = 0;

         virtual IConstReverseIteratorImpl& RBegin() const = 0;
         virtual IConstReverseIteratorImpl& REnd() const = 0;

         virtual size_t Size() const = 0;
         virtual bool   Empty() const = 0;
      };

      class ContainerBase {
      public:
         ContainerBase(IContainerImpl* coll): fCont(coll) {}
         ~ContainerBase(); // deletes fCont

      protected:
         Internal::IContainerImpl* fCont; // actual collection wrapper
      };
   }

   template <typename T>
   class Container: public Internal::ContainerBase {
   private:
      const Container* operator&() const;  // intentionally not implemented
      Container* operator&();  // intentionally not implemented
      Container(const Container&); // intentionally not implemented
      Container& operator=(const Container& rhs); // intentionally not implemented

   public:
      Container(Internal::IContainerImpl* coll): Internal::ContainerBase(coll) {}
         
      ConstIterator<T> Begin() const { return (ConstIterator<T>&) fCont->Begin(); }
      ConstIterator<T> End() const   { return (ConstIterator<T>&) fCont->End(); }

      ConstReverseIterator<T> RBegin() const { return (ConstReverseIterator<T>&) fCont->RBegin(); }
      ConstReverseIterator<T> REnd() const { return (ConstReverseIterator<T>&) fCont->REnd(); }

      size_t Size() const  { return fCont->Size(); }
      bool   Empty() const { return fCont->Empty(); }
   };

} // namespace Reflex

#endif
