// @(#)root/reflex:$Name:  $:$Id$
// Author: Axel Naumann, 2007

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

#include "CatalogImpl.h"

#include "ScopeName.h"
#include "Reflex/EntityProperty.h"
#include "Namespace.h"
#include "TypeName.h"
#include "TypeBase.h"
#include "ScopeName.h"
#include "Fundamental.h"
#include "Typedef.h"

//-------------------------------------------------------------------------------
Reflex::Internal::PairTypeInfoType::PairTypeInfoType(const TypeName& type):
   fType(&type), fTI(type.ToTypeBase()->TypeInfo()) {}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
Reflex::Internal::CatalogImpl&
Reflex::Internal::CatalogImpl::Instance() {
//-------------------------------------------------------------------------------
// Return the global instance of the reflection catalog.
   static CatalogImpl instance;

   return instance;
}


//-------------------------------------------------------------------------------
Reflex::Internal::CatalogImpl::CatalogImpl():
   fScopes(this), fTypes(this), fName("Global Reflex Catalog") {
//-------------------------------------------------------------------------------
   fScopes.Init();
   fTypes.Init();
}


//-------------------------------------------------------------------------------
Reflex::Internal::CatalogImpl::CatalogImpl(const std::string& name):
   fScopes(this), fTypes(this), fName(name) {
//-------------------------------------------------------------------------------
   fScopes.Init();
   // fundamental types go only into the static dict,
   // and that is initialized via the default constructor:
   // fTypes.Init();
}


#define RFLX_TYPECAT_DECLFUND(type) \
tb = new Internal::Fundamental(#type, sizeof(type), typeid(type), myCatalog);\
tb->Properties().AddProperty("Description", "fundamental type")

//-------------------------------------------------------------------------------
void
Reflex::Internal::TypeCatalogImpl::Init() {
//-------------------------------------------------------------------------------
// Initialize the fundamental types of a catalog; only called for the static
// instance of the CatalogImpl.

   Internal::Fundamental * tb = 0;
   Type t;
   Catalog myCatalog(fCatalog->ThisCatalog());
 
   // char [3.9.1.1]
   RFLX_TYPECAT_DECLFUND(char);

   // signed integer types [3.9.1.2]
   RFLX_TYPECAT_DECLFUND(signed char);

   RFLX_TYPECAT_DECLFUND(short int);
   t = tb->ThisType();
   new Internal::Typedef("short", t, myCatalog, kETFundamental, t);
   new Internal::Typedef("signed short", t, myCatalog, kETFundamental, t);
   new Internal::Typedef("short signed", t, myCatalog, kETFundamental, t);
   new Internal::Typedef("signed short int", t, myCatalog, kETFundamental, t);
   new Internal::Typedef("short signed int", t, myCatalog, kETFundamental, t);

   RFLX_TYPECAT_DECLFUND(int);
   t = tb->ThisType();
   new Internal::Typedef("signed", t, myCatalog, kETFundamental, t);
   new Internal::Typedef("signed int", t, myCatalog, kETFundamental, t);

   RFLX_TYPECAT_DECLFUND(long int);
   t = tb->ThisType();
   new Internal::Typedef("long", t, myCatalog, kETFundamental, t);
   new Internal::Typedef("signed long", t, myCatalog, kETFundamental, t);
   new Internal::Typedef("long signed", t, myCatalog, kETFundamental, t);
   new Internal::Typedef("signed long int", t, myCatalog, kETFundamental, t);
   new Internal::Typedef("long signed int", t, myCatalog, kETFundamental, t);

   // unsigned integer types [3.9.1.3]
   RFLX_TYPECAT_DECLFUND(unsigned char);

   RFLX_TYPECAT_DECLFUND(unsigned short int);
   t = tb->ThisType();
   new Internal::Typedef("unsigned short", t, myCatalog, kETFundamental, t);
   new Internal::Typedef("short unsigned int", t, myCatalog, kETFundamental, t);

   RFLX_TYPECAT_DECLFUND(unsigned int);
   t = tb->ThisType();
   new Internal::Typedef("unsigned", t, myCatalog, kETFundamental, t);

   RFLX_TYPECAT_DECLFUND(unsigned long int);
   t = tb->ThisType();
   new Internal::Typedef("unsigned long", t, myCatalog, kETFundamental, t);
   new Internal::Typedef("long unsigned", t, myCatalog, kETFundamental, t);
   new Internal::Typedef("long unsigned int", t, myCatalog, kETFundamental, t);

   /* // w_chart [3.9.1.5]
      RFLX_TYPECAT_DECLFUND(w_chart);
   */

   // bool [3.9.1.6]
   RFLX_TYPECAT_DECLFUND(bool);

   // floating point types [3.9.1.8]
   RFLX_TYPECAT_DECLFUND(float);

   RFLX_TYPECAT_DECLFUND(double);

   RFLX_TYPECAT_DECLFUND(long double);

   // void [3.9.1.9]
   tb = new Internal::Fundamental("void", 0, typeid(void), myCatalog);
   tb->Properties().AddProperty("Description", "fundamental type");

   // Large integer definition depends of the platform
#if defined(_MSC_VER) && !defined(__CINT__)
   typedef __int64 longlong;
   typedef unsigned __int64 ulonglong;
#else
   typedef long long int longlong; /* */
   typedef unsigned long long int /**/ ulonglong;
#endif

   // non fundamental types but also supported at initialisation
   tb = new Internal::Fundamental("long long", sizeof(longlong), 
                                  typeid(longlong), myCatalog);
   tb->Properties().AddProperty("Description", "fundamental type");
   t = tb->ThisType();
   new Internal::Typedef("long long int", t, myCatalog, kETFundamental, t);

   tb = new Internal::Fundamental("unsigned long long", sizeof(ulonglong), 
                                  typeid(ulonglong), myCatalog);
   tb->Properties().AddProperty("Description", "fundamental type");
   t = tb->ThisType();
   new Internal::Typedef("long long unsigned", t, myCatalog, kETFundamental, t);
   new Internal::Typedef("unsigned long long int", t, myCatalog, kETFundamental, t);
   new Internal::Typedef("long long unsigned int", t, myCatalog, kETFundamental, t);
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::TypeCatalogImpl::ByName(const std::string & name) const {
//-------------------------------------------------------------------------------
// Lookup a type by name.
   size_t pos = 0;
   while (name[pos] == ':') ++pos;
   TypeContainer_t::const_iterator it = fAllTypes.Find(name.substr(pos));
   if (it) return *it;
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::TypeCatalogImpl::ByTypeInfo(const std::type_info & ti) const {
//-------------------------------------------------------------------------------
// Lookup a type by type_info.
   TypeInfoTypeMap_t::const_iterator it = fTypeInfoTypeMap.Find(ti.name());
   if (it) return *it;
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
void Reflex::Internal::TypeCatalogImpl::CleanUp() const {
//-------------------------------------------------------------------------------
   // Cleanup memory allocations for types.
   /* SHOULD BE DONE BY ScopeName!
   for (TypeContainer_t::const_iterator it = fAllTypes.Begin(); it != fAllTypes.End(); ++it) {
      TypeName * tn = (TypeName*)it->Id();
      Type * t = tn->fThisType;
      if (*t) t->Unload();
   }
   */
}


//-------------------------------------------------------------------------------
void Reflex::Internal::TypeCatalogImpl::Add(Reflex::Internal::TypeName& type, const std::type_info * ti) {
//-------------------------------------------------------------------------------
// Add a type_info to the map.
   fAllTypes.Insert(&type);
   if (ti) fTypeInfoTypeMap.Insert(Internal::PairTypeInfoType(type, *ti));
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::TypeCatalogImpl::Remove(TypeName& type) {
//-------------------------------------------------------------------------------
// Remove the type from the list of known types.
   fAllTypes.Remove(&type);
}


//-------------------------------------------------------------------------------
void Reflex::Internal::TypeCatalogImpl::UpdateTypeId(const Reflex::Internal::TypeName& type, const std::type_info & newti, 
                                    const std::type_info & oldti /* =typeid(NullType) */) {
//-------------------------------------------------------------------------------
// Update a type_info in the map.
   if (oldti != typeid(NullType))
      fTypeInfoTypeMap.Remove(Internal::PairTypeInfoType(type, oldti));
   if (newti != typeid(NullType))
      fTypeInfoTypeMap.Insert(Internal::PairTypeInfoType(type, newti));
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeCatalogImpl::Init() {
//-------------------------------------------------------------------------------
// create the global scope and its "declaring scope" as returned by __NIRVANA__().
   fNirvana = new ScopeName("@N@I@R@V@A@N@A@", 0, fCatalog->ThisCatalog());
   fGlobalScope = (new Namespace(fCatalog->ThisCatalog()))->ToScopeName();
}

//-------------------------------------------------------------------------------
void Reflex::Internal::ScopeCatalogImpl::Add(Reflex::Internal::ScopeName& scope) {
//-------------------------------------------------------------------------------
// Add a scope to the map.
   fAllScopes.Insert(&scope);
}


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Internal::ScopeCatalogImpl::ByName(const std::string & name) const {
//-------------------------------------------------------------------------------
// Lookup a scope by fully qualified name.
   size_t pos = 0;
   while (name[pos] == ':') ++pos;
   const std::string & k = name.substr(pos);
   ScopeContainer_t::const_iterator it = fAllScopes.Find(k);
   if (it) return *it;
   //else                        return Dummy::Scope();
   // HERE STARTS AN UGLY HACK WHICH HAS TO BE UNDONE ASAP
   // (also remove inlcude Reflex/Type.h)
   Type t = fCatalog->Types().ByName(name);
   if (t && t.Is(kTypedef)) {
      while (t.Is(kTypedef)) t = t.ToType();
      if (t.Is(kClassOrStruct || kEnum || kUnion))
         return t.operator Scope ();
   }
   return Dummy::Scope();
   // END OF UGLY HACK
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeCatalogImpl::CleanUp() const {
//-------------------------------------------------------------------------------
   // Cleanup memory allocations for scopes.

   /* SHOULD BE DONE BY ScopeName!
   ScopeVec_t::iterator it;
   for (it = sScopeVec().begin(); it != sScopeVec().end(); ++it) {
      Scope * s = ((ScopeName*)it->Id())->fThisScope;
      if (*s) s->Unload();
      delete s;
   }
   for (it = sScopeVec().begin(); it != sScopeVec().end(); ++it) {
      delete ((ScopeName*)it->Id());
   }
   */
}

//-------------------------------------------------------------------------------
const Reflex::Type&
Reflex::Internal::TypeCatalogImpl::Get(EFUNDAMENTALTYPE type) {
//-------------------------------------------------------------------------------
   static Type sFundamentalTypes[kNotFundamental + 1] = {
      Catalog::Instance().ByName("char"),
      Catalog::Instance().ByName("signed char"),
      Catalog::Instance().ByName("short int"),
      Catalog::Instance().ByName("long int"),
      Catalog::Instance().ByName("unsigned char"),
      Catalog::Instance().ByName("unsigned short int"),
      Catalog::Instance().ByName("unsigned int"),
      Catalog::Instance().ByName("unsigned long int"),
      Catalog::Instance().ByName("bool"),
      Catalog::Instance().ByName("float"),
      Catalog::Instance().ByName("double"),
      Catalog::Instance().ByName("long double"),
      Catalog::Instance().ByName("void"),
      Catalog::Instance().ByName("long long"),
      Catalog::Instance().ByName("unsigned long long"),
      Type() // kNotFundamental
   };

   return sFundamentalTypes[type];
}


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Internal::ScopeCatalogImpl::GlobalScope() const {
//-------------------------------------------------------------------------------
// Return the global scope's Scope object.
   return fGlobalScope;
}


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Internal::ScopeCatalogImpl::__NIRVANA__() const {
//-------------------------------------------------------------------------------
// Return the global scope's Scope object.
   return fNirvana;
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeCatalogImpl::Remove(ScopeName& scope) {
//-------------------------------------------------------------------------------
// Remove the scope from the list of known scopes.
   fAllScopes.Remove(&scope);
}
