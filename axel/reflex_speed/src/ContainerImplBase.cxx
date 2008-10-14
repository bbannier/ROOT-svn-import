// @(#)root/reflex:$Name:  $:$Id$
// Author: Axel Naumann, 2007

// Copyright CERN, CH-1211 Geneva 23, 2004-2007, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BUILD
#define REFLEX_BUILD
#endif

#include "ContainerImplBase.h"
#include "ContainerArena.h"
#include <iostream>
#include <cmath>


//-------------------------------------------------------------------------------
const size_t
Reflex::Internal::ContainerImplBase::fgPrimeArraySqrt3[19] = {
//-------------------------------------------------------------------------------
   2,7,23,71,223,673,2027,6089,18269,54829,164503,493523,1480571,4441721,
   13325171,39975553,119926691,359780077,1079340313
};


//-------------------------------------------------------------------------------
Reflex::Internal::ContainerImplBase::ContainerImplBase(size_t nodeSize, const IContainerImpl* other):
//-------------------------------------------------------------------------------
   fBuckets(17), fRehashPaused(false), fCollisions(0),
   fSize(0), fNodeArena(NodeArena_t::Instance(nodeSize)), fOther(other)
{
   // Initialize a ContainerImplBaseT with a minimum size.
   // The actual size will be the next element in fgPrimeArraySqrt3
   // greater or equal to the size given.
};


//-------------------------------------------------------------------------------
Reflex::Internal::ContainerImplBase::ContainerImplBase(size_t nodeSize, size_t size, const IContainerImpl* other):
//-------------------------------------------------------------------------------
   fBuckets(GetBucketSize(size)), fRehashPaused(false), fCollisions(0),
   fSize(0), fNodeArena(NodeArena_t::Instance(nodeSize)), fOther(other)
{
   // Initialize a ContainerImplBaseT with a minimum size.
   // The actual size will be the next element in fgPrimeArraySqrt3
   // greater or equal to the size given.
};


//-------------------------------------------------------------------------------
size_t
Reflex::Internal::ContainerImplBase::GetBucketSize(size_t requested) {
//-------------------------------------------------------------------------------
// Static function returning the next element in fgPrimeArraySqrt3
// greater of equal to requested.
   if (requested != 17) {
      int i = 0;
      for (; i < 19; ++i)
         if (fgPrimeArraySqrt3[i] >= requested) {
            requested = fgPrimeArraySqrt3[i];
            i = 2007;
            break;
         }
      if (i != 2007)
         // too many entries - we'll have to live with a lot of collisions.
         requested = fgPrimeArraySqrt3[18];
   }
   return requested;
}

//-------------------------------------------------------------------------------
Reflex::Internal::ContainerTools::Link1Base*
Reflex::Internal::ContainerImplBase::First() const {
//-------------------------------------------------------------------------------
// Return the first node != 0
   ContainerTools::Link1Base* c = 0;
   size_t firstBucket = 0;
   const size_t size = fBuckets.size();
   do {
      c = fBuckets[firstBucket++];
   } while (!c && firstBucket < size);
   return c;
}


//-------------------------------------------------------------------------------
bool
Reflex::Internal::ContainerImplBase::InsertNodeBase(ContainerTools::Link1Base* node, Hash_t hash) {
//-------------------------------------------------------------------------------
// Insert node with hash into the container. The hash defines
// the container's bucket to store the node in. Returns true in case
// there is a collition, i.e. the node is inserted into an already filled bucket.
   REFLEX_RWLOCK_W(fLock);
   ++fSize;
   const size_t bucketidx = BucketIndex(hash);
   if (fBuckets[bucketidx]) {
      // collision, simply add behind front:
      fBuckets[bucketidx]->InsertAfter(node);
      ++fCollisions;
      return true;
   };

   // no collision; this node is the first one in its bucket.
   fBuckets[bucketidx] = node;
   // find previous
   size_t bucketidxprev = bucketidx;
   while (bucketidxprev > 0 && !fBuckets[--bucketidxprev]);
   // fBuckets[bucketidxprev] points to bucket containing the node before "node":
   //   [bucketidxprev]:    n0 -> n1 -> n2 (-> n3)
   //   [...]:              0...
   //   [bucketidx]:        node
   //   [...]:              0...
   //   [bucketidxnext]:    n3
   const ContainerTools::Link1Base* n2 = fBuckets[bucketidxprev];
   if (n2) {
      // Find bucketidxnext:
      size_t bucketidxnext = bucketidx;
      while (++bucketidxnext < fBuckets.size() && !fBuckets[bucketidxnext]);
      if (bucketidxnext < fBuckets.size()) {
         // have valid n2 and n3.
         // n2 is the one before n3, i.e.
         const ContainerTools::Link1Base* n3 = fBuckets[bucketidxnext];
         while (n2->Next() != n3)
            n2 = n2->Next();
      }
      // else: valid n2 but no n3, thus pref is already the last node.

      const_cast<ContainerTools::Link1Base*>(n2)->InsertAfter(node);
   } else {
      // there is no n2; node will be the first node in the bucket vector.
      // Need to find the next one to set node->fNext:
      size_t bucketidxnext = bucketidx;
      while (++bucketidxnext < fBuckets.size() && !fBuckets[bucketidxnext]);
      if (bucketidxnext < fBuckets.size())
         node->SetNext(fBuckets[bucketidxnext]);
      fBuckets[bucketidx] = node;
   }
   return false;
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ContainerImplBase::GetStatistics(Statistics& stat) const {
//-------------------------------------------------------------------------------
// Collect statistics about the collection, see the Statistics class.

   REFLEX_RWLOCK_R(fLock);
   int maxcoll = 0;
   double squared = 0.;
   const ContainerTools::Link1Base* maxcollisionbucket = 0;
   for (size_t b = 0; b < fBuckets.size(); ++b) {
      const ContainerTools::Link1Base* c = fBuckets[b];
      int bucketEntries = 0;
      do {
         ++bucketEntries;
      } while ((c = c->Next()));

      squared += bucketEntries * bucketEntries;
      if (maxcoll < bucketEntries) {
         maxcoll = bucketEntries;
         maxcollisionbucket = fBuckets[b];
      }
   }

   stat.fSize = Size();
   stat.fCollisions = fCollisions;
   stat.fMaxCollisionPerBucket = maxcoll;
   stat.fCollisionPerBucketRMS = std::sqrt(squared - maxcoll*maxcoll);
   if (maxcollisionbucket) {
      stat.fCollidingNodes.resize(maxcoll);
      const ContainerTools::Link1Base* c = maxcollisionbucket;
      do {
         stat.fCollidingNodes[--maxcoll] = c;
      } while ((c = c->Next()));
   }
   stat.fNumBuckets = fBuckets.size();
   stat.fMaxNumRehashes = 0;
   while (stat.fMaxNumRehashes < 19
      && fgPrimeArraySqrt3[stat.fMaxNumRehashes] < fBuckets.size())
      ++stat.fMaxNumRehashes;
}

