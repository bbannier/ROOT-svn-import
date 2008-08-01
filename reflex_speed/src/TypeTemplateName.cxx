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

#include "TypeTemplateName.h"

#include "Reflex/TypeTemplate.h"
#include "Reflex/Type.h"
#include "Reflex/Tools.h"

#include "TypeTemplateImpl.h"
#include "ContainerSTLAdaptor.h"

#include "stl_hash.h"
#include <vector>

//-------------------------------------------------------------------------------
typedef __gnu_cxx::hash_multimap < const std::string *, Reflex::TypeTemplate > Name2TypeTemplate_t;
typedef std::vector< Reflex::TypeTemplate > TypeTemplateVec_t;


//-------------------------------------------------------------------------------
static Name2TypeTemplate_t & sTypeTemplates() {
//-------------------------------------------------------------------------------
   // Static wrapper around the type template map.
   static Name2TypeTemplate_t t;
   return t;
}

//-------------------------------------------------------------------------------
static TypeTemplateVec_t & sTypeTemplateVec() {
//-------------------------------------------------------------------------------
   // Static wrapper around the type template vector.
   static TypeTemplateVec_t t;
   return t;
}


//-------------------------------------------------------------------------------
static const Reflex::Internal::ContainerSTLAdaptor<TypeTemplateVec_t>& sTypeTemplateVecAdaptor() {
//-------------------------------------------------------------------------------
   static Reflex::Internal::ContainerSTLAdaptor<TypeTemplateVec_t> t(sTypeTemplateVec());
   return t;
}


//-------------------------------------------------------------------------------
Reflex::Internal::TypeTemplateName::TypeTemplateName(const char * name,
                                                      TypeTemplateImpl * typeTemplateImpl)
//-------------------------------------------------------------------------------
   : fName(name),
     fTypeTemplateImpl(typeTemplateImpl) {
   // Constructor.
   fThisTypeTemplate = new TypeTemplate(this);
   sTypeTemplates().insert(std::make_pair<const std::string * const, TypeTemplate>(&fName, *fThisTypeTemplate));
   sTypeTemplateVec().push_back(* fThisTypeTemplate);
}


//-------------------------------------------------------------------------------
Reflex::Internal::TypeTemplateName::~TypeTemplateName() {
//-------------------------------------------------------------------------------
   // Destructor.
}


//-------------------------------------------------------------------------------
Reflex::TypeTemplate
Reflex::Internal::TypeTemplateName::ByName(const std::string & name,
                                                                   size_t nTemplateParams) {
//-------------------------------------------------------------------------------
   // Lookup a type template by its name.
   typedef Name2TypeTemplate_t::iterator IT;
   IT lower = sTypeTemplates().find(&name);
   if (lower != sTypeTemplates().end()) {
      if (! nTemplateParams) return lower->second;
      else {
         std::pair<IT,IT> bounds = sTypeTemplates().equal_range(&name);
         for (IT it = bounds.first; it != bounds.second; ++it) {
            if (it->second.TemplateParameterSize() == nTemplateParams) {
               return it->second;
            }
         }
      }
   }
   return Dummy::TypeTemplate();
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::TypeTemplateName::CleanUp() {
//-------------------------------------------------------------------------------
   // Do the final cleanup for the type templates.  
   for (TypeTemplateVec_t::iterator it = sTypeTemplateVec().begin(); it != sTypeTemplateVec().end(); ++it) {
      TypeTemplateName * tn = (TypeTemplateName*)it->Id();
      TypeTemplate * t = tn->fThisTypeTemplate;
      if (t) t->Unload();
      delete t;
      delete tn;
   }
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::TypeTemplateName::DeleteTypeTemplate() const {
//-------------------------------------------------------------------------------
   // Remove a type template dictionary information.
   delete fTypeTemplateImpl;
   fTypeTemplateImpl = 0;
}


//-------------------------------------------------------------------------------
const std::string&
Reflex::Internal::TypeTemplateName::Name(std::string& buf, unsigned int mod) const {
//-------------------------------------------------------------------------------
   // Return the name of this type template.
   if (mod & kScoped)
      buf += fName;
   else
      buf += Tools::GetBaseName(fName);
   return buf;
}


//-------------------------------------------------------------------------------
Reflex::TypeTemplate
Reflex::Internal::TypeTemplateName::ThisTypeTemplate() const {
//-------------------------------------------------------------------------------
   // Return the type template corresponding to this type template name.
   return * fThisTypeTemplate;
}


//-------------------------------------------------------------------------------
const Reflex::Internal::IContainerImpl&
Reflex::Internal::TypeTemplateName::TypeTemplates() {
//-------------------------------------------------------------------------------
   // Return teh nth type template.
   return sTypeTemplateVecAdaptor();
}
