// @(#)root/reflex:$Name: merge_reflex $:$Id$
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_ContainerNode
#define Reflex_ContainerNode

#include <cstddef>
#include <cstring>
#include <string>
#include "AtomicCount.h"
#include "ContainerLink.h"
#include "ContainerArena.h"

namespace Reflex {
   // Hash type used by Reflex::Container
   typedef unsigned long Hash_t;

namespace Internal {
   // Simple and fast hashing routine for std::string
   inline Hash_t StringHash(const char* str) {
      Hash_t hash = 5381;
      while (*str) {
         hash *= 5;
         hash += *(str++);
      }
      return hash;
   }

   // Simple and fast hashing routine for std::string
   inline Hash_t StringHash(const std::string& s) {
      return StringHash(s.c_str());
   }

   // Traits class used by Container
   template <class VALUE>
   struct NodeValidator {
      // set a value to invalid (e.g. for iterators pointing to removed nodes)
      static void Invalidate(VALUE& value) { value = VALUE(); }
      // check whether a value is invalidated (e.g. for iterators pointing to removed nodes)
      static bool IsValid(const VALUE& value) { return value != VALUE(); }
   };

   // specialization for pointer values: invalid means NULL pointer
   template <class VALUE>
   struct NodeValidator<VALUE*> {
      static void Invalidate(VALUE* &value) { value = 0; }
      static bool IsValid(const VALUE* value) { return (value); }
   };

   template <typename KEY, typename VALUE>
   struct ContainerTraits_KeyExtractor {
      KEY Get(const VALUE& v) const { return (KEY) v; }
      const KEY& Get(const VALUE& v, KEY& buf) const { return (buf = (KEY) v); }
   };

   template <typename KEY, typename VALUE>
   struct ContainerNodeTraits {
      // get the key for a value
      KEY Key(const VALUE& value) const {
         ContainerTraits_KeyExtractor<KEY, VALUE> ke;
         return ke.Get(value); }
      // get the key for a value, using a pre-allocated key buffer
      const KEY& Key(const VALUE& value, KEY& buf) const {
         ContainerTraits_KeyExtractor<KEY, VALUE> ke;
         return ke.Get(value, buf);
      }

      // test whether the key for a value matches the given key, using a pre-allocated key buffer
      bool KeyMatches(const KEY& key, const VALUE& value, KEY& buf) const {
         return KeyMatchesT(key, value, buf); }

      // test whether the key for a value matches the given key, using a pre-allocated key buffer
      // templated version for overload
      template <typename K, typename V>
      bool KeyMatchesT(const K& key, const V& value, K& buf) const {
         return key == Key(value, buf); }

      // get the hash for a key
      Hash_t Hash(const KEY& key) const { return HashT(key); }
      // get the hash for a value
      Hash_t ValueHash(const VALUE& value) const { return Hash(Key(value)); }

      template <typename T>
      Hash_t HashT(const T& key) const { return (Hash_t) key; }
      // overloads for std::string key: hash uses Reflex's Hash()
      Hash_t HashT(const std::string& key) const { return StringHash(key); }
      Hash_t HashT(const char* key) const { return StringHash(key); }

      // test whether the key for a value matches the given key, using a pre-allocated key buffer
      bool KeyMatchesT(const char* key, const VALUE& value, const char*&) const {
         const char* valuekey = Key(value);
         return (!key && !valuekey) || !strcmp(key, valuekey);
      }
   }; // struct ContainerNodeTraits


   template <typename KEY, typename VALUE, typename REFCOUNTER,
      typename NODETRAITS = ContainerNodeTraits<KEY, VALUE> >
   class ContainerNode: public ContainerTools::Link1<REFCOUNTER> {
   public:
      typedef KEY Key_t;
      typedef VALUE Value_t;
      typedef REFCOUNTER RefCounter_t;

      typedef NODETRAITS Traits_t;
      typedef ContainerTools::Link1Base Link1Base_t;
      typedef ContainerTools::NodeArena Arena_t;
      typedef ContainerTools::Link1<REFCOUNTER> Link1_t;

      ContainerNode(const VALUE& obj): fObj(obj) {}

      static ContainerNode*
      ToNode(Link1Base_t* link) {
         return (ContainerNode*)link;
      }
      static const ContainerNode*
      ToNode(const Link1Base_t* link) {
         return (const ContainerNode*)link;
      }

      bool IsValid() const { return NodeValidator<VALUE>::IsValid(fObj); };
      void Invalidate(Arena_t* arena) {
         // Delete the node if it is not referenced anymore, by telling the node
         // arena to delete it. Invalidate the node otherwise.
         if (!Link1_t::IsReferenced() && !IsValid()) {
            arena->Delete(this);
         } else {
            NodeValidator<VALUE>::Invalidate(fObj);
         }
      }
      void Delete(Arena_t* arena) {
         // Delete the node if it is not referenced anymore, by telling the node
         // arena to delete it. Do not invalidate the node otherwise.
         if (!Link1_t::IsReferenced() && !IsValid()) {
            arena->Delete(this);
         }
      }

      Hash_t Hash() { return fgTraits.ValueHash(fObj); }

      VALUE  fObj;

      static Traits_t fgTraits;
   }; // class ContainerNode

   template <typename KEY, typename VALUE, typename REFCOUNTER, typename NODETRAITS>
   typename ContainerNode<KEY, VALUE, REFCOUNTER, NODETRAITS>::Traits_t
      ContainerNode<KEY, VALUE, REFCOUNTER, NODETRAITS>::fgTraits;


   //-------------------------------------------------------------------------------

   template <class NODE>
   struct NodeRef {
      typedef ContainerTools::NodeArena Arena_t;
      NodeRef(): fNode(0), fArena(0) {}
      NodeRef(NODE* node, Arena_t* arena): fNode(node), fArena(arena) { IncRef(); }
      ~NodeRef() { DecRef(); }
      NodeRef& operator=(const NodeRef& ref) {
         DecRef();
         fNode = ref.fNode;
         IncRef();
         return *this;
      }
      NodeRef& operator=(NODE* node) {
         DecRef();
         fNode = node;
         IncRef();
         return *this;
      }

      void Swap(NodeRef& with) {
         NODE* tmp = with.fNode;
         with.fNode = fNode;
         fNode = tmp;
      }

      operator NODE*() const { return fNode; }
      operator NODE*&() { return fNode; }
      NODE* operator->() const { return fNode; }
      NODE& operator*() const { return *fNode; }

      Arena_t* Arena() const { return fArena; }

   private:
      void IncRef() const { if (fNode) fNode->IncRef(); }
      void DecRef() const {
         if (fNode) {
            fNode->DecRef();
            NODE::ToNode(fNode)->Delete(fArena);
         }
      }
      NODE* fNode; // referenced node
      Arena_t* fArena; // arena used for the nodes
   }; // struct NodeRef

}; // namespace Internal
}; // namespace Reflex

#endif // Reflex_ContainerNode
