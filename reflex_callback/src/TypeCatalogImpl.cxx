// @(#)root/reflex:$Name:  $:$Id$
// Author: Axel Naumann, 2009

// Copyright CERN, CH-1211 Geneva 23, 2004-2007, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BUILD
#define REFLEX_BUILD
#endif

#include "TypeCatalogImpl.h"

#include "CatalogImpl.h"
#include "Reflex/Type.h"
#include "Typedef.h"

namespace {
   // Helper to factor out common code
   class FundamentalDeclarator {
   public:
      FundamentalDeclarator(const char* name, size_t size, const std::type_info& ti,
                            Reflex::REPRESTYPE repres) {
         Reflex::TypeBase* tb = new Reflex::TypeBase(name, size, Reflex::FUNDAMENTAL,
                                                     ti, Reflex::Type(), fgCatalog, repres);
         tb->Properties().AddProperty( "Description", "fundamental type" );
         fType = tb->ThisType();
      }

      FundamentalDeclarator& Typedef(const char* name) {
         new Reflex::Typedef(name, fType, Reflex::FUNDAMENTAL, fType, fgCatalog);
         return *this;
      }

      static Reflex::Catalog fgCatalog;
   private:
      Reflex::Type fType;
   };

   // sizeof(void) doesn't work; we want it to return 0.
   // This template with the specialization does just that.
   template <typename T>
   struct GetSizeOf {
      size_t operator()() const { return sizeof(T); }
   };
   template <>
   struct GetSizeOf<void> {
      size_t operator()() const { return 0; }
   };

   // Helper function constructing the declarator
   template <typename T>
   FundamentalDeclarator DeclFundamental(const char* name, Reflex::REPRESTYPE repres) {
      return FundamentalDeclarator(name, GetSizeOf<T>()(), typeid(T), repres);
   }

   Reflex::Instance instantiate;

   Reflex::Catalog FundamentalDeclarator::fgCatalog;

}


//-------------------------------------------------------------------------------
void
Reflex::Internal::TypeCatalogImpl::Init() {
//-------------------------------------------------------------------------------
// Initialize the fundamental types of a catalog; only called for the static
// instance of the CatalogImpl.
   // initialising fundamental types

   FundamentalDeclarator::fgCatalog = fCatalog->ThisCatalog();
   // char [3.9.1.1]
   DeclFundamental<char>("char", REPRES_CHAR);

   // signed integer types [3.9.1.2]
   DeclFundamental<signed char>("signed char", REPRES_SIGNED_CHAR);

   DeclFundamental<short int>("short int", REPRES_SHORT_INT)
      .Typedef("short")
      .Typedef("signed short")
      .Typedef("short signed")
      .Typedef("signed short int")
      .Typedef("short signed int");

   DeclFundamental<int>("int", REPRES_INT)
      .Typedef("signed")
      .Typedef("signed int");

   DeclFundamental<long int>("long int", REPRES_LONG_INT)
      .Typedef("long")
      .Typedef("signed long")
      .Typedef("long signed")
      .Typedef("signed long int")
      .Typedef("long signed int");

   // unsigned integer types [3.9.1.3]
   DeclFundamental<unsigned char>("unsigned char", REPRES_UNSIGNED_CHAR);

   DeclFundamental<unsigned short int>("unsigned short int", REPRES_UNSIGNED_SHORT_INT)
      .Typedef("unsigned short")
      .Typedef("short unsigned int");

   DeclFundamental<unsigned int>("unsigned int", REPRES_UNSIGNED_INT)
      .Typedef("unsigned");

   DeclFundamental<unsigned long int>("unsigned long int", REPRES_UNSIGNED_LONG_INT)
      .Typedef("unsigned long")
      .Typedef("long unsigned")
      .Typedef("long unsigned int");

   /* w_chart [3.9.1.5]
      DeclFundamental<w_chart>("w_chart", REPRES_WCHART);
   */

   // bool [3.9.1.6]
   DeclFundamental<bool>("bool", REPRES_BOOL);

   // floating point types [3.9.1.8]
   DeclFundamental<float>("float", REPRES_FLOAT);
   DeclFundamental<double>("double", REPRES_DOUBLE);
   DeclFundamental<long double>("long double", REPRES_LONG_DOUBLE);

   // void [3.9.1.9]
   DeclFundamental<void>("void", REPRES_VOID);

      // Large integer definition depends of the platform
#if defined(_WIN32) && !defined(__CINT__)
   typedef __int64 longlong;
   typedef unsigned __int64 ulonglong;
#else
   typedef long long int longlong; /* */
   typedef unsigned long long int /**/ ulonglong;
#endif

   // non fundamental types but also supported at initialisation
   DeclFundamental<longlong>("long long", REPRES_LONGLONG)
      .Typedef("long long int");

   DeclFundamental<ulonglong>("unsigned long long", REPRES_ULONGLONG)
      .Typedef("long long unsigned")
      .Typedef("unsigned long long int")
      .Typedef("long long unsigned int");

}


//-------------------------------------------------------------------------------
Reflex::TypeName*
Reflex::Internal::TypeCatalogImpl::ByTypeName(const std::string & name) const {
//-------------------------------------------------------------------------------
// Lookup a type by name.
   Name2TypeNameMap_t::const_iterator it;
   if (name.size()>2 && name[0]==':' && name[1]==':') {
      const std::string & k = name.substr(2);
      it = fName2TypeNameMap.find(&k);
   } else {
      it = fName2TypeNameMap.find(&name);
   }
   if (it != fName2TypeNameMap.end())
      return it->second;

   return 0;
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::TypeCatalogImpl::ByTypeInfo(const std::type_info & ti) const {
//-------------------------------------------------------------------------------
// Lookup a type by type_info.
   TypeIdName2TypeNameMap_t::const_iterator it = fTypeIdName2TypeNameMap.find(ti.name());
   if (it != fTypeIdName2TypeNameMap.end())
      return it->second->ThisType();
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
void Reflex::Internal::TypeCatalogImpl::CleanUp() const {
//-------------------------------------------------------------------------------
   // Cleanup memory allocations for types.
   /* SHOULD BE DONE BY ScopeName!
   for (TypeContainer_t::const_iterator it = fAllTypeNames.Begin(); it != fAllTypeNames.End(); ++it) {
      TypeName * tn = (TypeName*)it->Id();
      Type * t = tn->fThisType;
      if (*t) t->Unload();
   }
   */
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::TypeCatalogImpl::Add(Reflex::TypeName& type, const std::type_info * ti) {
//-------------------------------------------------------------------------------
// Add a type_info to the map.
   NotifyInfoT<Type> ni(type.ThisType(), kNotifyType, kNotifyBefore, kNotifyNameCreated);
   std::map<std::string, std::set<Callback*> >::iterator iCallback = fOrphanedCallbacks.find(type.Name());
   bool handled = false;
   bool vetoed = false;
   if (iCallback != fOrphanedCallbacks.end()) {
      for(std::set<Callback*>::iterator i = iCallback->second.begin(),
             e = iCallback->second.end(); i != e; ++i) {
         // Whatever the selection: it has type's name so we register it
         type.RegisterCallback(*(*i));
         if (!handled && ((*i)->Transition() & kNotifyNameCreated) && ((*i)->When() & kNotifyBefore)) {
            int ret = (*i)->Invoke(ni);
            handled = ret & kCallbackReturnHandled;
            vetoed |= ret & kCallbackReturnVeto;
         }
      }
   }

   if (!vetoed) {
      fName2TypeNameMap[&type.Name()] = &type;
      if (ti) fTypeIdName2TypeNameMap[ti->name()] = &type;
      fTypeVec.push_back(type.ThisType());
      if (iCallback != fOrphanedCallbacks.end()) {
         handled = false;
         ni.fWhen = kNotifyAfter;
         for(std::set<Callback*>::iterator i = iCallback->second.begin(),
                e = iCallback->second.end(); i != e;) {
            if (!handled && ((*i)->Transition() & kNotifyNameCreated) && ((*i)->When() & kNotifyBefore)) {
               int ret = (*i)->Invoke(ni);
               handled = ret & kCallbackReturnHandled;
            }
            // Whatever the selection: it has type's name so we remove it
            std::set<Callback*>::iterator curr = i;
            ++i;
            iCallback->second.erase(curr);
         }
      }
   }
}

//-------------------------------------------------------------------------------
void
Reflex::Internal::TypeCatalogImpl::Remove(Reflex::TypeName& type) {
//-------------------------------------------------------------------------------
// Remove the type from the list of known types.
   Name2TypeNameMap_t::iterator iN2TNM = fName2TypeNameMap.find(&type.Name());
   if (iN2TNM != fName2TypeNameMap.end())
      fName2TypeNameMap.erase(iN2TNM);
   TypeVec_t::iterator iTV = std::find(fTypeVec.begin(), fTypeVec.end(), type.ThisType());
   if (iTV != fTypeVec.end()) {
      TypeIdName2TypeNameMap_t::iterator iTIN2TNM = fTypeIdName2TypeNameMap.find(iTV->TypeInfo().name());
      if (iTIN2TNM != fTypeIdName2TypeNameMap.end() && iTIN2TNM->second == &type)
         fTypeIdName2TypeNameMap.erase(iTIN2TNM);
      fTypeVec.erase(iTV);
   }
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::TypeCatalogImpl::RegisterCallback(Callback& cb) {
//-------------------------------------------------------------------------------
   if (cb.Name().empty()) {
      for (int i = 0; i < kNotifyNumTransitions / 2; ++i) {
         if (cb.Transition() & (1 << i)) // ->
            fAnonymousCallbacks[i].insert(&cb);
         if (cb.Transition() & (10 << i)) // <-
            fAnonymousCallbacks[i + kNotifyNumTransitions / 2].insert(&cb);
      }
   } else {
      TypeName* tn = ByTypeName(cb.Name());
      if (tn) tn->RegisterCallback(cb);
      else fOrphanedCallbacks[cb.Name()].insert(&cb);
   }
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::TypeCatalogImpl::UnregisterCallback(Callback& cb) {
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
      TypeName* tn = ByTypeName(cb.Name());
      if (tn) tn->UnregisterCallback(cb);      
      else {
         std::set<Callback*>& cbSet = fOrphanedCallbacks[cb.Name()];
         std::set<Callback*>::iterator iC = cbSet.find(&cb);
         if (iC != cbSet.end()) {
            cbSet.erase(iC);
         }
      }
   }
}


//-------------------------------------------------------------------------------
void Reflex::Internal::TypeCatalogImpl::UpdateTypeId(Reflex::TypeName& type,
                                                     const std::type_info & newti, 
                                                     const std::type_info & oldti /* =typeid(NullType) */) {
//-------------------------------------------------------------------------------
// Update a type_info in the map.
   if (oldti != typeid(NullType)) {
      TypeIdName2TypeNameMap_t::iterator iTIN2TNM = fTypeIdName2TypeNameMap.find(oldti.name());
      if (iTIN2TNM != fTypeIdName2TypeNameMap.end())
         fTypeIdName2TypeNameMap.erase(iTIN2TNM);
   }
   if (newti != typeid(NullType))
      fTypeIdName2TypeNameMap[newti.name()] = &type;
}
