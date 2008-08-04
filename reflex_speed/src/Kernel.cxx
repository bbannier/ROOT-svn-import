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

#include "Reflex/Kernel.h"

#include "Reflex/Scope.h"
#include "Reflex/PropertyList.h"
#include "Reflex/Type.h"
#include "Reflex/Base.h"
#include "Reflex/Member.h"
#include "Reflex/Object.h"
#include "Reflex/PropertyList.h"
#include "Reflex/MemberTemplate.h"
#include "Reflex/Any.h"
#include "Reflex/Container.h"

#include "Fundamental.h"
#include "Namespace.h"
#include "Typedef.h"
#include "Class.h"
#include "TypeTemplateName.h"
#include "CatalogImpl.h"
#include <typeinfo>



//-------------------------------------------------------------------------------
const std::string&
Reflex::TYPEName(ETYPE type) {
//-------------------------------------------------------------------------------
   static const std::string sTYPENames[] = {
      "kClass",
      "kStruct",
      "kEnum",
      "kUnion",
      "kNamespace",
      "kFunction",
      "kArray",
      "kFundamental",
      "kPointer",
      "kPointerToMember",
      "kTypedef",
      "kDataMember",
      "kFunctionMember",
      "kUnresolved",
      ""
   };

   if (type > kUnresolved + 1)
      type = (ETYPE) (kUnresolved + 1);
   return sTYPENames[type];
}


//-------------------------------------------------------------------------------
Reflex::Instance::Instance() {
//-------------------------------------------------------------------------------
// Initialisation of Reflex.Setup of global scope, fundamental types.

   /** initialisation of the global namespace */
   Internal::Namespace::GlobalScope();

   // initialising fundamental types
   Internal::Fundamental * tb = 0;
   Type t = Type();
 
   // char [3.9.1.1]
   tb = new Internal::Fundamental("char", 
                         sizeof(char), 
                         typeid(char));
   tb->Properties().AddProperty("Description", "fundamental type");

   // signed integer types [3.9.1.2]
   tb = new Internal::Fundamental("signed char", 
                         sizeof(signed char), 
                         typeid(signed char));
   tb->Properties().AddProperty("Description", "fundamental type");

   tb = new Internal::Fundamental("short int", 
                         sizeof(short int), 
                         typeid(short int));
   tb->Properties().AddProperty("Description", "fundamental type");
   t = tb->ThisType();
   new Internal::Typedef("short", t, kFundamental, t);
   new Internal::Typedef("signed short", t, kFundamental, t);
   new Internal::Typedef("short signed", t, kFundamental, t);
   new Internal::Typedef("signed short int", t, kFundamental, t);
   new Internal::Typedef("short signed int", t, kFundamental, t);

   tb = new Internal::Fundamental("int", 
                         sizeof(int), 
                         typeid(int));
   tb->Properties().AddProperty("Description", "fundamental type");
   t = tb->ThisType();
   new Internal::Typedef("signed", t, kFundamental, t);
   new Internal::Typedef("signed int", t, kFundamental, t);

   tb = new Internal::Fundamental("long int", 
                         sizeof(long int), 
                         typeid(long int));
   tb->Properties().AddProperty("Description", "fundamental type");
   t = tb->ThisType();
   new Internal::Typedef("long", t, kFundamental, t);
   new Internal::Typedef("signed long", t, kFundamental, t);
   new Internal::Typedef("long signed", t, kFundamental, t);
   new Internal::Typedef("signed long int", t, kFundamental, t);
   new Internal::Typedef("long signed int", t, kFundamental, t);

   // unsigned integer types [3.9.1.3]
   tb = new Internal::Fundamental("unsigned char", 
                         sizeof(unsigned char), 
                         typeid(unsigned char));
   tb->Properties().AddProperty("Description", "fundamental type");

   tb = new Internal::Fundamental("unsigned short int", 
                         sizeof(unsigned short int), 
                         typeid(unsigned short int));
   tb->Properties().AddProperty("Description", "fundamental type");
   t = tb->ThisType();
   new Internal::Typedef("unsigned short", t, kFundamental, t);
   new Internal::Typedef("short unsigned int", t, kFundamental, t);

   tb = new Internal::Fundamental("unsigned int", 
                         sizeof(unsigned int), 
                         typeid(unsigned int));
   tb->Properties().AddProperty("Description", "fundamental type");
   t = tb->ThisType();
   new Internal::Typedef("unsigned", t, kFundamental, t);

   tb = new Internal::Fundamental("unsigned long int", 
                         sizeof(unsigned long int), 
                         typeid(unsigned long int));
   tb->Properties().AddProperty("Description", "fundamental type");
   t = tb->ThisType();
   new Internal::Typedef("unsigned long", t, kFundamental, t);
   new Internal::Typedef("long unsigned", t, kFundamental, t);
   new Internal::Typedef("long unsigned int", t, kFundamental, t);

   /*/ w_chart [3.9.1.5]
     tb = new Internal::Fundamental("w_chart", 
     sizeof(w_chart), 
     & typeid(w_chart));
     tb->Properties().AddProperty("Description", "fundamental type");
   */

   // bool [3.9.1.6]
   tb = new Internal::Fundamental("bool", 
                         sizeof(bool), 
                         typeid(bool));
   tb->Properties().AddProperty("Description", "fundamental type");

   // floating point types [3.9.1.8]
   tb = new Internal::Fundamental("float", 
                         sizeof(float), 
                         typeid(float));
   tb->Properties().AddProperty("Description", "fundamental type");

   tb = new Internal::Fundamental("double", 
                         sizeof(double), 
                         typeid(double));
   tb->Properties().AddProperty("Description", "fundamental type");

   tb = new Internal::Fundamental("long double", 
                         sizeof(long double), 
                         typeid(long double));
   tb->Properties().AddProperty("Description", "fundamental type");

   // void [3.9.1.9]
   tb = new Internal::Fundamental("void", 
                         0, 
                         typeid(void));
   tb->Properties().AddProperty("Description", "fundamental type");

      // Large integer definition depends of the platform
#if defined(_MSC_VER) && !defined(__CINT__)
   typedef __int64 longlong;
   typedef unsigned __int64 ulonglong;
#else
   typedef long long int longlong; /* */
   typedef unsigned long long int /**/ ulonglong;
#endif

   // non fundamental types but also supported at initialisation
   tb = new Internal::Fundamental("long long", 
                         sizeof(longlong), 
                         typeid(longlong));
   tb->Properties().AddProperty("Description", "fundamental type");
   t = tb->ThisType();
   new Internal::Typedef("long long int", t, kFundamental, t);

   tb = new Internal::Fundamental("unsigned long long", 
                         sizeof(ulonglong), 
                         typeid(ulonglong));
   tb->Properties().AddProperty("Description", "fundamental type");
   t = tb->ThisType();
   new Internal::Typedef("long long unsigned", t, kFundamental, t);
   new Internal::Typedef("unsigned long long int", t, kFundamental, t);
   new Internal::Typedef("long long unsigned int", t, kFundamental, t);

}


//-------------------------------------------------------------------------------
void Reflex::Instance::Shutdown() {
//-------------------------------------------------------------------------------
   // function to be called at tear down of Reflex, removes all memory allocations
   Internal::MemberTemplateName::CleanUp();
   Internal::TypeTemplateName::CleanUp();
   Catalog::Instance().Impl()->Types().CleanUp();
   Catalog::Instance().Impl()->Scopes().CleanUp();
}


//-------------------------------------------------------------------------------

Reflex::Instance::~Instance() {
//-------------------------------------------------------------------------------
   // Destructor

   // Uncomment this once Unload work:
   // Shutdown;
}


namespace {
   using namespace Reflex::Internal;
   class EmptyContainerImpl: public Reflex::EmptyCont_Type_t {
   public:
      static class iterator: public IConstIteratorImpl {
      public:
         virtual ~iterator() {}

         virtual bool ProxyIsEqual(const IConstIteratorImpl& other) const { return true; }
         virtual void ProxyForward() { }
         virtual const void* ProxyElement() const { return 0; }
         virtual IConstIteratorImpl* ProxyClone() const { return 0; }
      } sBeginEnd;

      virtual void ProxyBegin(ConstIteratorBase& i) const { i.SetImpl(&sBeginEnd, false); }
      virtual void ProxyEnd(ConstIteratorBase& i) const { i.SetImpl(&sBeginEnd, false); }

      // empty implementation for unordered container
      virtual void ProxyRBegin(ConstIteratorBase& i) const { i.SetImpl(&sBeginEnd, false); }
      virtual void ProxyREnd(ConstIteratorBase& i) const { i.SetImpl(&sBeginEnd, false); }

      virtual size_t ProxySize() const { return 0; }
      virtual bool   ProxyEmpty() const { return true; }

      virtual void*  ProxyByName(const std::string& /*name*/) const { return 0; }
      virtual void*  ProxyByTypeInfo(const std::type_info& /*ti*/) const { return 0; }
   };

   EmptyContainerImpl::iterator EmptyContainerImpl::sBeginEnd;
}

//-------------------------------------------------------------------------------
const Reflex::EmptyCont_Type_t & Reflex::Dummy::EmptyContainer() {
//-------------------------------------------------------------------------------
// static wrapper for an empty container.
   static EmptyContainerImpl sEmptyCont;
   return sEmptyCont;
}


//-------------------------------------------------------------------------------
Reflex::Any & Reflex::Dummy::Any() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty Any object 
   static Reflex::Any i;
   if (i) i.Clear();
   return i;
}


//-------------------------------------------------------------------------------
const Reflex::Object & Reflex::Dummy::Object() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty Object 
   return Get< Reflex::Object >();
}


//-------------------------------------------------------------------------------
const Reflex::Type & Reflex::Dummy::Type() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty Type 
   return Get< Reflex::Type >();
}


//-------------------------------------------------------------------------------
const Reflex::TypeTemplate & Reflex::Dummy::TypeTemplate() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty TypeTemplate 
   return Get< Reflex::TypeTemplate >();
}


//-------------------------------------------------------------------------------
const Reflex::Base & Reflex::Dummy::Base() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty Base 
   return Get< Reflex::Base >();
}


//-------------------------------------------------------------------------------
const Reflex::PropertyList & Reflex::Dummy::PropertyList() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty PropertyList 
   return Get< Reflex::PropertyList >();
}


//-------------------------------------------------------------------------------
const Reflex::Member & Reflex::Dummy::Member() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty Member 
   return Get< Reflex::Member >();
}


//-------------------------------------------------------------------------------
const Reflex::MemberTemplate & Reflex::Dummy::MemberTemplate() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty MemberTemplate 
   return Get< Reflex::MemberTemplate >();
}


//-------------------------------------------------------------------------------
const Reflex::Scope & Reflex::Dummy::Scope() {
//-------------------------------------------------------------------------------
   // static wrapper for an empty Scope 
   return Get< Reflex::Scope >();
}


//-------------------------------------------------------------------------------
const std::string & Reflex::Argv0() {
//-------------------------------------------------------------------------------
// Return the name of the package.
   static std::string str = "REFLEX";
   return str;
}


//-------------------------------------------------------------------------------
namespace {
   Reflex::Instance initialise;
}
//-------------------------------------------------------------------------------



