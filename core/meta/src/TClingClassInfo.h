// @(#)root/core/meta:$Id$
// Author: Paul Russo   30/07/2012

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TClingClassInfo
#define ROOT_TClingClassInfo

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TClingClassInfo                                                      //
//                                                                      //
// Emulation of the CINT ClassInfo class.                               //
//                                                                      //
// The CINT C++ interpreter provides an interface to metadata about     //
// a class through the ClassInfo class.  This class provides the same   //
// functionality, using an interface as close as possible to ClassInfo  //
// but the class metadata comes from the Clang C++ compiler, not CINT.  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TClingMethodInfo.h"

#include <vector>
#include <string>

namespace cling {
   class Interpreter;
}

namespace clang {
   class CXXMethodDecl;
}

namespace ROOT {
   namespace TMetaUtils {
      class TNormalizedCtxt;
   }
}

class TClingClassInfo {

private:

   cling::Interpreter   *fInterp; // Cling interpreter, we do *not* own.
   bool                  fFirstTime; // We need to skip the first increment to support the cint Next() semantics.
   bool                  fDescend; // Flag for signaling the need to descend on this advancement.
   clang::DeclContext::decl_iterator fIter; // Current decl in scope.
   const clang::Decl    *fDecl; // Current decl, we do *not* own.
   const clang::Type    *fType; // Type representing the decl (conserves typedefs like Double32_t). (we do *not* own)
   std::vector<clang::DeclContext::decl_iterator> fIterStack; // Recursion stack for traversing nested scopes.
   std::string           fTitle; // The meta info for the class.
   mutable int           fNMethods; // Number of methods, assuming fNDecls is correct
   mutable llvm::SmallVector<const clang::Decl*, 2> fLastDeclForNMethods; // Last decls in the decl contexts during last update of fNMethods
   
   std::string           fDeclFileName; // Name of the file where the underlying entity is declared.

   explicit TClingClassInfo() /* = delete */; // NOT IMPLEMENTED
   TClingClassInfo &operator=(const TClingClassInfo &) /* = delete */; // NOT IMPLEMENTED
public: // Types

   enum MatchMode {
      ExactMatch = 0,
      ConversionMatch = 1,
      ConversionMatchBytecode = 2
   };
   enum InheritanceMode {
      InThisScope = 0,
      WithInheritance = 1
   };

public:

   explicit TClingClassInfo(cling::Interpreter *);
   explicit TClingClassInfo(cling::Interpreter *, const char *);
   explicit TClingClassInfo(cling::Interpreter *, const clang::Type &);

   long                 ClassProperty() const;
   void                 Delete(void *arena) const;
   void                 DeleteArray(void *arena, bool dtorOnly) const;
   void                 Destruct(void *arena) const;
   const clang::Decl   *GetDecl() const { return fDecl; } // Underlying representation without Double32_t
   TClingMethodInfo     GetMethod(const char *fname, const char *proto,
                                  long *poffset, MatchMode mode = ConversionMatch,
                                  InheritanceMode imode = WithInheritance) const;
   TClingMethodInfo     GetMethodWithArgs(const char *fname, const char *arglist,
                                  long *poffset, MatchMode mode = ConversionMatch,
                                  InheritanceMode imode = WithInheritance) const;
   int                  GetMethodNArg(const char *method, const char *proto) const;
   long                 GetOffset(const clang::CXXMethodDecl* md) const;
   const clang::Type   *GetType() const { return fType; } // Underlying representation with Double32_t
   bool                 HasDefaultConstructor() const;
   bool                 HasMethod(const char *name) const;
   void                 Init(const char *name);
   void                 Init(const clang::Decl*);
   void                 Init(int tagnum);
   void                 Init(const clang::Type &);
   bool                 IsBase(const char *name) const;
   static bool          IsEnum(cling::Interpreter *interp, const char *name);
   bool                 IsLoaded() const;
   bool                 IsValid() const;
   bool                 IsValidMethod(const char *method, const char *proto, long *offset) const;
   int                  InternalNext();
   int                  Next();
   void                *New() const;
   void                *New(int n) const;
   void                *New(int n, void *arena) const;
   void                *New(void *arena) const;
   int                  NMethods() const;
   long                 Property() const;
   int                  RootFlag() const;
   int                  Size() const;
   long                 Tagnum() const;
   const char          *FileName();
   const char          *FullName(const ROOT::TMetaUtils::TNormalizedCtxt &normCtxt) const;
   const char          *Name() const;
   const char          *Title();
   const char          *TmpltName() const;

};

#endif // ROOT_TClingClassInfo
