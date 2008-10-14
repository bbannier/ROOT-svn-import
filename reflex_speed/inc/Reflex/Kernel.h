// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Kernel
#define Reflex_Kernel

// These macros will allow selection on exported symbols
// taken from http://www.nedprod.com/programs/gccvisibility.html
// Shared library support

#if __GNUC__ >= 4 && !defined(__CINT__)
#  define GCC_HASCLASSVISIBILITY
#endif

#if !defined(REFLEX_DLL_VETO) && !defined(REFLEX_DLL) && !defined(__CINT__)
// we build Reflex as DLL by default, #define REFLEX_DLL_VETO to hide visibility / dllim/export code
#  define REFLEX_DLL
#endif

#ifdef _MSC_VER
#  define RFLX_IMPORT __declspec(dllimport)
#  define RFLX_EXPORT __declspec(dllexport)
#  define RFLX_DLLLOCAL
#  define RFLX_DLLPUBLIC
#else
#  ifdef GCC_HASCLASSVISIBILITY
#    define RFLX_EXPORT __attribute__((visibility("default")))
#    define RFLX_DLLLOCAL __attribute__((visibility("hidden")))
#    define RFLX_DLLPUBLIC __attribute__((visibility("default")))
#  else
#    define RFLX_EXPORT
#    define RFLX_DLLLOCAL
#    define RFLX_DLLPUBLIC
#  endif
#  define RFLX_IMPORT
#endif

// Define RFLX_API for DLL builds
#ifdef REFLEX_DLL
#  ifdef REFLEX_BUILD
#    define RFLX_API RFLX_EXPORT
#  else
#    define RFLX_API RFLX_IMPORT
#  endif // REFLEX_BUILD
#else
#  define RFLX_API
#endif // REFLEX_DLL

// Throwable classes must always be visible on GCC in all binaries
#ifdef _WIN32
#  define RFLX_EXCEPTIONAPI(api) api
#elif defined(GCC_HASCLASSVISIBILITY)
#  define RFLX_EXCEPTIONAPI(api) RFLX_EXPORT
#else
#  define RFLX_EXCEPTIONAPI(api)
#endif
// end macros for symbol selection


// include config.h generated by autoconf
#if defined (HAVE_CONFIG)
#include "config.h"
#endif

#if defined (_AIX)
#define ANSICPP
#define NEED_STRCASECMP
#endif

#if defined(__alpha) && !defined(__linux)
#ifndef __USE_STD_IOSTREAM
#define __USE_STD_IOSTREAM
#endif
#endif

// windows.h and oracle define kEDConst
#ifdef kEDConst
#undef kEDConst
#endif

#ifdef _MSC_VER
// Some pragmas to avoid warnings in MSVisualC
// Disable warning C4786: identifier was truncated to '255' characters in the debug information
#pragma warning (disable : 4786)
// Disable warning C4291: no matching operator delete found; memory will not be freed if
// initialization throws an exception
#pragma warning (disable : 4291)
// Disable warning C4250: inheritance via dominance
#pragma warning (disable : 4250)
#endif

// some compilers define the macros below in limits
#include <limits>

#ifndef LONGLONG_MAX
#define LONGLONG_MAX 0x7FFFFFFFFFFFFFFFLL
#endif
#ifndef LONGLONG_MIN
#define LONGLONG_MIN 0x8000000000000000LL
#endif
#ifndef ULONGLONG_MAX
#define ULONGLONG_MAX 0xFFFFFFFFFFFFFFFFLL
#endif
#ifndef ULONGLONG_MIN
#define ULONGLONG_MIN 0x0000000000000000LL
#endif


// Include files
#include <string>
#include <utility>
#include <exception>
#include <vector>

namespace Reflex {

#if 0
   // The definition of Reflex::longlong is now removed.
   // It is not needed by the Reflex API, and it interfers
   // with user code defining its own longlong, because
   // the dictionaries pull it in via "using namespace Reflex".

   // Large integer definition depends of the platform
#if defined(_MSC_VER) && !defined(__CINT__)
   typedef __int64 longlong;
   typedef unsigned __int64 ulonglong;
#else
   typedef long long int longlong; /* */
   typedef unsigned long long int /**/ ulonglong;
#endif
#endif

   // forward declarations
   class Any;
   class Type;
   class Base;
   class Scope;
   class Object;
   class Member;
   class PropertyList;
   class TypeTemplate;
   class MemberTemplate;
   class TemplateArgument;
   template <typename T> class ConstIterator;
   template <typename T> class ConstReverseIterator;

   namespace Internal {
      class IContainerImpl;
   }
   typedef Internal::IContainerImpl EmptyCont_Type_t;

   typedef ConstIterator<Type> Type_Iterator;
   typedef ConstIterator<Scope> Scope_Iterator;
   typedef ConstIterator<Member> Member_Iterator;
   typedef ConstIterator<Base> Base_Iterator;
   typedef ConstIterator<TypeTemplate> TypeTemplate_Iterator;
   typedef ConstIterator<MemberTemplate> MemberTemplate_Iterator;
   typedef ConstIterator<TemplateArgument> TemplateArgument_Iterator;
   typedef ConstIterator<std::string> String_Iterator;

   typedef ConstReverseIterator<Type> Reverse_Type_Iterator;
   typedef ConstReverseIterator<Scope> Reverse_Scope_Iterator;
   typedef ConstReverseIterator<Member> Reverse_Member_Iterator;
   typedef ConstReverseIterator<Base> Reverse_Base_Iterator;
   typedef ConstReverseIterator<TypeTemplate> Reverse_TypeTemplate_Iterator;
   typedef ConstReverseIterator<MemberTemplate> Reverse_MemberTemplate_Iterator;
   typedef ConstReverseIterator<TemplateArgument> Reverse_TemplateArgument_Iterator;
   typedef ConstReverseIterator<std::string> Reverse_String_Iterator;

   namespace Dummy {
      RFLX_API const EmptyCont_Type_t & EmptyContainer();
      RFLX_API Any & Any();
      RFLX_API const Object & Object();
      RFLX_API const Type & Type();
      RFLX_API const TypeTemplate & TypeTemplate();
      RFLX_API const Base & Base();
      RFLX_API const PropertyList & PropertyList();
      RFLX_API const Member & Member();
      RFLX_API const MemberTemplate & MemberTemplate();
      RFLX_API const Scope & Scope();
      template< class T > inline const T & Get() {
         static T t;
         return t;
      }
   }


   /** some general information about the Reflex package */
   class RFLX_API Instance {
   public:

      /** default constructor */
      Instance() {}

      /** destructor */
      ~Instance();

   private:
      void Shutdown();

   }; // struct Reflex

   /** the Name of the package - used for messages */
   RFLX_API const std::string & Argv0(); // returns "Reflex";

   // these defines are used for the modifiers they are used in the following
   // classes
   // BA = BASE
   // CL = kETClass
   // FU = kETFunction
   // DM = kETDataMember
   // FM = kETFunctionMember
   // TY = ETYPE
   // ME = MEMBER
   //                                BA  CL  DM  FM  TY  ME
   enum EENTITY_DESCRIPTION {
      kEDPublic          = (1<<0),  //  X       X   X       X
      kEDProtected       = (1<<1),  //  X       X   X       X
      kEDPrivate         = (1<<2),  //  X       X   X       X
      kEDRegister        = (1<<3),  //          X   X       X
      kEDStatic          = (1<<4),  //          X   X       X
      kEDConstructor     = (1<<5),  //              X       X
      kEDDestructor      = (1<<6) , //              X       X
      kEDExplicit        = (1<<7),  //              X       X
      kEDExtern          = (1<<8) , //          X   X       X
      kEDCopyConstructor = (1<<9) , //              X       X
      kEDOperator        = (1<<10), //              X       X
      kEDInline          = (1<<11), //              X       X
      kEDConverter       = (1<<12), //              X       X
      kEDAuto            = (1<<13), //          X           X
      kEDMutable         = (1<<14), //          X           X
      kEDConst           = (1<<15), //          X       X   X
      kEDVolatile        = (1<<16), //          X       X   X
      kEDReference       = (1<<17), //          X           X
      kEDAbstract        = (1<<18), //      X       X   X
      kEDVirtual         = (1<<19), //  X   X           X
      kEDTransient       = (1<<20), //          X           X
      kEDArtificial      = (1<<21), //  X   X   X   X   X   X
      // the bits 31 - 28 are reserved for template default arguments
      kEDTemplateDefaults1  = (0<<31)&(0<<30)&(0<<29)&(1<<28),
      kEDTemplateDefaults2  = (0<<31)&(0<<30)&(1<<29)&(0<<28),
      kEDTemplateDefaults3  = (0<<31)&(0<<30)&(1<<29)&(1<<28),
      kEDTemplateDefaults4  = (0<<31)&(1<<30)&(0<<29)&(0<<28),
      kEDTemplateDefaults5  = (0<<31)&(1<<30)&(0<<29)&(1<<28),
      kEDTemplateDefaults6  = (0<<31)&(1<<30)&(1<<29)&(0<<28),
      kEDTemplateDefaults7  = (0<<31)&(1<<30)&(1<<29)&(1<<28),
      kEDTemplateDefaults8  = (1<<31)&(0<<30)&(0<<29)&(0<<28),
      kEDTemplateDefaults9  = (1<<31)&(0<<30)&(0<<29)&(1<<28),
      kEDTemplateDefaults10 = (1<<31)&(0<<30)&(1<<29)&(0<<28),
      kEDTemplateDefaults11 = (1<<31)&(0<<30)&(1<<29)&(1<<28),
      kEDTemplateDefaults12 = (1<<31)&(1<<30)&(0<<29)&(0<<28),
      kEDTemplateDefaults13 = (1<<31)&(1<<30)&(0<<29)&(1<<28),
      kEDTemplateDefaults14 = (1<<31)&(1<<30)&(1<<29)&(0<<28),
      kEDTemplateDefaults15 = (1<<31)&(1<<30)&(1<<29)&(1<<28)
   };


   /** enum for printing names */
   enum EENTITY_HANDLING {
      kFinal     = (1<<0),
      kQualified = (1<<1),
      kScoped    = (1<<2),
      kF         = kFinal,
      kQ         = kQualified,
      kS         = kScoped
   };


   /** enum containing all possible types and scopes */
   enum ETYPE {
      kETClass,
      kETStruct,
      kETEnum,
      kETUnion,
      kETNamespace,
      kETScopeEndTag = kETNamespace,

      kETFunction,
      kETArray,
      kETFundamental,
      kETPointer,
      kETPointerToMember,
      kETTypedef,
      kETTypeTemplateInstance,
      kETMemberTemplateInstance,
      kETDataMember,
      kETFunctionMember,
      kETUnresolved,
      kETNIL
   };


   /** the max unsigned int */
   size_t NPos();

   const std::string& TYPEName(ETYPE type);


   /**
   * typedef for function member type (necessary for return value of
   * getter function)
   */
   typedef void * (* StubFunction) (void *, const std::vector < void * > &, void *);

   /** typedef for function for Offset calculation */
   typedef size_t (* OffsetFunction) (void *);

   /** dummy types for type_info purposes */
   class RFLX_API NullType {};
   class RFLX_API UnknownType {};
   /** place holders for protected types */
   class RFLX_API ProtectedClass {};
   class RFLX_API ProtectedEnum {};
   class RFLX_API ProtectedStruct {};
   class RFLX_API ProtectedUnion {};
   /** place holders for private types */
   class RFLX_API PrivateClass {};
   class RFLX_API PrivateEnum {};
   class RFLX_API PrivateStruct {};
   class RFLX_API PrivateUnion {};
   /** place holders for unnamed types (also typeinfo purposes) */
   class RFLX_API UnnamedClass {};
   class RFLX_API UnnamedEnum {};
   class RFLX_API UnnamedNamespace {};
   class RFLX_API UnnamedStruct {};
   class RFLX_API UnnamedUnion {};

   /** exception classes */
   class RFLX_EXCEPTIONAPI(RFLX_API) RuntimeError : public std::exception {
   public:
      RuntimeError(const std::string& msg) : fMsg(Reflex::Argv0() + ": " + msg) { }
      ~RuntimeError() throw() {}
      virtual const char * what() const throw() { return fMsg.c_str();}
      std::string fMsg;
   };

} // namespace Reflex

namespace ROOT {
   namespace Reflex {
      using namespace ::Reflex;
   }
}


//-------------------------------------------------------------------------------
inline size_t Reflex::NPos() {
//-------------------------------------------------------------------------------
   return (size_t) -1;
}

// includes that should be available to all of Reflex
#include "Reflex/Container.h"
#include "Reflex/EntityProperty.h"

#endif // Reflex_Kernel

