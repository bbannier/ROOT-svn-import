// @(#)root/reflex:$Id: Constructable.h 25003 2008-07-31 16:11:41Z axel $
// Author: Axel Naumann 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Constructable
#define Reflex_Constructable

// Include files
#include "ScopedType.h"
#include "ContainerSTLAdaptor.h"
#include <vector>

namespace Reflex {

   // forward declarations
   class Base;
   class Member;
   class MemberTemplate;
   class TypeTemplate;
   class DictionaryGenerator;

namespace Internal {

   /**
    * @class Constructable Constructable.h Reflex/Constructable.h
    * @author Axel Naumann
    * @date 2008-08-01
    * @ingroup Ref
    */
   class Constructable : public ScopedType
   {

   public:

      /** constructor */
      Constructable(const char* name,
         unsigned int modifiers,
         size_t size,
         ETYPE type,
         const std::type_info& ti);

      /** destructor */
      virtual ~Constructable() {}

      /**
       * Construct will call the constructor of a given At and Allocate the
       * memory for it
       * @param  signature of the constructor
       * @param  values for parameters of the constructor
       * @param  mem place in memory for implicit construction
       * @return pointer to new instance
       */
      virtual Object Construct(const Type& signature = Type(), const std::vector<void*>& values = std::vector<void*>(), void* mem = 0) const;

      /**
       * Destruct will call the destructor of a At and remove its memory
       * allocation if desired
       * @param  instance of the At in memory
       * @param  dealloc for also deallacoting the memory
       */
      virtual void Destruct(void* instance, bool dealloc = true) const;

      /**
      * AddMember will add the information about a function MemberAt
      */
      virtual void AddMember(const Member & dm) const;
      virtual void AddMember(const char * name,
         const Type & type,
         StubFunction stubFP,
         void * stubCtx = 0,
         const char * params = 0,
         unsigned int modifiers = 0) const;


      /**
      * RemoveDataMember will remove the information about a data MemberAt
      * @param dm pointer to data MemberAt
      */
      virtual void RemoveMember(const Member & dm) const;


   private:

      /**
       * short cut to constructors
       * @label constructors
       * @link aggregation
       * @clientCardinality 1
       * @supplierCardinality 1..*
       */
      mutable std::vector<Member> fConstructors;

      /** 
      * IContainerImpl interface for container of constructors
      */
      const ContainerSTLAdaptor< std::vector<Member> > fConstructorsAdaptor;

      /**
       * short cut to destructor
       * @label destructor
       * @link aggregation
       * @clientCardinality 1
       * @supplierCardinality 1
       */
      mutable Member fDestructor;

   }; // class Constructable
} //namespace Internal
} //namespace Reflex


//-------------------------------------------------------------------------------
inline
Reflex::Internal::Constructable::Constructable(const char* name, unsigned int modifiers,
                                               size_t size, ETYPE type, const std::type_info& ti):
   ScopedType(name, modifiers, size, type, ti),
   fConstructorsAdaptor(fConstructors) {}
//-------------------------------------------------------------------------------

#endif // Reflex_Constructable

