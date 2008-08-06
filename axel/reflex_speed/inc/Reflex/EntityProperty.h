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

   // Bitset-style class that allows to test for a combination of
   // EntityProperty enum constants via its operators &&, ||, !.
   class EntityProperty {
   public:
      enum EOP {
         kDesc,
         kType,
         kAnd,
         kOr,
         kNot
      };

   private:
      EOP fOp;
      int fValue;
      const EntityProperty& fLHS;
      const EntityProperty& fRHS;
      static EntityProperty fgNIL;

   public:
      EntityProperty(EENTITY_DESCRIPTION desc): fOp(kDesc), fValue(desc), fLHS(fgNIL), fRHS(fgNIL) {};
      EntityProperty(ETYPE type): fOp(kType), fValue(type), fLHS(fgNIL), fRHS(fgNIL) {};
      EntityProperty(EOP op, const EntityProperty& lhs, const EntityProperty& rhs = Reflex::EntityProperty::fgNIL):
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
   };

   static EntityProperty gPublic(kPublic);
   static EntityProperty gProtected(kProtected);
   static EntityProperty gPrivate(kPrivate);
   static EntityProperty gRegister(kRegister);
   static EntityProperty gStatic(kStatic);
   static EntityProperty gConstructor(kConstructor);
   static EntityProperty gDestructor(kDestructor);
   static EntityProperty gExplicit(kExplicit);
   static EntityProperty gExtern(kExtern);
   static EntityProperty gCopyconstructor(kCopyConstructor);
   static EntityProperty gOperator(kOperator);
   static EntityProperty gInline(kInline);
   static EntityProperty gConverter(kConverter);
   static EntityProperty gAuto(kAuto);
   static EntityProperty gMutable(kMutable);
   static EntityProperty gConst(kConst);
   static EntityProperty gVolatile(kVolatile);
   static EntityProperty gReference(kReference);
   static EntityProperty gAbstract(kAbstract);
   static EntityProperty gVirtual(kVirtual);
   static EntityProperty gTransient(kTransient);
   static EntityProperty gArtificial(kArtificial);

   static EntityProperty gClass(kClass);
   static EntityProperty gStruct(kStruct);
   static EntityProperty gEnum(kEnum);
   static EntityProperty gUnion(kUnion);
   static EntityProperty gNamespace(kNamespace);
   static EntityProperty gFunction(kFunction);
   static EntityProperty gArray(kArray);
   static EntityProperty gFundamental(kFundamental);
   static EntityProperty gPointer(kPointer);
   static EntityProperty gPointerToMember(kPointerToMember);
   static EntityProperty gTypedef(kTypedef);
   static EntityProperty gTypeTemplateInstance(kTypeTemplateInstance);
   static EntityProperty gMemberTemplateInstance(kMemberTemplateInstance);
   static EntityProperty gDataMember(kDataMember);
   static EntityProperty gFunctionMember(kFunctionMember);
   static EntityProperty gUnresolved(kUnresolved);

   static EntityProperty gTemplateInstance(EntityProperty::kOr, gTypeTemplateInstance, gMemberTemplateInstance);
#ifndef __CINT__
   static EntityProperty gClassOrStruct(gClass || gTypeTemplateInstance || gStruct);
#else
   static EntityProperty gClassOrStruct(EntityProperty::kOr, gClass,
                                        EntityProperty(EntityProperty::kOr, gTypeTemplateInstance, gStruct));
#endif
}

#endif // Reflex_EntityProperty

