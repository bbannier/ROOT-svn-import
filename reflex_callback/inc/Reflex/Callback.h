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
    * What to notify on.
    */
   enum ENotifyWhat {
      kNotifyType = 1,
      kNotifyScope = 2,
      kNotifyMember = 4,
      kNotifyAnyEntity = kNotifyType | kNotifyScope | kNotifyMember
   };

   /**
    * Callback messages to the callback handler; return values of the callback function
    */
   enum ECallbackReturn {
      kCallbackReturnNothing = 0, // default
      kCallbackReturnHandled = 1, // stop processing other callbacks
      kCallbackReturnVeto    = 2, // for kNotifyBefore: veto the action that should follow (e.g. unloading)
   };


   namespace Internal {
      template <class WHAT> struct NotifyWhatHelper_t {};
      template <> struct NotifyWhatHelper_t<Type> { enum { kValue = Reflex::kNotifyType }; };
      template <> struct NotifyWhatHelper_t<Scope> { enum { kValue = Reflex::kNotifyScope }; };
      template <> struct NotifyWhatHelper_t<Member> { enum { kValue = Reflex::kNotifyMember }; };
   }

   /** 
    * Common functionality for callbacks: registration and
    * notifyMask selection.
    */

   class RFLX_API CallbackBase {
   public:

      /** constructor */
      CallbackBase(char notifyMask = kNotifyDeclared | kNotifyUnloaded,
                   char notifyTiming = kNotifyAfter,
                   char notifyWhat = kNotifyAnyEntity):
         fNotifyMask(notifyMask), fNotifyTiming(notifyTiming), fNotifyWhat(notifyWhat),
         fRefCount(0)
      {}

      char GetNotifyMask() const { return fNotifyMask; }
      char GetNotifyTiming() const { return fNotifyTiming; }
      char GetNotifyWhat() const { return fNotifyWhat; }

      bool IsEnabled() const { return !(fNotifyTiming & kNotifyDisabled); }

      void Enable() { fNotifyTiming &= ~kNotifyDisabled; }
      void Disable() { fNotifyTiming |= kNotifyDisabled; }

      virtual int InvokeForType(const Type& t) { UnregisterFor(t); return true; }
      virtual int InvokeForScope(const Scope& s) { UnregisterFor(s); return true; }
      virtual int InvokeForMember(const Member& m) { UnregisterFor(m); return true; }

      virtual CallbackBase* Clone() const = 0;

      size_t AddRef() { return ++fRefCount; }
      size_t RemoveRef() { return --fRefCount; }

   protected:
      void UnregisterFor(const Type& dummy) const;
      void UnregisterFor(const Scope& dummy) const;
      void UnregisterFor(const Member& dummy) const;

   private:
      char fNotifyMask; // combination of ENotifyOnChange bits
      char fNotifyTiming; // combination of ENotifyTiming bits
      char fNotifyWhat; // combination of ENotifyWhat bits
      size_t fRefCount; // counter of referencing Callback objects
   }; // class CallbackBase


   namespace Internal {
      template <class WHAT, class IMPLEMENTS>
      struct CallbackInvocationAdaptor {
         int Invoke(const CallbackBase& cb, const WHAT& w, const Internal::CallbackFuncPtrBase<IMPLEMENTS>&) {
            cb.UnregisterFor(w);
            return true;
         }
      };

      template <class WHAT>
      struct CallbackInvocationAdaptor<WHAT, WHAT> {
         int Invoke(const CallbackBase&, const WHAT& t, const Internal::CallbackFuncPtrBase<WHAT>& func) {
            return func.Invoke(t);
         }
      };
   }

   /**
    * Callback given a (member) function
    */
   template <class WHAT>
   class RFLX_API CallbackT: public CallbackBase {
   public:

      /** constructor taking a free function int func_ptr(const WHAT&) */
      CallbackT(int (&callback)(const WHAT&),
               char notifyMask = kNotifyDeclared | kNotifyUnloaded,
               char notifyTiming = kNotifyAfter):
         CallbackBase(notifyMask, notifyTiming, Internal::NotifyWhatHelper_t<WHAT>::kValue),
         fCallback(new Internal::CallbackFreeFuncPtr<WHAT>(callback)) {}

      /** constructor taking a member function int obj::func_ptr(const WHAT&) */
      template <class MEMBEROF>
      CallbackT(MEMBEROF& obj, int (MEMBEROF::* callback)(const WHAT&),
               char notifyMask = kNotifyDeclared | kNotifyUnloaded,
               char notifyTiming = kNotifyAfter):
         CallbackBase(notifyMask, notifyTiming, Internal::NotifyWhatHelper_t<WHAT>::kValue),
         fCallback(new Internal::CallbackMemFuncPtr<MEMBEROF, WHAT>(obj, callback))
         {}

      /** constructor taking a member function int obj::func_ptr(const WHAT&) */
      CallbackT(const Internal::CallbackFuncPtrBase<WHAT>* callback,
               char notifyMask = kNotifyDeclared | kNotifyUnloaded,
               char notifyTiming = kNotifyAfter):
         CallbackBase(notifyMask, notifyTiming, Internal::NotifyWhatHelper_t<WHAT>::kValue),
         fCallback(callback->Clone())
         {}

      /** destructor */
      virtual ~CallbackT() { delete fCallback; }

      CallbackBase* Clone() const {
         return new CallbackT(fCallback, GetNotifyMask(), GetNotifyTiming()); }

      virtual int InvokeForType(const Type& t) { return Internal::CallbackInvocationAdaptor<Type,WHAT>(*this, t, fCallback); }
      virtual int InvokeForScope(const Scope& s) { return Internal::CallbackInvocationAdaptor<Scope,WHAT>(*this, s, fCallback); }
      virtual int InvokeForMember(const Member& m) { return Internal::CallbackInvocationAdaptor<Member,WHAT>(*this, m, fCallback); }
   private:
      Internal::CallbackFuncPtrBase<WHAT>* fCallback;
   }; // class CallbackT


   template <class WHAT>
   CallbackT<WHAT> MakeCallback(int (&callback)(const WHAT&)) {
      return CallbackT<WHAT>(callback);
   }

   template <class MEMBEROF, class WHAT>
   CallbackT<WHAT> MakeCallback(MEMBEROF& obj, int (&callback)(const WHAT&)) {
      return CallbackT<WHAT>(obj, callback);
   }

   /** 
   * Callback class for backward compatibility; derive from ICallbackT instead
   */
   class RFLX_API ICallback: public CallbackBase {

   public:

      /** constructor */
      ICallback() {}

      virtual void operator () (const Type& t) { InvokeForType(t); }
      virtual void operator () (const Member& m) { InvokeForMember(m); }

      /** destructor */
      virtual ~ICallback() {}
   }; // class ICallback


   /** 
   * Free callback functions for backward compatibility; use members instead
   */
   RFLX_API void InstallClassCallback( ICallback * cb );
   RFLX_API void UninstallClassCallback( ICallback * cb );
   RFLX_API void FireClassCallback( const Type & );
   RFLX_API void FireFunctionCallback( const Member & );

   /**
    * Wrapper object around the actual CallbackBase-derived implementation
    */

   class Callback {
   public:
      Callback(const CallbackBase& cb);
      Callback(Callback& other): fImpl(other.fImpl) { fImpl->AddRef(); }
      Callback& operator=(const Callback& other) {
         fImpl = other.fImpl;
         fImpl->AddRef();
      }
      ~Callback();
   private:
      CallbackBase* fImpl;
   };

} // namespace Reflex

#endif // Reflex_Callback
