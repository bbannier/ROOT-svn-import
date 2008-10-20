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

#include "Function.h"

#include "Reflex/Tools.h"
#include "TypeName.h"
#include "OwnedMember.h"

//-------------------------------------------------------------------------------
Reflex::Internal::Function::Function(const Type & retType,
                                     const std::vector< Type > & parameters,
                                     const std::type_info & ti,
                                     Catalog catalog,
                                     ETYPE functionType) 
//-------------------------------------------------------------------------------
// Default constructor for a function type.
: TypeBase(BuildTypeName(retType, parameters, 0, kQualified | kScoped).c_str(), 0, 0, functionType, ti, catalog),
     fParameters(parameters),
     fParametersAdaptor(fParameters),
     fReturnType(retType)
     { }


//-------------------------------------------------------------------------------
const std::string&
Reflex::Internal::Function::Name(std::string& buf, unsigned int mod) const {
//-------------------------------------------------------------------------------
// Return the name of the function type.
   if (! mod)
      return (buf += fTypeName->Name());
   return BuildTypeName(buf, fReturnType, fParameters, fTypeModifiers, mod, DeclaringScope());

}


//-------------------------------------------------------------------------------
const std::string&
Reflex::Internal::Function::BuildPointerTypeName(std::string& buf, const Type & ret,
                                                 const std::string& name,
                                                 const OrderedContainer< Type >::const_iterator & parsBegin,
                                                 const OrderedContainer< Type >::const_iterator & parsEnd,
                                                 unsigned int typemod, unsigned int mod,
                                                 const Scope& scope) {
//-------------------------------------------------------------------------------
// Build the name of the function type in the form <returntype><space>(<scope>::* <name>)(<param>*)
// Return type and parameter types will always be scoped and qualified, even if
// mod specifies non-scoped, non-qualified; the modification only changes the
// function name itself, i.e. "a::b (a::* ptr)() const" becomes "a::b (a::* ptr)()" for 
// non-qualified (scoped / non-scoped is ignored and must be handled when calulating the
// the name parameter).

   ret.Name(buf, kScoped | kQualified);

   buf += " (";
   if (scope && !scope.Is(kNamespace)) {
      scope.Name(buf, kScoped);
      buf += "::* ";
   }
   buf += name + ")(";
   for (Container<Type>::const_iterator ti = parsBegin; ti != parsEnd;) {
      ti->Name(buf, kQualified | kScoped);
      if (++ti != parsEnd)
         buf += ", ";
   }
   // leave it blank - the shorter the string the better.
   //else {
   //   buf += "void";
   //}
   buf += ")";
   if ((mod & kQualified) && (typemod & kEDConst))
      buf += " const";
   // should we or should we not? It's part of the type but overloading with
   // throiw / no throw is not allowed.
   // if (Is(kThrows)) buf += " throw()";
   return buf;
}


//-------------------------------------------------------------------------------
const std::string&
Reflex::Internal::Function::BuildTypeName(std::string& buf, const Type & ret,
                                          const std::vector< Type > & pars,
                                          unsigned int typemod, unsigned int mod,
                                          const Scope& scope) {
//-------------------------------------------------------------------------------
// Build the name of the function type in the form <returntype><space>(<param>*)
// Return type and parameter types will always be scoped and qualified, even if
// mod specifies non-scoped, non-qualified; the modification only changes the
// function name itself, i.e. "a::b a::() const" becomes "a::b ()" for non-scoped,
// non-qualified.
   ret.Name(buf, kScoped | kQualified);
   if (mod & kScoped)
      scope.Name(buf, kScoped);
   buf += " (";
   if (pars.size() > 0) {
      std::vector< Type >::const_iterator it;
      for (it = pars.begin(); it != pars.end();) {
         it->Name(buf, kScoped | kQualified);
         if (++it != pars.end()) buf += ", ";
      }
   }
   // leave it blank - the shorter the string the better.
   //else {
   //   buf += "void";
   //}
   buf += ")";
   if ((mod & kQualified) && (typemod & kEDConst))
      buf += " const";
   // should we or should we not? It's part of the type but overloading with
   // throiw / no throw is not allowed.
   // if (Is(kThrows)) buf += " throw()";
   return buf;
}
