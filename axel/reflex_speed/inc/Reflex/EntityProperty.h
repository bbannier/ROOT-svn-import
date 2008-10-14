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

      bool Eval(int desc, ETYPE type) const {
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

   static const EntityProperty kPublic(kEDPublic);
   static const EntityProperty kProtected(kEDProtected);
   static const EntityProperty kPrivate(kEDPrivate);
   static const EntityProperty kRegister(kEDRegister);
   static const EntityProperty kStatic(kEDStatic);
   static const EntityProperty kConstructor(kEDConstructor);
   static const EntityProperty kDestructor(kEDDestructor);
   static const EntityProperty kExplicit(kEDExplicit);
   static const EntityProperty kExtern(kEDExtern);
   static const EntityProperty kCopyconstructor(kEDCopyConstructor);
   static const EntityProperty kOperator(kEDOperator);
   static const EntityProperty kInline(kEDInline);
   static const EntityProperty kConverter(kEDConverter);
   static const EntityProperty kAuto(kEDAuto);
   static const EntityProperty kMutable(kEDMutable);
   static const EntityProperty kConst(kEDConst);
   static const EntityProperty kVolatile(kEDVolatile);
   static const EntityProperty kReference(kEDReference);
   static const EntityProperty kAbstract(kEDAbstract);
   static const EntityProperty kVirtual(kEDVirtual);
   static const EntityProperty kTransient(kEDTransient);
   static const EntityProperty kArtificial(kEDArtificial);

   static const EntityProperty kClass(kETClass);
   static const EntityProperty kStruct(kETStruct);
   static const EntityProperty kEnum(kETEnum);
   static const EntityProperty kUnion(kETUnion);
   static const EntityProperty kNamespace(kETNamespace);
   static const EntityProperty kFunction(kETFunction);
   static const EntityProperty kArray(kETArray);
   static const EntityProperty kFundamental(kETFundamental);
   static const EntityProperty kPointer(kETPointer);
   static const EntityProperty kPointerToMember(kETPointerToMember);
   static const EntityProperty kTypedef(kETTypedef);
   static const EntityProperty kTypeTemplateInstance(kETTypeTemplateInstance);
   static const EntityProperty kMemberTemplateInstance(kETMemberTemplateInstance);
   static const EntityProperty kDataMember(kETDataMember);
   static const EntityProperty kFunctionMember(kETFunctionMember);
   static const EntityProperty kUnresolved(kETUnresolved);

   static const EntityProperty kTemplateInstance(EntityProperty::kOr, kTypeTemplateInstance, kMemberTemplateInstance);
#ifndef __CINT__
   static const EntityProperty kClassOrStruct(kClass || kTypeTemplateInstance || kStruct);
#else
   static const EntityProperty kClassOrStruct(EntityProperty::kOr, kClass,
                                        EntityProperty(EntityProperty::kOr, kTypeTemplateInstance, kStruct));
#endif
}

#endif // Reflex_EntityProperty

