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

#include "Reflex/TypeTemplate.h"
#include "Reflex/Type.h"
#include "OwnedMember.h"
#include "TypeTemplateName.h"                                                             
#include "TypeTemplateImpl.h"                                                             

//-------------------------------------------------------------------------------
Reflex::TypeTemplate::operator bool () const {
//-------------------------------------------------------------------------------
   if (this->fTypeTemplateName && this->fTypeTemplateName->fTypeTemplateImpl) return true;
   return false;
}


//-------------------------------------------------------------------------------
const std::string&
Reflex::TypeTemplate::Name(std::string& buf, unsigned int mod) const {
//-------------------------------------------------------------------------------
   // Return the name of this type template.
   if (fTypeTemplateName) return fTypeTemplateName->Name(buf, mod);
   else                     return buf;
}


//-------------------------------------------------------------------------------
Reflex::TypeTemplate
Reflex::TypeTemplate::ByName(const std::string & name,
                                                               size_t nTemplateParams) {
//-------------------------------------------------------------------------------
   // Lookup a type template by name.
   return TypeTemplateName::ByName(name, nTemplateParams);
}


//-------------------------------------------------------------------------------
void
Reflex::TypeTemplate::AddTemplateInstance(const Type & templateInstance) const {
//-------------------------------------------------------------------------------
   // Add template instance to this template family.
   if (* this) fTypeTemplateName->fTypeTemplateImpl->AddTemplateInstance(templateInstance);
}


//-------------------------------------------------------------------------------
void
Reflex::TypeTemplate::Unload() const {
//-------------------------------------------------------------------------------
//  Unload a type template, i.e. delete the TypeTemplateName's TypeTemaplteImpl object.
   if (* this) delete fTypeTemplateName->fTypeTemplateImpl;
   fTypeTemplateName->fTypeTemplateImpl = 0;
}
