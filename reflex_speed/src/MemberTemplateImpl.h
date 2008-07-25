// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_MemberTemplateImpl
#define Reflex_MemberTemplateImpl  

// Include files
#include "Reflex/Kernel.h"
#include "Reflex/Scope.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif

namespace Reflex {

   // forward declarations
   class Member;
   class Scope;
   class MemberTemplate;

namespace Internal {

   // forward declarations
   class FunctionMemberTemplateInstance;
   class MemberTemplateName;

   /** 
   * @class MemberTemplateImpl MemberTemplateImpl.h Reflex/MemberTemplateImpl.h
   * @author Stefan Roiser
   * @date 2005-02-03
   * @ingroup Ref
   */
   class RFLX_API MemberTemplateImpl {

   public:

      /** default constructor */
      MemberTemplateImpl( const char * templateName,
         const Scope & scope,
         const std::vector < std::string > & parameterNames, 
         const std::vector < std::string > & parameterDefaults = std::vector<std::string>());


      /** destructor */
      virtual ~MemberTemplateImpl();


      /** 
      * operator == will return true if two At templates are the same
      * @return true if At templates match
      */
      bool operator == ( const MemberTemplateImpl & rh ) const;


      /**
      * TemplateInstances returns the collection of known template instances
      * @return collection of the instances
      */
      const Container<Member> TemplateInstances() const;


      /**
      * TemplateParameterNames returns a collection of the template parameter names
      * @return collection of names of the template parameters
      */
      const OrderedContainer<std::string> TemplateParameterNames() const;


      /**
      * TemplateParameterDefaults returns a collection of template parameters' default values as string
      * @return default values of template parameters
      */
      const OrderedContainer<std::string> TemplateParameterDefaults() const;


      /**
      * Return the member template API class corresponding to this member template impl
      * @return corresponding member template
      */
      MemberTemplate ThisMemberTemplate() const;

   public:

      /** 
      * AddTemplateInstance adds one TemplateInstanceAt of the template to the local container
      * @param templateInstance the template TemplateInstanceAt
      */
      void AddTemplateInstance( const Member & templateInstance ) const;

   private:

      /**
      * declaring scope of this member template
      * @link aggregation
      * @label member template scope
      * @clientCardinality 1
      * @supplierCardinality 1
      */
      Scope fScope;


      /** 
      * the class template instances
      * @link aggregation
      * @supplierCardinality 1..*
      * @clientCardinality 1
      * @label template instances
      */
      mutable
         std::vector < Member > fTemplateInstances;


      /**
      * container of function parameter template names
      */
      mutable
         std::vector < std::string > fParameterNames;


      /**
      * function  parameter template default values
      */
      mutable
         std::vector < std::string > fParameterDefaults;


      /**
      * number of required template parameters
      */
      size_t fReqParameters;


      /**
      * pointer back to the member template name
      * @link aggregation
      * @label member template name
      * @supplierCardinality 1
      * @clientCardinality 1
      */
      MemberTemplateName * fMemberTemplateName;


   }; // class MemberTemplateImpl

} // namespace Internal
} // namespace Reflex


#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif // Reflex_MemberTemplateImpl
