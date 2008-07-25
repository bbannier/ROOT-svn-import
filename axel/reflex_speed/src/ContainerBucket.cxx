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

#include "ContainerBucket.h"
#include "ContainerArena.h"


//-------------------------------------------------------------------------------
Reflex::Internal::ContainerTools::NodeArena*
Reflex::Internal::ContainerTools::BucketVector::fgBucketArena = 0;
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
void Reflex::Internal::ContainerTools::BucketVector::Init(size_t size) {
//-------------------------------------------------------------------------------
// Initialize (clear, resize, link) a bucket vector of a given size.
// Does not delete old fBucket elements; used e.g. in ContainerBase::Rehash().

   fBuckets.clear();
   if (fBuckets.size() != size)
      fBuckets.resize(size);

   // Create the buckets, passing the bucket index.
   for (size_t i = 0; i < size; ++i)
      fBuckets[i] = new Bucket(0, 0, size - i);
   // Link them up.
   for (size_t i = 0; i < size; ++i)
      fBuckets[i]->Set(i ? fBuckets[i - 1] : 0, i + 1 < size ? fBuckets[i + 1] : 0);
}

//-------------------------------------------------------------------------------
Reflex::Internal::ContainerTools::BucketVector::~BucketVector() {
//-------------------------------------------------------------------------------
// Destruct a bucket vector, deleting the Buckets.
// Does _not_ delete the links contained in the buckets!

   for (size_t i = 0; i < fBuckets.size(); ++i)
      delete fBuckets[i];
}
   
//-------------------------------------------------------------------------------
Reflex::Internal::ContainerTools::NodeArena*
Reflex::Internal::ContainerTools::BucketVector::GetBucketArena() {
//-------------------------------------------------------------------------------
// Initialize the fgBucketArena to the proper NodeArena instance
   static NodeArena* fgBucketArena = 0;
   if (!fgBucketArena)
      fgBucketArena = NodeArena::Instance(sizeof(Bucket));
   return fgBucketArena;
}

