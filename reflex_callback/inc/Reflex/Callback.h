// @(#)root/reflex:$Id$
// Author: Axel Naumann, 2009

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Callback
#define Reflex_Callback

// Include files
#include "Reflex/Kernel.h"
#include "Reflex/Type.h"
#include "Reflex/Member.h"
#include "Reflex/NotifyInfo.h"

#include "Reflex/internal/CallbackFuncPtr.h"

/************************************************************************************
 *
 * Callbacks to be notified of changes in Reflex data.
 *
 * Given the example (user provided) functions
 *    class MyClass {
 *       int MemberCallback(const Type& t); // or Scope or Member
 *    };
 *    MyClass myObject;
 *
 *    int MyCallback(const Type& t); // or Scope or Member
 *
 * The main use cases are
 *    Type::RegisterCallback(&MyCallback);
 *    // copy myObject into the callback:
 *    Type::RegisterCallback(MakeCallback(myObject, &MyClass::MyCallback));
 *    // share myObject with the callback, caller must ensure that the object stays valid:
 *    Type::RegisterCallback(MakeCallback(&myObject, &MyClass::MyCallback));
 *
 * The return value is an OR-ed set of bits as defined by ECallbackReturn.
 *
 ************************************************************************************/


namespace Reflex {

   /** 
    * Common functionality for callbacks: registration and
    * notifyMask selection.
    */

   class RFLX_API CallbackBase: public NotifySelection {
   public:

      /** constructor */
      CallbackBase(const NotifySelection& ni = NotifySelection()):
         NotifySelection(ni),
         fRefCount(0)
         {}

      bool IsEnabled() const { return !(fWhen & kNotifyDisabled); }

      void Enable() { fWhen &= ~kNotifyDisabled; }
      void Disable() { fWhen |= kNotifyDisabled; }

      size_t IncRef() { return ++fRefCount; }
      size_t DecRef() { return --fRefCount; }

   private:
      size_t fRefCount; // counter of referencing Callback objects
   }; // class CallbackBase


   /**
    * Wrapper object around the actual CallbackBase-derived implementation
    */

   class RFLX_API Callback: public CallbackBase {
   public:
      Callback(const CallbackInterface* ci,
               const NotifySelection& ni): // don't copy the interface object
         CallbackBase(ni),
         fCallbackImplPtr(ci)
         {}

      typedef int (*FreeCallbackFunc_t)(const NotifyInfo&);
      Callback(FreeCallbackFunc_t callback, const NotifySelection& ni):
         CallbackBase(ni),
         fCallbackImplPtr((CallbackInterface*)new Internal::CallbackFreeFuncPtr(callback))
         {}

      template <class MEMBEROF>
      Callback(const MEMBEROF& obj, int (MEMBEROF::* &ptr)(const NotifyInfo&),
               const NotifySelection& ni):
         CallbackBase(ni),
         fCallbackImplPtr((CallbackInterface*)new Internal::CallbackMemFuncPtr<MEMBEROF>(obj, ptr))
         {}

      template <class MEMBEROF>
      Callback(const MEMBEROF* objptr, int (MEMBEROF::* &ptr)(const NotifyInfo&),
               const NotifySelection& ni):
         CallbackBase(ni),
         fCallbackImplPtr((CallbackInterface*)new Internal::CallbackMemFuncPtr<MEMBEROF>(objptr, ptr))
         {}

      const CallbackInterface* Get() const { return fCallbackImplPtr; }

      bool operator==(const Callback& other) { return fCallbackImplPtr == other.fCallbackImplPtr; }

   private:
      Internal::RefCountedPtr<CallbackInterface> fCallbackImplPtr;
   };

   template <class MEMBEROF>
   Callback
   MakeCallback(const MEMBEROF& obj, int (MEMBEROF::* &ptr)(const NotifyInfo&),
                const NotifySelection& ni = NotifySelection()) {
      return Callback(obj, ptr, ni);
   }

   template <class MEMBEROF>
   Callback
   MakeCallback(const MEMBEROF* objptr, int (MEMBEROF::* &ptr)(const NotifyInfo&),
                const NotifySelection& ni = NotifySelection()) {
      return Callback(objptr, ptr, ni);
   }


   /** 
   * Callback class for backward compatibility; derive from CallbackInterface instead
   */
   class RFLX_API ICallback: public CallbackInterface {

   public:

      /** constructor */
      ICallback() {}

      /** destructor */
      virtual ~ICallback() {}

      virtual void operator () (const Type& t) = 0;
      virtual void operator () (const Member& m) = 0;

      int Invoke(const NotifyInfo& ni) const {
         if (ni.fWhat == kNotifyType)
            const_cast<ICallback*>(this)->operator()(reinterpret_cast< const NotifyInfoT<Type>& >(ni).fElem);
         else //if (ni.fWhat == kNotifyMember)
            const_cast<ICallback*>(this)->operator()(reinterpret_cast< const NotifyInfoT<Member>& >(ni).fElem);
            return kCallbackReturnNothing;
      }

   }; // class ICallback

   /** 
   * Free callback functions for backward compatibility;
   * use member functions of Catalog / Type / Scope / Member instead.
   */
   RFLX_API void InstallClassCallback( ICallback * cb );
   RFLX_API void UninstallClassCallback( ICallback * cb );
   RFLX_API void FireClassCallback( const Type & );
   RFLX_API void FireFunctionCallback( const Member & );

} // namespace Reflex

#endif // Reflex_Callback
