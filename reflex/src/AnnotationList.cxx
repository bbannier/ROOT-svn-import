// @(#)root/reflex:$Id$
// Author: Philippe Bourgau (Murex) 2010

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BUILD
# define REFLEX_BUILD
#endif

#include "Reflex/AnnotationList.h"

#include "Reflex/internal/AnnotationListImpl.h"
#include "Reflex/Any.h"

//-------------------------------------------------------------------------------
std::ostream&
Reflex::operator <<(std::ostream& s,
                    const AnnotationList& p) {
//-------------------------------------------------------------------------------
// Operator to put a Annotation list on the ostream.
   if (p.fAnnotationListImpl) {
      s << *(p.fAnnotationListImpl);
   }
   return s;
}





