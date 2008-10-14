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

   //-------------------------------------------------------------------------------

   class NodeArena;


   //-------------------------------------------------------------------------------
   // handles both iterator and reverse_iterator due to XOR of prev/next; the
   // initialization decides whether its reverse or not
   // NODE is a ContainerNode< OrderedContainerNode<VALUE> >
   enum EOrderedContainer_iterator_Direction {
      kOrderedContainer_iterator_Direction_Forward,
      kOrderedContainer_iterator_Direction_Backward
   };

   template <class NODE, EOrderedContainer_iterator_Direction DIR>
   struct EOrderedContainer_iterator_Step {
      NODE* Next(NODE* curr) { return (NODE*)curr->Next(); }
   };
   template <class NODE>
   struct EOrderedContainer_iterator_Step<NODE, kOrderedContainer_iterator_Direction_Backward> {
       NODE* Next(NODE* curr) { return (NODE*)curr->Prev(); }
   };


   template <class NODE, class HASHMAPNODE, EOrderedContainer_iterator_Direction DIR>
   class OrderedContainer_iterator: public IConstIteratorImpl {
   public:
      typedef NODE Node_t;
      typedef HASHMAPNODE HashMapNode_t;
      typedef ContainerTools::NodeArena Arena_t;
      typedef typename NODE::Value_t Value_t;
      typedef ContainerTools::Link2Base Link2Base_t;

      OrderedContainer_iterator() {} // == End(), should be as efficient as possible

      OrderedContainer_iterator(Arena_t* arena, Node_t* curr):
      fCurr(ToLink1(curr), arena), fNext(0) {}

      OrderedContainer_iterator(Arena_t* arena, Node_t* curr,
         const OrderedContainer_iterator& nextContainer):
      fCurr(ToLink1(curr), arena), fNext(CreateNext(nextContainer)) {}

      OrderedContainer_iterator(Arena_t* arena, HashMapNode_t* curr):
      fCurr(curr, arena), fNext(0) {}

      OrderedContainer_iterator(Arena_t* arena, HashMapNode_t* curr,
         const OrderedContainer_iterator& nextContainer):
      fCurr(curr, arena), fNext(CreateNext(nextContainer)) {}

      OrderedContainer_iterator(const OrderedContainer_iterator& curr,
         const OrderedContainer_iterator& nextContainer):
      fCurr(curr.fCurr), fNext(CreateNext(curr.NextContainerBegin())) {}

      OrderedContainer_iterator(const Container_iterator<HashMapNode_t>& mapi):
      fCurr(mapi.Curr(), mapi.Arena()) {}


      static HashMapNode_t* ToLink1(Node_t* node) {
         return node ? (HashMapNode_t*) (((char*)node) - kNodeOffset) : 0;
      }

      static Node_t* ToLink2(HashMapNode_t* node) {
         return node ? (Node_t*) (((char*)node) + kNodeOffset) : 0;
      }

      operator bool() const { return fCurr; }

      bool operator == (const OrderedContainer_iterator& rhs) const { return (rhs.fCurr == fCurr); }
      bool operator != (const OrderedContainer_iterator& rhs) const { return (rhs.fCurr != fCurr); }

      Arena_t* Arena() const { return fCurr.Arena(); }
      HashMapNode_t* Curr() const { return fCurr; }
      const Value_t* operator->() const { return &(fCurr->fObj.fObj); }
      const Value_t& operator*() const  { return fCurr->fObj.fObj; }

      OrderedContainer_iterator& operator++() {
         // Iterate over Link2 objects.

         // End() is unchanged
         if (!fCurr) return *this;

         EOrderedContainer_iterator_Step<Node_t, DIR> step;
         fCurr = ToLink1(step.Next(&fCurr->fObj));
         return *this;
      }

      // Collection Proxy / IConstIteratorImpl:
      bool ProxyIsEqual(const IConstIteratorImpl& other) const {
         return *this == ((OrderedContainer_iterator&)other);
      }

      void ProxyForward() { ++(*this); }

      const void* ProxyElement() const { return operator->(); }
      IConstIteratorImpl* ProxyClone() const { return new OrderedContainer_iterator(*this); }

   private:
      enum {
         // Calculate the offset between the enclosing HashMapNode_t and the inner Node_t.
         // Also ensures that HashMapNode_t::Value_t really is Node_t, by doing a dummy
         // const_cast to the expected type.
         kNodeOffset = ((char*)const_cast<Node_t*>(&((const HashMapNode_t*)0x64)->fObj)) - (char*)0x64
      };

      OrderedContainer_iterator* CreateNext(const OrderedContainer_iterator& next) {
         if (next) return new OrderedContainer_iterator(next);
         else return 0;
      }

   private:
      NodeRef<HashMapNode_t> fCurr; // current node pointed to by the iterator
      OrderedContainer_iterator<Node_t, HashMapNode_t, DIR>* fNext; // next container used for iteration beyond End()
   }; // class OrderedContainer_iterator


   //-------------------------------------------------------------------------------

   template <typename VALUE, typename REFCOUNTER>
   class OrderedContainerNode: public ContainerTools::Link2<REFCOUNTER> {
   public:
      typedef ContainerTools::Link2<REFCOUNTER> Link2_t;
      typedef VALUE Value_t;

      OrderedContainerNode(const VALUE& obj, OrderedContainerNode* prev):
      Link2_t(prev, 0), fObj(obj)
      {
         if (prev) prev->InsertAfter(this);
      }

      VALUE fObj;
   };

   //-------------------------------------------------------------------------------

   template <typename VALUE, typename REFCOUNTER>
   struct NodeValidator<OrderedContainerNode<VALUE, REFCOUNTER> >:
   public NodeValidator<VALUE> {
      typedef OrderedContainerNode<VALUE, REFCOUNTER> Node_t;
      typedef NodeValidator<VALUE> Base_t;
      // set a value to invalid (e.g. for iterators pointing to removed nodes)
      static void Invalidate(Node_t& node) { Base_t::Invalidate(node.fObj); }
      // check whether a value is invalidated (e.g. for iterators pointing to removed nodes)
      static bool IsValid(const Node_t& node) { return Base_t::IsValid(node.fObj); }
   };

   //-------------------------------------------------------------------------------

   template <typename KEY, typename ORDCONTNODE>
   struct OrderedContainerNodeTraits: public ContainerNodeTraits<KEY, typename ORDCONTNODE::Value_t> {
      typedef typename ORDCONTNODE::Value_t Value_t;
      typedef ContainerNodeTraits<KEY, Value_t> BaseCNT_t;

      // get the key for a value
      KEY Key(const ORDCONTNODE& node) const {
         ContainerTraits_KeyExtractor<KEY, Value_t> ke;
         return ke.Get(node.fObj); }
      // get the key for a value, using a pre-allocated key buffer
      const KEY& Key(const ORDCONTNODE& node, KEY& buf) const {
         ContainerTraits_KeyExtractor<KEY, Value_t> ke;
         return ke.Get(node.fObj, buf);
      }
      using BaseCNT_t::Key;

      // test whether the key for a value matches the given key, using a pre-allocated key buffer
      bool KeyMatches(const KEY& key, const ORDCONTNODE& node, KEY& buf) const {
         return BaseCNT_t::KeyMatchesT(key, node.fObj, buf); }
      Hash_t ValueHash(const ORDCONTNODE& node) const { return BaseCNT_t::Hash(Key(node.fObj)); }
      Hash_t ValueHash(const Value_t& value) const { return BaseCNT_t::Hash(Key(value)); }
   };

   template <typename KEY, typename VALUE, EUniqueness UNIQUENESS = Reflex::Internal::kMany,
      class REFCOUNTER = Reflex::Internal::ContainerTools::RefCounted,
      class NODE = OrderedContainerNode<VALUE, REFCOUNTER> >
   class OrderedContainerImpl: public IContainerImpl {
   public:
      typedef NODE OrderedNode_t;
      typedef OrderedContainerNodeTraits<KEY, OrderedNode_t> OrderedTraits_t;
      typedef ContainerNode<KEY, OrderedNode_t, Reflex::Internal::ContainerTools::NotRefCounted, OrderedTraits_t> HashMapNode_t;
      typedef ContainerImpl<KEY, OrderedNode_t, UNIQUENESS, ContainerTools::NotRefCounted, HashMapNode_t> Cont_t;
      typedef OrderedContainer_iterator<OrderedNode_t, HashMapNode_t, kOrderedContainer_iterator_Direction_Forward> iterator;
      typedef OrderedContainer_iterator<OrderedNode_t, HashMapNode_t, kOrderedContainer_iterator_Direction_Backward> reverse_iterator;
      typedef OrderedContainer_iterator<OrderedNode_t, HashMapNode_t, kOrderedContainer_iterator_Direction_Forward> const_iterator;
      typedef OrderedContainer_iterator<OrderedNode_t, HashMapNode_t, kOrderedContainer_iterator_Direction_Backward> const_reverse_iterator;

   public:
      OrderedContainerImpl(): fLast(0) {};
      OrderedContainerImpl(size_t size): fHashMap(size), fLast(0) {}
      virtual ~OrderedContainerImpl() {}

      iterator End() const { return iterator(); }
      iterator Begin() const { return iterator(fHashMap.Arena(), (OrderedNode_t*)fFirst.Next()); }

      reverse_iterator RBegin() const { return reverse_iterator(fHashMap.Arena(), fLast); }
      reverse_iterator REnd() const { return reverse_iterator(); }

      const VALUE& First() const { return ((const NODE*)(fFirst.Next(0)))->fObj; }
      const VALUE& Last() const { return fLast->fObj; }

      const typename HashMapNode_t::Traits_t& Traits() const { return HashMapNode_t::fgTraits; }
      Hash_t Hash(const KEY& key) const { return Traits().Hash(key); }
      Hash_t ValueHash(const VALUE& value) const { return Traits().ValueHash(value); }

      const VALUE* Insert(const VALUE& obj) {return Insert(obj, Traits().ValueHash(obj));}
      const VALUE* Insert(const VALUE& obj, Hash_t hash) {
         const OrderedNode_t* newLast = fHashMap.Insert(OrderedNode_t(obj, fLast), hash);
         if (newLast) {
            if (!fLast)
               fFirst.SetNext(newLast);
            fLast = const_cast<OrderedNode_t*>(newLast);
            return &newLast->fObj;
         }
         return 0;
      }
      void Remove(const VALUE& obj) {Remove(obj, Traits().ValueHash(obj));}
      void Remove(const VALUE& obj, Hash_t hash) {
         iterator iter(fHashMap.FindValue(OrderedNode_t(obj, 0), hash));
         Remove(iter);
      }
      void Remove(const iterator& it) {
         if (!it) return;
         OrderedNode_t* curr = &it.Curr()->fObj;
         ContainerTools::Link2Base* prev = const_cast<ContainerTools::Link2Base*>(curr->Prev());
         prev->RemoveAfter();
         fHashMap.Remove(*curr);
      }

      iterator Find(const KEY& key) const { return fHashMap.Find(key); }
      iterator Find(const KEY& key, Hash_t hash) const { return fHashMap.Find(key, hash); }
      iterator Find(const KEY& key, const iterator& start) const { return fHashMap.Find(key, start); }

      iterator FindValue(const VALUE& obj) const {
         return fHashMap.FindValue(obj); }
      iterator FindValue(const VALUE& obj, Hash_t hash) const { return fHashMap.FindValue(obj, hash); }

      bool Contains(const VALUE& obj) const {return fHashMap.Contains(obj);}
      bool Contains(const VALUE& obj, Hash_t hash) const { return fHashMap.Contains(obj, hash); }

      void   Clear() { fHashMap.Clear(); fLast = 0; fFirst.Set(0,0); }
      size_t Size() const { return fHashMap.Size(); }

      virtual void ProxyBegin(ConstIteratorBase& i) const { i.SetImpl(new iterator(Begin()), true); }
      virtual void ProxyEnd(ConstIteratorBase& i) const {
         static iterator sEnd = End();
         i.SetImpl(&sEnd, false);
      }

      virtual void ProxyRBegin(ConstIteratorBase& i) const { i.SetImpl(new reverse_iterator(RBegin()), true); }
      virtual void ProxyREnd(ConstIteratorBase& i) const {
         static reverse_iterator sREnd = REnd();
         i.SetImpl(&sREnd, false);
      }

      // return the size of the container via the API
      virtual size_t ProxySize() const { return Size(); }
      virtual bool ProxyEmpty() const {
         // return whether the container is empty via the API
         return !Size();
      }


   private:
      Cont_t fHashMap;
      ContainerTools::Link2Base fFirst; // empty root element
      OrderedNode_t *fLast;
   };
} // namespace Internal
} // namespace Reflex

#endif // Reflex_OrderedContainerImpl
