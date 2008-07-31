// @(#)root/reflex:$Name: merge_reflex $:$Id: ContainerTraitsImpl.h 24948 2008-07-25 13:27:07Z axel $
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_ContainerSTLAdaptor
#define Reflex_ContainerSTLAdaptor

#include "Reflex/Container.h"
#include <algorithm>

namespace Reflex {
namespace Internal {
   template <class ITERATOR>
   class STLConstIteratorImpl: public IConstIteratorImpl {
   public:
      STLConstIteratorImpl(const ITERATOR& iter): fIter(iter) {}
      ~STLConstIteratorImpl() {}

      bool ProxyIsEqual(const IConstIteratorImpl& other) const {
         return fIter == ((const STLConstIteratorImpl<ITERATOR>&)other).fIter; }
      void ProxyForward() { ++fIter; }
      const void* ProxyElement() const = { return &(*fIter); }
      IConstIteratorImpl* ProxyClone() const { return new STLConstIteratorImpl<ITERATOR>(*this); }
   private:
      ITERATOR fIter; // the STL iterator object
   };


   template <class T>
   struct NameFinderPredicate {
      NameFinderPredicate(const std::string& name): fName(name) {}
      bool operator()(const T& t) const { return fName == t.Name(); }
      const std::string& fName;
   };

   template <class CONTAINER>
   class ContainerSTLAdaptor: public IContainerImpl {
   public:
      typedef typename CONTAINER::const_iterator const_iterator;
      typedef typename CONTAINER::value_type value_type;
      typedef STLConstIteratorImpl<const_iterator> IterImpl_t;

      ContainerSTLAdaptor(const CONTAINER& cont): fCont(&cont) {}
      virtual ~ContainerSTLAdaptor() {};

      void ProxyBegin(ConstIteratorBase& i) const { i.SetImpl(new IterImpl_t(fCont.begin())); }
      void ProxyEnd(ConstIteratorBase& i) const { i.SetImpl(new IterImpl_t(fCont.end())); }

      void ProxyRBegin(ConstIteratorBase&) const { i.SetImpl(new IterImpl_t(fCont.rbegin())); }
      void ProxyREnd(ConstIteratorBase&) const { i.SetImpl(new IterImpl_t(fCont.rend())); }

      size_t ProxySize() const { return fCont.size(); }
      bool   ProxyEmpty() const { return fCont.empty(); }

      void*  ProxyByName(const std::string& name) const {
         const_iterator it = std::find_if(fCont.begin(), fCont.end(), NameFinderPredicate<value_type>(name));
         if (it != fCont.end())
            return &(*it);
         return 0;
      }
      void*  ProxyByTypeInfo(const std::type_info& /*ti*/) const { return 0; }

   private:
      CONTAINER* fCont; // STL container
   };

   template <class CONTAINER>
   ContainerSTLAdaptor<CONTAINER> Make_ContainerSTLAdaptor(const CONTAINER& cont) {
      return ContainerSTLAdaptor<CONTAINER>(cont);
   }
}
}

#endif // Reflex_ContainerSTLAdaptor
