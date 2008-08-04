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

#include "MemberTemplateImpl.h"

#include "Reflex/MemberTemplate.h"

#include "MemberTemplateName.h"
#include "OwnedMember.h"


//-------------------------------------------------------------------------------
Reflex::Internal::MemberTemplateImpl::MemberTemplateImpl(const char * templateName,
                                                      const Scope & scope,
                                                      const std::vector < std::string > & parameterNames,
                                                      const std::vector < std::string > & parameterDefaults)
//------------------------------------------------------------------------------- 
   : fScope(scope),
     fParameterNames(parameterNames),
     fParameterNamesAdaptor(fParameterNames),
     fParameterDefaults(parameterDefaults),
     fParameterDefaultsAdaptor(fParameterDefaults),
     fReqParameters(parameterNames.size() - parameterDefaults.size()) {
// Construct dictionary info for this template member function.
   MemberTemplate mt = MemberTemplate::ByName(templateName, parameterNames.size());
   if (mt.Id() == 0) {
      fMemberTemplateName = new MemberTemplateName(templateName, this);
   }
   else {
      fMemberTemplateName = (MemberTemplateName*)mt.Id();
      if (fMemberTemplateName->fMemberTemplateImpl) delete fMemberTemplateName->fMemberTemplateImpl;
      fMemberTemplateName->fMemberTemplateImpl = this;
   }
}


//-------------------------------------------------------------------------------
Reflex::Internal::MemberTemplateImpl::~MemberTemplateImpl() {
//-------------------------------------------------------------------------------
// Destructor.
   if (fMemberTemplateName->fMemberTemplateImpl == this) fMemberTemplateName->fMemberTemplateImpl = 0;
}


//-------------------------------------------------------------------------------
bool
Reflex::Internal::MemberTemplateImpl::operator == (const MemberTemplateImpl & mt) const {
//-------------------------------------------------------------------------------
// Equal operator.
   return ((fMemberTemplateName->fName == mt.fMemberTemplateName->fName) && 
            (fParameterNames.size() == mt.fParameterNames.size()));
}


//-------------------------------------------------------------------------------
Reflex::MemberTemplate
Reflex::Internal::MemberTemplateImpl::ThisMemberTemplate() const {
//-------------------------------------------------------------------------------
   // Return a ref to this member template.
   return fMemberTemplateName->ThisMemberTemplate();
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::MemberTemplateImpl::AddTemplateInstance(const Member & templateInstance) const {
//-------------------------------------------------------------------------------
// Add template instance to this family.
   fTemplateInstances.Insert(templateInstance);
}

