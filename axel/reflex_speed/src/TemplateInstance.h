// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_TemplateInstance
#define Reflex_TemplateInstance

// Include files
#include "Reflex/Kernel.h"
#include "Reflex/Type.h"
#include "Reflex/TemplateArgument.h"

#include "ContainerSTLAdaptor.h"

namespace Reflex {
   namespace Internal {

   // forward declarations

   /**
   * @class TemplateInstance TemplateInstance.h Reflex/TemplateInstance.h
   * @author Stefan Roiser
   * @date   2004-01-28
   * @ingroup Ref
   */
   class TemplateInstance {

   public:

      /** default constructor */
      TemplateInstance(): fTemplateArgumentsAdaptor(fTemplateArguments) {}


      /** constructor */
      TemplateInstance(const std::string & templateArguments);


      /** destructor */
      virtual ~TemplateInstance() {}


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


   protected:

      /** 
      * vector of template arguments 
      * @link aggregation
      * @label template arguments
      * @supplierCardinality 1
      * @clientCardinality 1..*
      */
      std::vector < TemplateArgument > fTemplateArguments;

      /** adaptor for IContainerImpl **/
      const ContainerSTLAdaptor< std::vector < TemplateArgument > > fTemplateArgumentsAdaptor;

   }; // class TemplateInstance

} // namespace Internal
} // namespace Reflex


//-------------------------------------------------------------------------------
inline const Reflex::Internal::IContainerImpl&
Reflex::Internal::TemplateInstance::TemplateArguments() const {
//-------------------------------------------------------------------------------
   return fTemplateArgumentsAdaptor;
}

#endif // Reflex_TemplateInstance
