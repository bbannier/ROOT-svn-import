// @(#)root/meta:$Id$
// Author: Fons Rademakers   01/03/96
// vim: sw=3

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// This class defines an interface to the CINT C/C++ interpreter made   //
// by Masaharu Goto from HP Japan.                                      //
//                                                                      //
// CINT is an almost full ANSI compliant C/C++ interpreter.             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TCint.h"
#include "TROOT.h"
#include "TApplication.h"
#include "TGlobal.h"
#include "TDataType.h"
#include "TClass.h"
#include "TClassEdit.h"
#include "TClassTable.h"
#include "TBaseClass.h"
#include "TDataMember.h"
#include "TMethod.h"
#include "TMethodArg.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TString.h"
#include "THashList.h"
#include "TOrdCollection.h"
#include "TVirtualPad.h"
#include "TSystem.h"
#include "TVirtualMutex.h"
#include "TError.h"
#include "TEnv.h"
#include "THashTable.h"
#include "RConfigure.h"
#include "compiledata.h"

#include "clang/AST/Decl.h"
#include "clang/AST/RecordLayout.h"
#include "clang/AST/Type.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"

#include "llvm/Support/raw_ostream.h"

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <limits.h>
#include <stdint.h>

#ifdef __APPLE__
#include <dlfcn.h>
#endif

using namespace std;

R__EXTERN int optind;


//______________________________________________________________________________
//
//
//

class tcling_ClassInfo {
public:
   ~tcling_ClassInfo();
   explicit tcling_ClassInfo();
   explicit tcling_ClassInfo(const char*);
   tcling_ClassInfo(const tcling_ClassInfo&);
   tcling_ClassInfo& operator=(const tcling_ClassInfo&);
   G__ClassInfo* GetClassInfo() const;
   clang::Decl* GetDecl() const;
   int GetIdx() const;
   bool IsValid() const;
private:
   G__ClassInfo* fClassInfo; // CINT class info for this class, we own.
   clang::Decl* fDecl; // Clang AST Node for this class, we do *not* own.
   int fIdx; // Index into tcling class table.
};

tcling_ClassInfo::~tcling_ClassInfo()
{
   delete fClassInfo;
   fClassInfo = 0;
   fDecl = 0;
}

tcling_ClassInfo::tcling_ClassInfo()
   : fClassInfo(0), fDecl(0), fIdx(-1)
{
}

tcling_ClassInfo::tcling_ClassInfo(const tcling_ClassInfo& rhs)
{
   fClassInfo = new G__ClassInfo(rhs.fClassInfo->Tagnum());
   fDecl = rhs.fDecl;
   fIdx = rhs.fIdx;
}

tcling_ClassInfo& tcling_ClassInfo::operator=(const tcling_ClassInfo& rhs)
{
   if (this != &rhs) {
      delete fClassInfo;
      fClassInfo = new G__ClassInfo(rhs.fClassInfo->Tagnum());
      fDecl = rhs.fDecl;
      fIdx = rhs.fIdx;
   }
   return *this;
}

tcling_ClassInfo::tcling_ClassInfo(const char* name)
   : fClassInfo(new G__ClassInfo(name)), fDecl(0), fIdx(-1)
{
   fprintf(stderr, "tcling_ClassInfo(name): looking up class: %s\n", name);
   std::multimap<const std::string, const clang::Decl*>::iterator iter =
      tcling_Dict::ClassNameToDecl()->find(name);
   if (iter != tcling_Dict::ClassNameToDecl()->end()) {
      fDecl = (clang::Decl*) iter->second;
      fprintf(stderr, "tcling_ClassInfo(name): found class: %s  Decl: 0x%016lx\n", name, (long) fDecl);
      std::map<const clang::Decl*, int>::iterator iter_idx =
         tcling_Dict::ClassDeclToIdx()->find(fDecl);
      if (iter_idx != tcling_Dict::ClassDeclToIdx()->end()) {
         fIdx = iter_idx->second;
      }
   }
}

G__ClassInfo* tcling_ClassInfo::GetClassInfo() const
{
   return fClassInfo;
}

clang::Decl* tcling_ClassInfo::GetDecl() const
{
   return fDecl;
}

int tcling_ClassInfo::GetIdx() const
{
   return fIdx;
}

bool tcling_ClassInfo::IsValid() const
{
   return fClassInfo && fDecl && (fIdx > -1) && (fIdx < (int) tcling_Dict::Classes()->size());
}

//______________________________________________________________________________
//
//
//

class tcling_BaseClassInfo {
public:
   ~tcling_BaseClassInfo();
   explicit tcling_BaseClassInfo(); // NOT IMPLEMENTED.
   explicit tcling_BaseClassInfo(tcling_ClassInfo*);
   tcling_BaseClassInfo(const tcling_BaseClassInfo&); // NOT IMPLEMENTED.
   tcling_BaseClassInfo& operator=(const tcling_BaseClassInfo&); // NOT IMPLEMENTED.
   G__BaseClassInfo* GetBaseClassInfo();
   clang::Decl* GetDecl();
private:
   G__BaseClassInfo* fBaseClassInfo; // CINT base class info, we own.
   clang::Decl* fDecl; // Current Clang AST Node for iterator, we do *not* own.
};

tcling_BaseClassInfo::~tcling_BaseClassInfo()
{
   delete fBaseClassInfo;
   fBaseClassInfo = 0;
   fDecl = 0;
}

tcling_BaseClassInfo::tcling_BaseClassInfo(tcling_ClassInfo* tcling_class_info)
   : fBaseClassInfo(new G__BaseClassInfo(*tcling_class_info->GetClassInfo())), fDecl(0)
{
}

G__BaseClassInfo* tcling_BaseClassInfo::GetBaseClassInfo()
{
   return fBaseClassInfo;
}

clang::Decl* tcling_BaseClassInfo::GetDecl()
{
   return fDecl;
}

//______________________________________________________________________________
//
//
//

class tcling_DataMemberInfo {
public:
   ~tcling_DataMemberInfo();
   explicit tcling_DataMemberInfo();
   explicit tcling_DataMemberInfo(tcling_ClassInfo*);
   tcling_DataMemberInfo(const tcling_DataMemberInfo&);
   tcling_DataMemberInfo& operator=(const tcling_DataMemberInfo&);
   G__DataMemberInfo* GetDataMemberInfo() const;
   G__ClassInfo* GetClassInfo() const;
   tcling_ClassInfo* GetTClingClassInfo() const;
   clang::Decl* GetDecl() const;
   int GetIdx() const;
   int ArrayDim() const;
   bool IsValid() const;
   int MaxIndex(int dim) const;
   bool Next();
   long Offset() const;
   long Property() const;
   long TypeProperty() const;
   int TypeSize() const;
   const char* TypeName() const;
   const char* TypeTrueName() const;
   const char* Name() const;
   const char* Title() const;
   const char* ValidArrayIndex() const;
private:
   void InternalNextValidMember();
private:
   //
   // CINT material.
   //
   /// CINT data member info, we own.
   G__DataMemberInfo* fDataMemberInfo;
   /// CINT class info, we own.
   G__ClassInfo* fClassInfo;
   //
   // Clang material.
   //
   /// Class we are iterating over, we own.
   tcling_ClassInfo* fTClingClassInfo;
   /// We need to skip the first increment.
   bool fFirstTime;
   /// Current decl.
   clang::DeclContext::decl_iterator fIter;
   /// Recursion stack for traversing nested transparent scopes.
   std::vector<clang::DeclContext::decl_iterator> fIterStack;
};

tcling_DataMemberInfo::~tcling_DataMemberInfo()
{
   // CINT material.
   delete fDataMemberInfo;
   fDataMemberInfo = 0;
   delete fClassInfo;
   fClassInfo = 0;
   // Clang material.
   delete fTClingClassInfo;
   fTClingClassInfo = 0;
}

tcling_DataMemberInfo::tcling_DataMemberInfo()
   : fDataMemberInfo(new G__DataMemberInfo())
   , fClassInfo(0)
   , fTClingClassInfo(0)
   , fFirstTime(true)
   , fIter(tcling_Dict::GetTranslationUnitDecl()->decls_begin())
{
   // Move to first global variable.
   InternalNextValidMember();
}

tcling_DataMemberInfo::tcling_DataMemberInfo(tcling_ClassInfo* tcling_class_info)
   : fDataMemberInfo(0)
   , fClassInfo(0)
   , fTClingClassInfo(0)
   , fFirstTime(true)
{
   if (!tcling_class_info || !tcling_class_info->IsValid()) {
      return;
   }
   fDataMemberInfo = new G__DataMemberInfo(*tcling_class_info->GetClassInfo());
   fClassInfo  = new G__ClassInfo(tcling_class_info->GetClassInfo()->Tagnum());
   fTClingClassInfo = new tcling_ClassInfo(*tcling_class_info);
   fIter = llvm::cast<clang::DeclContext>(tcling_class_info->GetDecl())->
      decls_begin();
   // Move to first data member.
   InternalNextValidMember();
}

tcling_DataMemberInfo::tcling_DataMemberInfo(const tcling_DataMemberInfo& rhs)
{
   fDataMemberInfo = new G__DataMemberInfo(*rhs.fDataMemberInfo);
   fClassInfo = new G__ClassInfo(rhs.fClassInfo->Tagnum());
   fTClingClassInfo = new tcling_ClassInfo(*rhs.fTClingClassInfo);
   fFirstTime = rhs.fFirstTime;
   fIter = rhs.fIter;
   fIterStack = rhs.fIterStack;
}

tcling_DataMemberInfo& tcling_DataMemberInfo::operator=(const tcling_DataMemberInfo& rhs)
{
   if (this != &rhs) {
      delete fDataMemberInfo;
      fDataMemberInfo = new G__DataMemberInfo(*rhs.fDataMemberInfo);
      delete fClassInfo;
      fClassInfo = new G__ClassInfo(rhs.fClassInfo->Tagnum());
      delete fTClingClassInfo;
      fTClingClassInfo = new tcling_ClassInfo(*rhs.fTClingClassInfo);
      fFirstTime = rhs.fFirstTime;
      fIter = rhs.fIter;
      fIterStack = rhs.fIterStack;
   }
   return *this;
}

G__DataMemberInfo* tcling_DataMemberInfo::GetDataMemberInfo() const
{
   return fDataMemberInfo;
}

G__ClassInfo* tcling_DataMemberInfo::GetClassInfo() const
{
   return fClassInfo;
}

tcling_ClassInfo* tcling_DataMemberInfo::GetTClingClassInfo() const
{
   return fTClingClassInfo;
}

clang::Decl* tcling_DataMemberInfo::GetDecl() const
{
   return *fIter;
}

int tcling_DataMemberInfo::ArrayDim() const
{
   //return fDataMemberInfo->ArrayDim();
   if (!IsValid()) {
      return -1;
   }
   // Sanity check the current data member.
   clang::Decl::Kind DK = fIter->getKind();
   if (
      (DK != clang::Decl::Field) &&
      (DK != clang::Decl::Var) &&
      (DK != clang::Decl::EnumConstant)
   ) {
      // Error, was not a data member, variable, or enumerator.
      return -1L;
   }
   if (DK == clang::Decl::EnumConstant) {
      // We know that an enumerator value does not have array type.
      return 0;
   }
   // To get this information we must count the number
   // of arry type nodes in the canonical type chain.
   clang::ValueDecl* VD = llvm::cast<clang::ValueDecl>(*fIter);
   clang::QualType QT = VD->getType().getCanonicalType();
   int cnt = 0;
   while (1) {
      if (QT->isArrayType()) {
         ++cnt;
         QT = llvm::cast<clang::ArrayType>(QT)->getElementType();
         continue;
      }
      else if (QT->isReferenceType()) {
         QT = llvm::cast<clang::ReferenceType>(QT)->getPointeeType();
         continue;
      }
      else if (QT->isPointerType()) {
         QT = llvm::cast<clang::PointerType>(QT)->getPointeeType();
         continue;
      }
      else if (QT->isMemberPointerType()) {
         QT = llvm::cast<clang::MemberPointerType>(QT)->getPointeeType();
         continue;
      }
      break;
   }
   return cnt;
}

bool tcling_DataMemberInfo::IsValid() const
{
   return *fIter;
}

int tcling_DataMemberInfo::MaxIndex(int dim) const
{
   //return fDataMemberInfo->MaxIndex(dim);
   if (!IsValid()) {
      return -1;
   }
   // Sanity check the current data member.
   clang::Decl::Kind DK = fIter->getKind();
   if (
      (DK != clang::Decl::Field) &&
      (DK != clang::Decl::Var) &&
      (DK != clang::Decl::EnumConstant)
   ) {
      // Error, was not a data member, variable, or enumerator.
      return -1L;
   }
   if (DK == clang::Decl::EnumConstant) {
      // We know that an enumerator value does not have array type.
      return 0;
   }
   // To get this information we must count the number
   // of arry type nodes in the canonical type chain.
   clang::ValueDecl* VD = llvm::cast<clang::ValueDecl>(*fIter);
   clang::QualType QT = VD->getType().getCanonicalType();
   int paran = ArrayDim();
   if ((dim < 0) || (dim >= paran)) {
      // Passed dimension is out of bounds.
      return -1;
   }
   int cnt = dim;
   int max = 0;
   while (1) {
      if (QT->isArrayType()) {
         if (cnt == 0) {
            if (const clang::ConstantArrayType* CAT =
                  llvm::dyn_cast<clang::ConstantArrayType>(QT)
            ) {
               max = static_cast<int>(CAT->getSize().getZExtValue());
            }
            else if (const clang::IncompleteArrayType* IAT =
                  llvm::dyn_cast<clang::IncompleteArrayType>(QT)
            ) {
               max = INT_MAX;
            }
            else {
               max = -1;
            }
            break;
         }
         --cnt;
         QT = llvm::cast<clang::ArrayType>(QT)->getElementType();
         continue;
      }
      else if (QT->isReferenceType()) {
         QT = llvm::cast<clang::ReferenceType>(QT)->getPointeeType();
         continue;
      }
      else if (QT->isPointerType()) {
         QT = llvm::cast<clang::PointerType>(QT)->getPointeeType();
         continue;
      }
      else if (QT->isMemberPointerType()) {
         QT = llvm::cast<clang::MemberPointerType>(QT)->getPointeeType();
         continue;
      }
      break;
   }
   return max;
}

void tcling_DataMemberInfo::InternalNextValidMember()
{
   //static std::string buf;
   // Move to next acceptable data member.
   while (1) {
      // Reject members we do not want, and recurse into
      // transparent contexts.
      while (*fIter) {
         // Valid decl, recurse into it, accept it, or reject it.
         clang::Decl::Kind DK = fIter->getKind();
         if (DK == clang::Decl::Enum) {
            // Recurse down into a transparent context.
            //buf = "recurse into: ";
            //buf += fIter->getDeclKindName();
            //if (llvm::dyn_cast<clang::NamedDecl>(*fIter)) {
            //   buf += " " + llvm::dyn_cast<clang::NamedDecl>(*fIter)->getNameAsString();
            //}
            //if (llvm::dyn_cast<clang::DeclContext>(*fIter)) {
            //   if (llvm::dyn_cast<clang::DeclContext>(*fIter)->
            //          isTransparentContext()
            //   ) {
            //      buf += " transparent";
            //   }
            //}
            //fprintf(stderr, "%s\n", buf.c_str());
            fIterStack.push_back(fIter);
            fIter = llvm::cast<clang::DeclContext>(*fIter)->decls_begin();
            continue;
         }
         else if (
            (DK == clang::Decl::Field) ||
            (DK == clang::Decl::EnumConstant) ||
            (DK == clang::Decl::Var)
         ) {
            // We will process these kinds of members.
            break;
         }
         // Rejected, next member.
         //buf = "rejected: ";
         //buf += fIter->getDeclKindName();
         //if (llvm::dyn_cast<clang::NamedDecl>(*fIter)) {
         //   buf += " " + llvm::dyn_cast<clang::NamedDecl>(*fIter)->getNameAsString();
         //}
         //if (llvm::dyn_cast<clang::DeclContext>(*fIter)) {
         //   if (llvm::dyn_cast<clang::DeclContext>(*fIter)->
         //          isTransparentContext()
         //   ) {
         //      buf += " transparent";
         //   }
         //}
         //fprintf(stderr, "%s\n", buf.c_str());
         ++fIter;
      }
      // Accepted member, or at end of decl context.
      if (!*fIter && fIterStack.size()) {
         // End of decl context, and we have more to go.
         //fprintf(stderr, "pop stack:\n");
         fIter = fIterStack.back();
         fIterStack.pop_back();
         ++fIter;
         continue;
      }
      // Accepted member, or at end of outermost decl context.
      //if (*fIter) {
         //buf = "accepted: ";
         //buf += fIter->getDeclKindName();
         //if (llvm::dyn_cast<clang::NamedDecl>(*fIter)) {
         //   buf += " " + llvm::dyn_cast<clang::NamedDecl>(*fIter)->getNameAsString();
         //}
         //if (llvm::dyn_cast<clang::DeclContext>(*fIter)) {
         //   if (llvm::dyn_cast<clang::DeclContext>(*fIter)->
         //          isTransparentContext()
         //   ) {
         //      buf += " transparent";
         //   }
         //}
         //fprintf(stderr, "%s\n", buf.c_str());
      //}
      //else {
         //fprintf(stderr, "end of outermost decl context:\n");
      //}
      break;
   }
}

bool tcling_DataMemberInfo::Next()
{
   //return fDataMemberInfo->Next();
   if (!fIterStack.size() && !*fIter) {
      // Terminate early if we are already invalid.
      //fprintf(stderr, "Next: early termination!\n");
      return false;
   }
   if (fFirstTime) {
      // No increment for first data member, the cint interface is awkward.
      //fprintf(stderr, "Next: first time!\n");
      fFirstTime = false;
   }
   else {
      // Move to next data member.
      ++fIter;
      InternalNextValidMember();
   }
   // Accepted member, or at end of outermost decl context.
   if (!*fIter) {
      // We are now invalid, return that.
      return false;
   }
   // We are now pointing at the next data member, return that we are valid.
   return true;
}

long tcling_DataMemberInfo::Offset() const
{
   //return fDataMemberInfo->Offset();
   if (!IsValid()) {
      return -1L;
   }
   // Sanity check the current data member.
   clang::Decl::Kind DK = fIter->getKind();
   if (
      (DK != clang::Decl::Field) &&
      (DK != clang::Decl::Var) &&
      (DK != clang::Decl::EnumConstant)
   ) {
      // Error, was not a data member, variable, or enumerator.
      return -1L;
   }
   if (DK == clang::Decl::Field) {
      // The current member is a non-static data member.
      const clang::FieldDecl* FD = llvm::dyn_cast<clang::FieldDecl>(*fIter);
      clang::ASTContext& Context = FD->getASTContext();
      const clang::RecordDecl* RD = FD->getParent();
      const clang::ASTRecordLayout& Layout = Context.getASTRecordLayout(RD);
      uint64_t bits = Layout.getFieldOffset(FD->getFieldIndex());
      int64_t offset = Context.toCharUnitsFromBits(bits).getQuantity();
      return static_cast<long>(offset);
   }
   // The current member is static data member, enumerator constant,
   // or a global variable.
   // FIXME: We are supposed to return the address of the storage
   //        for the member here, only the interpreter knows that.
   return -1L;
}

long tcling_DataMemberInfo::Property() const
{
   return fDataMemberInfo->Property();
}

long tcling_DataMemberInfo::TypeProperty() const
{
   return fDataMemberInfo->Type()->Property();
}

int tcling_DataMemberInfo::TypeSize() const
{
   //return fDataMemberInfo->Type()->Size();
   if (!IsValid()) {
      return -1L;
   }
   // Sanity check the current data member.
   clang::Decl::Kind DK = fIter->getKind();
   if (
      (DK != clang::Decl::Field) &&
      (DK != clang::Decl::Var) &&
      (DK != clang::Decl::EnumConstant)
   ) {
      // Error, was not a data member, variable, or enumerator.
      return -1L;
   }
   clang::ValueDecl* VD = llvm::dyn_cast<clang::ValueDecl>(*fIter);
   clang::QualType QT = VD->getType();
   if (QT->isIncompleteType()) {
      // We cannot determine the size of forward-declared types.
      return -1L;
   }
   clang::ASTContext& Context = fIter->getASTContext();
   return static_cast<int>(Context.getTypeSizeInChars(QT).getQuantity());
}

const char* tcling_DataMemberInfo::TypeName() const
{
   //return fDataMemberInfo->Type()->Name();
   static std::string buf;
   if (!IsValid()) {
      return 0;
   }
   clang::PrintingPolicy P(fIter->getASTContext().PrintingPolicy);
   P.AnonymousTagLocations = false;
   if (llvm::dyn_cast<clang::ValueDecl>(*fIter)) {
      buf = llvm::cast<clang::ValueDecl>(*fIter)->getType().getAsString(P);
      //llvm::cast<clang::ValueDecl>(*fIter)->getType().dump();
   }
   else {
      buf = "";
   }
   return buf.c_str();
}

const char* tcling_DataMemberInfo::TypeTrueName() const
{
   //return fDataMemberInfo->Type()->TrueName();
   static std::string buf;
   if (!IsValid()) {
      return 0;
   }
   clang::PrintingPolicy P(fIter->getASTContext().PrintingPolicy);
   P.AnonymousTagLocations = false;
   if (clang::dyn_cast<clang::ValueDecl>(*fIter)) {
      buf = clang::cast<clang::ValueDecl>(*fIter)->
         getType().getCanonicalType().getAsString(P);
      //llvm::cast<clang::ValueDecl>(*fIter)->getType().
      //   getCanonicalType().dump();
   }
   else {
      buf = "";
   }
   return buf.c_str();
}

const char* tcling_DataMemberInfo::Name() const
{
   //return fDataMemberInfo->Name();
   static std::string buf;
   if (!IsValid()) {
      return 0;
   }
   //buf = fIter->getDeclKindName();
   if (llvm::dyn_cast<clang::NamedDecl>(*fIter)) {
      buf = llvm::cast<clang::NamedDecl>(*fIter)->getNameAsString();
   }
   else {
      buf = "";
   }
#if 0
   {
      clang::SourceLocation Loc = fIter->getLocation();
   }
   {
      clang::SourceLocation LocStart = fIter->getLocStart();
   }
   {
      clang::SourceLocation LocEnd = fIter->getLocEnd();
   }
   {
      clang::SourceRange LocRange = fIter->getSourceRange();
      {
         clang::SourceLocation Loc = LocRange.getBegin();
         std::string empty;
         llvm::raw_string_ostream OS(empty);
         clang::CompilerInstance* CI = tcling_Dict::GetCI();
         Loc.print(OS, CI->getSourceManager());
         buf += " " + OS.str();
      }
      {
         clang::SourceLocation Loc = LocRange.getEnd();
         std::string empty;
         llvm::raw_string_ostream OS(empty);
         clang::CompilerInstance* CI = tcling_Dict::GetCI();
         Loc.print(OS, CI->getSourceManager());
         buf += " " + OS.str();
      }
   }
#endif // 0
   //if (llvm::dyn_cast<clang::DeclContext>(*fIter)) {
   //   if (llvm::cast<clang::DeclContext>(*fIter)->
   //          isTransparentContext()
   //   ) {
   //      buf += " transparent";
   //   }
   //}
   return buf.c_str();
}

const char* tcling_DataMemberInfo::Title() const
{
   return fDataMemberInfo->Title();
   static std::string buf;
   if (!IsValid()) {
      return 0;
   }
   if (fTClingClassInfo) {
      // We are a data member iterator.
      // FIXME:  Need to get this from the comment string.
      if (clang::dyn_cast<clang::FieldDecl>(*fIter)) {
         buf = clang::cast<clang::FieldDecl>(*fIter)->getNameAsString();
      }
      else {
         buf = "";
      }
      return buf.c_str();
   }
   // We are a global variable iterator.
   // FIXME:  Need to get this from the comment string.
   if (clang::dyn_cast<clang::VarDecl>(*fIter)) {
      buf = clang::cast<clang::VarDecl>(*fIter)->getNameAsString();
   }
   else {
      buf = "";
   }
   return buf.c_str();
}

const char* tcling_DataMemberInfo::ValidArrayIndex() const
{
   return fDataMemberInfo->ValidArrayIndex();
}

//______________________________________________________________________________
//
//
//

class tcling_TypeInfo {
public:
   ~tcling_TypeInfo();
   explicit tcling_TypeInfo();
   tcling_TypeInfo(const tcling_TypeInfo&);
   tcling_TypeInfo& operator=(const tcling_TypeInfo&);
   tcling_TypeInfo(G__value* val);
   G__TypeInfo* GetTypeInfo();
   G__ClassInfo* GetClassInfo();
   clang::Decl* GetDecl();
private:
   G__TypeInfo* fTypeInfo; // CINT type info, we own.
   G__ClassInfo* fClassInfo; // CINT class info, we own.
   clang::Decl* fDecl; // Clang AST Node for class, we do *not* own.
};

tcling_TypeInfo::~tcling_TypeInfo()
{
   delete fTypeInfo;
   fTypeInfo = 0;
   delete fClassInfo;
   fClassInfo = 0;
   fDecl = 0;
}

tcling_TypeInfo::tcling_TypeInfo()
   : fTypeInfo(new G__TypeInfo()), fClassInfo(0), fDecl(0)
{
}

tcling_TypeInfo::tcling_TypeInfo(const tcling_TypeInfo& rhs)
{
   fTypeInfo = new G__TypeInfo(*rhs.fTypeInfo);
   fClassInfo = new G__ClassInfo(rhs.fClassInfo->Tagnum());
   fDecl = rhs.fDecl;
}

tcling_TypeInfo& tcling_TypeInfo::operator=(const tcling_TypeInfo& rhs)
{
   if (this != &rhs) {
      delete fTypeInfo;
      fTypeInfo = new G__TypeInfo(*rhs.fTypeInfo);
      delete fClassInfo;
      fClassInfo = new G__ClassInfo(rhs.fClassInfo->Tagnum());
      fDecl = rhs.fDecl;
   }
   return *this;
}

tcling_TypeInfo::tcling_TypeInfo(G__value* val)
   : fTypeInfo(0), fClassInfo(0), fDecl(0)
{
   fTypeInfo = new G__TypeInfo(*val);
   int tagnum = fTypeInfo->Tagnum();
   if (tagnum != -1) {
      fClassInfo  = new G__ClassInfo(tagnum);
      const char* name = fClassInfo->Fullname();
      fprintf(stderr, "tcling_TypeInfo(name): looking up class: %s\n", name);
      std::multimap<const std::string, const clang::Decl*>::iterator iter =
         tcling_Dict::ClassNameToDecl()->find(name);
      if (iter != tcling_Dict::ClassNameToDecl()->end()) {
         fDecl = (clang::Decl*) iter->second;
         fprintf(stderr, "tcling_TypeInfo(name): found class: %s  Decl: 0x%016lx\n", name, (long) fDecl);
      }
   }
}

G__TypeInfo* tcling_TypeInfo::GetTypeInfo()
{
   return fTypeInfo;
}

G__ClassInfo* tcling_TypeInfo::GetClassInfo()
{
   return fClassInfo;
}

clang::Decl* tcling_TypeInfo::GetDecl()
{
   return fDecl;
}

//______________________________________________________________________________
//
//
//

class tcling_TypedefInfo {
public:
   ~tcling_TypedefInfo();
   explicit tcling_TypedefInfo();
   explicit tcling_TypedefInfo(const char*);
   explicit tcling_TypedefInfo(int);
   tcling_TypedefInfo(const tcling_TypedefInfo&);
   tcling_TypedefInfo& operator=(const tcling_TypedefInfo&);
   G__TypedefInfo* GetTypedefInfo();
   clang::Decl* GetDecl();
private:
   G__TypedefInfo* fTypedefInfo; // CINT typedef info for this class, we own.
   clang::Decl* fDecl; // Clang AST Node for this typedef, we do *not* own.
};

tcling_TypedefInfo::~tcling_TypedefInfo()
{
   delete fTypedefInfo;
   fTypedefInfo = 0;
   fDecl = 0;
}

tcling_TypedefInfo::tcling_TypedefInfo()
   : fTypedefInfo(0), fDecl(0)
{
}

tcling_TypedefInfo::tcling_TypedefInfo(const tcling_TypedefInfo& rhs)
{
   fTypedefInfo = new G__TypedefInfo(rhs.fTypedefInfo->Typenum());
   fDecl = rhs.fDecl;
}

tcling_TypedefInfo& tcling_TypedefInfo::operator=(const tcling_TypedefInfo& rhs)
{
   if (this != &rhs) {
      delete fTypedefInfo;
      fTypedefInfo = new G__TypedefInfo(rhs.fTypedefInfo->Typenum());
      fDecl = rhs.fDecl;
   }
   return *this;
}

tcling_TypedefInfo::tcling_TypedefInfo(const char* name)
   : fTypedefInfo(new G__TypedefInfo(name)), fDecl(0)
{
   //fprintf(stderr, "tcling_TypedefInfo(name): looking up typedef: %s\n", name);
   //std::multimap<const std::string, const clang::Decl*>::iterator iter =
   //tcling_Dict::ClassNameToDecl()->find(name);
   //if (iter != tcling_Dict::ClassNameToDecl()->end()) {
   //fDecl = (clang::Decl*) iter->second;
   //fprintf(stderr, "tcling_TypedefInfo(name): found typedef: %s  Decl: 0x%016lx\n", name, (long) fDecl);
   //}
}

tcling_TypedefInfo::tcling_TypedefInfo(int typenum)
   : fTypedefInfo(new G__TypedefInfo(typenum)), fDecl(0)
{
   //const char* name = fTypedefInfo->Name();
   //fprintf(stderr, "tcling_TypedefInfo(name): looking up typedef: %s\n", name);
   //std::multimap<const std::string, const clang::Decl*>::iterator iter =
   //tcling_Dict::ClassNameToDecl()->find(name);
   //if (iter != tcling_Dict::ClassNameToDecl()->end()) {
   //fDecl = (clang::Decl*) iter->second;
   //fprintf(stderr, "tcling_TypedefInfo(name): found typedef: %s  Decl: 0x%016lx\n", name, (long) fDecl);
   //}
}

G__TypedefInfo* tcling_TypedefInfo::GetTypedefInfo()
{
   return fTypedefInfo;
}

clang::Decl* tcling_TypedefInfo::GetDecl()
{
   return fDecl;
}

//______________________________________________________________________________
//
//
//

extern "C" int ScriptCompiler(const char* filename, const char* opt)
{
   return gSystem->CompileMacro(filename, opt);
}

extern "C" int IgnoreInclude(const char* fname, const char* expandedfname)
{
   return gROOT->IgnoreInclude(fname, expandedfname);
}

extern "C" void TCint_UpdateClassInfo(char* c, Long_t l)
{
   TCint::UpdateClassInfo(c, l);
}

extern "C" int TCint_AutoLoadCallback(char* c, char* l)
{
   ULong_t varp = G__getgvp();
   G__setgvp((Long_t)G__PVOID);
   string cls(c);
   int result =  TCint::AutoLoadCallback(cls.c_str(), l);
   G__setgvp(varp);
   return result;
}

extern "C" void* TCint_FindSpecialObject(char* c, G__ClassInfo* ci, void** p1, void** p2)
{
   return TCint::FindSpecialObject(c, ci, p1, p2);
}

//______________________________________________________________________________
//
//
//

#if 0
//______________________________________________________________________________
static void collect_comment(Preprocessor& PP, ExpectedData& ED)
{
   // Create a raw lexer to pull all the comments out of the main file.
   // We don't want to look in #include'd headers for expected-error strings.
   SourceManager& SM = PP.getSourceManager();
   FileID FID = SM.getMainFileID();
   if (SM.getMainFileID().isInvalid()) {
      return;
   }
   // Create a lexer to lex all the tokens of the main file in raw mode.
   const llvm::MemoryBuffer* FromFile = SM.getBuffer(FID);
   Lexer RawLex(FID, FromFile, SM, PP.getLangOptions());
   // Return comments as tokens, this is how we find expected diagnostics.
   RawLex.SetCommentRetentionState(true);
   Token Tok;
   Tok.setKind(tok::comment);
   while (Tok.isNot(tok::eof)) {
      RawLex.Lex(Tok);
      if (!Tok.is(tok::comment)) {
         continue;
      }
      std::string Comment = PP.getSpelling(Tok);
      if (Comment.empty()) {
         continue;
      }
      // Find all expected errors/warnings/notes.
      ParseDirective(&Comment[0], Comment.size(), ED, PP, Tok.getLocation());
   };
}
#endif // 0

//______________________________________________________________________________
//
//
//

int TCint_GenerateDictionary(const std::vector<std::string> &classes,
                             const std::vector<std::string> &headers,
                             const std::vector<std::string> &fwdDecls,
                             const std::vector<std::string> &unknown)
{
   //This function automatically creates the "LinkDef.h" file for templated
   //classes then executes CompileMacro on it.
   //The name of the file depends on the class name, and it's not generated again
   //if the file exist.
   if (classes.empty()) {
      return 0;
   }
   // Use the name of the first class as the main name.
   const std::string& className = classes[0];
   //(0) prepare file name
   TString fileName = "AutoDict_";
   std::string::const_iterator sIt;
   for (sIt = className.begin(); sIt != className.end(); sIt++) {
      if (*sIt == '<' || *sIt == '>' ||
            *sIt == ' ' || *sIt == '*' ||
            *sIt == ',' || *sIt == '&' ||
            *sIt == ':') {
         fileName += '_';
      }
      else {
         fileName += *sIt;
      }
   }
   if (classes.size() > 1) {
      Int_t chk = 0;
      std::vector<std::string>::const_iterator it = classes.begin();
      while ((++it) != classes.end()) {
         for (UInt_t cursor = 0; cursor != it->length(); ++cursor) {
            chk = chk * 3 + it->at(cursor);
         }
      }
      fileName += TString::Format("_%u", chk);
   }
   fileName += ".cxx";
   if (gSystem->AccessPathName(fileName) != 0) {
      //file does not exist
      //(1) prepare file data
      // If STL, also request iterators' operators.
      // vector is special: we need to check whether
      // vector::iterator is a typedef to pointer or a
      // class.
      static std::set<std::string> sSTLTypes;
      if (sSTLTypes.empty()) {
         sSTLTypes.insert("vector");
         sSTLTypes.insert("list");
         sSTLTypes.insert("deque");
         sSTLTypes.insert("map");
         sSTLTypes.insert("multimap");
         sSTLTypes.insert("set");
         sSTLTypes.insert("multiset");
         sSTLTypes.insert("queue");
         sSTLTypes.insert("priority_queue");
         sSTLTypes.insert("stack");
         sSTLTypes.insert("iterator");
      }
      std::vector<std::string>::const_iterator it;
      std::string fileContent("");
      for (it = headers.begin(); it != headers.end(); ++it) {
         fileContent += "#include \"" + *it + "\"\n";
      }
      for (it = unknown.begin(); it != unknown.end(); ++it) {
         TClass* cl = TClass::GetClass(it->c_str());
         if (cl && cl->GetDeclFileName()) {
            TString header(gSystem->BaseName(cl->GetDeclFileName()));
            TString dir(gSystem->DirName(cl->GetDeclFileName()));
            TString dirbase(gSystem->BaseName(dir));
            while (dirbase.Length() && dirbase != "."
                   && dirbase != "include" && dirbase != "inc"
                   && dirbase != "prec_stl") {
               gSystem->PrependPathName(dirbase, header);
               dir = gSystem->DirName(dir);
            }
            fileContent += TString("#include \"") + header + "\"\n";
         }
      }
      for (it = fwdDecls.begin(); it != fwdDecls.end(); ++it) {
         fileContent += "class " + *it + ";\n";
      }
      fileContent += "#ifdef __CINT__ \n";
      fileContent += "#pragma link C++ nestedclasses;\n";
      fileContent += "#pragma link C++ nestedtypedefs;\n";
      for (it = classes.begin(); it != classes.end(); ++it) {
         std::string n(*it);
         size_t posTemplate = n.find('<');
         std::set<std::string>::const_iterator iSTLType = sSTLTypes.end();
         if (posTemplate != std::string::npos) {
            n.erase(posTemplate, std::string::npos);
            if (n.compare(0, 5, "std::") == 0) {
               n.erase(0, 5);
            }
            iSTLType = sSTLTypes.find(n);
         }
         fileContent += "#pragma link C++ class ";
         fileContent +=    *it + "+;\n" ;
         fileContent += "#pragma link C++ class ";
         if (iSTLType != sSTLTypes.end()) {
            // STL class; we cannot (and don't need to) store iterators;
            // their shadow and the compiler's version don't agree. So
            // don't ask for the '+'
            fileContent +=    *it + "::*;\n" ;
         }
         else {
            // Not an STL class; we need to allow the I/O of contained
            // classes (now that we have a dictionary for them).
            fileContent +=    *it + "::*+;\n" ;
         }
         std::string oprLink("#pragma link C++ operators ");
         oprLink += *it;
         // Don't! Requests e.g. op<(const vector<T>&, const vector<T>&):
         // fileContent += oprLink + ";\n";
         if (iSTLType != sSTLTypes.end()) {
            if (n == "vector") {
               fileContent += "#ifdef G__VECTOR_HAS_CLASS_ITERATOR\n";
            }
            fileContent += oprLink + "::iterator;\n";
            fileContent += oprLink + "::const_iterator;\n";
            fileContent += oprLink + "::reverse_iterator;\n";
            if (n == "vector") {
               fileContent += "#endif\n";
            }
         }
      }
      fileContent += "#endif\n";
      //end(1)
      //(2) prepare the file
      FILE* filePointer;
      filePointer = fopen(fileName, "w");
      if (filePointer == NULL) {
         //can't open a file
         return 1;
      }
      //end(2)
      //write data into the file
      fprintf(filePointer, "%s", fileContent.c_str());
      fclose(filePointer);
   }
   //(3) checking if we can compile a macro, if not then cleaning
   Int_t oldErrorIgnoreLevel = gErrorIgnoreLevel;
   gErrorIgnoreLevel = kWarning; // no "Info: creating library..."
   Int_t ret = gSystem->CompileMacro(fileName, "k");
   gErrorIgnoreLevel = oldErrorIgnoreLevel;
   if (ret == 0) { //can't compile a macro
      return 2;
   }
   //end(3)
   return 0;
}

int TCint_GenerateDictionary(const std::string& className,
                             const std::vector<std::string> &headers,
                             const std::vector<std::string> &fwdDecls,
                             const std::vector<std::string> &unknown)
{
   //This function automatically creates the "LinkDef.h" file for templated
   //classes then executes CompileMacro on it.
   //The name of the file depends on the class name, and it's not generated again
   //if the file exist.
   std::vector<std::string> classes;
   classes.push_back(className);
   return TCint_GenerateDictionary(classes, headers, fwdDecls, unknown);
}

//______________________________________________________________________________
//
//
//

// It is a "fantom" method to synchronize user keyboard input
// and ROOT prompt line (for WIN32)
const char* fantomline = "TRint::EndOfLineAction();";

//______________________________________________________________________________
//
//
//

void* TCint::fgSetOfSpecials = 0;

ClassImp(TCint)

//______________________________________________________________________________
TCint::TCint(const char* name, const char* title) : TInterpreter(name, title), fSharedLibs(""), fSharedLibsSerial(-1), fGlobalsListSerial(-1)
{
   // Initialize the CINT interpreter interface.
   fMore      = 0;
   fPrompt[0] = 0;
   fMapfile   = 0;
   fRootmapFiles = 0;
   fLockProcessLine = kTRUE;
   // Disable the autoloader until it is explicitly enabled.
   G__set_class_autoloading(0);
   G__RegisterScriptCompiler(&ScriptCompiler);
   G__set_ignoreinclude(&IgnoreInclude);
   G__InitUpdateClassInfo(&TCint_UpdateClassInfo);
   G__InitGetSpecialObject(&TCint_FindSpecialObject);
   // check whether the compiler is available:
   char* path = gSystem->Which(gSystem->Getenv("PATH"), gSystem->BaseName(COMPILER));
   if (path && path[0]) {
      G__InitGenerateDictionary(&TCint_GenerateDictionary);
   }
   delete[] path;
   ResetAll();
#ifndef R__WIN32
   optind = 1;  // make sure getopt() works in the main program
#endif
   // Make sure that ALL macros are seen as C++.
   G__LockCpp();
   // Initialize for ROOT:
   // Disallow the interpretation of Rtypes.h, TError.h and TGenericClassInfo.h
   ProcessLine("#define ROOT_Rtypes 0");
   ProcessLine("#define ROOT_TError 0");
   ProcessLine("#define ROOT_TGenericClassInfo 0");
   TString include;
   // Add the root include directory to list searched by default
#ifndef ROOTINCDIR
   include = gSystem->Getenv("ROOTSYS");
   include.Append("/include");
#else
   include = ROOTINCDIR;
#endif
   TCint::AddIncludePath(include);
   // Allow the usage of ClassDef and ClassImp in interpreted macros
   // if RtypesCint.h can be found (think of static executable without include/)
   char* whichTypesCint = gSystem->Which(include, "RtypesCint.h");
   if (whichTypesCint) {
      ProcessLine("#include <RtypesCint.h>");
      delete[] whichTypesCint;
   }
}

//______________________________________________________________________________
TCint::~TCint()
{
   // Destroy the CINT interpreter interface.
   if (fMore != -1) {
      // only close the opened files do not free memory:
      // G__scratch_all();
      G__close_inputfiles();
   }
   delete fMapfile;
   delete fRootmapFiles;
   gCint = 0;
#ifdef R__COMPLETE_MEM_TERMINATION
   G__scratch_all();
#endif
   //--
}

//______________________________________________________________________________
void TCint::ClearFileBusy()
{
   // Reset CINT internal state in case a previous action was not correctly
   // terminated by G__init_cint() and G__dlmod().
   R__LOCKGUARD(gCINTMutex);
   G__clearfilebusy(0);
}

//______________________________________________________________________________
void TCint::ClearStack()
{
   // Delete existing temporary values
   R__LOCKGUARD(gCINTMutex);
   G__clearstack();
}

//______________________________________________________________________________
Int_t TCint::InitializeDictionaries()
{
   // Initialize all registered dictionaries. Normally this is already done
   // by G__init_cint() and G__dlmod().
   R__LOCKGUARD(gCINTMutex);
   return G__call_setup_funcs();
}

//______________________________________________________________________________
void TCint::EnableAutoLoading()
{
   // Enable the automatic loading of shared libraries when a class
   // is used that is stored in a not yet loaded library. Uses the
   // information stored in the class/library map (typically
   // $ROOTSYS/etc/system.rootmap).
   R__LOCKGUARD(gCINTMutex);
   G__set_class_autoloading_callback(&TCint_AutoLoadCallback);
   G__set_class_autoloading(1);
   LoadLibraryMap();
}

//______________________________________________________________________________
void TCint::EndOfLineAction()
{
   // It calls a "fantom" method to synchronize user keyboard input
   // and ROOT prompt line.
   ProcessLineSynch(fantomline);
}

//______________________________________________________________________________
Bool_t TCint::IsLoaded(const char* filename) const
{
   // Return true if the file has already been loaded by cint.
   // We will try in this order:
   //   actual filename
   //   filename as a path relative to
   //            the include path
   //            the shared library path
   R__LOCKGUARD(gCINTMutex);
   G__SourceFileInfo file(filename);
   if (file.IsValid()) {
      return kTRUE;
   };
   char* next = gSystem->Which(TROOT::GetMacroPath(), filename, kReadPermission);
   if (next) {
      file.Init(next);
      delete [] next;
      if (file.IsValid()) {
         return kTRUE;
      };
   }
   TString incPath = gSystem->GetIncludePath(); // of the form -Idir1  -Idir2 -Idir3
   incPath.Append(":").Prepend(" ");
   incPath.ReplaceAll(" -I", ":");      // of form :dir1 :dir2:dir3
   while (incPath.Index(" :") != -1) {
      incPath.ReplaceAll(" :", ":");
   }
   incPath.Prepend(".:");
# ifdef CINTINCDIR
   TString cintdir = CINTINCDIR;
# else
   TString cintdir = "$(ROOTSYS)/cint";
# endif
   incPath.Append(":");
   incPath.Append(cintdir);
   incPath.Append("/include:");
   incPath.Append(cintdir);
   incPath.Append("/stl");
   next = gSystem->Which(incPath, filename, kReadPermission);
   if (next) {
      file.Init(next);
      delete [] next;
      if (file.IsValid()) {
         return kTRUE;
      };
   }
   next = gSystem->DynamicPathName(filename, kTRUE);
   if (next) {
      file.Init(next);
      delete [] next;
      if (file.IsValid()) {
         return kTRUE;
      };
   }
   return kFALSE;
}

//______________________________________________________________________________
Int_t TCint::Load(const char* filename, Bool_t system)
{
   // Load a library file in CINT's memory.
   // if 'system' is true, the library is never unloaded.
   R__LOCKGUARD2(gCINTMutex);
   int i;
   if (!system) {
      i = G__loadfile(filename);
   }
   else {
      i = G__loadsystemfile(filename);
   }
   UpdateListOfTypes();
   return i;
}

//______________________________________________________________________________
void TCint::LoadMacro(const char* filename, EErrorCode* error)
{
   // Load a macro file in CINT's memory.
   ProcessLine(Form(".L %s", filename), error);
   UpdateListOfTypes();
   UpdateListOfGlobals();
   UpdateListOfGlobalFunctions();
}

//______________________________________________________________________________
Long_t TCint::ProcessLine(const char* line, EErrorCode* error)
{
   // Let CINT process a command line.
   // If the command is executed and the result of G__process_cmd is 0,
   // the return value is the int value corresponding to the result of the command
   // (float and double return values will be truncated).
   Long_t ret = 0;
   if (gApplication) {
      if (gApplication->IsCmdThread()) {
         if (gGlobalMutex && !gCINTMutex && fLockProcessLine) {
            gGlobalMutex->Lock();
            if (!gCINTMutex) {
               gCINTMutex = gGlobalMutex->Factory(kTRUE);
            }
            gGlobalMutex->UnLock();
         }
         R__LOCKGUARD(fLockProcessLine ? gCINTMutex : 0);
         gROOT->SetLineIsProcessing();
         G__value local_res;
         G__setnull(&local_res);
         // It checks whether the input line contains the "fantom" method
         // to synchronize user keyboard input and ROOT prompt line
         if (strstr(line, fantomline)) {
            G__free_tempobject();
            TCint::UpdateAllCanvases();
         }
         else {
            int local_error = 0;
            int prerun = G__getPrerun();
            G__setPrerun(0);
            ret = G__process_cmd((char*)line, fPrompt, &fMore, &local_error, &local_res);
            G__setPrerun(prerun);
            if (local_error == 0 && G__get_return(&fExitCode) == G__RETURN_EXIT2) {
               ResetGlobals();
               gApplication->Terminate(fExitCode);
            }
            if (error) {
               *error = (EErrorCode)local_error;
            }
         }
         if (ret == 0) {
            // prevent overflow signal
            double resd = G__double(local_res);
            if (resd > LONG_MAX) {
               ret = LONG_MAX;
            }
            else if (resd < LONG_MIN) {
               ret = LONG_MIN;
            }
            else {
               ret = G__int_cast(local_res);
            }
         }
         gROOT->SetLineHasBeenProcessed();
      }
      else {
         ret = ProcessLineAsynch(line, error);
      }
   }
   else {
      if (gGlobalMutex && !gCINTMutex && fLockProcessLine) {
         gGlobalMutex->Lock();
         if (!gCINTMutex) {
            gCINTMutex = gGlobalMutex->Factory(kTRUE);
         }
         gGlobalMutex->UnLock();
      }
      R__LOCKGUARD(fLockProcessLine ? gCINTMutex : 0);
      gROOT->SetLineIsProcessing();
      G__value local_res;
      G__setnull(&local_res);
      int local_error = 0;
      int prerun = G__getPrerun();
      G__setPrerun(0);
      ret = G__process_cmd((char*)line, fPrompt, &fMore, &local_error, &local_res);
      G__setPrerun(prerun);
      if (local_error == 0 && G__get_return(&fExitCode) == G__RETURN_EXIT2) {
         ResetGlobals();
         exit(fExitCode);
      }
      if (error) {
         *error = (EErrorCode)local_error;
      }
      if (ret == 0) {
         // prevent overflow signal
         double resd = G__double(local_res);
         if (resd > LONG_MAX) {
            ret = LONG_MAX;
         }
         else if (resd < LONG_MIN) {
            ret = LONG_MIN;
         }
         else {
            ret = G__int_cast(local_res);
         }
      }
      gROOT->SetLineHasBeenProcessed();
   }
   return ret;
}

//______________________________________________________________________________
Long_t TCint::ProcessLineAsynch(const char* line, EErrorCode* error)
{
   // Let CINT process a command line asynch.
   return ProcessLine(line, error);
}

//______________________________________________________________________________
Long_t TCint::ProcessLineSynch(const char* line, EErrorCode* error)
{
   // Let CINT process a command line synchronously, i.e we are waiting
   // it will be finished.
   R__LOCKGUARD(fLockProcessLine ? gCINTMutex : 0);
   if (gApplication) {
      if (gApplication->IsCmdThread()) {
         return ProcessLine(line, error);
      }
      return 0;
   }
   return ProcessLine(line, error);
}

//______________________________________________________________________________
Long_t TCint::Calc(const char* line, EErrorCode* error)
{
   // Directly execute an executable statement (e.g. "func()", "3+5", etc.
   // however not declarations, like "Int_t x;").
   Long_t result;
#ifdef R__WIN32
   // Test on ApplicationImp not being 0 is needed because only at end of
   // TApplication ctor the IsLineProcessing flag is set to 0, so before
   // we can not use it.
   if (gApplication && gApplication->GetApplicationImp()) {
      while (gROOT->IsLineProcessing() && !gApplication) {
         Warning("Calc", "waiting for CINT thread to free");
         gSystem->Sleep(500);
      }
      gROOT->SetLineIsProcessing();
   }
#endif
   R__LOCKGUARD2(gCINTMutex);
   result = (Long_t) G__int_cast(G__calc((char*)line));
   if (error) {
      *error = (EErrorCode)G__lasterror();
   }
#ifdef R__WIN32
   if (gApplication && gApplication->GetApplicationImp()) {
      gROOT->SetLineHasBeenProcessed();
   }
#endif
   return result;
}

//______________________________________________________________________________
void TCint::PrintIntro()
{
   // Print CINT introduction and help message.
   Printf("\nCINT/ROOT C/C++ Interpreter version %s", G__cint_version());
   Printf("Type ? for help. Commands must be C++ statements.");
   Printf("Enclose multiple statements between { }.");
}

//______________________________________________________________________________
void TCint::SetGetline(const char * (*getlineFunc)(const char* prompt),
                       void (*histaddFunc)(const char* line))
{
   // Set a getline function to call when input is needed.
   G__SetGetlineFunc(getlineFunc, histaddFunc);
}


//______________________________________________________________________________
void TCint::RecursiveRemove(TObject* obj)
{
   // Delete object from CINT symbol table so it can not be used anymore.
   // CINT objects are always on the heap.
   R__LOCKGUARD(gCINTMutex);
   if (obj->IsOnHeap() && fgSetOfSpecials && !((std::set<TObject*>*)fgSetOfSpecials)->empty()) {
      std::set<TObject*>::iterator iSpecial = ((std::set<TObject*>*)fgSetOfSpecials)->find(obj);
      if (iSpecial != ((std::set<TObject*>*)fgSetOfSpecials)->end()) {
         DeleteGlobal(obj);
         ((std::set<TObject*>*)fgSetOfSpecials)->erase(iSpecial);
      }
   }
}

//______________________________________________________________________________
void TCint::Reset()
{
   // Reset the CINT state to the state saved by the last call to
   // TCint::SaveContext().
   R__LOCKGUARD(gCINTMutex);
   G__scratch_upto(&fDictPos);
}

//______________________________________________________________________________
void TCint::ResetAll()
{
   // Reset the CINT state to its initial state.
   R__LOCKGUARD(gCINTMutex);
   G__init_cint("cint +V");
   G__init_process_cmd();
}

//______________________________________________________________________________
void TCint::ResetGlobals()
{
   // Reset the CINT global object state to the state saved by the last
   // call to TCint::SaveGlobalsContext().
   R__LOCKGUARD(gCINTMutex);
   G__scratch_globals_upto(&fDictPosGlobals);
}

//______________________________________________________________________________
void TCint::ResetGlobalVar(void* obj)
{
   // Reset the CINT global object state to the state saved by the last
   // call to TCint::SaveGlobalsContext().
   R__LOCKGUARD(gCINTMutex);
   G__resetglobalvar(obj);
}

//______________________________________________________________________________
void TCint::RewindDictionary()
{
   // Rewind CINT dictionary to the point where it was before executing
   // the current macro. This function is typically called after SEGV or
   // ctlr-C after doing a longjmp back to the prompt.
   R__LOCKGUARD(gCINTMutex);
   G__rewinddictionary();
}

//______________________________________________________________________________
Int_t TCint::DeleteGlobal(void* obj)
{
   // Delete obj from CINT symbol table so it cannot be accessed anymore.
   // Returns 1 in case of success and 0 in case object was not in table.
   R__LOCKGUARD(gCINTMutex);
   return G__deleteglobal(obj);
}

//______________________________________________________________________________
void TCint::SaveContext()
{
   // Save the current CINT state.
   R__LOCKGUARD(gCINTMutex);
   G__store_dictposition(&fDictPos);
}

//______________________________________________________________________________
void TCint::SaveGlobalsContext()
{
   // Save the current CINT state of global objects.
   R__LOCKGUARD(gCINTMutex);
   G__store_dictposition(&fDictPosGlobals);
}

//______________________________________________________________________________
void TCint::UpdateListOfGlobals()
{
   // Update the list of pointers to global variables. This function
   // is called by TROOT::GetListOfGlobals().
   if (!gROOT->fGlobals) {
      // No globals registered yet, trigger it:
      gROOT->GetListOfGlobals();
      // It already called us again.
      return;
   }
   if (fGlobalsListSerial == G__DataMemberInfo::SerialNumber()) {
      return;
   }
   fGlobalsListSerial = G__DataMemberInfo::SerialNumber();

   R__LOCKGUARD2(gCINTMutex);
   tcling_DataMemberInfo* tcling_info = (tcling_DataMemberInfo*) DataMemberInfo_Factory();
   G__DataMemberInfo* t = tcling_info->GetDataMemberInfo();
   // Loop over all global vars known to cint.
   while (t->Next()) {
      if (t->IsValid() && t->Name()) {
         // Remove any old version in the list.
         {
            TGlobal* g = (TGlobal*) gROOT->fGlobals->FindObject(t->Name());
            if (g) {
               gROOT->fGlobals->Remove(g);
               delete g;
            }
         }
         gROOT->fGlobals->Add(new TGlobal((DataMemberInfo_t*) new G__DataMemberInfo(*t)));
      }
   }
}

//______________________________________________________________________________
void TCint::UpdateListOfGlobalFunctions()
{
   // Update the list of pointers to global functions. This function
   // is called by TROOT::GetListOfGlobalFunctions().
   if (!gROOT->fGlobalFunctions) {
      // No global functions registered yet, trigger it:
      gROOT->GetListOfGlobalFunctions();
      // We were already called by TROOT::GetListOfGlobalFunctions()
      return;
   }
   R__LOCKGUARD2(gCINTMutex);
   G__MethodInfo t, *a;
   void* vt = 0;
   while (t.Next()) {
      // if name cannot be obtained no use to put in list
      if (t.IsValid() && t.Name()) {
         Bool_t needToAdd = kTRUE;
         // first remove if already in list
         TList* listFuncs = ((THashTable*)(gROOT->fGlobalFunctions))->GetListForObject(t.Name());
         if (listFuncs && (vt = (void*)t.InterfaceMethod())) {
            Long_t prop = -1;
            TIter iFunc(listFuncs);
            TFunction* f = 0;
            Bool_t foundStart = kFALSE;
            while (needToAdd && (f = (TFunction*)iFunc())) {
               if (strcmp(f->GetName(), t.Name())) {
                  if (foundStart) {
                     break;
                  }
                  continue;
               }
               foundStart = kTRUE;
               if (vt == f->InterfaceMethod()) {
                  if (prop == -1) {
                     prop = t.Property();
                  }
                  needToAdd = !((prop & G__BIT_ISCOMPILED)
                                || t.GetMangledName() == f->GetMangledName());
               }
            }
         }
         if (needToAdd) {
            a = new G__MethodInfo(t);
            gROOT->fGlobalFunctions->Add(new TFunction(a));
         }
      }
   }
}

//______________________________________________________________________________
void TCint::UpdateListOfTypes()
{
   // Update the list of pointers to Datatype (typedef) definitions. This
   // function is called by TROOT::GetListOfTypes().
   R__LOCKGUARD2(gCINTMutex);
   // Remember the index of the last type that we looked at,
   // so that we don't keep reprocessing the same types.
   static int last_typenum = -1;
   // Also remember the count from the last time the dictionary
   // was rewound.  If it's been rewound since the last time we've
   // been called, then we recan everything.
   static int last_scratch_count = 0;
   int this_scratch_count = G__scratch_upto(0);
   if (this_scratch_count != last_scratch_count) {
      last_scratch_count = this_scratch_count;
      last_typenum = -1;
   }
   // Scan from where we left off last time.
   tcling_TypedefInfo* tcling_info = (tcling_TypedefInfo*) TypedefInfo_Factory(last_typenum);
   G__TypedefInfo* t = tcling_info->GetTypedefInfo();
   while (t->Next()) {
      const char* name = t->Name();
      if (gROOT && gROOT->fTypes && t->IsValid() && name) {
         TDataType* d = (TDataType*) gROOT->fTypes->FindObject(name);
         // only add new types, don't delete old ones with the same name
         // (as is done in UpdateListOfGlobals()),
         // this 'feature' is being used in TROOT::GetType().
         if (!d) {
            gROOT->fTypes->Add(new TDataType(new tcling_TypedefInfo(t->Typenum())));
         }
         last_typenum = t->Typenum();
      }
   }
}

//______________________________________________________________________________
void TCint::SetClassInfo(TClass* cl, Bool_t reload)
{
   // Set pointer to CINT's G__ClassInfo in TClass.
   R__LOCKGUARD2(gCINTMutex);
   if (!cl->fClassInfo || reload) {
      delete(G__ClassInfo*)cl->fClassInfo;
      cl->fClassInfo = 0;
      cl->fDecl = 0;
      std::string name(cl->GetName());
      if (!CheckClassInfo(name.c_str())) {
         // Try resolving all the typedefs (even Float_t and Long64_t)
         name =  TClassEdit::ResolveTypedef(name.c_str(), kTRUE);
         if (name == cl->GetName() || !CheckClassInfo(name.c_str())) {
            // Nothing found, nothing to do.
            return;
         }
      }
      G__ClassInfo* info = new G__ClassInfo(name.c_str());
      cl->fClassInfo = info;
      //{
      //std::multimap<const std::string, const clang::Decl*>::iterator B =
      //tcling_Dict::ClassNameToDecl()->begin();
      //std::multimap<const std::string, const clang::Decl*>::iterator E =
      //tcling_Dict::ClassNameToDecl()->end();
      //std::multimap<const std::string, const clang::Decl*>::iterator I = B;
      //if (tcling_Dict::ClassNameToDecl()->empty()) {
      //fprintf(stderr, "tcling_Dict::ClassNameToDecl() is empty!\n");
      //}
      //for (; I != E; ++I) {
      //fprintf(stderr, "table class: %s  decl: 0x%016lx\n", I->first.c_str(), (long) I->second);
      //}
      //}
      //fprintf(stderr, "looking up class: %s\n", name.c_str());
      std::multimap<const std::string, const clang::Decl*>::iterator iter =
         tcling_Dict::ClassNameToDecl()->find(name);
      if (iter != tcling_Dict::ClassNameToDecl()->end()) {
         cl->fDecl = (clang::Decl*) iter->second;
         //fprintf(stderr, "found class: %s  Decl: 0x%016lx\n", name.c_str(), (long) cl->fDecl);
      }
      Bool_t zombieCandidate = kFALSE;
      // In case a class contains an external enum, the enum will be seen as a
      // class. We must detect this special case and make the class a Zombie.
      // Here we assume that a class has at least one method.
      // We can NOT call TClass::Property from here, because this method
      // assumes that the TClass is well formed to do a lot of information
      // caching. The method SetClassInfo (i.e. here) is usually called during
      // the building phase of the TClass, hence it is NOT well formed yet.
      if (info->IsValid() &&
            !(info->Property() & (kIsClass | kIsStruct | kIsNamespace))) {
         zombieCandidate = kTRUE; // cl->MakeZombie();
      }
      if (!info->IsLoaded()) {
         if (info->Property() & (kIsNamespace)) {
            // Namespaces can have info but no corresponding CINT dictionary
            // because they are auto-created if one of their contained
            // classes has a dictionary.
            zombieCandidate = kTRUE; // cl->MakeZombie();
         }
         // this happens when no CINT dictionary is available
         delete info;
         cl->fClassInfo = 0;
      }
      if (zombieCandidate && !TClassEdit::IsSTLCont(cl->GetName())) {
         cl->MakeZombie();
      }
   }
}

//______________________________________________________________________________
Bool_t TCint::CheckClassInfo(const char* name, Bool_t autoload /*= kTRUE*/)
{
   // Checks if a class with the specified name is defined in CINT.
   // Returns kFALSE is class is not defined.
   // In the case where the class is not loaded and belongs to a namespace
   // or is nested, looking for the full class name is outputing a lots of
   // (expected) error messages.  Currently the only way to avoid this is to
   // specifically check that each level of nesting is already loaded.
   // In case of templates the idea is that everything between the outer
   // '<' and '>' has to be skipped, e.g.: aap<pipo<noot>::klaas>::a_class
   R__LOCKGUARD(gCINTMutex);
   Int_t nch = strlen(name) * 2;
   char* classname = new char[nch];
   strlcpy(classname, name, nch);
   char* current = classname;
   while (*current) {
      while (*current && *current != ':' && *current != '<') {
         current++;
      }
      if (!*current) {
         break;
      }
      if (*current == '<') {
         int level = 1;
         current++;
         while (*current && level > 0) {
            if (*current == '<') {
               level++;
            }
            if (*current == '>') {
               level--;
            }
            current++;
         }
         continue;
      }
      // *current == ':', must be a "::"
      if (*(current + 1) != ':') {
         Error("CheckClassInfo", "unexpected token : in %s", classname);
         delete [] classname;
         return kFALSE;
      }
      *current = '\0';
      G__ClassInfo info(classname);
      if (!info.IsValid()) {
         delete [] classname;
         return kFALSE;
      }
      *current = ':';
      current += 2;
   }
   strlcpy(classname, name, nch);
   int flag = 2;
   if (!autoload) {
      flag = 3;
   }
   Int_t tagnum = G__defined_tagname(classname, flag); // This function might modify the name (to add space between >>).
   if (tagnum >= 0) {
      G__ClassInfo info(tagnum);
      // If autoloading is off then Property() == 0 for autoload entries.
      if (!autoload && !info.Property()) {
         return kTRUE;
      }
      if (info.Property() & (G__BIT_ISENUM | G__BIT_ISCLASS | G__BIT_ISSTRUCT | G__BIT_ISUNION | G__BIT_ISNAMESPACE)) {
         // We are now sure that the entry is not in fact an autoload entry.
         delete [] classname;
         return kTRUE;
      }
   }
   G__TypedefInfo t(name);
   if (t.IsValid() && !(t.Property()&G__BIT_ISFUNDAMENTAL)) {
      delete [] classname;
      return kTRUE;
   }
   delete [] classname;
   return kFALSE;
}

//______________________________________________________________________________
void TCint::CreateListOfBaseClasses(TClass* cl)
{
   // Create list of pointers to base class(es) for TClass cl.
   R__LOCKGUARD2(gCINTMutex);
   if (!cl->fBase) {
      cl->fBase = new TList;
      G__BaseClassInfo t(*(G__ClassInfo*)cl->GetClassInfo()), *a;
      while (t.Next()) {
         // if name cannot be obtained no use to put in list
         if (t.IsValid() && t.Name()) {
            a = new G__BaseClassInfo(t);
            cl->fBase->Add(new TBaseClass(a, cl));
         }
      }
   }
}

//______________________________________________________________________________
void TCint::CreateListOfDataMembers(TClass* cl)
{
   // Create list of pointers to data members for TClass cl.
   R__LOCKGUARD2(gCINTMutex);
   if (!cl->fData) {
      cl->fData = new TList;
      G__DataMemberInfo t(*(G__ClassInfo*)cl->GetClassInfo()), *a;
      while (t.Next()) {
         // if name cannot be obtained no use to put in list
         if (t.IsValid() && t.Name() && strcmp(t.Name(), "G__virtualinfo")) {
            a = new G__DataMemberInfo(t);
            cl->fData->Add(new TDataMember(a, cl));
         }
      }
   }
}

//______________________________________________________________________________
void TCint::CreateListOfMethods(TClass* cl)
{
   // Create list of pointers to methods for TClass cl.
   R__LOCKGUARD2(gCINTMutex);
   if (!cl->fMethod) {
      cl->fMethod = new THashList;
      G__MethodInfo* a;
      G__MethodInfo t(*(G__ClassInfo*)cl->GetClassInfo());
      while (t.Next()) {
         // if name cannot be obtained no use to put in list
         if (t.IsValid() && t.Name()) {
            a = new G__MethodInfo(t);
            cl->fMethod->Add(new TMethod(a, cl));
         }
      }
   }
}

//______________________________________________________________________________
void TCint::UpdateListOfMethods(TClass* cl)
{
   // Update the list of pointers to method for TClass cl, if necessary
   if (cl->fMethod) {
      R__LOCKGUARD2(gCINTMutex);
      G__ClassInfo* info = (G__ClassInfo*)cl->GetClassInfo();
      if (!info || cl->fMethod->GetEntries() == info->NMethods()) {
         return;
      }
      delete cl->fMethod;
      cl->fMethod = 0;
   }
   CreateListOfMethods(cl);
}

//______________________________________________________________________________
void TCint::CreateListOfMethodArgs(TFunction* m)
{
   // Create list of pointers to method arguments for TMethod m.
   R__LOCKGUARD2(gCINTMutex);
   if (!m->fMethodArgs) {
      m->fMethodArgs = new TList;
      G__MethodArgInfo t(*(G__MethodInfo*)m->fInfo), *a;
      while (t.Next()) {
         // if type cannot be obtained no use to put in list
         if (t.IsValid() && t.Type()) {
            a = new G__MethodArgInfo(t);
            m->fMethodArgs->Add(new TMethodArg(a, m));
         }
      }
   }
}

//______________________________________________________________________________
Int_t TCint::GenerateDictionary(const char* classes, const char* includes /* = 0 */, const char* /* options  = 0 */)
{
   // Generate the dictionary for the C++ classes listed in the first
   // argmument (in a semi-colon separated list).
   // 'includes' contains a semi-colon separated list of file to
   // #include in the dictionary.
   // For example:
   //    gInterpreter->GenerateDictionary("vector<vector<float> >;list<vector<float> >","list;vector");
   // or
   //    gInterpreter->GenerateDictionary("myclass","myclass.h;myhelper.h");
   if (classes == 0 || classes[0] == 0) {
      return 0;
   }
   // Split the input list
   std::vector<std::string> listClasses;
   for (const char* current = classes, *prev = classes; *current != 0; ++current) {
      if (*current == ';') {
         listClasses.push_back(std::string(prev, current - prev));
         prev = current + 1;
      }
      else if (*(current + 1) == 0) {
         listClasses.push_back(std::string(prev, current + 1 - prev));
         prev = current + 1;
      }
   }
   std::vector<std::string> listIncludes;
   for (const char* current = includes, *prev = includes; *current != 0; ++current) {
      if (*current == ';') {
         listIncludes.push_back(std::string(prev, current - prev));
         prev = current + 1;
      }
      else if (*(current + 1) == 0) {
         listIncludes.push_back(std::string(prev, current + 1 - prev));
         prev = current + 1;
      }
   }
   // Generate the temporary dictionary file
   return TCint_GenerateDictionary(listClasses, listIncludes, std::vector<std::string>(), std::vector<std::string>());
}


//______________________________________________________________________________
TString TCint::GetMangledName(TClass* cl, const char* method,
                              const char* params)
{
   // Return the CINT mangled name for a method of a class with parameters
   // params (params is a string of actual arguments, not formal ones). If the
   // class is 0 the global function list will be searched.
   R__LOCKGUARD2(gCINTMutex);
   G__CallFunc  func;
   Long_t       offset;
   if (cl) {
      func.SetFunc((G__ClassInfo*)cl->GetClassInfo(), method, params, &offset);
   }
   else {
      G__ClassInfo gcl;   // default G__ClassInfo is global environment
      func.SetFunc(&gcl, method, params, &offset);
   }
   return func.GetMethodInfo().GetMangledName();
}

//______________________________________________________________________________
TString TCint::GetMangledNameWithPrototype(TClass* cl, const char* method,
      const char* proto)
{
   // Return the CINT mangled name for a method of a class with a certain
   // prototype, i.e. "char*,int,float". If the class is 0 the global function
   // list will be searched.
   R__LOCKGUARD2(gCINTMutex);
   Long_t             offset;
   if (cl) {
      return ((G__ClassInfo*)cl->GetClassInfo())->GetMethod(method, proto, &offset).GetMangledName();
   }
   G__ClassInfo gcl;   // default G__ClassInfo is global environment
   return gcl.GetMethod(method, proto, &offset).GetMangledName();
}

//______________________________________________________________________________
void* TCint::GetInterfaceMethod(TClass* cl, const char* method,
                                const char* params)
{
   // Return pointer to CINT interface function for a method of a class with
   // parameters params (params is a string of actual arguments, not formal
   // ones). If the class is 0 the global function list will be searched.
   R__LOCKGUARD2(gCINTMutex);
   G__CallFunc  func;
   Long_t       offset;
   if (cl) {
      func.SetFunc((G__ClassInfo*)cl->GetClassInfo(), method, params, &offset);
   }
   else {
      G__ClassInfo gcl;   // default G__ClassInfo is global environment
      func.SetFunc(&gcl, method, params, &offset);
   }
   return (void*)func.InterfaceMethod();
}

//______________________________________________________________________________
void* TCint::GetInterfaceMethodWithPrototype(TClass* cl, const char* method,
      const char* proto)
{
   // Return pointer to CINT interface function for a method of a class with
   // a certain prototype, i.e. "char*,int,float". If the class is 0 the global
   // function list will be searched.
   R__LOCKGUARD2(gCINTMutex);
   G__InterfaceMethod f;
   Long_t             offset;
   if (cl) {
      f = ((G__ClassInfo*)cl->GetClassInfo())->GetMethod(method, proto, &offset).InterfaceMethod();
   }
   else {
      G__ClassInfo gcl;   // default G__ClassInfo is global environment
      f = gcl.GetMethod(method, proto, &offset).InterfaceMethod();
   }
   return (void*)f;
}

//______________________________________________________________________________
const char* TCint::GetInterpreterTypeName(const char* name, Bool_t full)
{
   // The 'name' is known to the interpreter, this function returns
   // the internal version of this name (usually just resolving typedefs)
   // This is used in particular to synchronize between the name used
   // by rootcint and by the run-time enviroment (TClass)
   // Return 0 if the name is not known.
   R__LOCKGUARD(gCINTMutex);
   if (!gInterpreter->CheckClassInfo(name)) {
      return 0;
   }
   G__ClassInfo cl(name);
   if (cl.IsValid()) {
      if (full) {
         return cl.Fullname();
      }
      else {
         return cl.Name();
      }
   }
   else {
      return 0;
   }
}

//______________________________________________________________________________
void TCint::Execute(const char* function, const char* params, int* error)
{
   // Execute a global function with arguments params.
   R__LOCKGUARD2(gCINTMutex);
   G__CallFunc  func;
   G__ClassInfo cl;
   Long_t       offset;
   // set pointer to interface method and arguments
   func.SetFunc(&cl, function, params, &offset);
   // call function
   func.Exec(0);
   if (error) {
      *error = G__lasterror();
   }
}

//______________________________________________________________________________
void TCint::Execute(TObject* obj, TClass* cl, const char* method,
                    const char* params, int* error)
{
   // Execute a method from class cl with arguments params.
   R__LOCKGUARD2(gCINTMutex);
   void*       address;
   Long_t      offset;
   G__CallFunc func;
   // If the actuall class of this object inherit 2nd (or more) from TObject,
   // 'obj' is unlikely to be the start of the object (as described by IsA()),
   // hence gInterpreter->Execute will improperly correct the offset.
   void* addr = cl->DynamicCast(TObject::Class(), obj, kFALSE);
   // set pointer to interface method and arguments
   func.SetFunc((G__ClassInfo*)cl->GetClassInfo(), method, params, &offset);
   // call function
   address = (void*)((Long_t)addr + offset);
   func.Exec(address);
   if (error) {
      *error = G__lasterror();
   }
}

//______________________________________________________________________________
void TCint::Execute(TObject* obj, TClass* cl, TMethod* method, TObjArray* params,
                    int* error)
{
   // Execute a method from class cl with the arguments in array params
   // (params[0] ... params[n] = array of TObjString parameters).
   // Convert the TObjArray array of TObjString parameters to a character
   // string of comma separated parameters.
   // The parameters of type 'char' are enclosed in double quotes and all
   // internal quotes are escaped.
   if (!method) {
      Error("Execute", "No method was defined");
      return;
   }
   TList* argList = method->GetListOfMethodArgs();
   // Check number of actual parameters against of expected formal ones
   Int_t nparms = argList->LastIndex() + 1;
   Int_t argc   = params ? params->LastIndex() + 1 : 0;
   if (nparms != argc) {
      Error("Execute", "Wrong number of the parameters");
      return;
   }
   const char* listpar = "";
   TString complete(10);
   if (params) {
      // Create a character string of parameters from TObjArray
      TIter next(params);
      for (Int_t i = 0; i < argc; i ++) {
         TMethodArg* arg = (TMethodArg*) argList->At(i);
         G__TypeInfo type(arg->GetFullTypeName());
         TObjString* nxtpar = (TObjString*)next();
         if (i) {
            complete += ',';
         }
         if (strstr(type.TrueName(), "char")) {
            TString chpar('\"');
            chpar += (nxtpar->String()).ReplaceAll("\"", "\\\"");
            // At this point we have to check if string contains \\"
            // and apply some more sophisticated parser. Not implemented yet!
            complete += chpar;
            complete += '\"';
         }
         else {
            complete += nxtpar->String();
         }
      }
      listpar = complete.Data();
   }
   Execute(obj, cl, (char*)method->GetName(), (char*)listpar, error);
}

//______________________________________________________________________________
Long_t TCint::ExecuteMacro(const char* filename, EErrorCode* error)
{
   // Execute a CINT macro.
   R__LOCKGUARD(gCINTMutex);
   return TApplication::ExecuteFile(filename, (int*)error);
}

//______________________________________________________________________________
const char* TCint::GetTopLevelMacroName() const
{
   // Return the file name of the current un-included interpreted file.
   // See the documentation for GetCurrentMacroName().
   G__SourceFileInfo srcfile(G__get_ifile()->filenum);
   while (srcfile.IncludedFrom().IsValid()) {
      srcfile = srcfile.IncludedFrom();
   }
   return srcfile.Name();
}

//______________________________________________________________________________
const char* TCint::GetCurrentMacroName() const
{
   // Return the file name of the currently interpreted file,
   // included or not. Example to illustrate the difference between
   // GetCurrentMacroName() and GetTopLevelMacroName():
   // BEGIN_HTML <!--
   /* -->
      <span style="color:#ffffff;background-color:#7777ff;padding-left:0.3em;padding-right:0.3em">inclfile.h</span>
      <!--div style="border:solid 1px #ffff77;background-color: #ffffdd;float:left;padding:0.5em;margin-bottom:0.7em;"-->
      <div class="code">
      <pre style="margin:0pt">#include &lt;iostream&gt;
   void inclfunc() {
   std::cout &lt;&lt; "In inclfile.h" &lt;&lt; std::endl;
   std::cout &lt;&lt; "  TCint::GetCurrentMacroName() returns  " &lt;&lt;
      TCint::GetCurrentMacroName() &lt;&lt; std::endl;
   std::cout &lt;&lt; "  TCint::GetTopLevelMacroName() returns " &lt;&lt;
      TCint::GetTopLevelMacroName() &lt;&lt; std::endl;
   }</pre></div>
      <div style="clear:both"></div>
      <span style="color:#ffffff;background-color:#7777ff;padding-left:0.3em;padding-right:0.3em">mymacro.C</span>
      <div style="border:solid 1px #ffff77;background-color: #ffffdd;float:left;padding:0.5em;margin-bottom:0.7em;">
      <pre style="margin:0pt">#include &lt;iostream&gt;
   #include "inclfile.h"
   void mymacro() {
   std::cout &lt;&lt; "In mymacro.C" &lt;&lt; std::endl;
   std::cout &lt;&lt; "  TCint::GetCurrentMacroName() returns  " &lt;&lt;
      TCint::GetCurrentMacroName() &lt;&lt; std::endl;
   std::cout &lt;&lt; "  TCint::GetTopLevelMacroName() returns " &lt;&lt;
      TCint::GetTopLevelMacroName() &lt;&lt; std::endl;
   std::cout &lt;&lt; "  Now calling inclfunc..." &lt;&lt; std::endl;
   inclfunc();
   }</pre></div>
   <div style="clear:both"></div>
   <!-- */
   // --> END_HTML
   // Running mymacro.C will print:
   //
   // root [0] .x mymacro.C
   // In mymacro.C
   //   TCint::GetCurrentMacroName() returns  ./mymacro.C
   //   TCint::GetTopLevelMacroName() returns ./mymacro.C
   //   Now calling inclfunc...
   // In inclfile.h
   //   TCint::GetCurrentMacroName() returns  inclfile.h
   //   TCint::GetTopLevelMacroName() returns ./mymacro.C
   return G__get_ifile()->name;
}


//______________________________________________________________________________
const char* TCint::TypeName(const char* typeDesc)
{
   // Return the absolute type of typeDesc.
   // E.g.: typeDesc = "class TNamed**", returns "TNamed".
   // You need to use the result immediately before it is being overwritten.
   static char* t = 0;
   static unsigned int tlen = 0;
   R__LOCKGUARD(gCINTMutex); // Because of the static array.
   unsigned int dlen = strlen(typeDesc);
   if (dlen > tlen) {
      delete [] t;
      t = new char[dlen + 1];
      tlen = dlen;
   }
   char* s, *template_start;
   if (!strstr(typeDesc, "(*)(")) {
      s = (char*)strchr(typeDesc, ' ');
      template_start = (char*)strchr(typeDesc, '<');
      if (!strcmp(typeDesc, "long long")) {
         strlcpy(t, typeDesc, dlen + 1);
      }
      else if (!strncmp(typeDesc, "unsigned ", s + 1 - typeDesc)) {
         strlcpy(t, typeDesc, dlen + 1);
      }
      // s is the position of the second 'word' (if any)
      // except in the case of templates where there will be a space
      // just before any closing '>': eg.
      //    TObj<std::vector<UShort_t,__malloc_alloc_template<0> > >*
      else if (s && (template_start == 0 || (s < template_start))) {
         strlcpy(t, s + 1, dlen + 1);
      }
      else {
         strlcpy(t, typeDesc, dlen + 1);
      }
   }
   else {
      strlcpy(t, typeDesc, dlen + 1);
   }
   int l = strlen(t);
   while (l > 0 && (t[l - 1] == '*' || t[l - 1] == '&')) {
      t[--l] = 0;
   }
   return t;
}

//______________________________________________________________________________
Int_t TCint::LoadLibraryMap(const char* rootmapfile)
{
   // Load map between class and library. If rootmapfile is specified a
   // specific rootmap file can be added (typically used by ACLiC).
   // In case of error -1 is returned, 0 otherwise.
   // Cint uses this information to automatically load the shared library
   // for a class (autoload mechanism).
   // See also the AutoLoadCallback() method below.
   R__LOCKGUARD(gCINTMutex);
   // open the [system].rootmap files
   if (!fMapfile) {
      fMapfile = new TEnv(".rootmap");
      fMapfile->IgnoreDuplicates(kTRUE);
      fRootmapFiles = new TObjArray;
      fRootmapFiles->SetOwner();
      // Make sure that this information will be useable by inserting our
      // autoload call back!
      G__set_class_autoloading_callback(&TCint_AutoLoadCallback);
   }
   // Load all rootmap files in the dynamic load path ((DY)LD_LIBRARY_PATH, etc.).
   // A rootmap file must end with the string ".rootmap".
   TString ldpath = gSystem->GetDynamicPath();
   if (ldpath != fRootmapLoadPath) {
      fRootmapLoadPath = ldpath;
#ifdef WIN32
      TObjArray* paths = ldpath.Tokenize(";");
#else
      TObjArray* paths = ldpath.Tokenize(":");
#endif
      TString d;
      for (Int_t i = 0; i < paths->GetEntriesFast(); i++) {
         d = ((TObjString*)paths->At(i))->GetString();
         // check if directory already scanned
         Int_t skip = 0;
         for (Int_t j = 0; j < i; j++) {
            TString pd = ((TObjString*)paths->At(j))->GetString();
            if (pd == d) {
               skip++;
               break;
            }
         }
         if (!skip) {
            void* dirp = gSystem->OpenDirectory(d);
            if (dirp) {
               if (gDebug > 3) {
                  Info("LoadLibraryMap", "%s", d.Data());
               }
               const char* f1;
               while ((f1 = gSystem->GetDirEntry(dirp))) {
                  TString f = f1;
                  if (f.EndsWith(".rootmap")) {
                     TString p;
                     p = d + "/" + f;
                     if (!gSystem->AccessPathName(p, kReadPermission)) {
                        if (!fRootmapFiles->FindObject(f) && f != ".rootmap") {
                           if (gDebug > 4) {
                              Info("LoadLibraryMap", "   rootmap file: %s", p.Data());
                           }
                           fMapfile->ReadFile(p, kEnvGlobal);
                           fRootmapFiles->Add(new TNamed(f, p));
                        }
                        //                        else {
                        //                           fprintf(stderr,"Reject %s because %s is already there\n",p.Data(),f.Data());
                        //                           fRootmapFiles->FindObject(f)->ls();
                        //                        }
                     }
                  }
                  if (f.BeginsWith("rootmap")) {
                     TString p;
                     p = d + "/" + f;
                     FileStat_t stat;
                     if (gSystem->GetPathInfo(p, stat) == 0 && R_ISREG(stat.fMode)) {
                        Warning("LoadLibraryMap", "please rename %s to end with \".rootmap\"", p.Data());
                     }
                  }
               }
            }
            gSystem->FreeDirectory(dirp);
         }
      }
      delete paths;
      if (!fMapfile->GetTable()->GetEntries()) {
         return -1;
      }
   }
   if (rootmapfile && *rootmapfile) {
      // Add content of a specific rootmap file
      Bool_t ignore = fMapfile->IgnoreDuplicates(kFALSE);
      fMapfile->ReadFile(rootmapfile, kEnvGlobal);
      fRootmapFiles->Add(new TNamed(gSystem->BaseName(rootmapfile), rootmapfile));
      fMapfile->IgnoreDuplicates(ignore);
   }
   TEnvRec* rec;
   TIter next(fMapfile->GetTable());
   while ((rec = (TEnvRec*) next())) {
      TString cls = rec->GetName();
      if (!strncmp(cls.Data(), "Library.", 8) && cls.Length() > 8) {
         // get the first lib from the list of lib and dependent libs
         TString libs = rec->GetValue();
         if (libs == "") {
            continue;
         }
         TString delim(" ");
         TObjArray* tokens = libs.Tokenize(delim);
         const char* lib = ((TObjString*)tokens->At(0))->GetName();
         // convert "@@" to "::", we used "@@" because TEnv
         // considers "::" a terminator
         cls.Remove(0, 8);
         cls.ReplaceAll("@@", "::");
         // convert "-" to " ", since class names may have
         // blanks and TEnv considers a blank a terminator
         cls.ReplaceAll("-", " ");
         if (cls.Contains(":")) {
            // We have a namespace and we have to check it first
            int slen = cls.Length();
            for (int k = 0; k < slen; k++) {
               if (cls[k] == ':') {
                  if (k + 1 >= slen || cls[k + 1] != ':') {
                     // we expected another ':'
                     break;
                  }
                  if (k) {
                     TString base = cls(0, k);
                     if (base == "std") {
                        // std is not declared but is also ignored by CINT!
                        break;
                     }
                     else {
                        // Only declared the namespace do not specify any library because
                        // the namespace might be spread over several libraries and we do not
                        // know (yet?) which one the user will need!
                        // But what if it's not a namespace but a class?
                        // Does CINT already know it?
                        const char* baselib = G__get_class_autoloading_table((char*)base.Data());
                        if ((!baselib || !baselib[0]) && !rec->FindObject(base)) {
                           G__set_class_autoloading_table((char*)base.Data(), (char*)"");
                        }
                     }
                     ++k;
                  }
               }
               else if (cls[k] == '<') {
                  // We do not want to look at the namespace inside the template parameters!
                  break;
               }
            }
         }
         G__set_class_autoloading_table((char*)cls.Data(), (char*)lib);
         G__security_recover(stderr); // Ignore any error during this setting.
         if (gDebug > 6) {
            const char* wlib = gSystem->DynamicPathName(lib, kTRUE);
            if (wlib) {
               Info("LoadLibraryMap", "class %s in %s", cls.Data(), wlib);
            }
            else {
               Info("LoadLibraryMap", "class %s in %s (library does not exist)", cls.Data(), lib);
            }
            delete [] wlib;
         }
         delete tokens;
      }
   }
   return 0;
}

//______________________________________________________________________________
Int_t TCint::RescanLibraryMap()
{
   // Scan again along the dynamic path for library maps. Entries for the loaded
   // shared libraries are unloaded first. This can be useful after reseting
   // the dynamic path through TSystem::SetDynamicPath()
   // In case of error -1 is returned, 0 otherwise.
   UnloadAllSharedLibraryMaps();
   LoadLibraryMap();
   return 0;
}

//______________________________________________________________________________
Int_t TCint::ReloadAllSharedLibraryMaps()
{
   // Reload the library map entries coming from all the loaded shared libraries,
   // after first unloading the current ones.
   // In case of error -1 is returned, 0 otherwise.
   const TString sharedLibLStr = GetSharedLibs();
   const TObjArray* sharedLibL = sharedLibLStr.Tokenize(" ");
   const Int_t nrSharedLibs = sharedLibL->GetEntriesFast();
   for (Int_t ilib = 0; ilib < nrSharedLibs; ilib++) {
      const TString sharedLibStr = ((TObjString*)sharedLibL->At(ilib))->GetString();
      const  TString sharedLibBaseStr = gSystem->BaseName(sharedLibStr);
      const Int_t ret = UnloadLibraryMap(sharedLibBaseStr);
      if (ret < 0) {
         continue;
      }
      TString rootMapBaseStr = sharedLibBaseStr;
      if (sharedLibBaseStr.EndsWith(".dll")) {
         rootMapBaseStr.ReplaceAll(".dll", "");
      }
      else if (sharedLibBaseStr.EndsWith(".DLL")) {
         rootMapBaseStr.ReplaceAll(".DLL", "");
      }
      else if (sharedLibBaseStr.EndsWith(".so")) {
         rootMapBaseStr.ReplaceAll(".so", "");
      }
      else if (sharedLibBaseStr.EndsWith(".sl")) {
         rootMapBaseStr.ReplaceAll(".sl", "");
      }
      else if (sharedLibBaseStr.EndsWith(".dl")) {
         rootMapBaseStr.ReplaceAll(".dl", "");
      }
      else if (sharedLibBaseStr.EndsWith(".a")) {
         rootMapBaseStr.ReplaceAll(".a", "");
      }
      else {
         Error("ReloadAllSharedLibraryMaps", "Unknown library type %s", sharedLibBaseStr.Data());
         delete sharedLibL;
         return -1;
      }
      rootMapBaseStr += ".rootmap";
      const char* rootMap = gSystem->Which(gSystem->GetDynamicPath(), rootMapBaseStr);
      if (!rootMap) {
         Error("ReloadAllSharedLibraryMaps", "Could not find rootmap %s in path", rootMap);
         delete [] rootMap;
         delete sharedLibL;
         return -1;
      }
      const Int_t status = LoadLibraryMap(rootMap);
      if (status < 0) {
         Error("ReloadAllSharedLibraryMaps", "Error loading map %s", rootMap);
         delete [] rootMap;
         delete sharedLibL;
         return -1;
      }
      delete [] rootMap;
   }
   delete sharedLibL;
   return 0;
}

//______________________________________________________________________________
Int_t TCint::UnloadAllSharedLibraryMaps()
{
   // Unload the library map entries coming from all the loaded shared libraries.
   // Returns 0 if succesful
   const TString sharedLibLStr = GetSharedLibs();
   const TObjArray* sharedLibL = sharedLibLStr.Tokenize(" ");
   for (Int_t ilib = 0; ilib < sharedLibL->GetEntriesFast(); ilib++) {
      const TString sharedLibStr = ((TObjString*)sharedLibL->At(ilib))->GetString();
      const  TString sharedLibBaseStr = gSystem->BaseName(sharedLibStr);
      UnloadLibraryMap(sharedLibBaseStr);
   }
   delete sharedLibL;
   return 0;
}

//______________________________________________________________________________
Int_t TCint::UnloadLibraryMap(const char* library)
{
   // Unload library map entries coming from the specified library.
   // Returns -1 in case no entries for the specified library were found,
   // 0 otherwise.
   if (!fMapfile || !library || !*library) {
      return 0;
   }
   TEnvRec* rec;
   TIter next(fMapfile->GetTable());
   R__LOCKGUARD(gCINTMutex);
   Int_t ret = 0;
   while ((rec = (TEnvRec*) next())) {
      TString cls = rec->GetName();
      if (!strncmp(cls.Data(), "Library.", 8) && cls.Length() > 8) {
         // get the first lib from the list of lib and dependent libs
         TString libs = rec->GetValue();
         if (libs == "") {
            continue;
         }
         TString delim(" ");
         TObjArray* tokens = libs.Tokenize(delim);
         const char* lib = ((TObjString*)tokens->At(0))->GetName();
         // convert "@@" to "::", we used "@@" because TEnv
         // considers "::" a terminator
         cls.Remove(0, 8);
         cls.ReplaceAll("@@", "::");
         // convert "-" to " ", since class names may have
         // blanks and TEnv considers a blank a terminator
         cls.ReplaceAll("-", " ");
         if (cls.Contains(":")) {
            // We have a namespace and we have to check it first
            int slen = cls.Length();
            for (int k = 0; k < slen; k++) {
               if (cls[k] == ':') {
                  if (k + 1 >= slen || cls[k + 1] != ':') {
                     // we expected another ':'
                     break;
                  }
                  if (k) {
                     TString base = cls(0, k);
                     if (base == "std") {
                        // std is not declared but is also ignored by CINT!
                        break;
                     }
                     else {
                        // Only declared the namespace do not specify any library because
                        // the namespace might be spread over several libraries and we do not
                        // know (yet?) which one the user will need!
                        //G__remove_from_class_autoloading_table((char*)base.Data());
                     }
                     ++k;
                  }
               }
               else if (cls[k] == '<') {
                  // We do not want to look at the namespace inside the template parameters!
                  break;
               }
            }
         }
         if (!strcmp(library, lib)) {
            if (fMapfile->GetTable()->Remove(rec) == 0) {
               Error("UnloadLibraryMap", "entry for <%s,%s> not found in library map table", cls.Data(), lib);
               ret = -1;
            }
            G__set_class_autoloading_table((char*)cls.Data(), (char*) - 1);
            G__security_recover(stderr); // Ignore any error during this setting.
         }
         delete tokens;
      }
   }
   if (ret >= 0) {
      TString library_rootmap(library);
      library_rootmap.Append(".rootmap");
      TNamed* mfile = 0;
      while ((mfile = (TNamed*)fRootmapFiles->FindObject(library_rootmap))) {
         fRootmapFiles->Remove(mfile);
         delete mfile;
      }
      fRootmapFiles->Compress();
   }
   return ret;
}

//______________________________________________________________________________
Int_t TCint::AutoLoad(const char* cls)
{
   // Load library containing the specified class. Returns 0 in case of error
   // and 1 in case if success.
   R__LOCKGUARD(gCINTMutex);
   Int_t status = 0;
   if (!gROOT || !gInterpreter || gROOT->TestBit(TObject::kInvalidObject)) {
      return status;
   }
   // Prevent the recursion when the library dictionary are loaded.
   Int_t oldvalue = G__set_class_autoloading(0);
   // lookup class to find list of dependent libraries
   TString deplibs = GetClassSharedLibs(cls);
   if (!deplibs.IsNull()) {
      TString delim(" ");
      TObjArray* tokens = deplibs.Tokenize(delim);
      for (Int_t i = tokens->GetEntriesFast() - 1; i > 0; i--) {
         const char* deplib = ((TObjString*)tokens->At(i))->GetName();
         if (gROOT->LoadClass(cls, deplib) == 0) {
            if (gDebug > 0)
               ::Info("TCint::AutoLoad", "loaded dependent library %s for class %s",
                      deplib, cls);
         }
         else
            ::Error("TCint::AutoLoad", "failure loading dependent library %s for class %s",
                    deplib, cls);
      }
      const char* lib = ((TObjString*)tokens->At(0))->GetName();
      if (lib[0]) {
         if (gROOT->LoadClass(cls, lib) == 0) {
            if (gDebug > 0)
               ::Info("TCint::AutoLoad", "loaded library %s for class %s",
                      lib, cls);
            status = 1;
         }
         else
            ::Error("TCint::AutoLoad", "failure loading library %s for class %s",
                    lib, cls);
      }
      delete tokens;
   }
   G__set_class_autoloading(oldvalue);
   return status;
}

//______________________________________________________________________________
Int_t TCint::AutoLoadCallback(const char* cls, const char* lib)
{
   // Load library containing specified class. Returns 0 in case of error
   // and 1 in case if success.
   R__LOCKGUARD(gCINTMutex);
   if (!gROOT || !gInterpreter || !cls || !lib) {
      return 0;
   }
   // calls to load libCore might come in the very beginning when libCore
   // dictionary is not fully loaded yet, ignore it since libCore is always
   // loaded
   if (strstr(lib, "libCore")) {
      return 1;
   }
   // lookup class to find list of dependent libraries
   TString deplibs = gInterpreter->GetClassSharedLibs(cls);
   if (!deplibs.IsNull()) {
      if (gDebug > 0 && gDebug <= 4)
         ::Info("TCint::AutoLoadCallback", "loaded dependent library %s for class %s",
                deplibs.Data(), cls);
      TString delim(" ");
      TObjArray* tokens = deplibs.Tokenize(delim);
      for (Int_t i = tokens->GetEntriesFast() - 1; i > 0; i--) {
         const char* deplib = ((TObjString*)tokens->At(i))->GetName();
         if (gROOT->LoadClass(cls, deplib) == 0) {
            if (gDebug > 4)
               ::Info("TCint::AutoLoadCallback", "loaded dependent library %s for class %s",
                      deplib, cls);
         }
         else {
            ::Error("TCint::AutoLoadCallback", "failure loading dependent library %s for class %s",
                    deplib, cls);
         }
      }
      delete tokens;
   }
   if (lib[0]) {
      if (gROOT->LoadClass(cls, lib) == 0) {
         if (gDebug > 0)
            ::Info("TCint::AutoLoadCallback", "loaded library %s for class %s",
                   lib, cls);
         return 1;
      }
      else {
         ::Error("TCint::AutoLoadCallback", "failure loading library %s for class %s",
                 lib, cls);
      }
   }
   return 0;
}

//______________________________________________________________________________
void* TCint::FindSpecialObject(const char* item, G__ClassInfo* type,
                               void** prevObj, void** assocPtr)
{
   // Static function called by CINT when it finds an un-indentified object.
   // This function tries to find the UO in the ROOT files, directories, etc.
   // This functions has been registered by the TCint ctor.
   if (!*prevObj || *assocPtr != gDirectory) {
      *prevObj = gROOT->FindSpecialObject(item, *assocPtr);
      if (!fgSetOfSpecials) {
         fgSetOfSpecials = new std::set<TObject*>;
      }
      if (*prevObj) {
         ((std::set<TObject*>*)fgSetOfSpecials)->insert((TObject*)*prevObj);
      }
   }
   if (*prevObj) {
      type->Init(((TObject*)*prevObj)->ClassName());
   }
   return *prevObj;
}

//______________________________________________________________________________
// Helper class for UpdateClassInfo
namespace {
class TInfoNode {
private:
   string fName;
   Long_t fTagnum;
public:
   TInfoNode(const char* item, Long_t tagnum)
      : fName(item), fTagnum(tagnum)
   {}
   void Update() {
      TCint::UpdateClassInfoWork(fName.c_str(), fTagnum);
   }
};
}

//______________________________________________________________________________
void TCint::UpdateClassInfo(char* item, Long_t tagnum)
{
   // Static function called by CINT when it changes the tagnum for
   // a class (e.g. after re-executing the setup function). In such
   // cases we have to update the tagnum in the G__ClassInfo used by
   // the TClass for class "item".
   R__LOCKGUARD(gCINTMutex);
   if (gROOT && gROOT->GetListOfClasses()) {
      static Bool_t entered = kFALSE;
      static vector<TInfoNode> updateList;
      Bool_t topLevel;
      if (entered) {
         topLevel = kFALSE;
      }
      else {
         entered = kTRUE;
         topLevel = kTRUE;
      }
      if (topLevel) {
         UpdateClassInfoWork(item, tagnum);
      }
      else {
         // If we are called indirectly from within another call to
         // TCint::UpdateClassInfo, we delay the update until the dictionary loading
         // is finished (i.e. when we return to the top level TCint::UpdateClassInfo).
         // This allows for the dictionary to be fully populated when we actually
         // update the TClass object.   The updating of the TClass sometimes
         // (STL containers and when there is an emulated class) forces the building
         // of the TClass object's real data (which needs the dictionary info).
         updateList.push_back(TInfoNode(item, tagnum));
      }
      if (topLevel) {
         while (!updateList.empty()) {
            TInfoNode current(updateList.back());
            updateList.pop_back();
            current.Update();
         }
         entered = kFALSE;
      }
   }
}

//______________________________________________________________________________
void TCint::UpdateClassInfoWork(const char* item, Long_t tagnum)
{
   // This does the actual work of UpdateClassInfo.
   Bool_t load = kFALSE;
   if (strchr(item, '<') && TClass::GetClassShortTypedefHash()) {
      // We have a template which may have duplicates.
      TString resolvedItem(
         TClassEdit::ResolveTypedef(TClassEdit::ShortType(item,
                                    TClassEdit::kDropStlDefault).c_str(), kTRUE));
      if (resolvedItem != item) {
         TClass* cl = (TClass*)gROOT->GetListOfClasses()->FindObject(resolvedItem);
         if (cl) {
            load = kTRUE;
         }
      }
      if (!load) {
         TIter next(TClass::GetClassShortTypedefHash()->GetListForObject(resolvedItem));
         while (TClass::TNameMapNode* htmp =
                   static_cast<TClass::TNameMapNode*>(next())) {
            if (resolvedItem == htmp->String()) {
               TClass* cl = gROOT->GetClass(htmp->fOrigName, kFALSE);
               if (cl) {
                  // we found at least one equivalent.
                  // let's force a reload
                  load = kTRUE;
                  break;
               }
            }
         }
      }
   }
   TClass* cl = gROOT->GetClass(item, load);
   if (cl) {
      cl->ResetClassInfo(tagnum);
   }
}

//______________________________________________________________________________
void TCint::UpdateAllCanvases()
{
   // Update all canvases at end the terminal input command.
   TIter next(gROOT->GetListOfCanvases());
   TVirtualPad* canvas;
   while ((canvas = (TVirtualPad*)next())) {
      canvas->Update();
   }
}

//______________________________________________________________________________
const char* TCint::GetSharedLibs()
{
   // Return the list of shared libraries known to CINT.
   if (fSharedLibsSerial == G__SourceFileInfo::SerialNumber()) {
      return fSharedLibs;
   }
   fSharedLibsSerial = G__SourceFileInfo::SerialNumber();
   fSharedLibs.Clear();
   G__SourceFileInfo cursor(0);
   while (cursor.IsValid()) {
      const char* filename = cursor.Name();
      if (filename == 0) {
         continue;
      }
      Int_t len = strlen(filename);
      const char* end = filename + len;
      Bool_t needToSkip = kFALSE;
      if (len > 5 && ((strcmp(end - 4, ".dll") == 0) || (strstr(filename, "Dict.") != 0)  || (strstr(filename, "MetaTCint") != 0))) {
         // Filter out the cintdlls
         static const char* excludelist [] = {
            "stdfunc.dll", "stdcxxfunc.dll", "posix.dll", "ipc.dll", "posix.dll"
            "string.dll", "vector.dll", "vectorbool.dll", "list.dll", "deque.dll",
            "map.dll", "map2.dll", "set.dll", "multimap.dll", "multimap2.dll",
            "multiset.dll", "stack.dll", "queue.dll", "valarray.dll",
            "exception.dll", "stdexcept.dll", "complex.dll", "climits.dll",
            "libvectorDict.", "libvectorboolDict.", "liblistDict.", "libdequeDict.",
            "libmapDict.", "libmap2Dict.", "libsetDict.", "libmultimapDict.", "libmultimap2Dict.",
            "libmultisetDict.", "libstackDict.", "libqueueDict.", "libvalarrayDict."
         };
         static const unsigned int excludelistsize = sizeof(excludelist) / sizeof(excludelist[0]);
         static int excludelen[excludelistsize] = { -1};
         if (excludelen[0] == -1) {
            for (unsigned int i = 0; i < excludelistsize; ++i) {
               excludelen[i] = strlen(excludelist[i]);
            }
         }
         const char* basename = gSystem->BaseName(filename);
         for (unsigned int i = 0; !needToSkip && i < excludelistsize; ++i) {
            needToSkip = (!strncmp(basename, excludelist[i], excludelen[i]));
         }
      }
      if (!needToSkip &&
            (
#if defined(R__MACOSX) && defined(MAC_OS_X_VERSION_10_5)
               (dlopen_preflight(filename)) ||
#endif
               (len > 2 && strcmp(end - 2, ".a") == 0)    ||
               (len > 3 && (strcmp(end - 3, ".sl") == 0   ||
                            strcmp(end - 3, ".dl") == 0   ||
                            strcmp(end - 3, ".so") == 0)) ||
               (len > 4 && (strcasecmp(end - 4, ".dll") == 0)) ||
               (len > 6 && (strcasecmp(end - 6, ".dylib") == 0)))) {
         if (!fSharedLibs.IsNull()) {
            fSharedLibs.Append(" ");
         }
         fSharedLibs.Append(filename);
      }
      cursor.Next();
   }
   return fSharedLibs;
}

//______________________________________________________________________________
const char* TCint::GetClassSharedLibs(const char* cls)
{
   // Get the list of shared libraries containing the code for class cls.
   // The first library in the list is the one containing the class, the
   // others are the libraries the first one depends on. Returns 0
   // in case the library is not found.
   if (!cls || !*cls) {
      return 0;
   }
   // lookup class to find list of libraries
   if (fMapfile) {
      TString c = TString("Library.") + cls;
      // convert "::" to "@@", we used "@@" because TEnv
      // considers "::" a terminator
      c.ReplaceAll("::", "@@");
      // convert "-" to " ", since class names may have
      // blanks and TEnv considers a blank a terminator
      c.ReplaceAll(" ", "-");
      // Use TEnv::Lookup here as the rootmap file must start with Library.
      // and do not support using any stars (so we do not need to waste time
      // with the search made by TEnv::GetValue).
      TEnvRec *libs_record = fMapfile->Lookup(c);
      if (libs_record) {
         const char *libs = libs_record->GetValue();
         return (*libs) ? libs : 0;
      }
   }
   return 0;
}

//______________________________________________________________________________
const char* TCint::GetSharedLibDeps(const char* lib)
{
   // Get the list a libraries on which the specified lib depends. The
   // returned string contains as first element the lib itself.
   // Returns 0 in case the lib does not exist or does not have
   // any dependencies.
   if (!fMapfile || !lib || !lib[0]) {
      return 0;
   }
   TString libname(lib);
   Ssiz_t idx = libname.Last('.');
   if (idx != kNPOS) {
      libname.Remove(idx);
   }
   TEnvRec* rec;
   TIter next(fMapfile->GetTable());
   size_t len = libname.Length();
   while ((rec = (TEnvRec*) next())) {
      const char* libs = rec->GetValue();
      if (!strncmp(libs, libname.Data(), len) && strlen(libs) >= len
            && (!libs[len] || libs[len] == ' ' || libs[len] == '.')) {
         return libs;
      }
   }
   return 0;
}

//______________________________________________________________________________
Bool_t TCint::IsErrorMessagesEnabled() const
{
   // If error messages are disabled, the interpreter should suppress its
   // failures and warning messages from stdout.
   return !G__const_whatnoerror();
}

//______________________________________________________________________________
Bool_t TCint::SetErrorMessages(Bool_t enable)
{
   // If error messages are disabled, the interpreter should suppress its
   // failures and warning messages from stdout. Return the previous state.
   if (enable) {
      G__const_resetnoerror();
   }
   else {
      G__const_setnoerror();
   }
   return !G__const_whatnoerror();
}

//______________________________________________________________________________
void TCint::AddIncludePath(const char* path)
{
   // Add the given path to the list of directories in which the interpreter
   // looks for include files. Only one path item can be specified at a
   // time, i.e. "path1:path2" is not supported.
   R__LOCKGUARD(gCINTMutex);
   char* incpath = gSystem->ExpandPathName(path);
   G__add_ipath(incpath);
   delete [] incpath;
}

//______________________________________________________________________________
const char* TCint::GetIncludePath()
{
   // Refresh the list of include paths known to the interpreter and return it
   // with -I prepended.
   R__LOCKGUARD(gCINTMutex);
   fIncludePath = "";
   G__IncludePathInfo path;
   while (path.Next()) {
      const char* pathname = path.Name();
      fIncludePath.Append(" -I\"").Append(pathname).Append("\" ");
   }
   return fIncludePath;
}

//______________________________________________________________________________
const char* TCint::GetSTLIncludePath() const
{
   // Return the directory containing CINT's stl cintdlls.
   static TString stldir;
   if (!stldir.Length()) {
#ifdef CINTINCDIR
      stldir = CINTINCDIR;
#else
      stldir = gRootDir;
      stldir += "/cint";
#endif
      if (!stldir.EndsWith("/")) {
         stldir += '/';
      }
      stldir += "cint/stl";
   }
   return stldir;
}

//______________________________________________________________________________
//                      M I S C
//______________________________________________________________________________

int TCint::DisplayClass(FILE* fout, char* name, int base, int start) const
{
   // Interface to CINT function
   return G__display_class(fout, name, base, start);
}
//______________________________________________________________________________
int TCint::DisplayIncludePath(FILE* fout) const
{
   // Interface to CINT function
   return G__display_includepath(fout);
}
//______________________________________________________________________________
void*  TCint::FindSym(const char* entry) const
{
   // Interface to CINT function
   return G__findsym(entry);
}
//______________________________________________________________________________
void   TCint::GenericError(const char* error) const
{
   // Interface to CINT function
   G__genericerror(error);
}
//______________________________________________________________________________
Long_t TCint::GetExecByteCode() const
{
   // Interface to CINT function
   return (Long_t)G__exec_bytecode;
}

//______________________________________________________________________________
Long_t TCint::Getgvp() const
{
   // Interface to CINT function
   return (Long_t)G__getgvp();
}
//______________________________________________________________________________
const char* TCint::Getp2f2funcname(void* receiver) const
{
   // Interface to CINT function
   return G__p2f2funcname(receiver);
}
//______________________________________________________________________________
int    TCint::GetSecurityError() const
{
   // Interface to CINT function
   return G__get_security_error();
}
//______________________________________________________________________________
int    TCint::LoadFile(const char* path) const
{
   // Interface to CINT function
   return G__loadfile(path);
}
//______________________________________________________________________________
void   TCint::LoadText(const char* text) const
{
   // Interface to CINT function
   G__load_text(text);
}
//______________________________________________________________________________
const char* TCint::MapCppName(const char* name) const
{
   // Interface to CINT function
   return G__map_cpp_name(name);
}
//______________________________________________________________________________
void   TCint::SetAlloclockfunc(void (*p)()) const
{
   // Interface to CINT function
   G__set_alloclockfunc(p);
}
//______________________________________________________________________________
void   TCint::SetAllocunlockfunc(void (*p)()) const
{
   // Interface to CINT function
   G__set_allocunlockfunc(p);
}
//______________________________________________________________________________
int    TCint::SetClassAutoloading(int autoload) const
{
   // Interface to CINT function
   return G__set_class_autoloading(autoload);
}
//______________________________________________________________________________
void   TCint::SetErrmsgcallback(void* p) const
{
   // Interface to CINT function
   G__set_errmsgcallback(p);
}
//______________________________________________________________________________
void   TCint::Setgvp(Long_t gvp) const
{
   // Interface to CINT function
   G__setgvp(gvp);
}
//______________________________________________________________________________
void   TCint::SetRTLD_NOW() const
{
   // Interface to CINT function
   G__Set_RTLD_NOW();
}
//______________________________________________________________________________
void   TCint::SetRTLD_LAZY() const
{
   // Interface to CINT function
   G__Set_RTLD_LAZY();
}
//______________________________________________________________________________
void   TCint::SetTempLevel(int val) const
{
   // Interface to CINT function
   G__settemplevel(val);
}
//______________________________________________________________________________
int    TCint::UnloadFile(const char* path) const
{
   // Interface to CINT function
   return G__unloadfile(path);
}



//______________________________________________________________________________
// G__CallFunc interface
//______________________________________________________________________________
void  TCint::CallFunc_Delete(CallFunc_t* func) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   delete f;
}
//______________________________________________________________________________
void  TCint::CallFunc_Exec(CallFunc_t* func, void* address) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   f->Exec(address);
}
//______________________________________________________________________________
Long_t  TCint::CallFunc_ExecInt(CallFunc_t* func, void* address) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   return f->ExecInt(address);
}
//______________________________________________________________________________
Long_t  TCint::CallFunc_ExecInt64(CallFunc_t* func, void* address) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   return f->ExecInt64(address);
}
//______________________________________________________________________________
Double_t  TCint::CallFunc_ExecDouble(CallFunc_t* func, void* address) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   return f->ExecDouble(address);
}
//______________________________________________________________________________
CallFunc_t* TCint::CallFunc_Factory() const
{
   // Interface to CINT function
   G__CallFunc* f = new G__CallFunc();
   return f;
}
//______________________________________________________________________________
CallFunc_t* TCint::CallFunc_FactoryCopy(CallFunc_t* func) const
{
   // Interface to CINT function
   G__CallFunc* f1 = (G__CallFunc*)func;
   G__CallFunc* f  = new G__CallFunc(*f1);
   return f;
}
//______________________________________________________________________________
MethodInfo_t* TCint::CallFunc_FactoryMethod(CallFunc_t* func) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   G__MethodInfo* info = new G__MethodInfo((*f).GetMethodInfo());
   return info;
}
//______________________________________________________________________________
void  TCint::CallFunc_Init(CallFunc_t* func) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   f->Init();
}
//______________________________________________________________________________
bool  TCint::CallFunc_IsValid(CallFunc_t* func) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   return f->IsValid();
}
//______________________________________________________________________________
void  TCint::CallFunc_ResetArg(CallFunc_t* func) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   f->ResetArg();
}
//______________________________________________________________________________
void  TCint::CallFunc_SetArg(CallFunc_t* func, Long_t param) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   f->SetArg(param);
}
//______________________________________________________________________________
void  TCint::CallFunc_SetArg(CallFunc_t* func, Double_t param) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   f->SetArg(param);
}
//______________________________________________________________________________
void  TCint::CallFunc_SetArg(CallFunc_t* func, Long64_t param) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   f->SetArg(param);
}
//______________________________________________________________________________
void  TCint::CallFunc_SetArg(CallFunc_t* func, ULong64_t param) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   f->SetArg(param);
}
//______________________________________________________________________________
void  TCint::CallFunc_SetArgArray(CallFunc_t* func, Long_t* paramArr, Int_t nparam) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   f->SetArgArray(paramArr, nparam);
}
//______________________________________________________________________________
void  TCint::CallFunc_SetArgs(CallFunc_t* func, const char* param) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   f->SetArgs(param);
}
//______________________________________________________________________________
void  TCint::CallFunc_SetFunc(CallFunc_t* func, ClassInfo_t* info, const char* method, const char* params, Long_t* offset) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   f->SetFunc((G__ClassInfo*)info, method, params, offset);
}
//______________________________________________________________________________
void  TCint::CallFunc_SetFunc(CallFunc_t* func, MethodInfo_t* info) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   G__MethodInfo* minfo = (G__MethodInfo*)info;
   f->SetFunc(*minfo);
}
//______________________________________________________________________________
void  TCint::CallFunc_SetFuncProto(CallFunc_t* func, ClassInfo_t* info, const char* method, const char* proto, Long_t* offset) const
{
   // Interface to CINT function
   G__CallFunc* f = (G__CallFunc*)func;
   G__ClassInfo* cinfo = (G__ClassInfo*)info;
   f->SetFuncProto(cinfo, method, proto, offset);
}



//______________________________________________________________________________
//
//  G__ClassInfo interface
//

Long_t TCint::ClassInfo_ClassProperty(ClassInfo_t* cinfo) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->ClassProperty();
}

//______________________________________________________________________________
void TCint::ClassInfo_Delete(ClassInfo_t* cinfo) const
{
   delete(tcling_ClassInfo*) cinfo;
}

//______________________________________________________________________________
void TCint::ClassInfo_Delete(ClassInfo_t* cinfo, void* arena) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   info->Delete(arena);
}

//______________________________________________________________________________
void TCint::ClassInfo_DeleteArray(ClassInfo_t* cinfo, void* arena, bool dtorOnly) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   info->DeleteArray(arena, dtorOnly);
}

//______________________________________________________________________________
void TCint::ClassInfo_Destruct(ClassInfo_t* cinfo, void* arena) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   info->Destruct(arena);
}

//______________________________________________________________________________
ClassInfo_t* TCint::ClassInfo_Factory() const
{
   return (ClassInfo_t*) new tcling_ClassInfo();
}

//______________________________________________________________________________
ClassInfo_t* TCint::ClassInfo_Factory(ClassInfo_t* cinfo) const
{
   return (ClassInfo_t*) new tcling_ClassInfo(*(tcling_ClassInfo*)cinfo);
}

//______________________________________________________________________________
ClassInfo_t* TCint::ClassInfo_Factory(const char* name) const
{
   return (ClassInfo_t*) new tcling_ClassInfo(name);
}

//______________________________________________________________________________
int TCint::ClassInfo_GetMethodNArg(ClassInfo_t* cinfo, const char* method, const char* proto) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   G__MethodInfo meth;
   if (info) {
      Long_t offset = 0L;
      meth = info->GetMethod(method, proto, &offset);
   }
   if (meth.IsValid()) {
      return meth.NArg();
   }
   return -1;
}

//______________________________________________________________________________
bool TCint::ClassInfo_HasDefaultConstructor(ClassInfo_t* cinfo) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->HasDefaultConstructor();
}

//______________________________________________________________________________
bool TCint::ClassInfo_HasMethod(ClassInfo_t* cinfo, const char* name) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->HasMethod(name);
}

//______________________________________________________________________________
void TCint::ClassInfo_Init(ClassInfo_t* cinfo, const char* funcname) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   info->Init(funcname);
}

//______________________________________________________________________________
void TCint::ClassInfo_Init(ClassInfo_t* cinfo, int tagnum) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   info->Init(tagnum);
}

//______________________________________________________________________________
bool TCint::ClassInfo_IsBase(ClassInfo_t* cinfo, const char* name) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->IsBase(name);
}

//______________________________________________________________________________
bool TCint::ClassInfo_IsEnum(const char* name) const
{
   G__ClassInfo info(name);
   if (info.IsValid() && (info.Property() & G__BIT_ISENUM)) {
      return kTRUE;
   }
   return kFALSE;
}

//______________________________________________________________________________
bool TCint::ClassInfo_IsLoaded(ClassInfo_t* cinfo) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->IsLoaded();
}

//______________________________________________________________________________
bool TCint::ClassInfo_IsValid(ClassInfo_t* cinfo) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->IsValid();
}

//______________________________________________________________________________
bool TCint::ClassInfo_IsValidMethod(ClassInfo_t* cinfo, const char* method, const char* proto, Long_t* offset) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->GetMethod(method, proto, offset).IsValid();
}

//______________________________________________________________________________
int TCint::ClassInfo_Next(ClassInfo_t* cinfo) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->Next();
}

//______________________________________________________________________________
void* TCint::ClassInfo_New(ClassInfo_t* cinfo) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->New();
}

//______________________________________________________________________________
void* TCint::ClassInfo_New(ClassInfo_t* cinfo, int n) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->New(n);
}

//______________________________________________________________________________
void* TCint::ClassInfo_New(ClassInfo_t* cinfo, int n, void* arena) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->New(n, arena);
}

//______________________________________________________________________________
void* TCint::ClassInfo_New(ClassInfo_t* cinfo, void* arena) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->New(arena);
}

//______________________________________________________________________________
Long_t TCint::ClassInfo_Property(ClassInfo_t* cinfo) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->Property();
}

//______________________________________________________________________________
int TCint::ClassInfo_RootFlag(ClassInfo_t* cinfo) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->RootFlag();
}

//______________________________________________________________________________
int TCint::ClassInfo_Size(ClassInfo_t* cinfo) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->Size();
}

//______________________________________________________________________________
Long_t TCint::ClassInfo_Tagnum(ClassInfo_t* cinfo) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->Tagnum();
}

//______________________________________________________________________________
const char* TCint::ClassInfo_FileName(ClassInfo_t* cinfo) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->FileName();
}

//______________________________________________________________________________
const char* TCint::ClassInfo_FullName(ClassInfo_t* cinfo) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->Fullname();
}

//______________________________________________________________________________
const char* TCint::ClassInfo_Name(ClassInfo_t* cinfo) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->Name();
}

//______________________________________________________________________________
const char* TCint::ClassInfo_Title(ClassInfo_t* cinfo) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->Title();
}

//______________________________________________________________________________
const char* TCint::ClassInfo_TmpltName(ClassInfo_t* cinfo) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* info = tcling_info->GetClassInfo();
   return info->TmpltName();
}



//______________________________________________________________________________
//
//  G__BaseClassInfo interface
//

//______________________________________________________________________________
void TCint::BaseClassInfo_Delete(BaseClassInfo_t* bcinfo) const
{
   tcling_BaseClassInfo* tcling_info = (tcling_BaseClassInfo*) bcinfo;
   G__BaseClassInfo* info = tcling_info->GetBaseClassInfo();
   delete info;
}

//______________________________________________________________________________
BaseClassInfo_t* TCint::BaseClassInfo_Factory(ClassInfo_t* cinfo) const
{
   tcling_ClassInfo* tcling_info = (tcling_ClassInfo*) cinfo;
   G__ClassInfo* cinfo1 = tcling_info->GetClassInfo();
   G__BaseClassInfo* info = new G__BaseClassInfo(*cinfo1);
   return info;
}

//______________________________________________________________________________
int TCint::BaseClassInfo_Next(BaseClassInfo_t* bcinfo) const
{
   tcling_BaseClassInfo* tcling_info = (tcling_BaseClassInfo*) bcinfo;
   G__BaseClassInfo* info = tcling_info->GetBaseClassInfo();
   return info->Next();
}

//______________________________________________________________________________
int TCint::BaseClassInfo_Next(BaseClassInfo_t* bcinfo, int onlyDirect) const
{
   tcling_BaseClassInfo* tcling_info = (tcling_BaseClassInfo*) bcinfo;
   G__BaseClassInfo* info = tcling_info->GetBaseClassInfo();
   return info->Next(onlyDirect);
}

//______________________________________________________________________________
Long_t TCint::BaseClassInfo_Offset(BaseClassInfo_t* bcinfo) const
{
   tcling_BaseClassInfo* tcling_info = (tcling_BaseClassInfo*) bcinfo;
   G__BaseClassInfo* info = tcling_info->GetBaseClassInfo();
   return info->Offset();
}

//______________________________________________________________________________
Long_t TCint::BaseClassInfo_Property(BaseClassInfo_t* bcinfo) const
{
   tcling_BaseClassInfo* tcling_info = (tcling_BaseClassInfo*) bcinfo;
   G__BaseClassInfo* info = tcling_info->GetBaseClassInfo();
   return info->Property();
}

//______________________________________________________________________________
Long_t TCint::BaseClassInfo_Tagnum(BaseClassInfo_t* bcinfo) const
{
   tcling_BaseClassInfo* tcling_info = (tcling_BaseClassInfo*) bcinfo;
   G__BaseClassInfo* info = tcling_info->GetBaseClassInfo();
   return info->Tagnum();
}

//______________________________________________________________________________
const char* TCint::BaseClassInfo_FullName(BaseClassInfo_t* bcinfo) const
{
   tcling_BaseClassInfo* tcling_info = (tcling_BaseClassInfo*) bcinfo;
   G__BaseClassInfo* info = tcling_info->GetBaseClassInfo();
   return info->Fullname();
}

//______________________________________________________________________________
const char* TCint::BaseClassInfo_Name(BaseClassInfo_t* bcinfo) const
{
   tcling_BaseClassInfo* tcling_info = (tcling_BaseClassInfo*) bcinfo;
   G__BaseClassInfo* info = tcling_info->GetBaseClassInfo();
   return info->Name();
}

//______________________________________________________________________________
const char* TCint::BaseClassInfo_TmpltName(BaseClassInfo_t* bcinfo) const
{
   tcling_BaseClassInfo* tcling_info = (tcling_BaseClassInfo*) bcinfo;
   G__BaseClassInfo* info = tcling_info->GetBaseClassInfo();
   return info->TmpltName();
}

//______________________________________________________________________________
//
//  G__DataMemberInfo interface
//

//______________________________________________________________________________
int TCint::DataMemberInfo_ArrayDim(DataMemberInfo_t* dminfo) const
{
   tcling_DataMemberInfo* tcling_info = (tcling_DataMemberInfo*) dminfo;
   return tcling_info->ArrayDim();
}

//______________________________________________________________________________
void TCint::DataMemberInfo_Delete(DataMemberInfo_t* dminfo) const
{
   delete(tcling_DataMemberInfo*) dminfo;
}

//______________________________________________________________________________
DataMemberInfo_t* TCint::DataMemberInfo_Factory(ClassInfo_t* clinfo /*= 0*/) const
{
   tcling_ClassInfo* tcling_class_info = (tcling_ClassInfo*) clinfo;
   if (tcling_class_info) {
      G__ClassInfo* clinfo1 = tcling_class_info->GetClassInfo();
      if (clinfo1) {
         return (DataMemberInfo_t*) new tcling_DataMemberInfo(tcling_class_info);
      }
   }
   return (DataMemberInfo_t*) new tcling_DataMemberInfo();
}

//______________________________________________________________________________
DataMemberInfo_t* TCint::DataMemberInfo_FactoryCopy(DataMemberInfo_t* dminfo) const
{
   tcling_DataMemberInfo* tcling_info = (tcling_DataMemberInfo*) dminfo;
   return (DataMemberInfo_t*) new tcling_DataMemberInfo(*tcling_info);
}

//______________________________________________________________________________
bool TCint::DataMemberInfo_IsValid(DataMemberInfo_t* dminfo) const
{
   tcling_DataMemberInfo* tcling_info = (tcling_DataMemberInfo*) dminfo;
   return tcling_info->IsValid();
}

//______________________________________________________________________________
int TCint::DataMemberInfo_MaxIndex(DataMemberInfo_t* dminfo, Int_t dim) const
{
   tcling_DataMemberInfo* tcling_info = (tcling_DataMemberInfo*) dminfo;
   return tcling_info->MaxIndex(dim);
}

//______________________________________________________________________________
int TCint::DataMemberInfo_Next(DataMemberInfo_t* dminfo) const
{
   tcling_DataMemberInfo* tcling_info = (tcling_DataMemberInfo*) dminfo;
   return tcling_info->Next();
}

//______________________________________________________________________________
Long_t TCint::DataMemberInfo_Offset(DataMemberInfo_t* dminfo) const
{
   tcling_DataMemberInfo* tcling_info = (tcling_DataMemberInfo*) dminfo;
   return tcling_info->Offset();
}

//______________________________________________________________________________
Long_t TCint::DataMemberInfo_Property(DataMemberInfo_t* dminfo) const
{
   tcling_DataMemberInfo* tcling_info = (tcling_DataMemberInfo*) dminfo;
   return tcling_info->Property();
}

//______________________________________________________________________________
Long_t TCint::DataMemberInfo_TypeProperty(DataMemberInfo_t* dminfo) const
{
   tcling_DataMemberInfo* tcling_info = (tcling_DataMemberInfo*) dminfo;
   return tcling_info->TypeProperty();
}

//______________________________________________________________________________
int TCint::DataMemberInfo_TypeSize(DataMemberInfo_t* dminfo) const
{
   tcling_DataMemberInfo* tcling_info = (tcling_DataMemberInfo*) dminfo;
   return tcling_info->TypeSize();
}

//______________________________________________________________________________
const char* TCint::DataMemberInfo_TypeName(DataMemberInfo_t* dminfo) const
{
   tcling_DataMemberInfo* tcling_info = (tcling_DataMemberInfo*) dminfo;
   return tcling_info->TypeName();
}

//______________________________________________________________________________
const char* TCint::DataMemberInfo_TypeTrueName(DataMemberInfo_t* dminfo) const
{
   tcling_DataMemberInfo* tcling_info = (tcling_DataMemberInfo*) dminfo;
   return tcling_info->TypeTrueName();
}

//______________________________________________________________________________
const char* TCint::DataMemberInfo_Name(DataMemberInfo_t* dminfo) const
{
   tcling_DataMemberInfo* tcling_info = (tcling_DataMemberInfo*) dminfo;
   return tcling_info->Name();
}

//______________________________________________________________________________
const char* TCint::DataMemberInfo_Title(DataMemberInfo_t* dminfo) const
{
   tcling_DataMemberInfo* tcling_info = (tcling_DataMemberInfo*) dminfo;
   return tcling_info->Title();
}

//______________________________________________________________________________
const char* TCint::DataMemberInfo_ValidArrayIndex(DataMemberInfo_t* dminfo) const
{
   tcling_DataMemberInfo* tcling_info = (tcling_DataMemberInfo*) dminfo;
   return tcling_info->ValidArrayIndex();
}



//______________________________________________________________________________
// G__MethodInfo interface
//______________________________________________________________________________
void  TCint::MethodInfo_Delete(MethodInfo_t* minfo) const
{
   // Interface to CINT function
   G__MethodInfo* info = (G__MethodInfo*)minfo;
   delete info;
}
//______________________________________________________________________________
void  TCint::MethodInfo_CreateSignature(MethodInfo_t* minfo, TString& signature) const
{
   // Interface to CINT function
   G__MethodInfo* info = (G__MethodInfo*)minfo;
   G__MethodArgInfo arg(*info);
   int ifirst = 0;
   signature = "(";
   while (arg.Next()) {
      if (ifirst) {
         signature += ", ";
      }
      if (arg.Type() == 0) {
         break;
      }
      signature += arg.Type()->Name();
      if (arg.Name() && strlen(arg.Name())) {
         signature += " ";
         signature += arg.Name();
      }
      if (arg.DefaultValue()) {
         signature += " = ";
         signature += arg.DefaultValue();
      }
      ifirst++;
   }
   signature += ")";
}
//______________________________________________________________________________
MethodInfo_t* TCint::MethodInfo_Factory() const
{
   // Interface to CINT function
   G__MethodInfo* info = new G__MethodInfo();
   return info;
}
//______________________________________________________________________________
MethodInfo_t* TCint::MethodInfo_FactoryCopy(MethodInfo_t* minfo) const
{
   // Interface to CINT function
   G__MethodInfo* info1 = (G__MethodInfo*)minfo;
   G__MethodInfo* info  = new G__MethodInfo(*info1);
   return info;
}
//______________________________________________________________________________
void* TCint::MethodInfo_InterfaceMethod(MethodInfo_t* minfo) const
{
   // Interface to CINT function
   G__MethodInfo* info = (G__MethodInfo*)minfo;
   G__InterfaceMethod pfunc = info->InterfaceMethod();
   if (!pfunc) {
      struct G__bytecodefunc* bytecode = info->GetBytecode();
      if (bytecode) {
         pfunc = (G__InterfaceMethod)G__exec_bytecode;
      }
      else {
         pfunc = (G__InterfaceMethod)NULL;
      }
   }
   return (void*)pfunc;
}
//______________________________________________________________________________
bool  TCint::MethodInfo_IsValid(MethodInfo_t* minfo) const
{
   // Interface to CINT function
   G__MethodInfo* info = (G__MethodInfo*)minfo;
   return info->IsValid();
}
//______________________________________________________________________________
int   TCint::MethodInfo_NArg(MethodInfo_t* minfo) const
{
   // Interface to CINT function
   G__MethodInfo* info = (G__MethodInfo*)minfo;
   return info->NArg();
}
//______________________________________________________________________________
int   TCint::MethodInfo_NDefaultArg(MethodInfo_t* minfo) const
{
   // Interface to CINT function
   G__MethodInfo* info = (G__MethodInfo*)minfo;
   return info->NDefaultArg();
}
//______________________________________________________________________________
int   TCint::MethodInfo_Next(MethodInfo_t* minfo) const
{
   // Interface to CINT function
   G__MethodInfo* info = (G__MethodInfo*)minfo;
   return info->Next();
}
//______________________________________________________________________________
Long_t  TCint::MethodInfo_Property(MethodInfo_t* minfo) const
{
   // Interface to CINT function
   G__MethodInfo* info = (G__MethodInfo*)minfo;
   return info->Property();
}
//______________________________________________________________________________
void* TCint::MethodInfo_Type(MethodInfo_t* minfo) const
{
   // Interface to CINT function
   G__MethodInfo* info = (G__MethodInfo*)minfo;
   return info->Type();
}
//______________________________________________________________________________
const char* TCint::MethodInfo_GetMangledName(MethodInfo_t* minfo) const
{
   // Interface to CINT function
   G__MethodInfo* info = (G__MethodInfo*)minfo;
   return info->GetMangledName();
}
//______________________________________________________________________________
const char* TCint::MethodInfo_GetPrototype(MethodInfo_t* minfo) const
{
   // Interface to CINT function
   G__MethodInfo* info = (G__MethodInfo*)minfo;
   return info->GetPrototype();
}
//______________________________________________________________________________
const char* TCint::MethodInfo_Name(MethodInfo_t* minfo) const
{
   // Interface to CINT function
   G__MethodInfo* info = (G__MethodInfo*)minfo;
   return info->Name();
}
//______________________________________________________________________________
const char* TCint::MethodInfo_TypeName(MethodInfo_t* minfo) const
{
   // Interface to CINT function
   G__MethodInfo* info = (G__MethodInfo*)minfo;
   return info->Type()->Name();
}
//______________________________________________________________________________
const char* TCint::MethodInfo_Title(MethodInfo_t* minfo) const
{
   // Interface to CINT function
   G__MethodInfo* info = (G__MethodInfo*)minfo;
   return info->Title();
}

//______________________________________________________________________________
// G__MethodArgInfo interface
//______________________________________________________________________________
void  TCint::MethodArgInfo_Delete(MethodArgInfo_t* marginfo) const
{
   // Interface to CINT function
   G__MethodArgInfo* info = (G__MethodArgInfo*)marginfo;
   delete info;
}
//______________________________________________________________________________
MethodArgInfo_t* TCint::MethodArgInfo_Factory() const
{
   // Interface to CINT function
   G__MethodArgInfo* info = new G__MethodArgInfo();
   return info;
}
//______________________________________________________________________________
MethodArgInfo_t* TCint::MethodArgInfo_FactoryCopy(MethodArgInfo_t* marginfo) const
{
   // Interface to CINT function
   G__MethodArgInfo* info1 = (G__MethodArgInfo*)marginfo;
   G__MethodArgInfo* info  = new G__MethodArgInfo(*info1);
   return info;
}
//______________________________________________________________________________
bool  TCint::MethodArgInfo_IsValid(MethodArgInfo_t* marginfo) const
{
   // Interface to CINT function
   G__MethodArgInfo* info = (G__MethodArgInfo*)marginfo;
   return info->IsValid();
}
//______________________________________________________________________________
int  TCint::MethodArgInfo_Next(MethodArgInfo_t* marginfo) const
{
   // Interface to CINT function
   G__MethodArgInfo* info = (G__MethodArgInfo*)marginfo;
   return info->Next();
}
//______________________________________________________________________________
Long_t TCint::MethodArgInfo_Property(MethodArgInfo_t* marginfo) const
{
   // Interface to CINT function
   G__MethodArgInfo* info = (G__MethodArgInfo*)marginfo;
   return info->Property();
}
//______________________________________________________________________________
const char* TCint::MethodArgInfo_DefaultValue(MethodArgInfo_t* marginfo) const
{
   // Interface to CINT function
   G__MethodArgInfo* info = (G__MethodArgInfo*)marginfo;
   return info->DefaultValue();;
}
//______________________________________________________________________________
const char* TCint::MethodArgInfo_Name(MethodArgInfo_t* marginfo) const
{
   // Interface to CINT function
   G__MethodArgInfo* info = (G__MethodArgInfo*)marginfo;
   return info->Name();
}
//______________________________________________________________________________
const char* TCint::MethodArgInfo_TypeName(MethodArgInfo_t* marginfo) const
{
   // Interface to CINT function
   G__MethodArgInfo* info = (G__MethodArgInfo*)marginfo;
   return info->Type()->Name();
}


//______________________________________________________________________________
//
//  G__TypeInfo interface
//

//______________________________________________________________________________
void TCint::TypeInfo_Delete(TypeInfo_t* tinfo) const
{
   delete(tcling_TypeInfo*) tinfo;
}

//______________________________________________________________________________
TypeInfo_t* TCint::TypeInfo_Factory() const
{
   return (TypeInfo_t*) new tcling_TypeInfo();
}

//______________________________________________________________________________
TypeInfo_t* TCint::TypeInfo_Factory(G__value* pvalue) const
{
   return (TypeInfo_t*) new tcling_TypeInfo(pvalue);
}

//______________________________________________________________________________
TypeInfo_t* TCint::TypeInfo_FactoryCopy(TypeInfo_t* tinfo) const
{
   return (TypeInfo_t*) new tcling_TypeInfo(*(tcling_TypeInfo*)tinfo);
}

//______________________________________________________________________________
void TCint::TypeInfo_Init(TypeInfo_t* tinfo, const char* name) const
{
   tcling_TypeInfo* tcling_info = (tcling_TypeInfo*) tinfo;
   G__TypeInfo* info = tcling_info->GetTypeInfo();
   info->Init(name);
}

//______________________________________________________________________________
bool TCint::TypeInfo_IsValid(TypeInfo_t* tinfo) const
{
   tcling_TypeInfo* tcling_info = (tcling_TypeInfo*) tinfo;
   G__TypeInfo* info = tcling_info->GetTypeInfo();
   return info->IsValid();
}

//______________________________________________________________________________
const char* TCint::TypeInfo_Name(TypeInfo_t* tinfo) const
{
   tcling_TypeInfo* tcling_info = (tcling_TypeInfo*) tinfo;
   G__TypeInfo* info = tcling_info->GetTypeInfo();
   return info->Name();
}

//______________________________________________________________________________
Long_t TCint::TypeInfo_Property(TypeInfo_t* tinfo) const
{
   tcling_TypeInfo* tcling_info = (tcling_TypeInfo*) tinfo;
   G__TypeInfo* info = tcling_info->GetTypeInfo();
   return info->Property();
}

//______________________________________________________________________________
int TCint::TypeInfo_RefType(TypeInfo_t* tinfo) const
{
   tcling_TypeInfo* tcling_info = (tcling_TypeInfo*) tinfo;
   G__TypeInfo* info = tcling_info->GetTypeInfo();
   return info->Reftype();
}

//______________________________________________________________________________
int TCint::TypeInfo_Size(TypeInfo_t* tinfo) const
{
   tcling_TypeInfo* tcling_info = (tcling_TypeInfo*) tinfo;
   G__TypeInfo* info = tcling_info->GetTypeInfo();
   return info->Size();
}

//______________________________________________________________________________
const char* TCint::TypeInfo_TrueName(TypeInfo_t* tinfo) const
{
   tcling_TypeInfo* tcling_info = (tcling_TypeInfo*) tinfo;
   G__TypeInfo* info = tcling_info->GetTypeInfo();
   return info->TrueName();
}


//______________________________________________________________________________
//
//  G__TypedefInfo interface
//

//______________________________________________________________________________
void TCint::TypedefInfo_Delete(TypedefInfo_t* tinfo) const
{
   delete(tcling_TypedefInfo*) tinfo;
}

//______________________________________________________________________________
TypedefInfo_t* TCint::TypedefInfo_Factory() const
{
   return (TypedefInfo_t*) new tcling_TypedefInfo();
}

//______________________________________________________________________________
TypedefInfo_t* TCint::TypedefInfo_Factory(int typenum) const
{
   return (TypedefInfo_t*) new tcling_TypedefInfo(typenum);
}

//______________________________________________________________________________
TypedefInfo_t* TCint::TypedefInfo_FactoryCopy(TypedefInfo_t* tinfo) const
{
   return (TypedefInfo_t*) new tcling_TypedefInfo(*(tcling_TypedefInfo*)tinfo);
}

//______________________________________________________________________________
TypedefInfo_t TCint::TypedefInfo_Init(TypedefInfo_t* tinfo, const char* name) const
{
   tcling_TypedefInfo* tcling_info = (tcling_TypedefInfo*) tinfo;
   G__TypedefInfo* info = tcling_info->GetTypedefInfo();
   info->Init(name);
}

//______________________________________________________________________________
bool TCint::TypedefInfo_IsValid(TypedefInfo_t* tinfo) const
{
   tcling_TypedefInfo* tcling_info = (tcling_TypedefInfo*) tinfo;
   G__TypedefInfo* info = tcling_info->GetTypedefInfo();
   return info->IsValid();
}

//______________________________________________________________________________
Long_t TCint::TypedefInfo_Property(TypedefInfo_t* tinfo) const
{
   tcling_TypedefInfo* tcling_info = (tcling_TypedefInfo*) tinfo;
   G__TypedefInfo* info = tcling_info->GetTypedefInfo();
   return info->Property();
}

//______________________________________________________________________________
int TCint::TypedefInfo_Size(TypedefInfo_t* tinfo) const
{
   tcling_TypedefInfo* tcling_info = (tcling_TypedefInfo*) tinfo;
   G__TypedefInfo* info = tcling_info->GetTypedefInfo();
   return info->Size();
}

//______________________________________________________________________________
const char* TCint::TypedefInfo_TrueName(TypedefInfo_t* tinfo) const
{
   tcling_TypedefInfo* tcling_info = (tcling_TypedefInfo*) tinfo;
   G__TypedefInfo* info = tcling_info->GetTypedefInfo();
   return info->TrueName();
}

//______________________________________________________________________________
const char* TCint::TypedefInfo_Name(TypedefInfo_t* tinfo) const
{
   tcling_TypedefInfo* tcling_info = (tcling_TypedefInfo*) tinfo;
   G__TypedefInfo* info = tcling_info->GetTypedefInfo();
   return info->Name();
}

//______________________________________________________________________________
const char* TCint::TypedefInfo_Title(TypedefInfo_t* tinfo) const
{
   tcling_TypedefInfo* tcling_info = (tcling_TypedefInfo*) tinfo;
   G__TypedefInfo* info = tcling_info->GetTypedefInfo();
   return info->Title();
}

