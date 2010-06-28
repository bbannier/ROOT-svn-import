// @(#)root/reflex:$Id$
// Author: Philippe Bourgau (Murex) 2010

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_AnnotationList
#define Reflex_AnnotationList


// Include files
#include "Reflex/Kernel.h"
#include <iostream>

namespace Reflex {
// forward declarations
class AnnotationListImpl;
class Any;

/**
 * @class AnnotationList AnnotationList.h Reflex/AnnotationList.h
 * @author Philippe Bourgau (Murex)
 * @date 1/06/2010
 * @ingroup Ref
 */
class RFLX_API AnnotationList {
   friend class OwnedAnnotationList;
   friend RFLX_API std::ostream& operator <<(std::ostream& s,
                                             const AnnotationList& p);

public:
   /** default constructor */
   AnnotationList(AnnotationListImpl * AnnotationListImpl = 0);


   /** copy constructor */
   AnnotationList(const AnnotationList &pl);


   /** destructor */
   ~AnnotationList();


   /**
    * operator bool will return true if the Annotation list is implemented
    * @return true if Annotation list is not a fake one
    */
   operator bool() const;


   /**
    * AddAnnotation will add a Annotation value pair to the Annotation list
    * @param value the value of the Annotation (as any object)
    */
   void AddAnnotation(const Any& value) const;


   /**
    * HasAnnotation will return true if the Annotation list contains a valid Annotation with type
    * @param  type the Annotation type
    * @return true if there is an Annotation with type
    */
   bool HasAnnotation(const std::type_info& type) const;


   /**
    * AnnotationCount will return the number of annotations attached to this item.
    * @return number of properties
    */
   size_t AnnotationSize() const;


   /**
    * Annotation_Begin will return the begin iterator of the annotation container
    * @return begin iterator of annotation container
    */
   Any_Iterator Annotation_Begin() const;


   /**
    * Annotation_End will return the end iterator of the annotation container
    * @return end iterator of annotation container
    */
   Any_Iterator Annotation_End() const;


   /**
    * Annotation_RBegin will return the rbegin iterator of the annotation container
    * @return rbegin iterator of annotation container
    */
   Reverse_Any_Iterator Annotation_RBegin() const;


   /**
    * Annotation_REnd will return the rend iterator of the annotation container
    * @return rend iterator of annotation container
    */
   Reverse_Any_Iterator Annotation_REnd() const;


   /**
    * AnnotationAt will return the nth attached annotation
    * @param nth key currently allocated
    * @return key as a string
    */
   const Any& AnnotationAt(size_t nth) const;


   /**
    * Annotation will return the first Annotation with type
    * @param  type the Annotation type
    * @return Annotation value
    */
   const Any& AnnotationWithType(const std::type_info& type) const;


   /**
    * Annotation will return the first Annotation with type
    * @param  type the Annotation type
    * @return Annotation value
    */
   template<class AnnotationType>
   const AnnotationType* AnnotationWithType() const;


private:
   /**
    * the annotations of the item
    * @link aggregation
    * @clentCardinality 1
    * @supplierCardinality 0..1
    * @label AnnotationList impl
    */
   AnnotationListImpl* fAnnotationListImpl;

};    // class AnnotationList

/**
 * will put the Annotation (key and value) on the ostream if printable
 * @param s the reference to the stream
 * @return the stream
 */
RFLX_API std::ostream& operator <<(std::ostream& s,
                                   const AnnotationList& p);

} //namespace Reflex

#include "Reflex/internal/AnnotationListImpl.h"

//-------------------------------------------------------------------------------
inline
Reflex::AnnotationList::operator bool() const {
//-------------------------------------------------------------------------------
   return 0 != fAnnotationListImpl;
}


//-------------------------------------------------------------------------------
inline Reflex::AnnotationList::AnnotationList(AnnotationListImpl* AnnotationListImpl)
//-------------------------------------------------------------------------------
   : fAnnotationListImpl(AnnotationListImpl) {
}


//-------------------------------------------------------------------------------
inline Reflex::AnnotationList::AnnotationList(const AnnotationList& pl)
//-------------------------------------------------------------------------------
   : fAnnotationListImpl(pl.fAnnotationListImpl) {
}


//-------------------------------------------------------------------------------
inline Reflex::AnnotationList::~AnnotationList() {
//-------------------------------------------------------------------------------
}


//-------------------------------------------------------------------------------
inline void
Reflex::AnnotationList::AddAnnotation(const Any& value) const {
//-------------------------------------------------------------------------------
   if (fAnnotationListImpl) {
      return fAnnotationListImpl->AddAnnotation(value);
   }
}


//-------------------------------------------------------------------------------
inline bool
Reflex::AnnotationList::HasAnnotation(const std::type_info& type) const {
//-------------------------------------------------------------------------------
   if (fAnnotationListImpl) {
      return fAnnotationListImpl->HasAnnotation(type);
   }
   return false;
}


//-------------------------------------------------------------------------------
inline size_t
Reflex::AnnotationList::AnnotationSize() const {
//-------------------------------------------------------------------------------
   if (fAnnotationListImpl) {
      return fAnnotationListImpl->AnnotationSize();
   }
   return 0;
}


//-------------------------------------------------------------------------------
inline Reflex::Any_Iterator
Reflex::AnnotationList::Annotation_Begin() const {
//-------------------------------------------------------------------------------
// Return the begin iterator of the annotations container.
   if (fAnnotationListImpl) {
      return fAnnotationListImpl->Annotation_Begin();
   }
   return Dummy::Any_Cont_Type().begin();
}


//-------------------------------------------------------------------------------
inline Reflex::Any_Iterator
Reflex::AnnotationList::Annotation_End() const {
//-------------------------------------------------------------------------------
// Return the end iterator of the annotations container.
   if (fAnnotationListImpl) {
      return fAnnotationListImpl->Annotation_End();
   }
   return Dummy::Any_Cont_Type().end();
}


//-------------------------------------------------------------------------------
inline Reflex::Reverse_Any_Iterator
Reflex::AnnotationList::Annotation_RBegin() const {
//-------------------------------------------------------------------------------
// Return the rbegin iterator of the annotations container.
   if (fAnnotationListImpl) {
      return fAnnotationListImpl->Annotation_RBegin();
   }
   return Dummy::Any_Cont_Type().rbegin();
}


//-------------------------------------------------------------------------------
inline Reflex::Reverse_Any_Iterator
Reflex::AnnotationList::Annotation_REnd() const {
//-------------------------------------------------------------------------------
// Return the rend iterator of the annotations container.
   if (fAnnotationListImpl) {
      return fAnnotationListImpl->Annotation_REnd();
   }
   return Dummy::Any_Cont_Type().rend();
}


//-------------------------------------------------------------------------------
inline const Reflex::Any&
Reflex::AnnotationList::AnnotationAt(size_t nth) const {
//-------------------------------------------------------------------------------
// Return key at position nth.
   if (fAnnotationListImpl) {
      return fAnnotationListImpl->AnnotationAt(nth);
   }
   return Dummy::Any();
}


//-------------------------------------------------------------------------------
inline const Reflex::Any&
Reflex::AnnotationList::AnnotationWithType(const std::type_info& type) const {
//-------------------------------------------------------------------------------
// Annotation will return the first Annotation with type
   if (fAnnotationListImpl) {
      return fAnnotationListImpl->AnnotationWithType(type);
   }
   return Dummy::Any();
}


//-------------------------------------------------------------------------------
template<class AnnotationType> const AnnotationType*
Reflex::AnnotationList::AnnotationWithType() const {
//-------------------------------------------------------------------------------
// Annotation will return the first Annotation with type
   if (fAnnotationListImpl) {
      return fAnnotationListImpl->AnnotationWithType<AnnotationType>();
   }
   return 0;
}

#endif // Reflex_AnnotationList
