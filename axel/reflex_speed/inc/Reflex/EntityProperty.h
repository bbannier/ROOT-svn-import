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
         if (fOp == kDesc) return (desc && fValue);
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

   static EntityProperty gPUBLIC(PUBLIC);
   static EntityProperty gPROTECTED(PROTECTED);
   static EntityProperty gPRIVATE(PRIVATE);
   static EntityProperty gREGISTER(REGISTER);
   static EntityProperty gSTATIC(STATIC);
   static EntityProperty gCONSTRUCTOR(CONSTRUCTOR);
   static EntityProperty gDESTRUCTOR(DESTRUCTOR);
   static EntityProperty gEXPLICIT(EXPLICIT);
   static EntityProperty gEXTERN(EXTERN);
   static EntityProperty gCOPYCONSTRUCTOR(COPYCONSTRUCTOR);
   static EntityProperty gOPERATOR(OPERATOR);
   static EntityProperty gINLINE(INLINE);
   static EntityProperty gCONVERTER(CONVERTER);
   static EntityProperty gAUTO(AUTO);
   static EntityProperty gMUTABLE(MUTABLE);
   static EntityProperty gCONST(CONST);
   static EntityProperty gVOLATILE(VOLATILE);
   static EntityProperty gREFERENCE(REFERENCE);
   static EntityProperty gABSTRACT(ABSTRACT);
   static EntityProperty gVIRTUAL(VIRTUAL);
   static EntityProperty gTRANSIENT(TRANSIENT);
   static EntityProperty gARTIFICIAL(ARTIFICIAL);

   static EntityProperty gCLASS(CLASS);
   static EntityProperty gSTRUCT(STRUCT);
   static EntityProperty gENUM(ENUM);
   static EntityProperty gUNION(UNION);
   static EntityProperty gNAMESPACE(NAMESPACE);
   static EntityProperty gFUNCTION(FUNCTION);
   static EntityProperty gARRAY(ARRAY);
   static EntityProperty gFUNDAMENTAL(FUNDAMENTAL);
   static EntityProperty gPOINTER(POINTER);
   static EntityProperty gPOINTERTOMEMBER(POINTERTOMEMBER);
   static EntityProperty gTYPEDEF(TYPEDEF);
   static EntityProperty gTYPETEMPLATEINSTANCE(TYPETEMPLATEINSTANCE);
   static EntityProperty gMEMBERTEMPLATEINSTANCE(MEMBERTEMPLATEINSTANCE);
   static EntityProperty gDATAMEMBER(DATAMEMBER);
   static EntityProperty gFUNCTIONMEMBER(FUNCTIONMEMBER);
   static EntityProperty gUNRESOLVED(UNRESOLVED);

}

#endif // Reflex_EntityProperty

