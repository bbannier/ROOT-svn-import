// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Union
#define Reflex_Union

// Include Files
#include "Reflex/Member.h"
#include "Constructable.h"

namespace Reflex {
namespace Internal {

   /**
    * @class Union Union.h Reflex/Union.h
    * @author Stefan Roiser
    * @date 24/11/2003
    * @ingroup Ref
    */
   class Union : public Constructable
   {

   public:

      /** constructor */
      Union(const char* name, size_t size, const std::type_info& ti,
            const Catalog& catalog, unsigned int modifiers, ETYPE unionType = kUnion):
         Constructable(name, modifiers, size, unionType, ti, catalog) {}

      /** destructor */
      virtual ~Union() {}

   }; // class Union

} // namespace Internal
} // namespace Reflex

#endif // Reflex_Union

