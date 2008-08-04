// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Class
#define Reflex_Class

// Include files
#include "Constructable.h"
#include "OwnedMember.h"
#include "ContainerSTLAdaptor.h"
#include <map>
#include <vector>

namespace Reflex {

   // forward declarations
   class Base;
   class Member;
   class DictionaryGenerator;

namespace Internal {

   /**
    * @class Class Class.h Reflex/Class.h
    * @author Stefan Roiser
    * @date 24/11/2003
    * @ingroup Ref
    */
   class Class : public Constructable
   {

   public:

      /** constructor */
      Class(const char* typ, size_t size, const std::type_info& ti, unsigned int modifiers = 0, ETYPE classType = kClass);

      /** destructor */
      virtual ~Class();

      /**
      * BaseAt returns the collection of base class information
      * @return pointer to base class information
      */
      virtual const IContainerImpl& Bases() const;


      /**
       * CastObject an object from this class At to another one
       * @param  to is the class At to cast into
       * @param  obj the memory AddressGet of the object to be casted
       */
      virtual Object CastObject(const Type& to, const Object& obj) const;

      /**
       * DynamicType is used to discover whether an object represents the
       * current class At or not
       * @param  mem is the memory AddressGet of the object to checked
       * @return the actual class of the object
       */
      virtual Type DynamicType(const Object& obj) const;

      /**
       * GenerateDict will produce the dictionary information of this type
       * @param generator a reference to the dictionary generator instance
       */
      virtual void GenerateDict(DictionaryGenerator& generator) const;

      /**
       * HasBase will check whether this class has a BaseAt class given
       * as argument
       * @param  cl the BaseAt-class to check for
       * @return the Base info if it is found, an empty base otherwise (can be tested for bool)
       */
      virtual bool HasBase(const Type& cl) const;

      /**
       * HasBase will check whether this class has a BaseAt class given
       * as argument
       * @param  cl the BaseAt-class to check for
       * @param  path optionally the path to the BaseAt can be retrieved
       * @return true if this class has a BaseAt-class cl, false otherwise
       */
      bool HasBase(const Type& cl, std::vector<Base>& path) const;

      /* IsComplete will return true if all classes and base classes of this 
      * class are resolved and fully known in the system
      */
      bool IsComplete() const;

      /**
       * PathToBase will return a vector of function pointers to the base class
       * (!!! Attention !!! the most derived class comes first)
       * @param base the scope to look for
       * @return vector of function pointers to calculate base offset
       */
      const std::vector<OffsetFunction>& PathToBase(const Scope& bas) const;

      /**
       * AddBase will add the information about a BaseAt class
       * @param  BaseAt At of the BaseAt class
       * @param  OffsetFP the pointer to the stub function for calculating the Offset
       * @param  modifiers the modifiers of the BaseAt class
       * @return this
       */
      virtual void AddBase(const Type & bas, OffsetFunction offsFP, unsigned int modifiers = 0) const;

      /**
       * AddBase will add the information about a BaseAt class
       * @param b the pointer to the BaseAt class info
       */
      virtual void AddBase(const Base& b) const;


   private:

      /** map with the class as a key and the path to it as the value
          the key (void*) is a pointer to the unique ScopeName */
      typedef std::map<ScopeName*, std::vector<OffsetFunction>* > PathsToBase;

      /**
       * NewBases will return true if new BaseAt classes have been discovered
       * since the last time it was called
       * @return true if new BaseAt classes were resolved
       */
      bool NewBases() const;

      /**
       * internal recursive checking for completeness
       * @return true if class is complete (all bases are resolved)
       */
      bool UpdateCompleteness() const;

      /**
       * AllBases will return the number of all BaseAt classes
       * (double count even in case of virtual inheritance)
       * @return number of all BaseAt classes
       */
      size_t AllBases() const;

   private:

      /**
       * container of base classes
       * @label class bases
       * @link aggregation
       * @clientCardinality 1
       * @supplierCardinality 0..*
       */
      mutable std::vector<Base> fBases;

      /** 
      * IContainerImpl interface for container of bases
      */
      const ContainerSTLAdaptor< std::vector<Base> > fBasesAdaptor;

      /** caches */
      /** all currently known BaseAt classes */
      mutable size_t fAllBases;

      /** boolean is true if the whole object is resolved */
      mutable bool fCompleteType;

      /** map to all inherited datamembers and their inheritance path */
      mutable PathsToBase fPathsToBase;

   }; // class Class
} //namespace Internal
} //namespace Reflex

#include "Reflex/Base.h"


//-------------------------------------------------------------------------------
inline void Reflex::Internal::Class::AddBase(const Base & b) const
{
//-------------------------------------------------------------------------------
   fBases.push_back(b);
}


//-------------------------------------------------------------------------------
inline const Reflex::Internal::IContainerImpl&
Reflex::Internal::Class::Bases() const
{
//-------------------------------------------------------------------------------
   return fBasesAdaptor;
}


//-------------------------------------------------------------------------------
inline bool
Reflex::Internal::Class::HasBase(const Type & cl) const
{
//-------------------------------------------------------------------------------
// Return true if this class has a base class of type cl.
   std::vector<Base> v;
   return HasBase(cl, v);
}

#endif // Reflex_Class

