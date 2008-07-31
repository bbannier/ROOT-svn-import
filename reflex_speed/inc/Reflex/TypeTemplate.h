// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_TypeTemplate
#define Reflex_TypeTemplate

// Include files
#include "Reflex/Kernel.h"

namespace Reflex {

   // forward declarations
   class Type;

   namespace Internal {
      class TypeTemplateName;
   }

   /** 
   * @class TypeTemplate TypeTemplate.h Reflex/TypeTemplate.h
   * @author Stefan Roiser
   * @date 2005-02-03
   * @ingroup Ref
   */
   class RFLX_API TypeTemplate {

   public:

      /** default constructor */
      TypeTemplate(const Internal::TypeTemplateName * typeTemplateName = 0);


      /** copy constructor */
      TypeTemplate(const TypeTemplate & rh);


      /** destructor */
      ~TypeTemplate();


      /** 
      * operator bool will return true if the type template is resolved
      * @return true if type template is resolved
      */
      operator bool () const;


      /** 
      * operator == will return true if two type templates are the same
      * @return true if type templates match
      */
      bool operator == (const TypeTemplate & rh) const;


      /**
      * ByName will return a type template corresponding to the argument name
      * @param type template name to lookup
      * @param nTemplateParams looks up the template family with this number of template parameters
      *        if it is set to 0, the first occurence of the template family name will be returned
      * @return corresponding type template to name
      */
      static TypeTemplate ByName(const std::string & name,
         size_t nTemplateParams = 0);


      /**
      * Id will return a memory address which is a unique id for this type template
      * @return unique id of this type template
      */
      void * Id() const;


      /**
      * Name will return the Name of the template family and a list of
      * all currently available instantiations
      * @return template family Name with all instantiantion
      */
      std::string Name(unsigned int mod = kScoped | kQualified) const;


      /**
      * Name will return the Name of the template family and a list of
      * all currently available instantiations
      * @return template family Name with all instantiantion
      */
      const std::string& Name(std::string& buf, unsigned int mod = kScoped | kQualified) const;


      /**
      * TemplateInstances returns the collection of instances of this template.
      * @return the collection of the instances
      */
      const Container<Type>& TemplateInstances() const;


      /**
      * TemplateParameterSize will return the number of template parameters
      * @return number of template parameters
      */
      size_t TemplateParameterSize() const;


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
      * TypeTemplates returns the collection of all known type templates
      * @return collection of type templates
      */
      static const Container<TypeTemplate>& TypeTemplates();


      /**
      * Unload will unload the dictionary information of a type template
      */
      void Unload() const;

   public:

      /** 
      * AddTemplateInstance adds one TemplateInstanceAt of the template to the local container
      * @param templateInstance the template TemplateInstanceAt
      */
      void AddTemplateInstance(const Type & templateInstance) const;

   private:

      /** 
      * pointer to the type template implementation
      * @link aggregation
      * @supplierCardinality 1
      * @clientCardinality 1
      * @label type template impl
      */
      const Internal::TypeTemplateName * fTypeTemplateName;

   }; // class TypeTemplate

} // namespace Reflex


//-------------------------------------------------------------------------------
inline
Reflex::TypeTemplate::TypeTemplate(const Internal::TypeTemplateName * typeTemplateName)
//------------------------------------------------------------------------------- 
   : fTypeTemplateName(typeTemplateName) {}


//-------------------------------------------------------------------------------
inline
Reflex::TypeTemplate::TypeTemplate(const TypeTemplate & rh) 
//-------------------------------------------------------------------------------
   : fTypeTemplateName(rh.fTypeTemplateName) {}


//-------------------------------------------------------------------------------
inline
Reflex::TypeTemplate::~TypeTemplate() {}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
inline bool
Reflex::TypeTemplate::operator == (const TypeTemplate & rh) const {
//-------------------------------------------------------------------------------
   return (fTypeTemplateName == rh.fTypeTemplateName);
}


//-------------------------------------------------------------------------------
inline void *
Reflex::TypeTemplate::Id() const {
//-------------------------------------------------------------------------------
  return (void*)fTypeTemplateName;
}


//-------------------------------------------------------------------------------
inline std::string
Reflex::TypeTemplate::Name(unsigned int mod) const {
//-------------------------------------------------------------------------------
   // Return the name of this type template.
   std::string buf;
   return Name(buf, mod);
}


//-------------------------------------------------------------------------------
inline size_t
Reflex::TypeTemplate::TemplateParameterSize() const {
//-------------------------------------------------------------------------------
   // Return the name of this type template.
   return TemplateParameterNames().Size();
}


#endif // Reflex_TypeTemplate
