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
    */
   enum ENotifyTransition {
      kNotifyNameCreated = 0x01, // A TypeName / ScopeName object has been created. Event does not exist for Members.
      kNotifyDeclared = 0x02, // A TypeBase / ScopeBase / MemberBase object has been created.
      kNotifyAddDataMembers = 0x04, // Data members have been added to the ScopeBase.
      kNotifyAddFunctionMembers = 0x08, // Function members have been added to the ScopeBase.
      kNotifyAddMembers = 0x04 | 0x08, // Members have been added to the ScopeBase - note this might be called twice, once for data, once for function members.
      kNotifyAllLoad = 0x0f, // any transition in declaring direction

      kNotifyMembersUnloaded = 0x40 | 0x80, // Members have been removed from the ScopeBase - note this might be called twice, once for data, once for function members.
      kNotifyFunctionMembersUnloaded = 0x80, // Function members have been removed from the ScopeBase.
      kNotifyDataMembersUnloaded = 0x40, // Function members have been removed from the ScopeBase.
      kNotifyUnresolved = 0x20, // A TypeBase / ScopeBase / MemberBase object has been deleted.
      kNotifyNameUnloaded = 0x10, // A TypeName / ScopeName object has been removed from the database. Event does not exist for Members.
      kNotifyAllUnload = 0xf0, // any transition in unloading direction

      kNotifyAllTransitions = kNotifyAllLoad | kNotifyAllUnload,

      kNotifyNumTransitions = 8 // -> and -<
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
      // Name is the fully qualified name
      NotifySelection(std::string name = "",
                      int what = kNotifyEverything,
                      int when = kNotifyAfter,
                      int transition = kNotifyDeclared | kNotifyNameUnloaded):
         fName(name),
         fWhat(what),
         fWhen(when),
         fTransition(transition)
      {}

      const std::string& Name() const { return fName; }
      char What() const { return fWhat; }
      char When() const { return fWhen; }
      char Transition() const { return fTransition; }

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
      NotifyInfoT(ELEM elem, const std::string& name, ENotifyWhat what, ENotifyWhen when, ENotifyTransition trans):
         NotifyInfo(name, what, when, trans),
         fElem(elem) {}

      ELEM fElem;
   };

} // namespace Reflex

#endif // Reflex_NotifyInfo
