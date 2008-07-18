// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_TypeTemplateImpl
#define Reflex_TypeTemplateImpl

// Include files
#include "Reflex/Kernel.h"
#include "Reflex/Scope.h"
#include "ContainerImpl.h"
#include "ContainerAdaptorImpl.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif

namespace Reflex {

   // forward declarations
   class Type;
   class TypeTemplate;

namespace Internal {

   // forward declarations
   class TypeTemplateName;
   class ClassTemplateInstance;

   /** 
   * @class TypeTemplateImpl TypeTemplateImpl.h Reflex/TypeTemplateImpl.h
   * @author Stefan Roiser
   * @date 2005-02-03
   * @ingroup Ref
   */
   class RFLX_API TypeTemplateImpl {

   public:

      /** default constructor */
      TypeTemplateImpl( const char * templateName,
         const Scope & scop,
         const std::vector < std::string >& parameterNames, 
         const std::vector < std::string >& parameterDefaults = std::vector< std::string >());


      /** destructor */
      virtual ~TypeTemplateImpl();


      /** 
      * operator == will return true if two At templates are the same
      * @return true if At templates match
      */
      bool operator == ( const TypeTemplateImpl & rh ) const;


      /**
      * TemplateInstances returns collecvtion of the instances of this template
      * @return the collection of the instances
      */
      const Container<Type>& TemplateInstances() const;


      /**
      * TemplateParameterDefaults returns the collection of template default values as string
      * @return collection of default value of template parameters
      */
      const OrderedContainer<std::string>& TemplateParameterDefaults() const;


      /**
      * TemplateParameterNames returns the names of the template parameters
      * @return Collection of names of template template parameters
      */
      const OrderedContainer<std::string>& TemplateParameterNames() const;


      /**
      * Return the member template API class corresponding to this member template impl
      * @return corresponding member template
      */
      TypeTemplate ThisTypeTemplate() const;

   public:

      /** 
      * AddTemplateInstance adds one TemplateInstanceAt of the template to the local container
      * @param templateInstance the template TemplateInstanceAt
      */
      void AddTemplateInstance( const Type & templateInstance ) const;

   private:

      /**
      * pointer back to the corresponding scope
      * @label type template scope
      * @clientCardinality 0..*
      * @supplierCardinality 1
      */
      Scope fScope;


      /** 
      * pointer to the class template instances
      * @supplierCardinality 1..*
      * @clientCardinality 0..1
      * @label template instances
      */
      mutable
         ContainerImpl < std::string, Type > fTemplateInstances;


      /**
      * container of template parameter names
      */
      mutable
         std::vector < std::string > fParameterNames;


      /**
      * template parameter default values
      */
      mutable
         std::vector < std::string > fParameterDefaults;


      /**
      * number of required template parameters
      */
      size_t fReqParameters;


      /**
      * pointer back to the template name
      * @label type template name
      * @link aggregation
      * @clientCardinality 1
      * @supplierCardinality 1
      */
      TypeTemplateName * fTypeTemplateName;

   }; // class TypeTemplateImpl

} // namespace Internal
} // namespace Reflex


#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif // Reflex_TypeTemplateImpl
