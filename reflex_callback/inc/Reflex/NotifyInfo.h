// @(#)root/reflex:$Id$
// Author: Axel Naumann, 2009

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_NotifyInfo
#define Reflex_NotifyInfo

namespace Reflex {

   /** What to call the callback for
    */
   enum ENotifyWhat {
      kNotifyType = 1,
      kNotifyScope = 2,
      kNotifyMember = 4,
      kNotifyEverything = 7
   };

   /**
    * Enum for bit mask defining what a callback should be called for.
    * The states of types / scopes / members are
    *   1) unknown
    *   2) only name known (e.g. TypeName, but no TypeBase)
    *   3) forward declaration exists (TypeBase without type's data, e.g. "class" but no scope yet)
    *   4) type fully declared (sizeof etc)
    *   5) fully declared (incl. scope)
    */
   enum ENotifyTransition {
      kNotifyNameCreated = 0x01, // transition 1 -> 2
      kNotifyNameResolved = 0x02, // transition 2 -> 3
      kNotifyTypeDeclared = 0x04, // transition 3 -> 4
      kNotifyFullyDeclared = 0x08, // transition 4 -> 5
      kNotifyAllLoad = 0x0f, // any transition in -> direction

      kNotifyPartiallyUndeclared = 0x80, // transition 5 -> 4
      kNotifyTypeUndeclared = 0x40, // transition 4 -> 3
      kNotifyUnresolved = 0x20, // transition 3 -> 2
      kNotifyUnloaded = 0x10, // transition 2 -> 1
      kNotifyAllUnload = 0xf0, // any transition in <- direction

      kNotifyAllTransitions = kNotifyAllLoad | kNotifyAllUnload
   };

   /**
    * When to call the callback: before or after the transition has occurred.
    */
   enum ENotifyWhen {
      kNotifyBefore = 1, // called before the transition happens
      kNotifyAfter = 2, // called after the transition happens
      kNotifyBoth = kNotifyBefore | kNotifyAfter,

      kNotifyDisabled = 0x10, // never called; bit combined with kNotifyBefore and / or kNotifyAfter
   };

   /**
    * Callback messages to the callback handler; return values of the callback function
    */
   enum ECallbackReturn {
      kCallbackReturnNothing = 0, // default
      kCallbackReturnHandled = 1, // stop processing other callbacks of the same NotifyInfo
      kCallbackReturnVeto    = 2, // for kNotifyBefore: veto the action that should follow (e.g. unloading)
   };

   class NotifySelection {
   public:
      NotifySelection(std::string name = "",
                      int what = kNotifyEverything,
                      int when = kNotifyAfter,
                      int transition = kNotifyFullyDeclared | kNotifyUnloaded):
         fName(name),
         fWhat(what),
         fWhen(when),
         fTransition(transition)
      {}

      const std::string& Name() const { return fName; }
      char What() const { return fWhat; }
      char When() const { return fWhen; }
      char Transition() const { return fTransition; }

   protected:
      std::string fName; // name this should trigger on
      char fWhat; // (combination of) ENotifyWhat
      char fWhen; // (combination of) ENotifyWhen
      char fTransition; // (combination of) ENotifyTransition
   };

   template <typename T> class NotifyInfoT;

   struct NotifyInfo {
      NotifyInfo(const std::string& name, ENotifyWhat what, ENotifyWhen when, ENotifyTransition trans):
         fName(name), fWhat(what), fWhen(when), fTransition(trans) {}

      template <class ELEM>
      const ELEM &
      Element() const {
         return reinterpret_cast< const NotifyInfoT<ELEM>& >(*this).fElem;
      }
      const std::string& fName;
      ENotifyWhat fWhat;
      ENotifyWhen fWhen;
      ENotifyTransition fTransition;
   };

   template <class ELEM> // Type, Scope, or Member
   struct NotifyInfoT: NotifyInfo {
      NotifyInfoT(ELEM elem, ENotifyWhat what, ENotifyWhen when, ENotifyTransition trans):
         NotifyInfo(elem.Name(), what, when, trans),
         fElem(elem) {}

      ELEM fElem;
   };

} // namespace Reflex

#endif // Reflex_NotifyInfo
