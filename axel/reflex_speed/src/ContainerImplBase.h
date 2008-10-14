// @(#)root/reflex:$Name: merge_reflex $:$Id$
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_ContainerImplBase
#define Reflex_ContainerImplBase

#include <vector>
#include <string>

#include "Reflex/Kernel.h"

#include "RWLock.h"

// re-hash level
#ifndef REFLEX_CONTAINER_REHASH_LEVEL
#define REFLEX_CONTAINER_REHASH_LEVEL 3
#endif

namespace Reflex {
   //-------------------------------------------------------------------------------

   // Hash type used by Reflex::Container
   typedef unsigned long Hash_t;

   //-------------------------------------------------------------------------------
   namespace Internal {
      class  IContainerImpl;

      template <typename NODETRAITS, class REFCOUNTER>
      class ContainerImplBase_iterator;

      namespace ContainerTools {
         class Link1Base;
         template <class REFCOUNTER> class Link1;
         template <typename NODETRAITS, class REFCOUNTER> class Link1Iter;
         class NodeArena;
      }

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

      //-------------------------------------------------------------------------------
      //
      // Base class for Reflex::Container.
      // Contains the non-templated part of Reflex::Container (against code bloat).
      // ContainerImplBaseT implements a hash map. It containes elements deriving from
      // class ContainerImplBaseT::Link (e.g. Container::Node<T>).
      //
      // It provides memory management optimized for a large number of small elements,
      // fast retrieval, and non-fragmentation of memory. Thread safety will be
      // implemented soon.

      class ContainerImplBase {
      public:
         typedef std::vector<ContainerTools::Link1Base*> BucketVector_t;
         typedef ContainerTools::NodeArena NodeArena_t;

         static size_t GetBucketSize(size_t requested);

         void SetFindProxyContainer(const IContainerImpl* other) { fOther = other; }

         ContainerTools::Link1Base* First() const;

         // Arena used for the collection's nodes
         NodeArena_t* Arena() const { return fNodeArena; }

         // Statistics holder for the collection
         struct Statistics {
            size_t fSize; // number of nodes stored in the collection
            size_t fCollisions; // number of nodes sharing the same bucket
            int    fMaxCollisionPerBucket; // maximum number of collisions seen in any of the collection's buckets
            double fCollisionPerBucketRMS; // root mean squared of the distribution of collisions of the buckets
            int    fNumBuckets; // the container's number of buckets
            int    fMaxNumRehashes; // upper limit of the (expensive) rehash operations the collection could have observed
            std::vector<const ContainerTools::Link1Base*> fCollidingNodes; // vector of nodes that have to share their bucket with other nodes
         };

         // Fill a Statistics object from the collection
         void GetStatistics(Statistics& stat) const;

         size_t Size() const {
            // Number of elements stored in the container
            REFLEX_RWLOCK_R(fLock);
            return fSize;
         }

         size_t BucketIndex(Hash_t hash) const { return hash % fBuckets.size(); }

         bool InsertNodeBase(ContainerTools::Link1Base* node, Hash_t hash);

      protected:
         ContainerImplBase(size_t nodeSize, const IContainerImpl* other);
         ContainerImplBase(size_t nodeSize, size_t size, const IContainerImpl* other);

         bool NeedRehash() const {
            // Determine whether the container needs to be rehashed, i.e.
            // whether the number of colliding nodes exceeds the number of nodes 
            // by a factor REFLEX_CONTAINER_REHASH_LEVEL (3 by default)
            REFLEX_RWLOCK_R(fLock);
            return fCollisions > fSize * REFLEX_CONTAINER_REHASH_LEVEL;
         }

      protected:
         mutable RWLock fLock; // Read/Write lock for this container
         BucketVector_t fBuckets; // collection of buckets
         bool          fRehashPaused; // whether insertions can cause a rehash to reduce the number of collisions
         size_t        fCollisions; // number of nodes sharing a bucket with other nodes
         size_t        fSize; // number of elements this container holds
         NodeArena_t*  fNodeArena; // the container's node storage manager
         const IContainerImpl* fOther;
         static const size_t fgPrimeArraySqrt3[19]; // a pre-computed array of prime numbers used for growing the buckets
      }; // class ContainerImplBase

   } // namespace Internal

} // namespace Reflex

#endif
