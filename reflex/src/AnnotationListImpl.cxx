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

#include "Reflex/internal/AnnotationListImpl.h"
#include "Reflex/Any.h"
#include "Reflex/Tools.h"

#include <sstream>

// SOLARIS CC FIX (this include file is needed for a fix for std::distance)
#include "stl_hash.h"

//-------------------------------------------------------------------------------
std::ostream&
Reflex::operator <<(std::ostream& s,
                    const AnnotationListImpl& p) {
//-------------------------------------------------------------------------------
// Operator to put properties on the ostream.
   if (p.fAnnotations) {
      for (size_t i = 0; i < p.fAnnotations->size(); ++i) {
         Any a = p.AnnotationAt(i);

         if (a) {
            s << a << std::endl;
         }
      }
   }
   return s;
}


//-------------------------------------------------------------------------------
Reflex::AnnotationListImpl::~AnnotationListImpl() {
//-------------------------------------------------------------------------------
// Destruct, deleting our fAnnotations.
   delete fAnnotations;
}


//-------------------------------------------------------------------------------
bool
Reflex::AnnotationListImpl::HasAnnotation(const std::type_info& type) const {
//-------------------------------------------------------------------------------
// Return true if there is an Annotation with type
   size_t i = IndexWithType(type);
   return (i != NPos());
}


//-------------------------------------------------------------------------------
Reflex::Any_Iterator
Reflex::AnnotationListImpl::Annotation_Begin() const {
//-------------------------------------------------------------------------------
// Return begin iterator of annotation container
   if (fAnnotations) {
      return fAnnotations->begin();
   }
   return Dummy::Any_Cont_Type().begin();
}


//-------------------------------------------------------------------------------
Reflex::Any_Iterator
Reflex::AnnotationListImpl::Annotation_End() const {
//-------------------------------------------------------------------------------
// Return end iterator of annotation container
   if (fAnnotations) {
      return fAnnotations->end();
   }
   return Dummy::Any_Cont_Type().end();
}


//-------------------------------------------------------------------------------
Reflex::Reverse_Any_Iterator
Reflex::AnnotationListImpl::Annotation_RBegin() const {
//-------------------------------------------------------------------------------
// Return rbegin iterator of annotation container
   if (fAnnotations) {
      return fAnnotations->rbegin();
   }
   return Dummy::Any_Cont_Type().rbegin();
}


//-------------------------------------------------------------------------------
Reflex::Reverse_Any_Iterator
Reflex::AnnotationListImpl::Annotation_REnd() const {
//-------------------------------------------------------------------------------
// Return rend iterator of annotation container
   if (fAnnotations) {
      return fAnnotations->rend();
   }
   return Dummy::Any_Cont_Type().rend();
}


//-------------------------------------------------------------------------------
const Reflex::Any&
Reflex::AnnotationListImpl::AnnotationAt(size_t nth) const {
//-------------------------------------------------------------------------------
// Return the nth Annotation.
   if (fAnnotations && (nth != NPos()) && (nth < AnnotationSize())) {
      return fAnnotations->at(nth);
   }
   return Dummy::Any();
}


//-------------------------------------------------------------------------------
size_t
Reflex::AnnotationListImpl::AnnotationSize() const {
//-------------------------------------------------------------------------------
// Return number of all allocated keys.
   if (fAnnotations) {
      return fAnnotations->size();
   }
   return 0;
}


//-------------------------------------------------------------------------------
const Reflex::Any&
Reflex::AnnotationListImpl::AnnotationWithType(const std::type_info& type) const {
//-------------------------------------------------------------------------------
// Return a Annotation as an Any object.
   return AnnotationAt(IndexWithType(type));
}


//-------------------------------------------------------------------------------
size_t
Reflex::AnnotationListImpl::IndexWithType(const std::type_info& type) const {
//-------------------------------------------------------------------------------
// Return index of the annotation, end() if not found
   if (fAnnotations) {
      for(size_t i = 0; i != fAnnotations->size(); ++i) {
         if ((*fAnnotations)[i].TypeInfo() == type) {
            return i;
         }
      }
   }
   return NPos();
}