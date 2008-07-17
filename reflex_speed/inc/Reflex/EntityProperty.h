// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_EntityProperty
#define Reflex_EntityProperty

#include "Kernel.h"

namespace Reflex {

   class EntityProperty {
   public:
      enum EOP {
         kDesc,
         kType,
         kAnd,
         kOr,
         kNot
      };

      EntityProperty(ENTITY_DESCRIPTION desc): fOp(kDesc), fValue(desc), fLHS(fgNIL), fRHS(fgNIL) {};
      EntityProperty(TYPE type): fOp(kType), fValue(type), fLHS(fgNIL), fRHS(fgNIL) {};
      EntityProperty(EOP op, const EntityProperty& lhs, const EntityProperty& rhs = fgNIL):
      fOp(op), fValue(0), fLHS(lhs), fRHS(rhs) {}

      bool Eval(int desc, int type) const {
         if (fOp == kDesc)  return (desc & fValue);
         if (fOp == kType)  return (type == fValue);
         if (fOp == kOr)    return fLHS.Eval(desc, type) || fRHS.Eval(desc, type);
         if (fOp == kAnd)   return fLHS.Eval(desc, type) && fRHS.Eval(desc, type);
         if (fOp == kNot)   return !fLHS.Eval(desc, type);
         return true;
      }

      EntityProperty operator && (const EntityProperty& rhs) const { return EntityProperty(kAnd, *this, rhs); }
      EntityProperty operator || (const EntityProperty& rhs) const { return EntityProperty(kOr, *this, rhs); }
      EntityProperty operator ! () const { return EntityProperty(kNot, *this); }

   private:
      EOP fOp;
      int fValue;
      const EntityProperty& fLHS;
      const EntityProperty& fRHS;
      static EntityProperty fgNIL;
   };

   static EntityProperty gPublic(PUBLIC);
   static EntityProperty gProtected(PROTECTED);
   static EntityProperty gPrivate(PRIVATE);
   static EntityProperty gRegister(REGISTER);
   static EntityProperty gStatic(STATIC);
   static EntityProperty gConstructor(CONSTRUCTOR);
   static EntityProperty gDestructor(DESTRUCTOR);
   static EntityProperty gExplicit(EXPLICIT);
   static EntityProperty gExtern(EXTERN);
   static EntityProperty gCopyconstructor(COPYCONSTRUCTOR);
   static EntityProperty gOperator(OPERATOR);
   static EntityProperty gInline(INLINE);
   static EntityProperty gConverter(CONVERTER);
   static EntityProperty gAuto(AUTO);
   static EntityProperty gMutable(MUTABLE);
   static EntityProperty gConst(CONST);
   static EntityProperty gVolatile(VOLATILE);
   static EntityProperty gReference(REFERENCE);
   static EntityProperty gAbstract(ABSTRACT);
   static EntityProperty gVirtual(VIRTUAL);
   static EntityProperty gTransient(TRANSIENT);
   static EntityProperty gArtificial(ARTIFICIAL);

   static EntityProperty gClass(CLASS);
   static EntityProperty gStruct(STRUCT);
   static EntityProperty gEnum(ENUM);
   static EntityProperty gUnion(UNION);
   static EntityProperty gNamespace(NAMESPACE);
   static EntityProperty gFunction(FUNCTION);
   static EntityProperty gArray(ARRAY);
   static EntityProperty gFundamental(FUNDAMENTAL);
   static EntityProperty gPointer(POINTER);
   static EntityProperty gPointerToMember(POINTERTOMEMBER);
   static EntityProperty gTypedef(TYPEDEF);
   static EntityProperty gTypeTemplateInstance(TYPETEMPLATEINSTANCE);
   static EntityProperty gMemberTemplateInstance(MEMBERTEMPLATEINSTANCE);
   static EntityProperty gDataMember(DATAMEMBER);
   static EntityProperty gFunctionMember(FUNCTIONMEMBER);
   static EntityProperty gUnresolved(UNRESOLVED);

   static EntityProperty gTemplateInstance(gTypeTemplateInstance || gMemberTemplateInstance);
   static EntityProperty gClassOrStruct(gClass || gTypeTemplateInstance || gStruct);

}

#endif // Reflex_EntityProperty

