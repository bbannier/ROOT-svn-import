// @(#)root/reflex:$Name:  $:$Id$
// Author: Axel Naumann, 2009

// Copyright CERN, CH-1211 Geneva 23, 2004-2009, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BUILD
#define REFLEX_BUILD
#endif

#include "ScopeCatalogImpl.h"

#include "Reflex/internal/ScopeName.h"
#include "CatalogImpl.h"
#include "Namespace.h"

//-------------------------------------------------------------------------------
Reflex::Internal::ScopeCatalogImpl::~ScopeCatalogImpl() {
//-------------------------------------------------------------------------------
// Cleanup memory allocations for scopes.

   ScopeVec_t::const_iterator it, itE;
   for ( it = fScopeVec.begin(), itE = fScopeVec.end(); it != itE; ++it ) {
      delete ((ScopeName*)it->Id());
   }
}

//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeCatalogImpl::Init() {
//-------------------------------------------------------------------------------
// create the global scope and its "declaring scope" as returned by __NIRVANA__().
   fNirvana = new ScopeName("@N@I@R@V@A@N@A@", 0, fCatalog->ThisCatalog());
   fGlobalScope = (new Namespace(fCatalog))->ToScopeName();
}

//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeCatalogImpl::Add(Reflex::ScopeName& scope) {
//-------------------------------------------------------------------------------
// Add a scope to the map.
   NotifyInfoT<Scope> ni(scope.ThisScope(), scope.Name(), kNotifyType, kNotifyBefore, kNotifyNameCreated);
   std::map<std::string, std::set<Callback*> >::iterator iCallback = fOrphanedCallbacks.find(scope.Name());
   bool handled = false;
   bool vetoed = false;
   if (iCallback != fOrphanedCallbacks.end()) {
      for(std::set<Callback*>::iterator i = iCallback->second.begin(),
             e = iCallback->second.end(); i != e; ++i) {
         // Whatever the selection: it has scope's name so we register it
         scope.RegisterCallback(*(*i));
         if (!handled && ((*i)->Transition() & kNotifyNameCreated) && ((*i)->When() & kNotifyBefore)) {
            int ret = (*i)->Invoke(ni);
            handled = ret & kCallbackReturnHandled;
            vetoed |= ret & kCallbackReturnVeto;
         }
      }
   }

   if (!vetoed) {
      fName2ScopeMap[&scope.Name()] = scope.ThisScope();
      fScopeVec.push_back(scope.ThisScope());
      if (iCallback != fOrphanedCallbacks.end()) {
         handled = false;
         ni.fWhen = kNotifyAfter;
         for(std::set<Callback*>::iterator i = iCallback->second.begin(),
                e = iCallback->second.end(); i != e;) {
            if (!handled && ((*i)->Transition() & kNotifyNameCreated) && ((*i)->When() & kNotifyBefore)) {
               int ret = (*i)->Invoke(ni);
               handled = ret & kCallbackReturnHandled;
            }
            // Whatever the selection: it has scope's name so we remove it
            std::set<Callback*>::iterator curr = i;
            ++i;
            iCallback->second.erase(curr);
         }
      }
   }
}


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Internal::ScopeCatalogImpl::ByName(const std::string & name) const {
//-------------------------------------------------------------------------------
// Lookup a scope by fully qualified name.
   Name2ScopeMap_t::const_iterator it;
   if (name.size()>2 && name[0]==':' && name[1]==':') {
      const std::string & k = name.substr(2);
      it = Name2ScopeMap().find(&k);
   } else {
      it = Name2ScopeMap().find(&name);
   }
   if (it != Name2ScopeMap().end())
      return it->second;
   else {
      // HERE STARTS AN UGLY HACK WHICH HAS TO BE UNDONE ASAP
      // Lookup scope name as typedef-to-scope (i.e. in types)
      // (also remove include Reflex/Type.h)
      Type t = fCatalog->Types().ByName(name);
      if (t && t.IsTypedef()) {
         while (t.IsTypedef()) t = t.ToType();
         if (t.IsClass() || t.IsEnum() || t.IsUnion())
            return t;
      }
   }
   return Dummy::Scope();
   // END OF UGLY HACK
}

//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeCatalogImpl::Remove(Reflex::ScopeName& scope) {
//-------------------------------------------------------------------------------
// Remove the scope from the list of known scopes.
   Name2ScopeMap_t::iterator iName2ScopeMap = fName2ScopeMap.find(&scope.Name());
   if (iName2ScopeMap != fName2ScopeMap.end()) {
      fName2ScopeMap.erase(iName2ScopeMap);
      fScopeVec.erase(std::find(fScopeVec.begin(), fScopeVec.end(), scope.ThisScope()));
   }
}

//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeCatalogImpl::RegisterCallback(Callback& cb) {
//-------------------------------------------------------------------------------
   if (cb.Name().empty()) {
      for (int i = 0; i < kNotifyNumTransitions / 2; ++i) {
         if (cb.Transition() & (1 << i)) // ->
            fAnonymousCallbacks[i].insert(&cb);
         if (cb.Transition() & (10 << i)) // <-
            fAnonymousCallbacks[i + kNotifyNumTransitions / 2].insert(&cb);
      }
   } else {
      Name2ScopeMap_t::const_iterator iScope = fName2ScopeMap.find(&cb.Name());
      if (iScope != fName2ScopeMap.end()) {
         ScopeName* tn = (ScopeName*) iScope->second.Id();
         if (tn) tn->RegisterCallback(cb);
         else fOrphanedCallbacks[cb.Name()].insert(&cb);
      } else {
         fOrphanedCallbacks[cb.Name()].insert(&cb);
      }
   }
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeCatalogImpl::UnregisterCallback(Callback& cb) {
//-------------------------------------------------------------------------------
   if (cb.Name().empty()) {
      for (int i = 0; i < kNotifyNumTransitions / 2; ++i) {
         std::set<Callback*>::iterator iC;
         if (cb.Transition() & (1 << i))  { // ->
            iC = fAnonymousCallbacks[i].find(&cb);
            if (iC != fAnonymousCallbacks[i].end())
               fAnonymousCallbacks[i].erase(iC);
         }
         if (cb.Transition() & (10 << i)) { // <-
            int idx = i + kNotifyNumTransitions / 2;
            iC = fAnonymousCallbacks[idx].find(&cb);
            if (iC != fAnonymousCallbacks[idx].end())
               fAnonymousCallbacks[idx].erase(iC);
         }
      }
   } else {
      Name2ScopeMap_t::const_iterator iScope = fName2ScopeMap.find(&cb.Name());
      ScopeName* sn = 0;
      if (iScope != fName2ScopeMap.end())
         sn = (ScopeName*) iScope->second.Id();
      if (sn) sn->UnregisterCallback(cb);
      else {
         std::set<Callback*>& cbSet = fOrphanedCallbacks[cb.Name()];
         std::set<Callback*>::iterator iC = cbSet.find(&cb);
         if (iC != cbSet.end()) {
            cbSet.erase(iC);
         }
      }
   }
}


