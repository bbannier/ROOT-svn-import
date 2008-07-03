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

#include "Reflex/internal/ContainerBase.h"
#include "Reflex/internal/ContainerArena.h"
#include <iostream>
#include <cmath>


//-------------------------------------------------------------------------------
const int Reflex::Internal::ContainerBase::fgPrimeArraySqrt3[19] = {
//-------------------------------------------------------------------------------
   2,7,23,71,223,673,2027,6089,18269,54829,164503,493523,1480571,4441721,
   13325171,39975553,119926691,359780077,1079340313
};


//-------------------------------------------------------------------------------
Reflex::Internal::ContainerBase::ContainerBase(size_t nodeSize, size_t size):
//-------------------------------------------------------------------------------
   fBuckets(GetBucketSize(size)), fRehashPaused(false), fCollisions(0),
   fSize(0)
{
   // Initialize a ContainerBase with a minimum size.
   // The actual size will be the next element in fgPrimeArraySqrt3
   // greater or equal to the size given.
   fNodeArena = NodeArena::Instance(nodeSize);
};


//-------------------------------------------------------------------------------
Reflex::Internal::ContainerBase::~ContainerBase() {
//-------------------------------------------------------------------------------
   // NEEDS TO BE DONE BY DERIVED CLASS!
   // The buckets d'tor needs to access the virtual DeleteNode()
   //   fBuckets.clear();
   fNodeArena->ReleaseInstance();
}


//-------------------------------------------------------------------------------
size_t
Reflex::Internal::ContainerBase::GetBucketSize(size_t requested) {
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
void
Reflex::Internal::ContainerBase::Rehash() {
//-------------------------------------------------------------------------------
// Resize fNodesList to reduce collisions. Check with NeedRehash() before
// calling this function. We assume that we have reached the amoutn of collisions
// defined by the Rehash level; to not Rehash that soon again we extend the list
// by at least (Rehash level * Rehash level); the new size will be the next
// prime larger than the current size * (Rehash level * Rehash level).
   REFLEX_RWLOCK_W(fLock);
   int newSize = fBuckets.size() * REFLEX_CONTAINER_REHASH_LEVEL;
   int i = 0;
   while (i < 19 && fgPrimeArraySqrt3[i] < newSize) ++i;
   if (i == 19) // exceeds max supported entries...
      return;
   newSize = fgPrimeArraySqrt3[i];
   BucketVector oldList = fBuckets;

   fBuckets.Init(newSize);
   fCollisions = 0;
   bool hadRehashPaused = fRehashPaused;
   fRehashPaused = true;
   for (ContainerTools::LinkIter b = oldList.Begin(); b != oldList.End(); ++b) {
      ContainerTools::Bucket* bucket = static_cast<ContainerTools::Bucket*>(b.Curr());
      Link* prev = &bucket->fFirst;
      Link* curr = prev->Next(0);
      while (curr) {
         // progress before inserting, or next will be fBucket's next, not oldList's!
         curr = curr->Progress(prev);
         if (fBuckets[GetHash(prev) % fBuckets.size()].InsertNode(prev))
            ++fCollisions;
      }
      // Prevent l from deleting its nodes - they are now owned by fBuckets
      // But then again, l doesn't delete its nodes
      // const_cast<Bucket&>(*l).fFirst.Set(0, 0);
   }
   fRehashPaused = hadRehashPaused;

   std::cout << "Rehashing from " << oldList.size() << " to " << newSize << " for " << fSize << " entries." << std::endl; // AND REMOVE IOSTREAM, TOO!
}

//-------------------------------------------------------------------------------
void
Reflex::Internal::ContainerBase::Clear() {
//-------------------------------------------------------------------------------
   REFLEX_RWLOCK_W(fLock);
   RemoveAllNodes();
   // NO! We need to keep our nodearena, we cannot have it deleted!
   // fNodeArena->ReleaseInstance();
   fCollisions = 0;
   fSize = 0;
   fRehashPaused = false;
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ContainerBase::InsertNode(Link* node, Hash_t hash) {
//-------------------------------------------------------------------------------
// Insert node with hash into the container. The hash defines
// the container's bucket to store the node in.
   REFLEX_RWLOCK_W(fLock);
   ++fSize;
   if (fBuckets[hash % fBuckets.size()].InsertNode(node)) {
      ++fCollisions;
      if (!fRehashPaused) {
         REFLEX_RWLOCK_W_RELEASE(fLock);
         if (NeedRehash())
            Rehash();
      }
   }
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ContainerBase::GetStatistics(Statistics& stat) const {
//-------------------------------------------------------------------------------
// Collect statistics about the collection, see the Statistics class.

   REFLEX_RWLOCK_R(fLock);
   int maxcoll = 0;
   double squared = 0.;
   const ContainerTools::Bucket* maxcollisionbucket = 0;
   for (long b = 0; b < fBuckets.size(); ++b) {
      const Link* p = 0;
      const Link* c = fBuckets[b].fFirst.Next(0);
      int listEntries = 0;
      do {
         ++listEntries;
      } while ((c = c->Progress(p)));

      squared += listEntries * listEntries;
      if (maxcoll < listEntries) {
         maxcoll = listEntries;
         maxcollisionbucket = &(fBuckets[b]);
      }
   }

   stat.fSize = Size();
   stat.fCollisions = fCollisions;
   stat.fMaxCollisionPerBucket = maxcoll;
   stat.fCollisionPerBucketRMS = std::sqrt(squared - maxcoll*maxcoll);
   if (maxcollisionbucket) {
      stat.fCollidingNodes.resize(maxcoll);
      const Link* p = 0;
      const Link* c = maxcollisionbucket->fFirst.Next(0);
      do {
         stat.fCollidingNodes[--maxcoll] = c;
      } while ((c = c->Progress(p)));
   }
   stat.fNumBuckets = fBuckets.size();
   stat.fMaxNumRehashes = 0;
   while (stat.fMaxNumRehashes < 19
      && fgPrimeArraySqrt3[stat.fMaxNumRehashes] < fBuckets.size())
      ++stat.fMaxNumRehashes;
}


//-------------------------------------------------------------------------------
Reflex::Internal::ContainerBase_iterator::ContainerBase_iterator(
   const LinkIter& linkiter, const LinkIter& bucketiter,
   const ContainerBase_iterator& nextContainer):
//-------------------------------------------------------------------------------
fLinkIter(linkiter), fBucketIter(bucketiter), fNextContainerBegin(0)
{
   // move to next valid node
   while (fBucketIter && !fLinkIter) {
      ContainerTools::Bucket* first = static_cast<ContainerTools::Bucket*>(fBucketIter.Curr());
      fLinkIter = LinkIter(linkiter.Helper(), linkiter.Arena(), &first->fFirst, first->fFirst.Next(0));
      if (!fLinkIter) ++fBucketIter;
   }

   if (nextContainer)
      fNextContainerBegin = new ContainerBase_iterator(nextContainer);
}


//-------------------------------------------------------------------------------
Reflex::Internal::ContainerBase_iterator::ContainerBase_iterator(
   const ContainerBase& container, const INodeHelper& helper,
   const ContainerBase_iterator& nextContainer):
//-------------------------------------------------------------------------------
   fBucketIter(container.Buckets().Begin()), fNextContainerBegin(0)
{
   while (fBucketIter && !fLinkIter) {
      ContainerTools::Bucket* first = static_cast<ContainerTools::Bucket*>(fBucketIter.Curr());
      fLinkIter = LinkIter(&helper, container.Arena(), &first->fFirst, first->fFirst.Next(0));
      if (!fLinkIter) ++fBucketIter;
   }

   if (nextContainer)
      fNextContainerBegin = new ContainerBase_iterator(nextContainer);
}

//-------------------------------------------------------------------------------
Reflex::Internal::ContainerBase_iterator::~ContainerBase_iterator() {
//-------------------------------------------------------------------------------
// destruct a ContainerBase_iterator
   delete fNextContainerBegin;
}

//-------------------------------------------------------------------------------
Reflex::Internal::ContainerBase_iterator&
Reflex::Internal::ContainerBase_iterator::operator++() {
//-------------------------------------------------------------------------------
// Prefix increment: move to next node, which might be in the next
// bucket or even in the next container.

   // End() is unchanged:
   if (!fLinkIter) return *this;

   ++fLinkIter;
   while (!fLinkIter) {
      if (++fBucketIter) {
         // the link has left bucket, go to next one
         fLinkIter = static_cast<ContainerTools::Bucket*>(fBucketIter.Curr())->Begin(fLinkIter.Helper(), fLinkIter.Arena());
      } else if (fNextContainerBegin) {
         // out of buckets, try next container
         ContainerBase_iterator* tobedeleted = fNextContainerBegin;
         *this = *fNextContainerBegin;
         delete tobedeleted;
      } else
         // no next node, no next bucket, no next collection: the End().
         break;
   }
   return *this;
}
