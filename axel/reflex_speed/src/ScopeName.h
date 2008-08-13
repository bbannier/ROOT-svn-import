// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_ScopeName
#define Reflex_ScopeName

// Include files
#include "Reflex/Kernel.h"
#include "Reflex/Catalog.h"
#include <string>

namespace Reflex {

   // forward declarations
   class Scope;

namespace Internal {

   // forward declarations
   class ScopeBase;

   /**
   * @class ScopeName ScopeName.h Reflex/ScopeName.h
   * @author Stefan Roiser
   * @date 24/11/2003
   * @ingroup Ref
   */
   class RFLX_API ScopeName {

      friend class Reflex::Scope;
      friend class ScopeBase;

   public:

      /** constructor */
      ScopeName(const char * name, ScopeBase * scopeBase, const Catalog& catalog);


      /**
      * DeleteScope will call the destructor of the ScopeBase this ScopeName is
      * pointing to and aremove it's information from the data structures. The
      * ScopeName information will remain.
      */
      void DeleteScope() const;


      void HideName();


      /**
      * Retrieve the Catalog containing the type.
      */
      const Catalog& InCatalog() const { return fCatalog; }


      /**
      * Name will return a string representation of Name of the Scope
      * @return string representation of the Scope
      */
      const std::string & Name() const;


      /** 
      * ThisScope will return the unqualified Scope object of this ScopeName
      * @return corresponding Scope
      */
      Scope ThisScope() const;


   private:

      /** destructor */
      ~ScopeName();

   private:

      /** pointer to the Name of the At in the static map */
      std::string fName;

      /**
      * pointer to the resolved Scope
      * @label scope base
      * @link aggregation
      * @supplierCardinality 0..1
      * @clientCardinality 1
      */
      mutable
         ScopeBase * fScopeBase;

      /**
      * This scope
      */
      Scope * fThisScope;

      /**
      * Catalog containing the scope
      */
      const Catalog fCatalog;

   }; // class ScopeName
} // namespace Internal
} // namespace Reflex


//-------------------------------------------------------------------------------
inline const std::string & Reflex::Internal::ScopeName::Name() const {
//-------------------------------------------------------------------------------
   return fName;
}

#endif //Reflex_ScopeName
