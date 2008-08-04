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

#include "Reflex/MemberTemplate.h"

#include "MemberTemplateName.h"
#include "MemberTemplateImpl.h"
#include "OwnedMember.h"


//-------------------------------------------------------------------------------
Reflex::MemberTemplate::operator bool () const {
//-------------------------------------------------------------------------------
   if (this->fMemberTemplateName && this->fMemberTemplateName->fMemberTemplateImpl) return true;
   return false;
}


//-------------------------------------------------------------------------------
void
Reflex::MemberTemplate::AddTemplateInstance(const Member & templateInstance) const {
//-------------------------------------------------------------------------------
// Add member templateInstance to this template family.
   if (* this) fMemberTemplateName->fMemberTemplateImpl->AddTemplateInstance(templateInstance);
}


//-------------------------------------------------------------------------------
Reflex::MemberTemplate
Reflex::MemberTemplate::ByName(const std::string & name, size_t nTemplateParams) {
//-------------------------------------------------------------------------------
   // Return a member template by name.
   return Internal::MemberTemplateName::ByName(name, nTemplateParams);
}


//-------------------------------------------------------------------------------
const std::string&
Reflex::MemberTemplate::Name(std::string& buf, unsigned int mod) const {
//-------------------------------------------------------------------------------
   // Return the name of the member template.
   if (fMemberTemplateName) return fMemberTemplateName->Name(buf, mod);
   return buf;
}

//-------------------------------------------------------------------------------
const Reflex::Container<Reflex::MemberTemplate>
Reflex::MemberTemplate::MemberTemplates() {
//-------------------------------------------------------------------------------
   Internal::MemberTemplateName::MemberTemplates();
   return Dummy::EmptyContainer();
}


//-------------------------------------------------------------------------------
const Reflex::Container<Reflex::Member>
Reflex::MemberTemplate::TemplateInstances() const {
//-------------------------------------------------------------------------------
// TemplateInstances returns the collection of known template instances
// @return collection of the instances
   if (*this) return fMemberTemplateName->fMemberTemplateImpl->TemplateInstances();
   return Dummy::EmptyContainer();
}


//-------------------------------------------------------------------------------
const Reflex::OrderedContainer<std::string>
Reflex::MemberTemplate::TemplateParameterNames() const {
//-------------------------------------------------------------------------------
// TemplateParameterNames returns a collection of the template parameter names
// @return collection of names of the template parameters
   if (*this) return fMemberTemplateName->fMemberTemplateImpl->TemplateParameterNames();
   return Dummy::EmptyContainer();
}


const Reflex::OrderedContainer<std::string>
Reflex::MemberTemplate::TemplateParameterDefaults() const {
//-------------------------------------------------------------------------------
// TemplateParameterDefaults returns a collection of template parameters' default values as string
// @return default values of template parameters
   if (*this) return fMemberTemplateName->fMemberTemplateImpl->TemplateParameterDefaults();
   return Dummy::EmptyContainer();
}

