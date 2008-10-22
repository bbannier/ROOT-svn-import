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
   class RFLX_API AttributesExpression {
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

   const AttributesExpression kPublic(kEDPublic);
   const AttributesExpression kProtected(kEDProtected);
   const AttributesExpression kPrivate(kEDPrivate);
   const AttributesExpression kRegister(kEDRegister);
   const AttributesExpression kStatic(kEDStatic);
   const AttributesExpression kConstructor(kEDConstructor);
   const AttributesExpression kDestructor(kEDDestructor);
   const AttributesExpression kExplicit(kEDExplicit);
   const AttributesExpression kExtern(kEDExtern);
   const AttributesExpression kCopyconstructor(kEDCopyConstructor);
   const AttributesExpression kOperator(kEDOperator);
   const AttributesExpression kInline(kEDInline);
   const AttributesExpression kConverter(kEDConverter);
   const AttributesExpression kAuto(kEDAuto);
   const AttributesExpression kMutable(kEDMutable);
   const AttributesExpression kConst(kEDConst);
   const AttributesExpression kVolatile(kEDVolatile);
   const AttributesExpression kReference(kEDReference);
   const AttributesExpression kAbstract(kEDAbstract);
   const AttributesExpression kVirtual(kEDVirtual);
   const AttributesExpression kTransient(kEDTransient);
   const AttributesExpression kArtificial(kEDArtificial);

   const AttributesExpression kConstVolatile(AttributesExpression::kAnd, kConst, kVolatile);

   const AttributesExpression kClass(kETClass);
   const AttributesExpression kStruct(kETStruct);
   const AttributesExpression kEnum(kETEnum);
   const AttributesExpression kUnion(kETUnion);
   const AttributesExpression kNamespace(kETNamespace);
   const AttributesExpression kFunction(kETFunction);
   const AttributesExpression kArray(kETArray);
   const AttributesExpression kFundamental(kETFundamental);
   const AttributesExpression kPointer(kETPointer);
   const AttributesExpression kPointerToMember(kETPointerToMember);
   const AttributesExpression kTypedef(kETTypedef);
   const AttributesExpression kTypeTemplateInstance(kETTypeTemplateInstance);
   const AttributesExpression kMemberTemplateInstance(kETMemberTemplateInstance);
   const AttributesExpression kDataMember(kETDataMember);
   const AttributesExpression kFunctionMember(kETFunctionMember);
   const AttributesExpression kUnresolved(kETUnresolved);

   const AttributesExpression kTemplateInstance(AttributesExpression::kOr, kTypeTemplateInstance, kMemberTemplateInstance);
#ifndef __CINT__
   const AttributesExpression kClassOrStruct(kClass || kTypeTemplateInstance || kStruct);
#else
   const AttributesExpression kClassOrStruct(AttributesExpression::kOr, kClass,
                                        AttributesExpression(AttributesExpression::kOr, kTypeTemplateInstance, kStruct));
#endif
}

#endif // Reflex_AttributesExpression

