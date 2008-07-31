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

namespace Reflex {

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
      TemplateInstance();


      /** constructor */
      TemplateInstance(const std::string & templateArguments);


      /** destructor */
      virtual ~TemplateInstance() {}


      /**
      * Name returns the full Name of the templated collection
      * @param  typedefexp expand typedefs or not
      * @return full Name of template collection
      */
      std::string Name(unsigned int mod = kScoped | kQualified) const;


      /**
      * TemplateArguments returns an ordered collection of the template arguments
      * @return reflection information of template arguments
      */
      const OrderedContainer<TemplateArgument>& TemplateArguments() const;


   private:

      /** 
      * vector of template arguments 
      * @link aggregation
      * @label template arguments
      * @supplierCardinality 1
      * @clientCardinality 1..*
      */
      mutable
         std::vector < TemplateArgument > fTemplateArguments;

   }; // class TemplateInstance

} // namespace Reflex


//-------------------------------------------------------------------------------
inline Reflex::TemplateInstance::TemplateInstance() {}
//-------------------------------------------------------------------------------


#endif // Reflex_TemplateInstance
