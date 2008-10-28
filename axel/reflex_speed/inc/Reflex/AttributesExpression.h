// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_AttributesExpressionT
#define Reflex_AttributesExpressionT

#include "Kernel.h"

namespace Reflex {

   // Bitset-style class that allows to test for a combination of
   // AttributesExpressionT enum constants via its operators &&, ||, !.

   template <typename OP1, typename OP2 = OP1> class AttributesExpressionT_And;
   template <typename OP1, typename OP2 = OP1> class AttributesExpressionT_Or;
   template <typename OP1>               class AttributesExpressionT_Not;
   class AttributesExpressionT_TypeType;
   class AttributesExpressionT_TypeDescription;
   template <typename ATTRIBUTESEXPRESSION> class AttributesExpressionImpl;
      
   class AttributesExpression {
   public:
      virtual bool Eval(int, ETYPE type) const = 0;
   };
      
   template <typename SELF>
   class AttributesExpressionT: public AttributesExpression {
   public:
      template <typename OPR>
      const AttributesExpressionT_And<SELF, OPR>
      operator &&(const OPR& opr) const;

      template <typename OPR>
      const AttributesExpressionT_Or<SELF, OPR>
      operator ||(const OPR& opr) const;
      
      const AttributesExpressionT_Not<SELF>
      operator !() const;
   };

   class RFLX_API AttributesExpressionT_TypeType:
      public AttributesExpressionT< AttributesExpressionT_TypeType > {
   public:
      AttributesExpressionT_TypeType(ETYPE type): fType(type) {}
      bool Eval(int, ETYPE type) const { return fType == type; }
   private:
      ETYPE fType;
   };

   class RFLX_API AttributesExpressionT_TypeDescription:
      public AttributesExpressionT< AttributesExpressionT_TypeDescription > {
   public:
      AttributesExpressionT_TypeDescription(int descr): fDescr(descr) {}
      bool Eval(int descr, ETYPE) const { return fDescr == descr; }
   private:
      int fDescr;
   };

#ifndef __MAKECINT__

   template <class SELF, class OP1, class OP2>
   class AttributesExpressionT_Binary: public AttributesExpressionT<SELF> {
   public:
      AttributesExpressionT_Binary(const OP1& op1, const OP2& op2):
         fLeft(op1), fRight(op2) {}
   protected:
      OP1 fLeft;
      OP2 fRight;
   };

   template <class OP1, class OP2 /* = OP1 */>
   class AttributesExpressionT_And:
      public AttributesExpressionT_Binary< AttributesExpressionT_And<OP1, OP2>, OP1, OP2> {
   public:
      AttributesExpressionT_And(const OP1& l, const OP2& r):
         AttributesExpressionT_Binary< AttributesExpressionT_And<OP1, OP2>, OP1, OP2>(l, r) {}
      bool Eval(int descr, ETYPE type) const { return this->fLeft.Eval(descr, type) && this->fRight.Eval(descr, type); }
   };

   template <class OP1, class OP2 /* = OP1 */>
   class AttributesExpressionT_Or:
      public AttributesExpressionT_Binary< AttributesExpressionT_Or<OP1, OP2>, OP1, OP2> {
   public:
      AttributesExpressionT_Or(const OP1& l, const OP2& r):
         AttributesExpressionT_Binary< AttributesExpressionT_Or<OP1, OP2>, OP1, OP2>(l, r) {}
      bool Eval(int descr, ETYPE type) const { return this->fLeft.Eval(descr, type) || this->fRight.Eval(descr, type); }
   };

   template <class OPR>
   class AttributesExpressionT_Not:
      public AttributesExpressionT< AttributesExpressionT_Not<OPR> > {
   public:
      AttributesExpressionT_Not(const OPR& op): fOp(op) {}
      bool Eval(int descr, ETYPE type) const { return fOp.Eval(descr, type); }
   protected:
      const OPR fOp;
   };

   template <typename SELF>
   template <typename OPR>
   inline
   const AttributesExpressionT_And<SELF, OPR>
   AttributesExpressionT<SELF>::operator &&(const OPR& opr) const {
      return AttributesExpressionT_And<SELF, OPR>(*reinterpret_cast<const SELF*>(this), opr);
   }

   template <typename SELF>
   template <typename OPR>
   inline
   const AttributesExpressionT_Or<SELF, OPR>
   AttributesExpressionT<SELF>::operator ||(const OPR& opr) const {
      return AttributesExpressionT_Or<SELF, OPR>(*reinterpret_cast<const SELF*>(this), opr);
   }

   template <typename SELF>
   inline
   const AttributesExpressionT_Not<SELF>
   AttributesExpressionT<SELF>::operator !() const {
      return AttributesExpressionT_Not<SELF>(*reinterpret_cast<const SELF*>(this));
   }

#endif // __MAKECINT__

   const AttributesExpressionT_TypeDescription kPublic(kEDPublic);
   const AttributesExpressionT_TypeDescription kProtected(kEDProtected);
   const AttributesExpressionT_TypeDescription kPrivate(kEDPrivate);
   const AttributesExpressionT_TypeDescription kRegister(kEDRegister);
   const AttributesExpressionT_TypeDescription kStatic(kEDStatic);
   const AttributesExpressionT_TypeDescription kConstructor(kEDConstructor);
   const AttributesExpressionT_TypeDescription kDestructor(kEDDestructor);
   const AttributesExpressionT_TypeDescription kExplicit(kEDExplicit);
   const AttributesExpressionT_TypeDescription kExtern(kEDExtern);
   const AttributesExpressionT_TypeDescription kCopyconstructor(kEDCopyConstructor);
   const AttributesExpressionT_TypeDescription kOperator(kEDOperator);
   const AttributesExpressionT_TypeDescription kInline(kEDInline);
   const AttributesExpressionT_TypeDescription kConverter(kEDConverter);
   const AttributesExpressionT_TypeDescription kAuto(kEDAuto);
   const AttributesExpressionT_TypeDescription kMutable(kEDMutable);
   const AttributesExpressionT_TypeDescription kConst(kEDConst);
   const AttributesExpressionT_TypeDescription kVolatile(kEDVolatile);
   const AttributesExpressionT_TypeDescription kReference(kEDReference);
   const AttributesExpressionT_TypeDescription kAbstract(kEDAbstract);
   const AttributesExpressionT_TypeDescription kVirtual(kEDVirtual);
   const AttributesExpressionT_TypeDescription kTransient(kEDTransient);
   const AttributesExpressionT_TypeDescription kArtificial(kEDArtificial);

   const AttributesExpressionT_And<AttributesExpressionT_TypeDescription>
      kConstVolatile(kConst, kVolatile);

   const AttributesExpressionT_TypeType kClass(kETClass);
   const AttributesExpressionT_TypeType kStruct(kETStruct);
   const AttributesExpressionT_TypeType kEnum(kETEnum);
   const AttributesExpressionT_TypeType kUnion(kETUnion);
   const AttributesExpressionT_TypeType kNamespace(kETNamespace);
   const AttributesExpressionT_TypeType kFunction(kETFunction);
   const AttributesExpressionT_TypeType kArray(kETArray);
   const AttributesExpressionT_TypeType kFundamental(kETFundamental);
   const AttributesExpressionT_TypeType kPointer(kETPointer);
   const AttributesExpressionT_TypeType kPointerToMember(kETPointerToMember);
   const AttributesExpressionT_TypeType kTypedef(kETTypedef);
   const AttributesExpressionT_TypeType kTypeTemplateInstance(kETTypeTemplateInstance);
   const AttributesExpressionT_TypeType kMemberTemplateInstance(kETMemberTemplateInstance);
   const AttributesExpressionT_TypeType kDataMember(kETDataMember);
   const AttributesExpressionT_TypeType kFunctionMember(kETFunctionMember);
   const AttributesExpressionT_TypeType kUnresolved(kETUnresolved);

   const AttributesExpressionT_Or<Reflex::AttributesExpressionT_TypeType>
      kTemplateInstance(kTypeTemplateInstance, kMemberTemplateInstance);
   const AttributesExpressionT_Or<Reflex::AttributesExpressionT_TypeType>
      kClassOrTemplateInstance(kClass, kTypeTemplateInstance);
#ifndef __CINT__
   const AttributesExpressionT_Or<
      AttributesExpressionT_Or<AttributesExpressionT_TypeType>,
      AttributesExpressionT_TypeType
   > kClassOrStruct(kClass || kTypeTemplateInstance || kStruct);
#else
   const AttributesExpressionT_Or<
      Reflex::AttributesExpressionT_Or<Reflex::AttributesExpressionT_TypeType>,
      Reflex::AttributesExpressionT_TypeType
   > kClassOrStruct(kClassOrTemplateInstance, kStruct);
#endif
}

#endif // Reflex_AttributesExpressionT

