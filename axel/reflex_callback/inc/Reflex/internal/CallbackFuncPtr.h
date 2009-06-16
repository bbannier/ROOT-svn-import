// @(#)root/reflex:$Id$
// Author: Axel Naumann, 2009

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_CallbackFuncPtr
#define Reflex_CallbackFuncPtr

#include "Reflex/NotifyInfo.h"

namespace Reflex {
   
   namespace Internal {
      class RefCounted {
      public:
         RefCounted(): fRefCount(0), fOwned(false) {}
         ~RefCounted() { if (fRefCount) printf("AXEL DEBUG: ref counter not 0 but %ld\n", fRefCount); }
         void IncRef() { ++fRefCount; }
         bool DecRef() { return --fRefCount; }

         void SetOwned() { fOwned = true; }
         bool IsOwned() const { return fOwned; }

      private:
         size_t fRefCount;
         bool   fOwned;
      };
   } // namespace Internal


   class CallbackInterface: public Internal::RefCounted {
   public:
      virtual int Invoke(const NotifyInfo&) const = 0;
      virtual bool IsEqual(const CallbackInterface* other) const { return this == other; }
   };

   namespace Internal {

      template <typename T /* e.g. deriving from RefCounted*/ >
      class RefCountedPtr {
      public:
         explicit RefCountedPtr(const T* ptr): fPtr(const_cast<T*>(ptr)) { fPtr->IncRef(); }
         explicit RefCountedPtr(T* ptr): fPtr(ptr) { fPtr->SetOwned(); fPtr->IncRef(); }
         RefCountedPtr(const RefCountedPtr& other): fPtr(other.fPtr) { fPtr->IncRef(); }
         ~RefCountedPtr() { if (fPtr->IsOwned() && !fPtr->DecRef()) delete fPtr; }

         bool operator==(const RefCountedPtr<T>& other) const {
            return fPtr->IsEqual(other.fPtr);
         }
         operator const T*() const { return fPtr; }

      private:
         T* fPtr;
      };

      class CallbackFreeFuncPtr: public CallbackInterface {
      public:
         typedef int (*FuncPtr_t)(const NotifyInfo&);
         CallbackFreeFuncPtr(FuncPtr_t ptr): fFuncPtr(ptr) {}

         int Invoke(const NotifyInfo& ni) const { return fFuncPtr(ni); }

         bool IsEqual(const CallbackInterface* other) const {
            const CallbackFreeFuncPtr* cbo = dynamic_cast<const CallbackFreeFuncPtr*>(other);
            if (cbo) return *this == *cbo;
            return false;
         }
         bool operator==(const CallbackFreeFuncPtr& other) const {
            return fFuncPtr == other.fFuncPtr;
         }

      private:
         FuncPtr_t fFuncPtr;
      };

      template <class MEMBEROF>
      class CallbackMemFuncPtr: public CallbackInterface  {
      public:
         typedef int (MEMBEROF::*FuncPtr_t)(const NotifyInfo&);
         CallbackMemFuncPtr(const MEMBEROF obj, const FuncPtr_t& ptr): fObj(obj), fFuncPtr(ptr) {}

         int Invoke(const NotifyInfo& ni) { return (fObj.*fFuncPtr)(ni); }

         bool IsEqual(const CallbackInterface* other) const {
            const CallbackMemFuncPtr* cbo = dynamic_cast<const CallbackMemFuncPtr*>(other);
            if (cbo) return *this == *cbo;
            return false;
         }
         bool operator==(const CallbackMemFuncPtr& other) const {
            return fObj == other.fObj && fFuncPtr == other.fFuncPtr;
         }

      private:
         MEMBEROF  fObj;
         FuncPtr_t fFuncPtr;
      };

      // Specialization for pointer-to-object construction,
      // where the object is not copied.
      template <class MEMBEROF>
      class CallbackMemFuncPtr<MEMBEROF*>: public CallbackInterface {
      public:
         typedef int (MEMBEROF::* FuncPtr_t)(const NotifyInfo&);
         CallbackMemFuncPtr(MEMBEROF* obj, const FuncPtr_t& ptr): fObjPtr(obj), fFuncPtr(ptr) {}

         int Invoke(const NotifyInfo& ni) const { return (fObjPtr->*fFuncPtr)(ni); }

         bool IsEqual(const CallbackInterface* other) const {
            const CallbackMemFuncPtr* cbo = dynamic_cast<const CallbackMemFuncPtr*>(other);
            if (cbo) return *this == *cbo;
            return false;
         }
         bool operator==(const CallbackMemFuncPtr& other) const {
            return fObjPtr == other.fObjPtr && fFuncPtr == other.fFuncPtr;
         }

      private:
         MEMBEROF* fObjPtr;
         FuncPtr_t fFuncPtr;
      };

   } // namespace Internal

} // namespace Reflex

#endif // Reflex_CallbackFuncPtr
