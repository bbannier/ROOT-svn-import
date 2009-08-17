// @(#)root/html:$Id$
// Author: Axel Naumann, 2007

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TDocInfo
#define ROOT_TDocInfo

#ifndef ROOT_TClass
#include "TClass.h"
#endif
#ifndef ROOT_THashList
#include "THashList.h"
#endif
#ifndef ROOT_TNamed
#include "TNamed.h"
#endif
#include <string>
#include <set>

class TDictionary;

namespace RootDoc {
class TModuleDocInfo;
class TFileSysEntry;

class TSourceFileDocInfo: public TObject {
public:
   TSourceFileDocInfo(const char* fsabs,
      Ssiz_t fsrel = 0,
      Ssiz_t incl = 0);

   const TString& GetFileSystemAbs() const { return fFileSysAbs; }
   const char* GetName() const { return fFileSysAbs.Data() + fBaseOff; }
   const char* GetAsIncluded() const { return fFileSysAbs.Data() + fAsIncludedOff; }
   const char* GetFileSystemRel() const { return fFileSysAbs.Data() + fFileSysRelOff; }
   void SetFileSystemEntry(TFileSysEntry* fs) { fFileSysEntry = fs; }
   void SetFileSystemAbs(const char* name) { fFileSysAbs = name; }
   void SetAsIncluded(Ssiz_t off) { fAsIncludedOff = off; }
   void SetFileSystemRel(Ssiz_t off) { fFileSysRelOff = off; }

private:
   TFileSysEntry* fFileSysEntry; //! file found on disk
   TString fFileSysRel; // file as set by the user or name as found on disk, relative from THtml::SetInputDir()
   Ssiz_t fBaseOff; // filename without directory, offset in fFileSysRel
   TString fAsIncluded; // name as used in #include
   ClassDef(TSourceFileDocInfo, 1); // File representation for THtml
};

class TMethodDocInfo: public TObject {
public:
   TMethodDocInfo(const char* decl, const char* doc):
      fDecl(decl), fDoc(doc) {}
   TMethodDocInfo(TMethod* meth, const char* doc):
      fMethod(meth), fDoc(doc) {}

   const TString& GetDoc() const { return fDoc; }
   const TString& GetDecl() const { return fDecl; }
   TMethod* GetMethod() const { return fMethod; }
   const TString& GetImplAnchor() const { return fImplAnchor; }
   void GetRefID(TString& id) const;

   void SetImplAnchor(const char* anchor) { fImplAnchor = anchor; }
   void SetMethod(TMethod* method) { fMethod = method; }

private:
   TString  fDoc; // documentation for the method
   TString  fDecl; // declaration of the method, to find it in the class's list of methods
   TMethod* fMethod; //! method to be documented
   TString  fImplAnchor; // href anchor in source file
   ClassDef(TMethodDocInfo, 1); // method documentation
};


//____________________________________________________________________
//
// Cache doc info for all known classes
//
class TClassDocInfo: public TObject {
public:
   // initialize given a class or a typedef
   TClassDocInfo(TDictionary* cl);

   virtual ~TClassDocInfo() {}

   TDictionary*    GetClass() const { return fClass; }
   const char*     GetName() const;
   const char*     GetHtmlFileName() const;
   TSourceFileDocInfo* GetDeclFile() const { return fDeclFile; }
   TSourceFileDocInfo* GetImplFile() const { return fImplFile; }

   void            SetModule(TModuleDocInfo* module) { fModule = module; }
   TModuleDocInfo* GetModule() const { return fModule; }

   void            SetSelected(Bool_t sel = kTRUE) { fSelected = sel; }
   Bool_t          IsSelected() const { return fSelected; }
   Bool_t          HaveSource() const { return fDeclFile.GetFileSystemAbs().Length()
                                          || (fClass && !dynamic_cast<TClass*>(fClass)); }

   void            SetHtmlFileName(const char* name) { fHtmlFileName = name; }

   ULong_t         Hash() const;

   TList&          GetListOfTypedefs() { return fTypedefs; }

   Bool_t          IsSortable() const { return kTRUE; }
   Int_t           Compare(const TObject* obj) const;

private:
   TClassDocInfo();

   TDictionary*            fClass; // class (or typedef) represented by this info object
   TModuleDocInfo*         fModule; // module this class is in
   TString                 fHtmlFileName; // name of the HTML doc file
   TSourceFileDocInfo*     fDeclFile; // header
   TSourceFileDocInfo*     fImplFile; // source
   TList                   fTypedefs; // typedefs to this class
   Bool_t                  fSelected; // selected for doc output
   TString                 fClassDoc; // documentation for this class
   TList                   fMethodDocs; // documentation for methods

   ClassDef(TClassDocInfo,0); // info cache for class documentation
};

//____________________________________________________________________
//
// Cache doc info for all known modules
//
class TModuleDocInfo: public TNamed {
public:
   TModuleDocInfo(const char* name, TModuleDocInfo* super, const char* doc = ""): 
      TNamed(name, doc), fSuper(super), fSub(0), fSelected(kTRUE) {
         if (super) super->GetSub().Add(this);
      }
   virtual ~TModuleDocInfo() {}

   void        SetDoc(const char* doc) { SetTitle(doc); }
   const char* GetDoc() const { return GetTitle(); }

   void        SetSelected(Bool_t sel = kTRUE) { fSelected = sel; }
   Bool_t      IsSelected() const { return fSelected; }

   void        AddClass(TClassDocInfo* cl) { fClasses.Add(cl); }
   TList*      GetClasses() { return &fClasses; }

   TModuleDocInfo* GetSuper() const { return fSuper; }
   THashList&  GetSub() { return fSub; }

private:
   TModuleDocInfo* fSuper; // module containing this module
   THashList   fSub; // modules contained in this module
   TList       fClasses;
   Bool_t      fSelected; // selected for doc output

   ClassDef(TModuleDocInfo,0); // documentation for a group of classes
};

//__________________________________________________________________________
//
// A library's documentation database:
// dependencies and sub-modules
//
class TLibraryDocInfo: public TNamed {
 public:
   TLibraryDocInfo() {}
   TLibraryDocInfo(const char* lib): TNamed(lib, "") {}

   std::set<std::string>& GetDependencies() {return fDependencies;}
   std::set<std::string>& GetModules() {return fModules;}
   void AddDependency(const std::string& lib) {fDependencies.insert(lib);}
   void AddModule(const std::string& module) {fModules.insert(module);}

 private:
   std::set<std::string> fDependencies; // dependencies on other libraries
   std::set<std::string> fModules; // modules in the library

   ClassDef(TLibraryDocInfo,0); // documentation for a library
};


#endif // ROOT_TDocInfo
