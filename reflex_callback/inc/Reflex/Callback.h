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
#include "Reflex/NotifyInfo.h"

/************************************************************************************
 *
 * Callbacks to be notified of changes in Reflex data as an observer.
 *
 * Example:
 *   int MyFunc(const NotifyInfo& ni, void* userData);
 *
 *   class MyClass: public Callback {
 *   public:
 *      int Invoke(const NotifyInfo& ni) const {
 *      // print the name of the changed type
 *         if (ni.fWhat == kNotifyType)
 *            printf("%s\n", ni.Elem<Type>().Name());
 *         return kCallbackReturnNothing;
 *      }
 *   };
 *   MyClass obj;
 *
 * The main use cases are
 *    Catalog::RegisterCallback(&MyFunc, NotifySelection(kNotifyMember, kNotifyBefore,
 *                                                       kNotifyAllTransitions));
 *    Catalog::RegisterCallback(&obj);
 *
 * The callbacks' return value is an OR-ed set of bits as defined by ECallbackReturn.
 * Callbacks must not modify the reflection database.
 *
 ************************************************************************************/


namespace Reflex {

   class CallbackInterface {
   public:
      virtual int Invoke(const NotifyInfo& ni) const = 0;
   };

   /**
    * Wrapper object around the actual CallbackBase-derived implementation
    */

   class RFLX_API Callback {
   public:

      Callback(const CallbackInterface* ci, const NotifySelection& ns):
         fSelection(ns),
         fFuncPtr(&CallbackInterfaceProxy),
         fCallbackInterface(ci)
         {}

      Callback(FreeCallbackFunc_t callback, const NotifySelection& ns, void* userData = 0):
         fSelection(ns),
         fFuncPtr(callback),
         fUserData(userData)
         {}

      bool IsEnabled() const { return !(When() & kNotifyDisabled); }

      void Enable() { fSelection.fWhen &= ~kNotifyDisabled; }
      void Disable() { fSelection.fWhen |= kNotifyDisabled; }

      bool operator==(const Callback& other) const {
         return fFuncPtr == other.fFuncPtr && fUserData == other.fUserData; }

      int Invoke(const NotifyInfo& ni) {
         if (IsEnabled())
            return (*fFuncPtr)(ni, fUserData);
         return kCallbackReturnNothing;
      }

      const std::string& Name() const { return fSelection.fName; }
      char What() const { return fSelection.fWhat; }
      char When() const { return fSelection.fWhen; }
      char Transition() const { return fSelection.fTransition; }

   private:
      static int CallbackInterfaceProxy(const NotifyInfo& ni, void* userData) {
         return reinterpret_cast<const CallbackInterface*>(userData)->Invoke(ni);
      }

      NotifySelection fSelection;
      FreeCallbackFunc_t fFuncPtr;
      union {
         void* fUserData;
         const CallbackInterface* fCallbackInterface;
      };
   };
} // namespace Reflex

#include "Reflex/Type.h"
#include "Reflex/Member.h"

namespace Reflex {
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
            const_cast<ICallback*>(this)->operator()(ni.Element<Type>());
         else //if (ni.fWhat == kNotifyMember)
            const_cast<ICallback*>(this)->operator()(ni.Element<Member>());
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
