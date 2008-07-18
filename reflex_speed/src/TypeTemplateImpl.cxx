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

#include "Reflex/internal/TypeTemplateImpl.h"

#include "Reflex/Type.h"
#include "Reflex/TypeTemplate.h"
#include "Reflex/internal/OwnedMember.h"


//-------------------------------------------------------------------------------
Reflex::Internal::TypeTemplateImpl::TypeTemplateImpl( const char * templateName,
                                                      const Scope & scop,
                                                      const std::vector < std::string >& parameterNames)
//------------------------------------------------------------------------------- 
   : fScope( scop ),
     fParameterNames( parameterNames ),
     fParameterDefaults( parameterDefaults ),
     fReqParameters( parameterNames.size() - parameterDefaults.size() ) {
   // Construct the type template family info.

   TypeTemplate tt = TypeTemplate::ByName( templateName, parameterNames.size() );
   if ( tt.Id() == 0 ) {
      fTypeTemplateName = new TypeTemplateName( templateName, this );
   }
   else {
      fTypeTemplateName = (TypeTemplateName*)tt.Id();
      if ( fTypeTemplateName->fTypeTemplateImpl ) delete fTypeTemplateName->fTypeTemplateImpl;
      fTypeTemplateName->fTypeTemplateImpl = this;
   }
}


//-------------------------------------------------------------------------------
Reflex::Internal::TypeTemplateImpl::~TypeTemplateImpl() {
//-------------------------------------------------------------------------------
// Destructor.
   for ( Type_Iterator ti = TemplateInstance_Begin(); ti != TemplateInstance_End(); ++ti ) {
      ti->Unload();
   }
   if ( fTypeTemplateName->fTypeTemplateImpl == this ) fTypeTemplateName->fTypeTemplateImpl = 0;
}


//-------------------------------------------------------------------------------
bool
Reflex::Internal::TypeTemplateImpl::operator == ( const TypeTemplateImpl & tt ) const {
//-------------------------------------------------------------------------------
   // Equal operator.
   return ( ( fTypeTemplateName->fName == tt.fTypeTemplateName->fName ) && 
            ( fParameterNames.size() == tt.fParameterNames.size() ) );
}


//-------------------------------------------------------------------------------
Reflex::TypeTemplate
Reflex::Internal::TypeTemplateImpl::ThisTypeTemplate() const {
//-------------------------------------------------------------------------------
   // Return a ref to this type template.
   return fTypeTemplateName->ThisTypeTemplate();
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::TypeTemplateImpl::AddTemplateInstance( const Type & templateInstance ) const {
//-------------------------------------------------------------------------------
// Add template instance to this family.
   fTemplateInstances.Insert( templateInstance );
}
