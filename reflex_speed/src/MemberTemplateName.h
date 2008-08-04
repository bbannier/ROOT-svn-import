// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_MemberTemplateName
#define Reflex_MemberTemplateName

// Include files
#include "Reflex/Kernel.h"


namespace Reflex {

   // forward declarations
   class MemberTemplate;

namespace Internal {

   // forward declarations
   class MemberTemplateImpl;

   /** 
   * @class MemberTemplateName MemberTemplateName.h Reflex/internal/MemberTemplateName.h
   * @author Stefan Roiser
   * @date 8/8/2006
   * @ingroup Ref
   */
   class RFLX_API MemberTemplateName {

      friend class Reflex::MemberTemplate;
      friend class MemberTemplateImpl;

   public:

      /** constructor */
      MemberTemplateName(const char * name,
         MemberTemplateImpl * memberTemplImpl);


      /**
      * ByName will return a member template corresponding to the argument name
      * @param member template name to lookup
      * @param nTemplateParams looks up the template family with this number of template parameters
      *        if it is set to 0, the first occurence of the template family name will be returned
      * @return corresponding member template to name
      */
      static MemberTemplate ByName(const std::string & name,
         size_t nTemplateParams = 0);


      /**
      * MemberTemplates returns the collection of known member templates
      * @return collection of member templates
      */
      static const IContainerImpl& MemberTemplates();


      /**
      * CleanUp is called at the end of the process
      */
      static void CleanUp();


      /*
      * DeleteMemberTemplate will remove the dictionary information
      * of one member template from memory
      */
      void DeleteMemberTemplate() const;


      /**
      * Name will return the name of the member template
      * @return name of member template
      */
      const std::string& Name(std::string& buf, unsigned int mod) const;


      /**
      * ThisMemberTemplate will return the MemberTemplate API class of this member template
      * @return API member template class
      */
      MemberTemplate ThisMemberTemplate() const;


   private:

      /** destructor */
      ~MemberTemplateName();

   private:


      /**
      * The name of the member template
      */
      std::string fName;


      /**
      * Pointer to the implementation of the member template
      * @link aggregation
      * @supplierCardinality 1
      * @clientCardinality 0..1
      * @label member template impl
      */
      mutable 
         MemberTemplateImpl * fMemberTemplateImpl;


      /**
      * pointer back to the member temlate
      * @label this member template
      * @link aggregation
      * @clientCardinality 1
      * @supplierCardinality 1
      */
      MemberTemplate * fThisMemberTemplate;

   }; // class MemberTemplate
} // namespace Internal
} // namespace Reflex


#endif // Reflex_MemberTemplateName
