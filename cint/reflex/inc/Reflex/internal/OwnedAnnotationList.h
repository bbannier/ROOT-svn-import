// @(#)root/reflex:$Id$
// Author: Philippe Bourgau (Murex) 2010

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_OwnedAnnotationList
#define Reflex_OwnedAnnotationList

// Include files
#include "Reflex/Kernel.h"
#include "Reflex/AnnotationList.h"

namespace Reflex {
// forward declarations
class AnnotationListImpl;

/**
 * @class OwnedAnnotationList OwnedAnnotationList.h OwnedAnnotationList.h
 * @author Philippe Bourgau
 * @date 1/06/2010
 * @ingroup Ref
 */
class RFLX_API OwnedAnnotationList: public AnnotationList {
public:
   /** constructor */
   OwnedAnnotationList(AnnotationListImpl * AnnotationListImpl = 0):
      AnnotationList(AnnotationListImpl) {}


   /** delete */
   void Delete();

};    // class OwnedAnnotationList

} // namespace Reflex

#endif // Reflex_OwnedAnnotationList
