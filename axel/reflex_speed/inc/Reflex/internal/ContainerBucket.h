// @(#)root/reflex:$Name: merge_reflex $:$Id$
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_ContainerBucket
#define Reflex_ContainerBucket

#include "Reflex/internal/ContainerNode.h"
#include <vector>

namespace Reflex {
   namespace Internal {
      namespace ContainerTools {

         class NodeArena;

         //-------------------------------------------------------------------------------
         // A hash map's bucket, holding the root of the linked list of nodes.
         // For a hash map without collisions, all buckets have one link fFirst
         // plus one link fFirst->Next(0).
         class Bucket: public Link {
         public:
            // Initialize a bucket
            Bucket(const Bucket* prev = 0, const Bucket* next = 0, long index = 0):
               Link(prev, next), fIndex(index) {}

            bool InsertNode(Link* node) {
               // Add a node (at the front), return true if collision
               fFirst.InsertAfter(0, node);
               return (node->Next(&fFirst));
            }

            LinkIter Begin(const INodeHelper* helper = 0, NodeArena* arena = 0) const {
               return LinkIter(helper, arena, const_cast<Link*>(&fFirst), fFirst.Next(0)); }
            LinkIter End() const { return LinkIter(); }

            Link fFirst; // the root of the node list; fFirst itself does not contain a node
            long fIndex; // size - index of the bucket within the bucket vector; -1 if invalidated
         }; // class Bucket

         //-------------------------------------------------------------------------------
         // Helper class, checking the validation status of a bucket using
         // the INodeHelper interface. Needed for DecRef() on a bucket.
         class BucketNodeHelper: public INodeHelper {
         public:
            bool IsInvalidated(const Link* link) const {
               return static_cast<const Bucket*>(link)->fIndex == -1; }
         };

         //-------------------------------------------------------------------------------
         // A bucket collection: both a linked list of Buckets and a vector of Buckets
         // Iterator over a vector of buckets is a LinkIter:
         // Instead of using direct access, we iterate using the
         // LinkIter interface, which ensures that the iterator
         // stays valid under deletion even of the container.
         class BucketVector {
         public:
            //-------------------------------------------------------------------------------
            BucketVector(size_t size): fBuckets(size) { Init(size); }
            ~BucketVector();

            void Init(size_t size);

            Bucket& operator[](size_t idx) { return *fBuckets[idx]; }
            const Bucket& operator[](size_t idx) const { return *fBuckets[idx]; }

            LinkIter Begin() const { return LinkIter(GetBucketNodeHelper(), GetBucketArena(), 0, fBuckets[0]); }
            LinkIter End() const { return LinkIter(); }
            LinkIter IterAt(size_t at) const {
               return LinkIter(GetBucketNodeHelper(), GetBucketArena(), at ? fBuckets[at - 1] : 0, fBuckets[at]);
            }

            size_t size() const { return fBuckets.size(); }
            void clear() { fBuckets.clear(); }

         protected:
            static NodeArena* GetBucketArena();
            static INodeHelper* GetBucketNodeHelper() {
               static BucketNodeHelper helper;
               return &helper;
            }

         private:
            std::vector<Bucket*> fBuckets;
            static NodeArena* fgBucketArena;
         }; // class BucketVector

      } // namespace ContainerTools
   } // namespace Internal
} // namespace Reflex

#endif // Reflex_ContainerBucket
