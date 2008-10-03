// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

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

#include "TypeName.h"
#include "TypeBase.h"

#include "Reflex/Type.h"

#include "OwnedMember.h"
#include "CatalogImpl.h"

/*

//-------------------------------------------------------------------------------
typedef __gnu_cxx::hash_map<const std::string *, Reflex::Internal::TypeName * > Name2Type_t;
typedef __gnu_cxx::hash_map<const char *, Reflex::Internal::TypeName * > TypeId2Type_t;
typedef std::vector< Reflex::Type > TypeVec_t;


//-------------------------------------------------------------------------------
static Name2Type_t & sTypes() {
//-------------------------------------------------------------------------------
// Static wrapper for type map.
   static Name2Type_t m;
   return m;
}


//-------------------------------------------------------------------------------
static TypeId2Type_t & sTypeInfos() {
//-------------------------------------------------------------------------------
// Static wrapper for type map (type_infos).
   static TypeId2Type_t m;
   return m;
}


//-------------------------------------------------------------------------------
static TypeVec_t & sTypeVec() {
//-------------------------------------------------------------------------------
// Static wrapper for type vector.
   static TypeVec_t m;
   return m;
}
*/

//-------------------------------------------------------------------------------
Reflex::Internal::TypeName::TypeName(const char * nam,
                                      TypeBase * typeBas,
                                      const Catalog& catalog,
                                      const std::type_info * ti)
   : fName(nam),
     fTypeBase(typeBas) {
//-------------------------------------------------------------------------------
// Construct a type name.
   fThisType = new Type(this);
   CatalogImpl* cati = catalog.Impl();
   cati->Types().Add(*this, ti);

   std::string sname = Tools::GetScopeName(nam);
   fScope = catalog.ScopeByName(sname);
   if (!fScope.Id() || fScope.InCatalog() != catalog)
      fScope = (new ScopeName(sname.c_str(), 0, catalog))->ThisScope();
}


//-------------------------------------------------------------------------------
Reflex::Internal::TypeName::~TypeName() {
//-------------------------------------------------------------------------------
// Destructor.
   delete fThisType;
   delete fTypeBase;
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::TypeName::UpdateTypeBase(TypeBase* tb) {
//-------------------------------------------------------------------------------
// If tb is neither fTypeBase nor NULL, UpdateTypeBase deletes the TypeBase
// this TypeName is pointing to and thus removes its information from the
// data structures. The TypeName information will be set to the new tb.
   if (fTypeBase == tb) return;
   if (tb) delete fTypeBase;
   // else we are called from ~TypeBase() already.
   fTypeBase = tb;
}


//-------------------------------------------------------------------------------
void Reflex::Internal::TypeName::SetTypeId(const std::type_info & ti) const {
//-------------------------------------------------------------------------------
// Add a type_info to the map.
   DeclaringScope().InCatalog().Impl()->Types().UpdateTypeId(*this, ti);
}


//-------------------------------------------------------------------------------
void Reflex::Internal::TypeName::HideName() {
//-------------------------------------------------------------------------------
// Append the string " @HIDDEN@" to a type name.
   if (fName.empty() || fName[fName.length()-1] != '@') {
      DeclaringScope().InCatalog().Impl()->Types().Remove(*this);
      fName += " @HIDDEN@";
      DeclaringScope().InCatalog().Impl()->Types().Add(*this, this->fTypeBase ? &this->fTypeBase->TypeInfo() : 0);
   }
}


//-------------------------------------------------------------------------------
Reflex::Type Reflex::Internal::TypeName::ThisType() const {
//-------------------------------------------------------------------------------
// Return Type of this TypeName.
   return *fThisType;
}
