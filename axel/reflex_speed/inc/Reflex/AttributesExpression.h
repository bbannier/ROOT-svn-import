// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_AttributesExpression
#define Reflex_AttributesExpression

#include "Kernel.h"

namespace Reflex {

   // Bitset-style class that allows to test for a combination of
   // AttributesExpression enum constants via its operators &&, ||, !.
   class AttributesExpression {
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
      const AttributesExpression& fLHS;
      const AttributesExpression& fRHS;
      static AttributesExpression fgNIL;

   public:
      AttributesExpression(EENTITY_DESCRIPTION desc): fOp(kDesc), fValue(desc), fLHS(fgNIL), fRHS(fgNIL) {};
      AttributesExpression(ETYPE type): fOp(kType), fValue(type), fLHS(fgNIL), fRHS(fgNIL) {};
      AttributesExpression(EOP op, const AttributesExpression& lhs, const AttributesExpression& rhs = Reflex::AttributesExpression::fgNIL):
      fOp(op), fValue(0), fLHS(lhs), fRHS(rhs) {}

      bool Eval(int desc, ETYPE type) const {
         if (fOp == kDesc)  return (desc & fValue);
         if (fOp == kType)  return (type == fValue);
         if (fOp == kOr)    return fLHS.Eval(desc, type) || fRHS.Eval(desc, type);
         if (fOp == kAnd)   return fLHS.Eval(desc, type) && fRHS.Eval(desc, type);
         if (fOp == kNot)   return !fLHS.Eval(desc, type);
         return true;
      }

      AttributesExpression operator && (const AttributesExpression& rhs) const { return AttributesExpression(kAnd, *this, rhs); }
      AttributesExpression operator || (const AttributesExpression& rhs) const { return AttributesExpression(kOr, *this, rhs); }
      AttributesExpression operator ! () const { return AttributesExpression(kNot, *this); }
   };

   static const AttributesExpression kPublic(kEDPublic);
   static const AttributesExpression kProtected(kEDProtected);
   static const AttributesExpression kPrivate(kEDPrivate);
   static const AttributesExpression kRegister(kEDRegister);
   static const AttributesExpression kStatic(kEDStatic);
   static const AttributesExpression kConstructor(kEDConstructor);
   static const AttributesExpression kDestructor(kEDDestructor);
   static const AttributesExpression kExplicit(kEDExplicit);
   static const AttributesExpression kExtern(kEDExtern);
   static const AttributesExpression kCopyconstructor(kEDCopyConstructor);
   static const AttributesExpression kOperator(kEDOperator);
   static const AttributesExpression kInline(kEDInline);
   static const AttributesExpression kConverter(kEDConverter);
   static const AttributesExpression kAuto(kEDAuto);
   static const AttributesExpression kMutable(kEDMutable);
   static const AttributesExpression kConst(kEDConst);
   static const AttributesExpression kVolatile(kEDVolatile);
   static const AttributesExpression kReference(kEDReference);
   static const AttributesExpression kAbstract(kEDAbstract);
   static const AttributesExpression kVirtual(kEDVirtual);
   static const AttributesExpression kTransient(kEDTransient);
   static const AttributesExpression kArtificial(kEDArtificial);

   static const AttributesExpression kClass(kETClass);
   static const AttributesExpression kStruct(kETStruct);
   static const AttributesExpression kEnum(kETEnum);
   static const AttributesExpression kUnion(kETUnion);
   static const AttributesExpression kNamespace(kETNamespace);
   static const AttributesExpression kFunction(kETFunction);
   static const AttributesExpression kArray(kETArray);
   static const AttributesExpression kFundamental(kETFundamental);
   static const AttributesExpression kPointer(kETPointer);
   static const AttributesExpression kPointerToMember(kETPointerToMember);
   static const AttributesExpression kTypedef(kETTypedef);
   static const AttributesExpression kTypeTemplateInstance(kETTypeTemplateInstance);
   static const AttributesExpression kMemberTemplateInstance(kETMemberTemplateInstance);
   static const AttributesExpression kDataMember(kETDataMember);
   static const AttributesExpression kFunctionMember(kETFunctionMember);
   static const AttributesExpression kUnresolved(kETUnresolved);

   static const AttributesExpression kTemplateInstance(AttributesExpression::kOr, kTypeTemplateInstance, kMemberTemplateInstance);
#ifndef __CINT__
   static const AttributesExpression kClassOrStruct(kClass || kTypeTemplateInstance || kStruct);
#else
   static const AttributesExpression kClassOrStruct(AttributesExpression::kOr, kClass,
                                        AttributesExpression(AttributesExpression::kOr, kTypeTemplateInstance, kStruct));
#endif
}

#endif // Reflex_AttributesExpression

