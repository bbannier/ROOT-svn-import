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
   if ( this->fMemberTemplateName && this->fMemberTemplateName->fMemberTemplateImpl ) return true;
   return false;
}


//-------------------------------------------------------------------------------
Reflex::MemberTemplate
Reflex::MemberTemplate::ByName( const std::string & name,
                                                                   size_t nTemplateParams ) {
//-------------------------------------------------------------------------------
   // Return a member template by name.
   return Internal::MemberTemplateName::ByName( name, nTemplateParams );
}


//-------------------------------------------------------------------------------
std::string
Reflex::MemberTemplate::Name( unsigned int mod ) const {
//-------------------------------------------------------------------------------
   // Return the name of the member template.
   if ( fMemberTemplateName ) return fMemberTemplateName->Name( mod );
   else                       return "";
}


//-------------------------------------------------------------------------------
void
Reflex::MemberTemplate::AddTemplateInstance( const Member & templateInstance ) const {
//-------------------------------------------------------------------------------
// Add member templateInstance to this template family.
   if ( * this ) fMemberTemplateName->fMemberTemplateImpl->AddTemplateInstance( templateInstance );
}

