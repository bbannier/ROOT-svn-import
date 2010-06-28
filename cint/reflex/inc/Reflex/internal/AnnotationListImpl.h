// @(#)root/reflex:$Id$
// Author: Philippe Bourgau (Murex) 2010

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_AnnotationListImpl
#define Reflex_AnnotationListImpl


// Include files
#include "Reflex/Kernel.h"
#include "Reflex/Any.h"
#include <map>
#include <iostream>

#ifdef _WIN32
# pragma warning( push )
# pragma warning( disable : 4251 )
#endif

namespace Reflex {
/**
 * @class AnnotationList AnnotationList.h Reflex/AnnotationList.h
 * @author Philippe Bourgau
 * @date 1/06/2010
 * @ingroup Ref
 */
class RFLX_API AnnotationListImpl {
   friend RFLX_API std::ostream& operator <<(std::ostream& s,
                                             const AnnotationListImpl& p);

public:
   /** default constructor */
   AnnotationListImpl();


   /** copy constructor */
   AnnotationListImpl(const AnnotationListImpl &pl);


   /** destructor */
   virtual ~AnnotationListImpl();


   /**
    * AddAnnotation will add a Annotation value pair to the Annotation list
    * @param value the value of the Annotation (as any object)
    */
   void AddAnnotation(const Any& value);


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
    * Index of the first annotation with type
    * @param  type the Annotation type
    * @return index of the annotation, NPos() if not found
    */
   size_t IndexWithType(const std::type_info& type) const;


   /** the annotations of the item
    * @label properties
    * @link aggregationByValue
    * @clientCardinality 1
    * @supplierCardinality 0..1
    */
   Any_Cont_Type_t* fAnnotations;

};    // class AnnotationListImpl

/**
 * will put the AnnotationNth (key and value) on the ostream if printable
 * @param s the reference to the stream
 * @return the stream
 */
RFLX_API std::ostream& operator <<(std::ostream& s,
                                   const AnnotationListImpl& p);

} //namespace Reflex


//-------------------------------------------------------------------------------
inline Reflex::AnnotationListImpl::AnnotationListImpl()
//-------------------------------------------------------------------------------
   : fAnnotations(0) {
}


//-------------------------------------------------------------------------------
inline Reflex::AnnotationListImpl::AnnotationListImpl(const AnnotationListImpl& pl)
//-------------------------------------------------------------------------------
   : fAnnotations(pl.fAnnotations) {
}


//-------------------------------------------------------------------------------
inline void
Reflex::AnnotationListImpl::AddAnnotation(const Any& value) {
//-------------------------------------------------------------------------------
   if (!fAnnotations) {
      fAnnotations = new Any_Cont_Type_t();
   }
   fAnnotations->push_back(value);
}


//-------------------------------------------------------------------------------
template<class AnnotationType> const AnnotationType*
Reflex::AnnotationListImpl::AnnotationWithType() const {
//-------------------------------------------------------------------------------
// Annotation will return the first Annotation with type
   return any_cast<AnnotationType>(&AnnotationWithType(typeid(AnnotationType)));
}

#ifdef _WIN32
# pragma warning( pop )
#endif

#endif // Reflex_AnnotationListImpl
