// @(#)root/reflex:$Name: merge_reflex $:$Id$
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_ATOMIC
# ifndef REFLEX_THREADING
#  define REFLEX_ATOMIC_NONE
#  define REFLEX_ATOMICFUNDAMENTAL long

# elif defined (__GCC__)
#  if (__GCC__ >= 4) && (__GCC_MINOR__ >= 1) || (__ICC__ >= 8)
#   define REFLEX_ATOMIC_GCC4
#   define REFLEX_ATOMICFUNDAMENTAL long
#  elif (__GCC__ >= 3)
#   include "bits/atomicity.h"
#   define REFLEX_ATOMIC_STDCXX3
#   define REFLEX_ATOMICFUNDAMENTAL _Atomic_word
#  else
#   define REFLEX_ATOMIC_PTHREAD
#   define REFLEX_ATOMICFUNDAMENTAL long
#  endif

# elif defined (_WIN32) || defined (_WIN64)
#  include "windows.h"
#  define REFLEX_ATOMIC_WIN
#  define REFLEX_ATOMICFUNDAMENTAL LONG

# else
#  include "pthread.h"
#  define REFLEX_ATOMIC_PTHREAD
#  define REFLEX_ATOMICFUNDAMENTAL long
#  define REFLEX_ATOMIC_LOCK pthread_mutex_t fLock
#  define REFLEX_ATOMIC_LOCKINIT pthread_mutex_init(&_M_ref_count_lock, 0)
# endif

# define REFLEX_ATOMIC
#endif

namespace Reflex {
   namespace Internal {
      class AtomicCount {
      public:
         AtomicCount(long v = 0): fValue(v) {
#ifdef REFLEX_ATOMIC_LOCKINIT
            REFLEX_ATOMIC_LOCKINIT;
#endif
         }
         AtomicCount& operator ++();
         AtomicCount& operator --();
         operator REFLEX_ATOMICFUNDAMENTAL () const { return fValue; }

      private:
         REFLEX_ATOMICFUNDAMENTAL fValue;
#ifdef REFLEX_ATOMIC_LOCK
         REFLEX_ATOMIC_LOCK;
#endif
      };
   }
}

#ifdef REFLEX_ATOMIC_NONE
inline Reflex::Internal::AtomicCount&
Reflex::Internal::AtomicCount::operator ++() {
   // prefix increment
   ++fValue;
   return *this;
} 
inline Reflex::Internal::AtomicCount&
Reflex::Internal::AtomicCount::operator --() {
   // prefix decrement
   --fValue;
   return *this;
}
#endif // REFLEX_ATOMIC_NONE

#ifdef REFLEX_ATOMIC_GCC
inline Reflex::Internal::AtomicCount&
Reflex::Internal::AtomicCount::operator ++() {
   // prefix increment
   __sync_add_and_fetch(&fValue, 1);
   return *this;
} 
inline Reflex::Internal::AtomicCount&
Reflex::Internal::AtomicCount::operator --() {
   // prefix decrement
   __sync_sub_and_fetch(&fValue, 1);
   return *this;
}
#endif // REFLEX_ATOMIC_GCC

#ifdef REFLEX_ATOMIC_WIN
inline Reflex::Internal::AtomicCount&
Reflex::Internal::AtomicCount::operator ++() {
   // prefix increment
   InterlockedIncrement(&fValue);
   return *this;
} 
inline Reflex::Internal::AtomicCount&
Reflex::Internal::AtomicCount::operator --() {
   // prefix decrement
   InterlockedDecrement(&fValue);
   return *this;
}
#endif // REFLEX_ATOMIC_WIN

#ifdef REFLEX_ATOMIC_STDCXX3
inline Reflex::Internal::AtomicCount&
Reflex::Internal::AtomicCount::operator ++() {
   // prefix increment
   __atomic_add(&fValue, 1);
   return *this;
} 
inline Reflex::Internal::AtomicCount&
Reflex::Internal::AtomicCount::operator --() {
   // prefix decrement
   __atomic_add(&fValue, -1);
   return *this;
}
#endif // REFLEX_ATOMIC_STDCXX3

#ifdef REFLEX_ATOMIC_PTHREAD
inline Reflex::Internal::AtomicCount&
Reflex::Internal::AtomicCount::operator ++() {
   // prefix increment
   pthread_mutex_lock(&fLock);
   ++fValue;
   pthread_mutex_unlock(&fLock);
   return *this;
} 
inline Reflex::Internal::AtomicCount&
Reflex::Internal::AtomicCount::operator --() {
   // prefix decrement
   pthread_mutex_lock(&fLock);
   --fValue;
   pthread_mutex_unlock(&fLock);
   return *this;
}
#endif // REFLEX_ATOMIC_PTHREAD
