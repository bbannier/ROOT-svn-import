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

#include "Class.h"

#include "Reflex/Object.h"
#include "Reflex/Type.h"
#include "Reflex/Catalog.h"
#include "Reflex/EntityProperty.h"
#include "Reflex/Tools.h"
#include "Reflex/DictionaryGenerator.h"

#include "DataMember.h"
#include "FunctionMember.h"

#include <typeinfo>
#include <iostream>
#include <sstream>
#include <algorithm>
#if defined (__linux) || defined (__APPLE__)
#include <cxxabi.h>
#endif


//-------------------------------------------------------------------------------
Reflex::Internal::Class::Class(const char *           typ,
                     size_t                 size,
                     const std::type_info & ti,
                     unsigned int           modifiers,
                     ETYPE                   classType)
//-------------------------------------------------------------------------------
// Construct a Class instance.
      : Constructable(typ, modifiers, size, classType, ti),
      fBasesAdaptor(fBases),
      fAllBases(0),
      fCompleteType(false) {}


//-------------------------------------------------------------------------------
Reflex::Internal::Class::~Class()
{
//-------------------------------------------------------------------------------
   for (PathsToBase::iterator it = fPathsToBase.begin(); it != fPathsToBase.end(); ++it) {
      delete it->second;
   }
}


//-------------------------------------------------------------------------------
void Reflex::Internal::Class::AddBase(const Type &   bas,
                            OffsetFunction offsFP,
                            unsigned int   modifiers) const
{
//-------------------------------------------------------------------------------
// Add a base class information.
   Base b(bas, offsFP, modifiers);
   fBases.push_back(b);
}


//-------------------------------------------------------------------------------
Reflex::Object Reflex::Internal::Class::CastObject(const Type & to,
      const Object & obj) const
{
//-------------------------------------------------------------------------------
// Cast an object. Will do up and down cast. Cross cast missing.
   std::vector< Base > path = std::vector< Base >();
   if (HasBase(to, path)) {    // up cast
      // in case of up cast the Offset has to be calculated by Reflex
      size_t obj2 = (size_t)obj.Address();
      for (std::vector< Base >::reverse_iterator bIter = path.rbegin();
            bIter != path.rend(); ++bIter) {
         obj2 += bIter->Offset((void*)obj2);
      }
      return Object(to, (void*)obj2);
   }
   path.clear();
   Type t = *this;
   if (to.HasBase(t)) {      // down cast
      // use the internal dynamic casting of the compiler (e.g. libstdc++.so)
      void * obj3 = 0;
#if defined (__linux) || defined (__APPLE__)
      obj3 = abi::__dynamic_cast(obj.Address(),
                                 (const abi::__class_type_info*) & this->TypeInfo(),
                                 (const abi::__class_type_info*) & to.TypeInfo(),
                                 -1);
#elif defined (_WIN32)
      obj3 = __RTDynamicCast(obj.Address(),
                             0,
                             (void*) & this->TypeInfo(),
                             (void*) & to.TypeInfo(),
                             0);
#endif
      return Object(to, obj3);
   }
   // fixme cross cast missing ?? internal cast possible ??

   // if the same At was passed return the object
   if ((Type)(*this) == to) return obj;

   // if everything fails return the dummy object
   return Object();
}


namespace {
   //-------------------------------------------------------------------------------
   struct DynType_t {
   //-------------------------------------------------------------------------------
      virtual ~DynType_t() {
         // dummy type with vtable.
      }
   };
}


//-------------------------------------------------------------------------------
Reflex::Type Reflex::Internal::Class::DynamicType(const Object & obj) const
{
//-------------------------------------------------------------------------------
// Discover the dynamic type of a class object and return it.
   // If no virtual_function_table return itself
   if (Is(gVirtual)) {
      // Avoid the case that the first word is a virtual_base_offset_table instead of
      // a virtual_function_table
      long int offset = **(long**)obj.Address();
      if (offset == 0) return ThisType();
      else {
         const Type & dytype = InCatalog()->ByTypeInfo(typeid(*(DynType_t*)obj.Address()));
         if (dytype && dytype.Is(gClass)) return dytype;
         else                              return ThisType();
      }
   }
   else {
      return ThisType();
   }
}


//-------------------------------------------------------------------------------
bool Reflex::Internal::Class::HasBase(const Type & cl,
                            std::vector< Base > & path) const
{
//-------------------------------------------------------------------------------
// Return true if this class has a base class of type cl. Return also the path
// to this type.
   if (! cl.Id()) return false;
   for (size_t i = 0; i < fBases.size(); ++i) {
      const Base& b = fBases[i];
      Type basetype = b.ToType();
      if (basetype.Id() == cl.Id() || basetype.FinalType().Id() == cl.Id()) {
         // remember the path to this class
         path.push_back(b);
         return true;
      }
      else if (basetype) {
         const Class* clbase = dynamic_cast<const Class*>(basetype.FinalType().ToTypeBase());
         if (clbase && clbase->HasBase(cl, path)) {
            // is the final base class one of the current class ?
            // if successfull remember path
            path.push_back(b);
            return true;
         }
      }
   }
   return false;
}


//-------------------------------------------------------------------------------
bool Reflex::Internal::Class::IsComplete() const
{
//-------------------------------------------------------------------------------
// Return true if this class is complete. I.e. all dictionary information for all
// data and function member types and base classes is available.
   if (! fCompleteType) fCompleteType = UpdateCompleteness();
   return fCompleteType;
}


//-------------------------------------------------------------------------------
bool Reflex::Internal::Class::UpdateCompleteness() const
{
//-------------------------------------------------------------------------------
// Return true if this class is complete. I.e. all dictionary information for all
// data and function member types and base classes is available (internal function).
   for (size_t i = 0; i < fBases.size(); ++i) {
      if (! fBases[i].ToScope().IsComplete())
         return false;
   }
   return true;
}


//-------------------------------------------------------------------------------
size_t Reflex::Internal::Class::AllBases() const
{
//-------------------------------------------------------------------------------
// Return the number of base classes.
   size_t aBases = 0;
   for (size_t i = 0; i < fBases.size(); ++i) {
      ++aBases;
      if (fBases[i]) {
         aBases += fBases[i].BaseClass()->AllBases();
      }
   }
   return aBases;
}


//-------------------------------------------------------------------------------
bool Reflex::Internal::Class::NewBases() const
{
//-------------------------------------------------------------------------------
// Check if information for new base classes has been added.
   if (! fCompleteType) {
      size_t numBases = AllBases();
      if (fAllBases != numBases) {
         fCompleteType = UpdateCompleteness();
         fAllBases = numBases;
         return true;
      }
   }
   return false;
}


//-------------------------------------------------------------------------------
const std::vector < Reflex::OffsetFunction > &
Reflex::Internal::Class::PathToBase(const Scope & bas) const
{
//-------------------------------------------------------------------------------
// Return a vector of offset functions from the current class to the base class.
   std::vector < OffsetFunction > * pathToBase = fPathsToBase[(ScopeName*) bas.Id()];
   if (! pathToBase) {
      pathToBase = fPathsToBase[(ScopeName*) bas.Id()];
   }
   return * pathToBase;
}


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void Reflex::Internal::Class::GenerateDict(DictionaryGenerator & generator) const
{
//-------------------------------------------------------------------------------
   // Generate Dictionary information about itself.

   // Selection file usage
   bool selected = true;

   /*
   // selection file used
   if (generator.fSelections.size() != 0 || generator.fPattern_selections.size() != 0) {
      selected = false;
      
      // normal selection
      for (unsigned i = 0; i < generator.fSelections.size(); ++i) {
         if (generator.fSelections.at(i) == (*this).Name(kScoped)) {
            selected = true;
         }
      }

      // pattern selection
      for (unsigned i = 0; i < generator.fPattern_selections.size(); ++i) {
         if ((*this).Name(kScoped).find(generator.fPattern_selections.at(i)) != std::string::npos) {
            selected = true;
         }
      }

   }
   // EndOf Selection file usage
   */

   if (selected == true) {

      std::string typenumber = generator.GetTypeNumber(ThisType());

      if (generator.fSelections.size() != 0 || generator.fPattern_selections.size() != 0) {
         std::cout << "  * selecting class " << Name(std::string(), kScoped) << "\n";
      }

      generator.AddIntoInstances("      " + generator.Replace_colon(ThisType().Name(kScoped)) + "_dict();\n");

      // Outputten only, if inside a namespace
      if (ThisType().DeclaringScope().IsTopScope() && (!DeclaringScope().Is(gNamespace))) {
         generator.AddIntoShadow("\nnamespace " + ThisType().Name() + " {");
      }

      // new
      if (ThisType().DeclaringScope().Is(gClassOrStruct)) {
         generator.AddIntoShadow("};");
      }




      // begin of the Dictionary-part
      generator.AddIntoShadow("\nclass " + generator.Replace_colon(ThisType().Name(kScoped)) + " {\n");
      generator.AddIntoShadow("public:\n");



      if (ThisType().DeclaringScope().Is(gClassOrStruct)) {
         generator.AddIntoFree(";\n}\n");
      }

      generator.AddIntoFree("\n\n// ------ Dictionary for class " + ThisType().Name() + "\n");
      generator.AddIntoFree("void " + generator.Replace_colon(ThisType().Name(kScoped)) + "_dict() {\n");
      generator.AddIntoFree("ClassBuilder(\"" + ThisType().Name(kScoped));
      if (Is(gPublic)) generator.AddIntoFree("\", typeid(" + ThisType().Name(kScoped) + "), sizeof(" + ThisType().Name(kScoped) + "), ");
      else if (Is(gProtected)) generator.AddIntoFree("\", typeid(Reflex::ProtectedClass), 0,");
      else if (Is(gPrivate)) generator.AddIntoFree("\", typeid(Reflex::PrivateClass), 0,");

      if (ThisType().Is(gPublic))  generator.AddIntoFree("kPublic");
      if (ThisType().Is(gPrivate)) generator.AddIntoFree("kPrivate");
      if (ThisType().Is(gProtected)) generator.AddIntoFree("kProtected");
      if (ThisType().Is(gVirtual)) generator.AddIntoFree(" | kVirtual");
      generator.AddIntoFree(" | kClass)\n");

      generator.AddIntoClasses("\n// -- Stub functions for class " + ThisType().Name() + "--\n");


      for (OrdOwnedMemberCont_t::iterator mi = Members().Begin(); mi; ++mi) {
         mi->GenerateDict(generator);      // call Members' own gendict
      }

      if (ThisType().DeclaringScope().IsTopScope() && (!DeclaringScope().Is(gNamespace))) {
         generator.AddIntoShadow("\nnamespace " + ThisType().Name() + " {");
      }

//       std::stringstream tempcounter;
//       tempcounter << generator.fMethodCounter;
//       generator.AddIntoClasses("\nstatic void* method_x" + tempcounter.str());
//       generator.AddIntoClasses(" (void*, const std::vector<void*>&, void*)\n{\n");
//       generator.AddIntoClasses("  static NewDelFunctions s_funcs;\n");

//       generator.AddIntoFree(".AddMember<void*(void)>(\"__getNewDelFunctions\", method_x" + tempcounter.str());
//       generator.AddIntoFree(", 0, 0, kPublic | kArtificial)");

//       std::string temp = "NewDelFunctionsT< ::" + ThisType().Name(kScoped) + " >::";
//       generator.AddIntoClasses("  s_funcs.fNew         = " + temp + "new_T;\n");
//       generator.AddIntoClasses("  s_funcs.fNewArray    = " + temp + "newArray_T;\n");
//       generator.AddIntoClasses("  s_funcs.fDelete      = " + temp + "delete_T;\n");
//       generator.AddIntoClasses("  s_funcs.fDeleteArray = " + temp + "deleteArray_T;\n");
//       generator.AddIntoClasses("  s_funcs.fDestructor  = " + temp + "destruct_T;\n");
//       generator.AddIntoClasses("  return &s_funcs;\n}\n ");

//       ++generator.fMethodCounter;

      if (ThisType().DeclaringScope().IsTopScope() && (!DeclaringScope().Is(gNamespace))) {
         generator.AddIntoShadow("}\n");        // End of top namespace
      }

      // Recursive call
      this->ScopeBase::GenerateDict(generator);


      if (!ThisType().DeclaringScope().Is(gClassOrStruct)) {
         generator.AddIntoShadow("};\n");
         generator.AddIntoFree(";\n}\n");
      }

   }//new type
}
