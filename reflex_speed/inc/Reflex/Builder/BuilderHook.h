// @(#)root/reflex:$Name:  $:$Id$
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_BuilderHook
#define Reflex_BuilderHook

// Include files
#include "Reflex/Kernel.h"

namespace Reflex {

   class Type;

   /** 
   * @class BuilderHook
   * Wraps a C style function pointer or pointer to member function 
   * to a setup function for a type's scope. Used e.g. for autoloading
   * of libraries ("we need type MyClass's members, we haven't loaded its
   * library yet, do it now!") and lazy dictionary initialization, where
   * not all members are loaded immediately but only on demand, when a
   * type's scope needs to be accessed.
   *
   * The Hook must have the form
   *   bool HookFunction(const Type& setMyUp);
   * When creating a BuilderHook you can specify the previous BuilderHook.
   * The previous BuilderHook will be called if this BuilderHook's Hook
   * function returns false. In other words: the hook function should return
   * true if it has successfully initialized the type.
   */

   class RFLX_API BuilderHook {
   protected:
      class IMemFuncPtr {
      public:
         virtual bool Hook(const Type& setMeUp) const = 0;
      };

      template <class T>
      class MemFuncPtrT: public IMemFuncPtr {
      public:
         typedef bool (T::* HookMeth_t)(const Type& setMeUp) const;
         MemFuncPtrT(const T& obj, HookMeth_t meth): fObj(obj), fMeth(meth) {}
         bool Hook(const Type& setMeUp) const { return (fObj->*fMeth)(setMeUp); }
      private:
         const T& fObj;
         const HookMeth_t fMeth;
      };

   public:
      typedef bool (* HookFun_t)(const Type& setMeUp);

      // Default constructor creating an invalid hook
      BuilderHook(BuilderHook* next = 0): fCHook(0), fMemFunc(0), fNext(next) {}

      // Hook function taking a free / static function pointer and
      // optionally a next element in the hook chain. The next element
      // will be owned by this BuilderHook, i.e. we delete it.
      // Call e.g. as
      //   fHook = new BuilderHook(MyClass::StaticHookFunc, fHook);
      BuilderHook(HookFun_t hook, BuilderHook* next = 0): fCHook(hook), fMemFunc(0), fNext(next) {}

      // Hook function taking a pointer to member function and
      // optionally a next element in the hook chain. The next element
      // will be owned by this BuilderHook, i.e. we delete it.
      // Call e.g. as
      //   MyClass obj;
      //   fHook = new BuilderHook(obj, HookFunc, fHook);
      // That way, fHook->Hook(type) will call
      //   obj->HookFunc(type);
      // and if that returns false, the previous fHook's function.
      // Note that BuilderHook only saves a reference to the object;
      // the object needs to stay valid for the hook function call!
      template <class T>
      BuilderHook(const T& obj, bool (T::* meth)(const Type& setMeUp) const, BuilderHook* next = 0):
         fCHook(0), fMemFunc(new MemFuncPtrT<T>(obj, meth)), fNext(next) {}

      ~BuilderHook() {
         delete fMemFunc;
         delete fNext;
      }

      bool Hook(const Type& setMeUp) const {
         // Call our hook function, if it returns false call the next in the chain.
         bool ret = false;
         if (fCHook) ret = (*fCHook)(setMeUp);
         else if (fMemFunc) ret = fMemFunc->Hook(setMeUp);
         if (ret) return true;
         if (fNext) return fNext->Hook(setMeUp);
         return false;
      }

      // Check for hook's validity
      operator bool() const { return fCHook || fMemFunc; }

      // Remove this hook call, and replace it with the next in the chain.
      // Invalidate this hook pointer if there is no next one.
      void Pop() {
         delete fMemFunc;
         if (fNext) *this = *fNext;
         else {
            fCHook = 0;
            fMemFunc = 0;
         }
      }

   private:
      HookFun_t    fCHook; // pointer to free function
      IMemFuncPtr* fMemFunc; // pointer to member function
      BuilderHook* fNext; // next hook in the chain; called if this one returned false
   };
}
#endif // Reflex_BuilderHook
