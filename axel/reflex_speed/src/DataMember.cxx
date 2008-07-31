
// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BUILD
#define REFLEX_BUILD
#endif

#include "DataMember.h"

#include "Reflex/Scope.h"
#include "Reflex/Object.h"
#include "Reflex/Member.h"
#include "Reflex/DictionaryGenerator.h"

#include "Reflex/EntityProperty.h"
#include "Reflex/Tools.h"
#include "Class.h"
#include <cstring>


//-------------------------------------------------------------------------------
Reflex::Internal::DataMember::DataMember(const char *  nam,
                                      const Type &  typ,
                                      size_t        offs,
                                      unsigned int  modifiers)
//-------------------------------------------------------------------------------
// Construct the dictionary information for a data member.
   : MemberBase (nam, typ, kDataMember, modifiers),
     fOffset(offs) { }


//-------------------------------------------------------------------------------
Reflex::Internal::DataMember::~DataMember() {
//-------------------------------------------------------------------------------
// Data member destructor.
}

//-------------------------------------------------------------------------------
std::string
Reflex::Internal::DataMember::Name(unsigned int mod) const {
//-------------------------------------------------------------------------------
// Return the scoped and qualified (if requested with mod) name of the data member
   std::string s = "";

   if (mod & kQualified) {
      if (Is(gPUBLIC))    s += "public ";
      if (Is(gPROTECTED)) s += "protected ";
      if (Is(gPRIVATE))   s += "private ";
      if (Is(gEXTERN))    s += "extern ";
      if (Is(gSTATIC))    s += "static ";
      if (Is(gAUTO))      s += "auto ";
      if (Is(gREGISTER))  s += "register ";
      if (Is(gMUTABLE))   s += "mutable ";
   }

   if (mod & kScoped && DeclaringScope().Is(gEnum)) {
      if (DeclaringScope().DeclaringScope()) {
         std::string sc = DeclaringScope().DeclaringScope().Name(kScoped);
         if (sc != "::") s += sc + "::";
      }
      s += MemberBase::Name(mod & ~kScoped);
   }
   else {
      s += MemberBase::Name(mod);
   }

   return s;
}


//-------------------------------------------------------------------------------
Reflex::Object
Reflex::Internal::DataMember::Get(const Object & obj) const {
//-------------------------------------------------------------------------------
// Get the value of this data member as stored in object obj.
   if (DeclaringScope().ScopeType() == kEnum) {
      return Object(Type::ByName("int"), (void*)&fOffset);
   }
   else {
      void * mem = CalculateBaseObject(obj);
      mem = (char*)mem + Offset();
      return Object(TypeOf(),mem);
   }
}


/*/-------------------------------------------------------------------------------
  void
Reflex::Internal::DataMember::Set(const Object & instance,
  const Object & value) const {
//-------------------------------------------------------------------------------
  void * mem = CalculateBaseObject(instance);
  mem = (char*)mem + Offset();
  if (TypeOf().IsClass()) {
  // Should use the asigment operator if exists (FIX-ME)
  memcpy(mem, value.Address(), TypeOf().SizeOf());
  }
  else {
  memcpy(mem, value.Address(), TypeOf().SizeOf());
  }
  }
*/


//-------------------------------------------------------------------------------
void
Reflex::Internal::DataMember::Set(const Object & instance,
                                    const void * value) const {
//-------------------------------------------------------------------------------
// Set the data member value in object instance.
   void * mem = CalculateBaseObject(instance);
   mem = (char*)mem + Offset();
   if (TypeOf().IsClass()) {
      // Should use the asigment operator if exists (FIX-ME)
      memcpy(mem, value, TypeOf().SizeOf());
   }
   else {
      memcpy(mem, value, TypeOf().SizeOf());
   }
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::DataMember::GenerateDict(DictionaryGenerator & generator) const {
//-------------------------------------------------------------------------------
// Generate Dictionary information about itself.

   const Scope & declScope = DeclaringScope();

   if (declScope.Is(gUnion)) {

      // FIXME

   }

   else if (declScope.Is(gEnum)) {

      std::stringstream tmp;
      tmp << Offset();

      if (declScope.DeclaringScope().Is(gNAMESPACE)) { 
         generator.AddIntoInstances("\n.AddItem(\"" + Name() + "\", " + tmp.str() + ")");
      }
      else { // class, struct
         generator.AddIntoFree(Name() + "=" + tmp.str());
      }
   }

   else { // class, struct

      const Type & rType = TypeOf().RawType();
        
      if (TypeOf().Is(gArray)) {      

         Type t = TypeOf();

         std::stringstream temp;
         temp<< t.ArrayLength();

         generator.AddIntoShadow(t.ToType().Name(kScoped) + " " + Name() + "[" + temp.str() + "];\n");
	     
      }
   
      else if (TypeOf().Is(gPointer) && TypeOf().RawType().Is(gFunction)) {
     
         Type t = TypeOf().ToType();
         generator.AddIntoShadow(t.ReturnType().Name(kScoped) + "(") ;
	
	
         if (t.DeclaringScope().IsClass()) {
            generator.AddIntoShadow(t.DeclaringScope().Name(kScoped) + "::");
         }
	
         generator.AddIntoShadow("*"+ t.Name()+")(");
	  
	
         for (size_t parameters = 0; parameters<  t.FunctionParameterSize();
              ++parameters) {
	       
            generator.AddIntoShadow(t.FunctionParameterAt(parameters).Name());
	       
            if (t.FunctionParameterSize()>parameters) {
               generator.AddIntoShadow(",");
            }	    
         }
	
         generator.AddIntoShadow(");\n");
	
      }
   
      else {
         std::string tname = TypeOf().Name(kScoped);
         if (rType.Is((gClass || gSTRUCT) && !gPUBLIC)) {
            tname = generator.Replace_colon(rType.Name(kScoped));
            if (rType != TypeOf()) tname = tname + TypeOf().Name(kScoped).substr(tname.length());
         }
         generator.AddIntoShadow(tname + " " + Name() + ";\n");
      }

      //register type and get its number
      std::string typenumber = generator.GetTypeNumber(TypeOf());
   
      generator.AddIntoFree(".AddDataMember(type_" + typenumber + ", \"" + Name() + "\", ") ;
      generator.AddIntoFree("OffsetOf (__shadow__::" + 
                            generator.Replace_colon((*this).DeclaringScope().Name(kScoped)));
      generator.AddIntoFree(", " + Name() + "), ");
      
      if (Is(gPUBLIC))    generator.AddIntoFree("kPublic");
      else if (Is(gPRIVATE))   generator.AddIntoFree("kPrivate");
      else if (Is(gPROTECTED)) generator.AddIntoFree("kProtected");
      if (Is(gVirtual))    generator.AddIntoFree(" | kVirtual");
      if (Is(gARTIFICIAL))  generator.AddIntoFree(" | kArtificial");
   
      generator.AddIntoFree(")\n");

   } 
}
