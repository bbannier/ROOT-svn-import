// @(#)root/reflex:$Name: merge_reflex $:$Id$
// Author: Axel Naumann, 2007

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_RWLock
#define Reflex_RWLock

#define REFLEX_RWLOCK_R(LOCK) \
   Reflex::Internal::RWLock::R reflex__internal__rwlock__r__##LOCK(LOCK)
#define REFLEX_RWLOCK_W(LOCK) \
   Reflex::Internal::RWLock::W reflex__internal__rwlock__w__##LOCK(LOCK)
#define REFLEX_RWLOCK_R_RELEASE(LOCK) \
   reflex__internal__rwlock__r__##LOCK.Release()
#define REFLEX_RWLOCK_W_RELEASE(LOCK) \
   reflex__internal__rwlock__w__##LOCK.Release()

#ifdef REFLEX_THREADING
#else
// !!!!!!!!!!!!!!!
# define DEBUG
// !!!!!!!!!!!!!!!
# ifdef DEBUG
#  include <iostream>

namespace Reflex { namespace Internal {
   namespace RWLockState {
      enum EStateDebug { kUnlocked, kR, kW };
   }
} }

inline
std::ostream& operator <<(std::ostream& out, const Reflex::Internal::RWLockState::EStateDebug& state) {
   switch (state) {
      case Reflex::Internal::RWLockState::kUnlocked: out << "[unlocked]"; break;
      case Reflex::Internal::RWLockState::kR: out << "[read]"; break;
      case Reflex::Internal::RWLockState::kW: out << "[write]"; break;
      default: out << "[ERROR!]"; break;
   }
   return out;
}
#  define REFLEX_RWLOCK_MEMBERS \
   enum RWLockState::EStateDebug fState;
# endif
#endif

namespace Reflex {
   namespace Internal {
      class RWLock {
      public:

         // Scope based grabbing of an RWLock
         class R {
         public:
            R(RWLock& lock): fLock(&lock) {
               lock.Read();
            }
            void Release() {
               if (fLock) {
                  fLock->ReleaseRead();
                  fLock = 0;
               }
            }
            ~R() { Release(); }

         private:
            RWLock* fLock;
         };

         // Scope based grabbing of an RWLock
         class W {
         public:
            W(RWLock& lock): fLock(&lock) {
               fLock->Write();
            }
            void Release() {
               if (fLock) {
                  fLock->ReleaseWrite();
                  fLock = 0;
               }
            }
            ~W() { Release(); }

         private:
            RWLock* fLock;
         };

      public:
         RWLock();
         ~RWLock();

         void Read();
         void Write();
         void ReleaseRead();
         void ReleaseWrite();
#ifdef REFLEX_RWLOCK_MEMBERS
      private:
         REFLEX_RWLOCK_MEMBERS
#endif
      };
   }
}

#ifdef REFLEX_THREADING

#else // !REFLEX_THREADING:

#ifndef DEBUG
inline
Reflex::Internal::RWLock::RWLock() {}

inline
Reflex::Internal::RWLock::~RWLock() {}

inline
void
Reflex::Internal::RWLock::Read() {}

inline
void
Reflex::Internal::RWLock::Write() {}

inline
void
Reflex::Internal::RWLock::ReleaseRead() {}

inline
void
Reflex::Internal::RWLock::ReleaseWrite() {}

# else // DEBUG:

inline
Reflex::Internal::RWLock::RWLock(): fState(RWLockState::kUnlocked) {}

inline
Reflex::Internal::RWLock::~RWLock() {
   if (fState != RWLockState::kUnlocked)
      std::cout << "~RWLock: state " << fState << std::endl;
}

inline
void
Reflex::Internal::RWLock::Read() {
   if (fState != RWLockState::kUnlocked)
      std::cout << "RWLock::Read(): state " << fState << std::endl;
   fState = RWLockState::kR;
}

inline
void
Reflex::Internal::RWLock::Write() {
   if (fState != RWLockState::kUnlocked)
      std::cout << "RWLock::Write(): state " << fState << std::endl;
   fState = RWLockState::kW;
}

inline
void
Reflex::Internal::RWLock::ReleaseRead() {
   if (fState != RWLockState::kR)
      std::cout << "RWLock::ReleaseRead(): state " << fState << std::endl;
   fState = RWLockState::kUnlocked;
}

inline
void
Reflex::Internal::RWLock::ReleaseWrite() {
   if (fState != RWLockState::kW)
      std::cout << "RWLock::ReleaseWrite(): state " << fState << std::endl;
   fState = RWLockState::kUnlocked;
}
# endif // DEBUG
#endif // REFLEX_THREADING

#endif // Reflex_RWLock
