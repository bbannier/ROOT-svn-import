// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_FunctionMemberTemplateInstance
#define Reflex_FunctionMemberTemplateInstance

// Include files
#include "FunctionMember.h"
#include "TemplateInstance.h"

namespace Reflex {

   // forward declarations
   class Type;

namespace Internal {

   /**
   * @class FunctionMemberTemplateInstance FunctionMemberTemplateInstance.h Reflex/FunctionMemberTemplateInstance.h
   * @author Stefan Roiser
   * @date 13/1/2004
   * @ingroup Ref
   */
   class FunctionMemberTemplateInstance : public FunctionMember, public TemplateInstance {

   public:

      /** default constructor */
      FunctionMemberTemplateInstance(const char * nam,
         const Type & typ,
         StubFunction stubFP,
         void * stubCtx = 0,
         const char * params = 0, 
         unsigned int modifiers = 0,
         const Scope & scop = Scope());


      /** destructor */
      virtual ~FunctionMemberTemplateInstance();


      /**
      * Name returns the fully qualified Name of the
      * templated function
      * @param  typedefexp expand typedefs or not
      * @return fully qualified Name of templated function
      */
      const std::string& Name(std::string& buf, unsigned int mod = 0) const;


      /**
      * TemplateArguments returns an ordered collection of the template arguments
      * @return reflection information of template arguments
      */
      const IContainerImpl& TemplateArguments() const;


      /**
      * TemplateFamily returns the corresponding MemberTemplate if any
      * @return corresponding MemberTemplate
      */
      MemberTemplate TemplateFamily() const;

   private:

      /** 
      * The template type (family)
      * @label template family
      * @link aggregation
      * @clientCardinality 1
      * @supplierCardinality 1
      */
      MemberTemplate fTemplateFamily;      

   }; // class FunctionMemberTemplateInstance
} // namespace Internal
} // namespace Reflex

//-------------------------------------------------------------------------------
inline
Reflex::Internal::FunctionMemberTemplateInstance::~FunctionMemberTemplateInstance() {}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
inline Reflex::MemberTemplate
Reflex::Internal::FunctionMemberTemplateInstance::TemplateFamily() const {
//-------------------------------------------------------------------------------
   return fTemplateFamily;
}


//-------------------------------------------------------------------------------
inline const Reflex::Internal::IContainerImpl&
Reflex::Internal::FunctionMemberTemplateInstance::TemplateArguments() const {
//-------------------------------------------------------------------------------
   return TemplateInstance::TemplateArguments();
}


#endif // Reflex_FunctionMemberTemplateInstance
