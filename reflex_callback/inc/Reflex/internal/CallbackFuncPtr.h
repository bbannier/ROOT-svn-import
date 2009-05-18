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
   class CallbackBase;

   namespace Internal {
      template <class WHAT>
      class CallbackFuncPtrBase {
      public:
         virtual bool Invoke(const WHAT&) const = 0;
         virtual CallbackFuncPtrBase* Clone() const = 0;
      };

      template <class WHAT>
      class CallbackFreeFuncPtr: public CallbackFuncPtrBase<WHAT> {
      public:
         typedef bool (*FuncPtr_t)(const WHAT&);
         CallbackFreeFuncPtr(const FuncPtr_t& ptr);

         bool Invoke(const WHAT& w) const { return fFuncPtr(w); }

         CallbackFuncPtrBase<WHAT>* Clone() const { return new CallbackFreeFuncPtr<WHAT>(fFuncPtr); }

      private:
         FuncPtr_t fFuncPtr;
      };

      template <class MEMBEROF, class WHAT>
      class CallbackMemFuncPtr: public CallbackFuncPtrBase<WHAT> {
      public:
         typedef bool (MEMBEROF::*FuncPtr_t)(const WHAT&);
         CallbackMemFuncPtr(const MEMBEROF obj, const FuncPtr_t& ptr): fObj(obj), fFuncPtr(ptr) {}

         bool Invoke(const WHAT& w) { return (fObj.*fFuncPtr)(w); }

         CallbackFuncPtrBase<WHAT>* Clone() const {
            return new CallbackMemFuncPtr<MEMBEROF,WHAT>(fObj,fFuncPtr);
         }

      private:
         MEMBEROF  fObj;
         FuncPtr_t fFuncPtr;
      };

      // Specialization for pointer-to-object construction,
      // where the object is not copied.
      template <class MEMBEROF, class WHAT>
      class CallbackMemFuncPtr<MEMBEROF*, WHAT>: public CallbackFuncPtrBase<WHAT> {
      public:
         typedef bool (MEMBEROF::* FuncPtr_t)(const WHAT&);
         CallbackMemFuncPtr(MEMBEROF* obj, const FuncPtr_t& ptr): fObjPtr(obj), fFuncPtr(ptr) {}

         bool Invoke(const WHAT& w) const { return (fObjPtr->*fFuncPtr)(w); }

         CallbackFuncPtrBase<WHAT>* Clone() const {
            return new CallbackMemFuncPtr<MEMBEROF,WHAT>(fObjPtr, fFuncPtr);
         }

      private:
         MEMBEROF* fObjPtr;
         FuncPtr_t fFuncPtr;
      };

   } // namespace Internal

} // namespace Reflex

#endif // Reflex_CallbackFuncPtr
