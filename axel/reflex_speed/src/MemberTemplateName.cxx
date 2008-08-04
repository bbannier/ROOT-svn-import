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

#include "MemberTemplateName.h"

#include "Reflex/MemberTemplate.h"
#include "Reflex/Member.h"
#include "Reflex/Tools.h"
#include "MemberTemplateImpl.h"
#include "ContainerSTLAdaptor.h"

#include "stl_hash.h"
#include <vector>

//-------------------------------------------------------------------------------
typedef __gnu_cxx::hash_multimap < const std::string *, Reflex::MemberTemplate > Name2MemberTemplate_t;
typedef std::vector< Reflex::MemberTemplate > MemberTemplateVec_t;


//-------------------------------------------------------------------------------
static Name2MemberTemplate_t & sMemberTemplates() {
//-------------------------------------------------------------------------------
   // Static wrapper around the member template map.
   static Name2MemberTemplate_t t;
   return t;
}


//-------------------------------------------------------------------------------
static MemberTemplateVec_t & sMemberTemplateVec() {
//-------------------------------------------------------------------------------
   // Static wrapper around the member template vector.
   static MemberTemplateVec_t t;
   return t;
}


//-------------------------------------------------------------------------------
Reflex::Internal::MemberTemplateName::MemberTemplateName(const char * name,
                                                      MemberTemplateImpl * memberTemplateImpl)
//-------------------------------------------------------------------------------
   : fName(name),
     fMemberTemplateImpl(memberTemplateImpl) {
   // Constructor.
   fThisMemberTemplate = new MemberTemplate(this);
   sMemberTemplates().insert(std::make_pair<const std::string * const,MemberTemplate>(&fName, *fThisMemberTemplate));
   sMemberTemplateVec().push_back(* fThisMemberTemplate);
}


//-------------------------------------------------------------------------------
Reflex::Internal::MemberTemplateName::~MemberTemplateName() {
//-------------------------------------------------------------------------------
   // Destructor.
}


//-------------------------------------------------------------------------------
Reflex::MemberTemplate
Reflex::Internal::MemberTemplateName::ByName(const std::string & name,
                                             size_t nTemplateParams) {
//-------------------------------------------------------------------------------
   // Lookup a member template by its name.
   typedef Name2MemberTemplate_t::iterator IT;
   IT lower = sMemberTemplates().find(&name);
   if (lower != sMemberTemplates().end()) {
      if (! nTemplateParams) return lower->second;
      else {
         std::pair<IT,IT> bounds = sMemberTemplates().equal_range(&name);
         for (IT it = bounds.first; it != bounds.second; ++it) {
            if (it->second.TemplateParameterNames().Size() == nTemplateParams) {
               return it->second;
            }
         }
      }
   }
   return Dummy::MemberTemplate();
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::MemberTemplateName::CleanUp() {
//-------------------------------------------------------------------------------
   // Do the final cleanup for the member templates.  
   for (MemberTemplateVec_t::iterator it = sMemberTemplateVec().begin(); it != sMemberTemplateVec().end(); ++it) {
      MemberTemplateName * tn = (MemberTemplateName*)it->Id();
      MemberTemplate * t = tn->fThisMemberTemplate;
      tn->DeleteMemberTemplate();
      delete t;
      delete tn;
   }
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::MemberTemplateName::DeleteMemberTemplate() const {
//-------------------------------------------------------------------------------
   // Remove a member template dictionary information.
   delete fMemberTemplateImpl;
   fMemberTemplateImpl = 0;
}


//-------------------------------------------------------------------------------
const Reflex::Internal::IContainerImpl&
Reflex::Internal::MemberTemplateName::MemberTemplates() {
//-------------------------------------------------------------------------------
   static ContainerSTLAdaptor<MemberTemplateVec_t> sAdaptor(sMemberTemplateVec());
   return sAdaptor;
}


//-------------------------------------------------------------------------------
const std::string&
Reflex::Internal::MemberTemplateName::Name(std::string& buf, unsigned int mod) const { 
//-------------------------------------------------------------------------------
   // Return the name of this member template in buf and as a reference to buf.
   if (mod & kScoped) return (buf += fName);
   else return (buf += Tools::GetBaseName(fName));
}


//-------------------------------------------------------------------------------
Reflex::MemberTemplate
Reflex::Internal::MemberTemplateName::ThisMemberTemplate() const {
//-------------------------------------------------------------------------------
   // Return the member template corresponding to this member template name.
   return * fThisMemberTemplate;
}
