// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_ClassTemplateInstance
#define Reflex_ClassTemplateInstance

// Include files
#include "Class.h"
#include "TemplateInstance.h"
#include "Reflex/TypeTemplate.h"
#include <string>


namespace Reflex {
namespace Internal {


   /**
   * @class ClassTemplateInstance ClassTemplateInstance.h Reflex/ClassTemplateInstance.h
   * @author Stefan Roiser
   * @date 13/1/2004
   * @ingroup Ref
   */
   class ClassTemplateInstance : public Class, public TemplateInstance {

   public:

      /** default constructor */
      ClassTemplateInstance(const char * typ, 
         size_t size, 
         const std::type_info & ti,
         const Catalog& catalog,
         unsigned int modifiers);


      /** destructor */
      virtual ~ClassTemplateInstance();


      /**
      * Name returns the name of the type
      * @param buf preallocated buffer to work on when calculating the name
      * @return name of type
      */
      virtual const std::string& Name(std::string& buf, unsigned int mod = kScoped | kQualified) const;


      /**
      * TemplateArguments returns an ordered collection of the template arguments
      * @return reflection information of template arguments
      */
      virtual const IContainerImpl& TemplateArguments() const;


      /**
      * TemplateFamily returns the corresponding TypeTemplate if any
      * @return corresponding TypeTemplate
      */
      TypeTemplate TemplateFamily() const;

   private:

      /** 
      * The template type (family)
      * @label template type
      * @link aggregation
      * @clientCardinality 1
      * @supplierCardinality 1
      */
      TypeTemplate fTemplateFamily;      

   }; // class ClassTemplateInstance
} // namespace Internal
} // namespace Reflex


//-------------------------------------------------------------------------------
inline Reflex::Internal::ClassTemplateInstance::~ClassTemplateInstance() {}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
inline const Reflex::Internal::IContainerImpl&
Reflex::Internal::ClassTemplateInstance::TemplateArguments() const {
//-------------------------------------------------------------------------------
   return TemplateInstance::TemplateArguments();
}


//-------------------------------------------------------------------------------
inline Reflex::TypeTemplate
Reflex::Internal::ClassTemplateInstance::TemplateFamily() const {
//-------------------------------------------------------------------------------
   return fTemplateFamily;
}

//-------------------------------------------------------------------------------
inline const std::string&
Reflex::Internal::ClassTemplateInstance::Name(std::string& buf, unsigned int mod) const {
//-------------------------------------------------------------------------------
   return Class::Name(buf, mod);
}

#endif // Reflex_ClassTemplateInstance
