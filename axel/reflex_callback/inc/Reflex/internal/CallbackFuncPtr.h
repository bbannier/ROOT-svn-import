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


   /**
    * Callback messages to the callback handler; return values of the callback function
    */
   enum ECallbackReturn {
      kCallbackReturnNothing = 0, // default
      kCallbackReturnHandled = 1, // stop processing other callbacks
      kCallbackReturnVeto    = 2, // for kNotifyBefore: veto the action that should follow (e.g. unloading)
   };


   template <class WHAT>
   class CallbackInterface: public Internal::RefCounted {
   public:
      virtual ECallbackReturn Invoke(const WHAT&) const = 0;
      virtual bool IsEqual(const CallbackInterface* other) const = 0;
   };

   namespace Internal {


      template <typename T /* e.g. deriving from RefCounted*/ >
      class RefCountedPtr {
      public:
         RefCountedPtr(T* ptr, bool owned): fPtr(ptr) { if (owned) fPtr->SetOwned(owned); fPtr->IncRef(); }
         RefCountedPtr(const RefCountedPtr& other): fPtr(other.fPtr) { fPtr->IncRef(); }
         ~RefCountedPtr() { if (fPtr->IsOwned() && !fPtr->DecRef()) delete fPtr; }

         bool operator==(const RefCountedPtr<T>& other) const {
            return fPtr->IsEqual(other.fPtr);
         }

      private:
         T* fPtr;
      };

      template <class WHAT>
      class CallbackFreeFuncPtr: public CallbackInterface<WHAT> {
      public:
         typedef ECallbackReturn (*FuncPtr_t)(const WHAT&);
         CallbackFreeFuncPtr(const FuncPtr_t& ptr): fFuncPtr(ptr) {}

         ECallbackReturn Invoke(const WHAT& w) const { return fFuncPtr(w); }

         bool IsEqual(const CallbackInterface<WHAT>* other) const {
            const CallbackFreeFuncPtr* cbo = dynamic_cast<const CallbackFreeFuncPtr*>(other);
            if (cbo) return *this == *cbo;
            return false;
         }
         bool operator==(const CallbackFreeFuncPtr<WHAT>& other) const {
            return fFuncPtr == other.fFuncPtr;
         }

      private:
         FuncPtr_t fFuncPtr;
      };

      template <class MEMBEROF, class WHAT>
      class CallbackMemFuncPtr: public CallbackInterface<WHAT>  {
      public:
         typedef ECallbackReturn (MEMBEROF::*FuncPtr_t)(const WHAT&);
         CallbackMemFuncPtr(const MEMBEROF obj, const FuncPtr_t& ptr): fObj(obj), fFuncPtr(ptr) {}

         ECallbackReturn Invoke(const WHAT& w) { return (fObj.*fFuncPtr)(w); }

         bool IsEqual(const CallbackInterface<WHAT>* other) const {
            const CallbackMemFuncPtr* cbo = dynamic_cast<const CallbackMemFuncPtr*>(other);
            if (cbo) return *this == *cbo;
            return false;
         }
         bool operator==(const CallbackFreeFuncPtr<WHAT>& other) const {
            return fObj == other->fObj && fFuncPtr == other.fFuncPtr;
         }

      private:
         MEMBEROF  fObj;
         FuncPtr_t fFuncPtr;
      };

      // Specialization for pointer-to-object construction,
      // where the object is not copied.
      template <class MEMBEROF, class WHAT>
      class CallbackMemFuncPtr<MEMBEROF*, WHAT>: public CallbackInterface<WHAT>  {
      public:
         typedef ECallbackReturn (MEMBEROF::* FuncPtr_t)(const WHAT&);
         CallbackMemFuncPtr(MEMBEROF* obj, const FuncPtr_t& ptr): fObjPtr(obj), fFuncPtr(ptr) {}

         ECallbackReturn Invoke(const WHAT& w) const { return (fObjPtr->*fFuncPtr)(w); }

         bool IsEqual(const CallbackInterface<WHAT>* other) const {
            const CallbackMemFuncPtr* cbo = dynamic_cast<const CallbackMemFuncPtr*>(other);
            if (cbo) return *this == *cbo;
            return false;
         }
         bool operator==(const CallbackFreeFuncPtr<WHAT>& other) const {
            return fObjPtr == other->fObjPtr && fFuncPtr == other.fFuncPtr;
         }

      private:
         MEMBEROF* fObjPtr;
         FuncPtr_t fFuncPtr;
      };

   } // namespace Internal

} // namespace Reflex

#endif // Reflex_CallbackFuncPtr
