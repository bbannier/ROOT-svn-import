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

#include "Reflex/Kernel.h"

#include "Reflex/PropertyList.h"
#include "Reflex/Type.h"
#include "Reflex/Base.h"
#include "Reflex/Member.h"
#include "Reflex/Object.h"
#include "Reflex/PropertyList.h"
#include "Reflex/MemberTemplate.h"
#include "Reflex/Any.h"
#include "Reflex/Catalog.h"
#include "Reflex/internal/ScopeName.h"

#include "Fundamental.h"
#include "Namespace.h"
#include "Typedef.h"
#include "Class.h"
#include <typeinfo>

namespace {
   Reflex::Instance instantiate;
}

//-------------------------------------------------------------------------------
Reflex::Instance* Reflex::Instance::fgSingleton = 0;
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
Reflex::Instance& Reflex::Instance::CreateReflexInstance() {
//-------------------------------------------------------------------------------
// Initialize the singleton.

   static Reflex::Instance instance((Reflex::Instance*)0);
   return instance;
}

//-------------------------------------------------------------------------------
Reflex::Instance::Instance() {
//-------------------------------------------------------------------------------
// Ensure that Reflex is properly initialized.
   CreateReflexInstance();
}

//-------------------------------------------------------------------------------
Reflex::Instance::Instance(Instance*) {
//-------------------------------------------------------------------------------
// Initialisation of Reflex.Setup of global scope, fundamental types.

   fgSingleton = this;

   /** initialisation of the catalog */
   Catalog::Instance();

}


//-------------------------------------------------------------------------------
void Reflex::Instance::Shutdown() {
//-------------------------------------------------------------------------------
// Function to be called at tear down of Reflex, removes all memory allocations.

   MemberTemplateName::CleanUp();
   TypeTemplateName::CleanUp();

   Catalog::Instance().Unload();
}


//-------------------------------------------------------------------------------

Reflex::Instance::~Instance() {
//-------------------------------------------------------------------------------
// Destructor.  This will shutdown Reflex only if this instance is the 'main'
// instance.

   if (fgSingleton == this)
      Shutdown();
}


//-------------------------------------------------------------------------------
const Reflex::StdString_Cont_Type_t & Reflex::Dummy::StdStringCont() {
//-------------------------------------------------------------------------------
// static wrapper for an empty container of std strings.
   return Get< StdString_Cont_Type_t >();
}


//-------------------------------------------------------------------------------
const Reflex::Type_Cont_Type_t & Reflex::Dummy::TypeCont() {
//-------------------------------------------------------------------------------
// static wrapper for an empty container of Types.
   return Get< Type_Cont_Type_t >();
}


//-------------------------------------------------------------------------------
const Reflex::Base_Cont_Type_t & Reflex::Dummy::BaseCont() {
//-------------------------------------------------------------------------------
// static wrapper for an empty container of Bases.
   return Get< Base_Cont_Type_t >();
}


//-------------------------------------------------------------------------------
const Reflex::Scope_Cont_Type_t & Reflex::Dummy::ScopeCont() {
//-------------------------------------------------------------------------------
// static wrapper for an empty container of Scopes.
   return Get< Scope_Cont_Type_t >();
}


//-------------------------------------------------------------------------------
const Reflex::Object_Cont_Type_t & Reflex::Dummy::ObjectCont() {
//-------------------------------------------------------------------------------
// static wrapper for an empty container of Objects.
   return Get< Object_Cont_Type_t >();
}


//-------------------------------------------------------------------------------
const Reflex::Member_Cont_Type_t & Reflex::Dummy::MemberCont() {
//-------------------------------------------------------------------------------
// static wrapper for an empty container of Members.
   return Get< Member_Cont_Type_t >();
}


//-------------------------------------------------------------------------------
const Reflex::TypeTemplate_Cont_Type_t & Reflex::Dummy::TypeTemplateCont() {
//-------------------------------------------------------------------------------
// static wrapper for an empty container of TypeTemplates.
   return Get< TypeTemplate_Cont_Type_t >();
}


//-------------------------------------------------------------------------------
const Reflex::MemberTemplate_Cont_Type_t & Reflex::Dummy::MemberTemplateCont() {
//-------------------------------------------------------------------------------
// static wrapper for an empty container of MemberTemplates.
   return Get< MemberTemplate_Cont_Type_t >();
}


//-------------------------------------------------------------------------------
Reflex::Any & Reflex::Dummy::Any() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty Any object 
   static Reflex::Any i;
   if ( i ) i.Clear();
   return i;
}


//-------------------------------------------------------------------------------
const Reflex::Object & Reflex::Dummy::Object() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty Object 
   return Get< Reflex::Object >();
}


//-------------------------------------------------------------------------------
const Reflex::Type & Reflex::Dummy::Type() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty Type 
   return Get< Reflex::Type >();
}


//-------------------------------------------------------------------------------
const Reflex::TypeTemplate & Reflex::Dummy::TypeTemplate() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty TypeTemplate 
   return Get< Reflex::TypeTemplate >();
}


//-------------------------------------------------------------------------------
const Reflex::Base & Reflex::Dummy::Base() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty Base 
   return Get< Reflex::Base >();
}


//-------------------------------------------------------------------------------
const Reflex::PropertyList & Reflex::Dummy::PropertyList() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty PropertyList 
   return Get< Reflex::PropertyList >();
}


//-------------------------------------------------------------------------------
const Reflex::Member & Reflex::Dummy::Member() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty Member 
   return Get< Reflex::Member >();
}


//-------------------------------------------------------------------------------
const Reflex::MemberTemplate & Reflex::Dummy::MemberTemplate() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty MemberTemplate 
   return Get< Reflex::MemberTemplate >();
}


//-------------------------------------------------------------------------------
const Reflex::Scope & Reflex::Dummy::Scope() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty Scope 
   return Get< Reflex::Scope >();
}


//-------------------------------------------------------------------------------
const std::string & Reflex::Argv0() {
//-------------------------------------------------------------------------------
// Return the name of the package.
   static std::string str = "REFLEX";
   return str;
}



