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
 *    Type::RegisterCallback(MakeCallback(&MyCallback));
 *    // copy myObject into the callback:
 *    Type::RegisterCallback(MakeCallback(myObject, &MyClass::MyCallback));
 *    // share myObject with the callback, caller must ensure that the object stays valid:
 *    Type::RegisterCallback(MakeCallback(&myObject, &MyClass::MyCallback));
 *
 * The return value is an OR-ed set of bits as defined by ECallbackReturn.
 *
 ************************************************************************************/


namespace Reflex {

   // forward declarations
   class Type;
   class Member;

   /**
    * Enum for bit mask defining what a callback should be called for.
    * The states of types / scopes / members are
    *   1) unknown
    *   2) only name known (e.g. TypeName, but no TypeBase)
    *   3) forward declaration exists (TypeBase without type's data, e.g. "class" but no scope yet)
    *   4) fully declared
    */
   enum ENotifyOnChange {
      kNotifyNothing = 0x00, // never called, used to disable a callback

      kNotifyNameCreated = 0x01, // transition 1 -> 2
      kNotifyNameResolved = 0x02, // transition 2 -> 3
      kNotifyDeclared = 0x04, // transition 3 -> 4
      kNotifyAllLoad = 0x07, // any transition in -> direction

      kNotifyUndeclared = 0x10, // transition 4 -> 3
      kNotifyUnresolved = 0x20, // transition 3 -> 2
      kNotifyUnloaded = 0x40, // transition 2 -> 1
      kNotifyAllUnload = 0x70, // any transition in <- direction

      kNotifyAllChanges = kNotifyAllLoad | kNotifyAllUnload
   };

   /**
    * When to call the callback: before or after the transition has occurred.
    */
   enum ENotifyTiming {
      kNotifyNever = 0, // never called

      kNotifyBefore = 1, // called before the transition happens
      kNotifyAfter = 2, // called after the transition happens
      kNotifyBoth = kNotifyBefore | kNotifyAfter,

      kNotifyDisabled = 4, // never called; bit combined with kNotifyBefore and / or kNotifyAfter
   };



   /** 
    * Common functionality for callbacks: registration and
    * notifyMask selection.
    */

   class RFLX_API CallbackBase {
   public:

      /** constructor */
      CallbackBase(char notifyMask = kNotifyDeclared | kNotifyUnloaded,
                   char notifyTiming = kNotifyAfter):
         fNotifyMask(notifyMask), fNotifyTiming(notifyTiming), fRefCount(0)
         {}

      char GetNotifyMask() const { return fNotifyMask; }
      char GetNotifyTiming() const { return fNotifyTiming; }

      bool IsEnabled() const { return !(fNotifyTiming & kNotifyDisabled); }

      void Enable() { fNotifyTiming &= ~kNotifyDisabled; }
      void Disable() { fNotifyTiming |= kNotifyDisabled; }

      size_t IncRef() { return ++fRefCount; }
      size_t DecRef() { return --fRefCount; }

   private:
      char fNotifyMask; // combination of ENotifyOnChange bits
      char fNotifyTiming; // combination of ENotifyTiming bits
      char fNotifyWhat; // combination of ENotifyWhat bits
      size_t fRefCount; // counter of referencing Callback objects
   }; // class CallbackBase


   /**
    * Wrapper object around the actual CallbackBase-derived implementation
    */

   template <class WHAT>
   class RFLX_API Callback: public CallbackBase {
   public:
      Callback(const CallbackInterface<WHAT>* ci,
               char notifyMask = kNotifyDeclared | kNotifyUnloaded,
               char notifyTiming = kNotifyAfter): // don't copy the interface object
         CallbackBase(notifyMask, notifyTiming),
         fCallback(ci, false)
         {}


      Callback(int (&callback)(const WHAT&),
               char notifyMask = kNotifyDeclared | kNotifyUnloaded,
               char notifyTiming = kNotifyAfter):
         CallbackBase(notifyMask, notifyTiming),
         fCallback(new Internal::CallbackFreeFuncPtr<WHAT>(callback), true)
         {}

      template <class MEMBEROF>
      Callback(const MEMBEROF& obj, ECallbackReturn (MEMBEROF::* &ptr)(const WHAT&),
               char notifyMask = kNotifyDeclared | kNotifyUnloaded,
               char notifyTiming = kNotifyAfter):
         CallbackBase(notifyMask, notifyTiming),
         fCallback(new Internal::CallbackMemFuncPtr<MEMBEROF, WHAT>(obj, ptr), true)
         {}

      template <class MEMBEROF>
      Callback(const MEMBEROF* objptr, ECallbackReturn (MEMBEROF::* &ptr)(const WHAT&),
               char notifyMask = kNotifyDeclared | kNotifyUnloaded,
               char notifyTiming = kNotifyAfter):
         CallbackBase(notifyMask, notifyTiming),
         fCallback(new Internal::CallbackMemFuncPtr<MEMBEROF, WHAT>(objptr, ptr), true)
         {}

      CallbackInterface<WHAT>* Get() const { return fCallback; }

      bool operator==(const Callback<WHAT>& other) { return fCallback == other.fCallback; }

   private:
      Internal::RefCountedPtr<CallbackInterface<WHAT> > fCallback;
   };


   /** 
   * Callback class for backward compatibility; derive from CallbackInterface instead
   */
   class RFLX_API ICallback: public CallbackInterface<Type>, public CallbackInterface<Member> {

   public:

      /** constructor */
      ICallback() {}

      virtual void operator () (const Type& t) = 0;
      virtual void operator () (const Member& m) = 0;

      virtual ECallbackReturn Invoke(const Type& t) const {
         const_cast<ICallback*>(this)->operator()(t); return kCallbackReturnNothing;
      }
      virtual ECallbackReturn Invoke(const Member& m) const {
         const_cast<ICallback*>(this)->operator()(m); return kCallbackReturnNothing;
      }
      virtual bool IsEqual(const CallbackInterface<Type>* other) const {
         return this == other;
      }
      virtual bool IsEqual(const CallbackInterface<Member>* other) const {
         return this == other;
      }

      /** destructor */
      virtual ~ICallback() {}
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
