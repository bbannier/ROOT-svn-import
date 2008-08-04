// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_MemberTemplate
#define Reflex_MemberTemplate

// Include files
#include "Reflex/Kernel.h"


namespace Reflex {

   // forward declarations
   class Member;

   namespace Internal {
      class MemberTemplateName;
      class OwnedMemberTemplate;
   }

   /** 
   * @class MemberTemplate MemberTemplate.h Reflex/MemberTemplate.h
   * @author Stefan Roiser
   * @date 2005-02-03
   * @ingroup Ref
   */
   class RFLX_API MemberTemplate {

      friend class Internal::OwnedMemberTemplate;

   public:

      /** default constructor */
      MemberTemplate(const Internal::MemberTemplateName * memberTemplateName = 0);


      /** copy constructor */
      MemberTemplate(const MemberTemplate & rh);


      /** destructor */
      ~MemberTemplate();


      /** 
      * operator bool will return true if the member template is resolved
      * @return true if member template is resolved
      */
      operator bool () const;


      /** 
      * operator == will return true if two member templates are the same
      * @return true if member templates match
      */
      bool operator == (const MemberTemplate & rh) const;


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
      * Id will return a memory address which is a unique id for this member template
      * @return unique id of this member template
      */
      void * Id() const;


      /**
      * MemberTemplates returns the collection of known member templates
      * @return collection of member templates
      */
      static const Container<MemberTemplate> MemberTemplates();


      /**
      * Name will return the name of the template family and a list of
      * all currently available instantiations
      * @return template family name with all instantiantion
      */
      const std::string& Name(std::string& buf, unsigned int mod = 0) const;


      /**
      * Name will return the name of the template family and a list of
      * all currently available instantiations
      * @return template family name with all instantiantion
      */
      std::string Name(unsigned int mod = 0) const;


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


   public:

      /** 
      * AddTemplateInstance adds one TemplateInstanceAt of the template to the local container
      * @param templateInstance the template TemplateInstanceAt
      */
      void AddTemplateInstance(const Member & templateInstance) const;

   private:

      /** 
      * pointer to the member template implementation
      * @label member template name
      * @link aggregation
      * @clientCardinality 1
      * @supplierCardinality 1
      */
      const Internal::MemberTemplateName * fMemberTemplateName;

   }; // class MemberTemplate

} // namespace Reflex


//-------------------------------------------------------------------------------
inline
Reflex::MemberTemplate::MemberTemplate(const Internal::MemberTemplateName * memberTemplateName)
//------------------------------------------------------------------------------- 
   : fMemberTemplateName(memberTemplateName) {}


//-------------------------------------------------------------------------------
inline
Reflex::MemberTemplate::MemberTemplate(const MemberTemplate & rh) 
//-------------------------------------------------------------------------------
   : fMemberTemplateName(rh.fMemberTemplateName) {}


//-------------------------------------------------------------------------------
inline
Reflex::MemberTemplate::~MemberTemplate() {}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
inline bool
Reflex::MemberTemplate::operator == (const MemberTemplate & rh) const {
//-------------------------------------------------------------------------------
   return (fMemberTemplateName == rh.fMemberTemplateName);
}


//-------------------------------------------------------------------------------
inline void *
Reflex::MemberTemplate::Id() const {
//-------------------------------------------------------------------------------
   return (void*)fMemberTemplateName;
}


//-------------------------------------------------------------------------------
inline std::string
Reflex::MemberTemplate::Name(unsigned int mod) const {
//-------------------------------------------------------------------------------
   // Return the name of the member template.
   std::string buf;
   return Name(buf, mod);
}

#endif // Reflex_MemberTemplate
