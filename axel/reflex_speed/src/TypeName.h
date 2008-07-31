// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_TypeName
#define Reflex_TypeName

// Include files
#include "Reflex/Kernel.h"
#include <string>
#include <typeinfo>

namespace Reflex {

   // forward declarations 
   class Type;
   class Catalog;

namespace Internal {

   // forward declarations 
   class TypeBase;

   /** 
   * class TypeName TypeName.h Reflex/TypeName.h
   * @author Stefan Roiser
   * @date 06/11/2004
   * @ingroup Ref
   */
   class RFLX_API TypeName {

   public:

      /** default constructor */
      TypeName(const char * nnam,
         TypeBase * typeBas,
         const std::type_info * ti = 0,
         const Catalog* catalog = 0);


      /**
      * UpdateTypeBase calls the destructor of the TypeBase this TypeName is
      * pointing to and removes its information from the data structures. The
      * TypeName information will be set to the new tb if != NULL.
      */
      void UpdateTypeBase(TypeBase* tb);


      /**
      * Retrieve the Catalog containing the type.
      */
      const Catalog* InCatalog() const { return fCatalog; }


      void HideName();


      /**
      * Name will return the string representation of the At (unique)
      * @return At Name as a string
      */
      const std::string & Name() const;


      /** 
      * At returns the At object of this TypeName
      * @return corresponding Type to this TypeName
      */
      Type ThisType() const;


      /**
      * Return the type implementation for this name
      */
      TypeBase* ToTypeBase() const { return fTypeBase; }


      /** destructor */
      ~TypeName();

      /** Set the type_info in the hash_map to this */
      void SetTypeId(const std::type_info & ti) const;

   private:

      /** the Name of the At */
      std::string fName;


      /**
      * pointer to a TypeBase if the At is implemented
      * @label type base
      * @link aggregation
      * @supplierCardinality 1
      * @clientCardinality 1
      */
      TypeBase * fTypeBase;

      /**
      * Pointer back to the type
      * @label this type
      * @link aggregation
      * @supplierCardinality 1
      * @clientCardinality 1
      */
      Type * fThisType;


      /**
      * Catalog containing the type
      */
      const Catalog* fCatalog;

   }; // class TypeName

} // namespace Internal
} // namespace Reflex

//-------------------------------------------------------------------------------
inline const std::string&
Reflex::Internal::TypeName::Name() const {
//-------------------------------------------------------------------------------
   return fName;
}

#endif // Reflex_TypeName
