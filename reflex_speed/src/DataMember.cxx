
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

#include "Reflex/Catalog.h"
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
   : MemberBase (nam, typ, kETDataMember, modifiers),
     fOffset(offs) { }


//-------------------------------------------------------------------------------
Reflex::Internal::DataMember::~DataMember() {
//-------------------------------------------------------------------------------
// Data member destructor.
}

//-------------------------------------------------------------------------------
const std::string&
Reflex::Internal::DataMember::Name(std::string& s, unsigned int mod) const {
//-------------------------------------------------------------------------------
// Return the scoped and qualified (if requested with mod) name of the data member
   if (mod & kQualified) {
      if (Is(kPublic))    s += "public ";
      if (Is(kProtected)) s += "protected ";
      if (Is(kPrivate))   s += "private ";
      if (Is(kExtern))    s += "extern ";
      if (Is(kStatic))    s += "static ";
      if (Is(kAuto))      s += "auto ";
      if (Is(kRegister))  s += "register ";
      if (Is(kMutable))   s += "mutable ";
   }

   if (mod & kScoped && DeclaringScope().Is(kEnum)) {
      if (DeclaringScope().DeclaringScope() && !DeclaringScope().DeclaringScope().IsTopScope()) {
         DeclaringScope().DeclaringScope().Name(s,kScoped);
         s += "::";
      }
      MemberBase::Name(s, mod & ~kScoped);
   }
   else {
      MemberBase::Name(s, mod);
   }

   return s;
}


//-------------------------------------------------------------------------------
Reflex::Object
Reflex::Internal::DataMember::Get(const Object & obj) const {
//-------------------------------------------------------------------------------
// Get the value of this data member as stored in object obj.
   if (DeclaringScope().ScopeType() == kETEnum) {
      return Object(Catalog::Instance().Get_int(), (void*)&fOffset);
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
   if (TypeOf().Is(kClassOrStruct)) {
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

   if (declScope.Is(kUnion)) {

      // FIXME

   }

   else if (declScope.Is(kEnum)) {
      std::string name;
      Name(name);

      std::stringstream tmp;
      tmp << Offset();

      if (declScope.DeclaringScope().Is(kNamespace)) { 
         generator.AddIntoInstances("\n.AddItem(\"" + name + "\", " + tmp.str() + ")");
      }
      else { // class, struct
         generator.AddIntoFree(name + "=" + tmp.str());
      }
   }

   else { // class, struct

      const Type & rType = TypeOf().RawType();
      std::string name;
      Name(name);
        
      if (TypeOf().Is(kArray)) {      

         Type t = TypeOf();

         std::stringstream temp;
         temp<< t.ArrayLength();

         std::string name;
         generator.AddIntoShadow(t.ToType().Name(kScoped) + " " + name + "[" + temp.str() + "];\n");
	     
      }
   
      else if (TypeOf().Is(kPointer) && TypeOf().RawType().Is(kFunction)) {
     
         Type t = TypeOf().ToType();
         generator.AddIntoShadow(t.ReturnType().Name(kScoped) + "(") ;
	
	
         if (t.DeclaringScope().Is(kClassOrStruct)) {
            generator.AddIntoShadow(t.DeclaringScope().Name(kScoped) + "::");
         }
	
         generator.AddIntoShadow("*"+ t.Name()+")(");
	  
	
         for (OrderedContainer<Type>::const_iterator iPar = t.FunctionParameters().Begin();
            iPar != t.FunctionParameters().End();) {
	       
            generator.AddIntoShadow(iPar->Name());
	    ++iPar;
            if (iPar != t.FunctionParameters().End()) {
               generator.AddIntoShadow(",");
            }	    
         }
	
         generator.AddIntoShadow(");\n");
	
      }
   
      else {
         std::string tname;
         TypeOf().Name(tname, kScoped);
         if (rType.Is((kClassOrStruct) && !kPublic)) {
            tname = generator.Replace_colon(rType.Name(kScoped));
            if (rType != TypeOf()) tname = tname + TypeOf().Name(kScoped).substr(tname.length());
         }
         generator.AddIntoShadow(tname + " " + name + ";\n");
      }

      //register type and get its number
      std::string typenumber = generator.GetTypeNumber(TypeOf());
   
      generator.AddIntoFree(".AddMember(type_" + typenumber + ", \"" + name + "\", ") ;
      generator.AddIntoFree("OffsetOf (__shadow__::" + 
                            generator.Replace_colon((*this).DeclaringScope().Name(kScoped)));
      generator.AddIntoFree(", " + name + "), ");
      
      if (Is(kPublic))    generator.AddIntoFree("kEDPublic");
      else if (Is(kPrivate))   generator.AddIntoFree("kEDPrivate");
      else if (Is(kProtected)) generator.AddIntoFree("kEDProtected");
      if (Is(kVirtual))    generator.AddIntoFree(" | kEDVirtual");
      if (Is(kArtificial))  generator.AddIntoFree(" | kEDArtificial");
   
      generator.AddIntoFree(")\n");

   } 
}
