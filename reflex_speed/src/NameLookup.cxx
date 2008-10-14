// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2006

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BUILD
#define REFLEX_BUILD
#endif

#include "NameLookup.h"
#include "Reflex/Base.h"
#include "Reflex/Scope.h"
#include "Reflex/Type.h"
#include "Reflex/Tools.h"
#include "Reflex/EntityProperty.h"

#include "OwnedMember.h"
#include "TypeBase.h"
#include "ScopeBase.h"

#include <sstream>
#include <string.h>

//______________________________________________________________________________
Reflex::NameLookup::NameLookup(const std::string& name, const Scope& current):
   fLookupName(name), fPosNamePart(0), fPosNamePartLen(std::string::npos),
   fCurrentScope(current), fPartialSuccess(false)
{
   // Initialize a NameLookup object used internally to keep track of lookup
   // states.
}

//______________________________________________________________________________
Reflex::Type Reflex::NameLookup::LookupType(const std::string& nam, const Scope& current)
{
   // Lookup up a (possibly scoped) type name appearing in the scope context
   // current. This is the public interface for type lookup.
   NameLookup lookup(nam, current);
   return lookup.Lookup<Type>();
}

//______________________________________________________________________________
Reflex::Scope Reflex::NameLookup::LookupScope(const std::string& nam, const Scope& current)
{
   // Lookup up a (possibly scoped) scope name appearing in the scope context
   // current. This is the public interface for scope lookup.
   NameLookup lookup(nam, current);
   return lookup.Lookup<Scope>();
}

/*
//______________________________________________________________________________
Reflex::Member LookupMember(const std::string& nam, const Scope& current)
{
   // Lookup up a (possibly scoped) member name appearing in the scope context
   // current. This is the public interface for member lookup.
   NameLookup lookup(nam, current);
   // this will not work, member lookup is too different from type lookup...
   return lookup.Lookup<Member>();
}
*/

//______________________________________________________________________________
template<class T> T Reflex::NameLookup::Lookup(bool isTemplateExpanded /* = false */)
{
   // Lookup a type using fLookupName, fCurrentScope.

   Scope startScope = fCurrentScope;
   T result;

   fPartialSuccess = false;
   fPosNamePart = 0;
   fPosNamePartLen = std::string::npos;
   FindNextScopePos();
   if (fPosNamePart == 2) {
      fLookedAtUsingDir.clear();
      // ::A...
      fCurrentScope = Scope::GlobalScope();
      result = LookupInScope<T>();
   } else {
      // A...
      result = LookupInUnknownScope<T>();
   }

   if (!isTemplateExpanded && !result && fLookupName.find('<') != std::string::npos) {

      // We need to replace the template argument both of this type
      // and any of it enclosing type:
      //    myspace::mytop<A,B>::mytype<C>
      
      std::ostringstream tmp;
      for(size_t i = 0, level = 0, sofar = 0; i < fLookupName.size(); ++i) {
         if (level==0) {
            tmp << fLookupName.substr(sofar, i+1 - sofar);
            sofar = i+1;
         }
         switch(fLookupName[i]) {
            case '<': ++level; break;
            case '>': --level; // intentional fall through to the ',' case
            case ',':
               if (level == (1 - (int)(fLookupName[i]=='>'))) {
                  std::string arg(fLookupName.substr(sofar, i-sofar));
                  
                  size_t p = arg.size();
         
                  while (p > 0 && (arg[p-1] == '*' || arg[p-1] == '&' || arg[p-1] == ' '))
                     --p;
                  
                  std::string end(arg.substr(p));
                  arg.erase(p );

                  const char *start = arg.c_str();
                  while(strncmp(start,"const ",6)==0) start+=6;

                  tmp << arg.substr(0, start - arg.c_str());

                  while(strncmp(start,"std::",5)==0) start+=5;

                  arg.erase(0, start - arg.c_str()); 

                  Reflex::Type type(LookupType(arg , startScope));

                  if (type) {
                     if (type.Name()!="Double32_t" && type.Name()!="Float16_t") {
                        // Use the real type rather than the typedef unless
                        // this is Double32_t or Float16_t
                        type = type.FinalType();
                     }
                     tmp << type.Name(Reflex::kScoped|Reflex::kQualified);
                  } else {
                     tmp << arg;
                  }
                  tmp << end;
                  
                  tmp << fLookupName[i];

                  sofar = i+1;
               } 
               break;
         }      
      }
      NameLookup next(tmp.str(), startScope);
      return next.Lookup<T>(true);
   }


   return result;
}

//______________________________________________________________________________
template<class T> T Reflex::NameLookup::LookupInScope()
{
   // Lookup a type in fCurrentScope.
   // Checks sub-types, sub-scopes, using directives, and base classes for
   // the name given by fLookupName, fPosNamePart, and fPosNamePartLen.
   // If the name part is found, and another name part follows in fPosNamePart,
   // LookupTypeInScope requests the scope found to lookup the next name
   // part. fPartialMatch reflexts that the left part of the name was matched;
   // even if the trailing part of fLookupName cannot be found, the lookup
   // cannot continue on declaring scopes and has to fail.
   // A list of lookups performed in namespaces pulled in via using directives is
   // kept in fLookedAtUsingDir, to prevent infinite loops due to
   //   namespace A{using namespace B;} namespace B{using namespace A;}
   // loops.
   // The lookup does not take the declaration order into account; the result of
   // parts of the lookup algorithm which depend on the order will be unpredictable.

   if (!fCurrentScope) {
      return Dummy::Get<T>();
   }

   // prevent inf loop from
   // ns A { using ns B; } ns B {using ns A;}
   if (fLookedAtUsingDir.find(fCurrentScope) != fLookedAtUsingDir.end()) {
      return Dummy::Get<T>();
   }

   int len = fCurrentScope.SubTypes().Size();
   int i = 0;
   for (Type_Iterator it = fCurrentScope.SubTypes().Begin(); i < len; ++it, ++i) {
      const Type& type = *it;
      const Internal::TypeBase* base = type.ToTypeBase();
      if (base) {
         size_t pos;
         const std::string &name(base->SimpleName(pos));
         //fprintf(stderr, "Reflex::NameLookup::LookupInScope<T>: looking up '%s', considering subscope '%s' ...\n", fLookupName.c_str(), name.c_str());
         if (
            (fLookupName[fPosNamePart] == name[pos]) &&
            !fLookupName.compare(fPosNamePart, fPosNamePartLen, name, pos, name.length())
        ) {
            //fprintf(stderr, "Reflex::NameLookup::LookupInScope<T>: lookup up '%s', partial success with subscope '%s' ...\n", fLookupName.c_str(), name.c_str());
            fPartialSuccess = true;
            fLookedAtUsingDir.clear();
            FindNextScopePos();
            if (fPosNamePart == std::string::npos) {
               return type;
            }
            if (it->Is(kTypedef)) {
               fCurrentScope = it->FinalType();
            }
            else {
               fCurrentScope = type;
            }
            return LookupInScope< T >();
         }
      }
   }

   Scope_Iterator subscope_end(fCurrentScope.SubScopes().End());
   for (Scope_Iterator in = fCurrentScope.SubScopes().Begin(); in != subscope_end; ++in) {
      // only take namespaces into account - classes were checked as part of SubType
      if (in->Is(kNamespace)) {
         const Scope& scope = *in;
         const Internal::ScopeBase* base = scope.ToScopeBase();
         if (base) {
            size_t pos;
            const std::string& name(base->SimpleName(pos));
            if (
               (fLookupName[fPosNamePart] == name[pos]) &&
               !fLookupName.compare(fPosNamePart, fPosNamePartLen, name, pos, name.length())
           ) {
               fPartialSuccess = true;
               fLookedAtUsingDir.clear();
               FindNextScopePos();
               if (fPosNamePart == std::string::npos) {
                  return (T) (*in);
               }
               fCurrentScope = (Scope) (*in);
               return LookupInScope<T>();
            }
         }
      }
   }

   if (fCurrentScope.UsingDirectives().Size()) {
      fLookedAtUsingDir.insert(fCurrentScope);
      Scope storeCurrentScope = fCurrentScope;
      Scope_Iterator usingscope_end(storeCurrentScope.UsingDirectives().End());
      for (Scope_Iterator si = storeCurrentScope.UsingDirectives().Begin(); si != usingscope_end; ++si) {
         fCurrentScope = *si;
         T t = LookupInScope<T>();
         if (fPartialSuccess) {
            return t;
         }
      }
      fCurrentScope = storeCurrentScope;
   }

   if (!fPosNamePart) { // only for "BaseClass...", not for "A::BaseClass..."
      Type currentScopeAsType(fCurrentScope);
      Base_Iterator base_end(currentScopeAsType.Bases().End());
      for (Base_Iterator bi = currentScopeAsType.Bases().Begin(); bi != base_end; ++bi) {
         if (!fLookupName.compare(fPosNamePart, fPosNamePartLen, bi->Name())) {
            fPartialSuccess = true;
            fLookedAtUsingDir.clear();
            FindNextScopePos();
            if (fPosNamePart == std::string::npos) {
               return bi->ToType();
            }
            fCurrentScope = bi->ToType().FinalType();
            return LookupInScope< T >();
         }
      }
   }

   if (((Type)fCurrentScope).Bases().Size()) {
      Scope storeCurrentScope = fCurrentScope;
      Type currentScopeAsType(fCurrentScope);
      Base_Iterator base_end(currentScopeAsType.Bases().End());
      for (Base_Iterator bi = currentScopeAsType.Bases().Begin(); bi != base_end; ++bi) {
         fCurrentScope = bi->ToScope();
         T t = LookupInScope<T>();
         if (fPartialSuccess) {
            return t;
         }
      }
      fCurrentScope = storeCurrentScope;
   }

   return Dummy::Get<T>();
}

//______________________________________________________________________________
template<class T> T Reflex::NameLookup::LookupInUnknownScope()
{
   // Lookup a type in fCurrentScope and its declaring scopes.
   for (fPartialSuccess = false; !fPartialSuccess && fCurrentScope; fCurrentScope = fCurrentScope.DeclaringScope()) {
      fLookedAtUsingDir.clear();
      T t = LookupInScope<T>();
      if (fPartialSuccess) {
         return t;
      }
      if (fCurrentScope.IsTopScope()) {
         break;
      }
   }
   return Dummy::Get<T>();
}

//______________________________________________________________________________
Reflex::Member Reflex::NameLookup::LookupMember(const std::string& nam, const Scope& current)
{
   // Lookup a member.
   if (Tools::GetBasePosition(nam)) {
      return LookupMemberQualified(nam);
   }
   return LookupMemberUnqualified(nam, current);
}

//______________________________________________________________________________
Reflex::Member Reflex::NameLookup::LookupMemberQualified(const std::string& nam)
{
   // Lookup of a qualified member.
   Scope bscope = Scope::ByName(Tools::GetScopeName(nam));
   if (bscope) {
      return LookupMemberUnqualified(Tools::GetBaseName(nam), bscope);
   }
   return Dummy::Member();
}

//______________________________________________________________________________
Reflex::Member Reflex::NameLookup::LookupMemberUnqualified(const std::string& nam, const Scope& current)
{
   // Lookup of an unqualified member.
   {
      Member m = current.Members().ByName(nam);
      if (m) {
         return m;
      }
   }
   for (Scope_Iterator si = current.UsingDirectives().Begin(); si != current.UsingDirectives().End(); ++si) {
      Member m = LookupMember(nam, *si);
      if (m) {
         return m;
      }
   }
   Type currentScopeAsType(current);
   for (Base_Iterator bi = currentScopeAsType.Bases().Begin(); bi != currentScopeAsType.Bases().End(); ++bi) {
      Member m = LookupMember(nam, bi->ToScope());
      if (m) {
         return m;
      }
   }
   if (!current.IsTopScope()) {
      return LookupMember(nam, current.DeclaringScope());
   }
   return Dummy::Member();
}

//______________________________________________________________________________
Reflex::Type Reflex::NameLookup::AccessControl(const Type& typ, const Scope& /*current*/)
{
   // Check access.
   // if (typ.IsPublic()) {
   //    return true;
   // }
   // else if (typ.IsProtected() && current.HasBase(typ.DeclaringScope())) {
   //    return true;
   // }
   return typ;
}

//______________________________________________________________________________
void Reflex::NameLookup::FindNextScopePos()
{
   // Move fPosNamePart to point to the next scope in fLookupName, updating
   // fPosNamePartLen. If fPosNamePartLen == std::string::npos, initialize
   // fPosNamePart and fPosNamePartLen. If there is no next scope left, fPosNamePart
   // will be set to std::string::npos and fPosNamePartLen will be set to 0.
   if (fPosNamePartLen != std::string::npos) {
      // we know the length, so jump
      fPosNamePart += fPosNamePartLen + 2;
      if (fPosNamePart > fLookupName.length()) {
         // past the string's end?
         fPosNamePart = std::string::npos;
         fPosNamePartLen = 0;
         return;
      }
   }
   else {
      // uninitialized
      // set fPosNamePartLen and check that fLookupName doesn't start with '::'
      fPosNamePart = 0;
      if (!fLookupName.compare(0, 2, "::")) {
         fPosNamePart = 2;
      }
   }
   fPosNamePartLen = Tools::GetFirstScopePosition(fLookupName.substr(fPosNamePart));
   if (!fPosNamePartLen) { // no next "::"
      fPosNamePartLen = fLookupName.length();
   }
   else { // no "::"
      fPosNamePartLen -= 2;
   }
}

