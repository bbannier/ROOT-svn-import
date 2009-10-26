// @(#)root/html:$Id$
// Author: Nenad Buncic   18/10/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
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
namespace Doc {
   class TClassDoc;

class TModuleDocInfo;
//____________________________________________________________________
//
// Cache doc info for all known classes
//
class TClassDocInfo: public TObject {
public:
   // initialize the object
   TClassDocInfo(Doc::TClassDoc* cd, const TString& htmlFileName, TList* srcfiles = 0):
      fClassDoc(cd),
      fHtmlFileName(htmlFileName),
      fSrcFiles(srcfiles),
      fSelected(kTRUE)
   { FindModule(); }

   virtual ~TClassDocInfo() {}

   Doc::TClassDoc* GetClass() const { return fClass; }
   const char*     GetName() const;
   TCollection*    GetFiles() const { return fList; }

   TModuleDocInfo* GetModule() const { return fModule; }

   void            SetSelected(Bool_t sel = kTRUE) { fSelected = sel; }
   Bool_t          IsSelected() const { return fSelected; }
   Bool_t          HaveSource() const { return (fSrcFiles); }
   
   ULong_t         Hash() const;

   TList&          GetListOfTypedefs() { return fTypedefs; }

   Bool_t          IsSortable() const { return kTRUE; }
   Int_t           Compare(const TObject* obj) const;

private:
   TClassDocInfo();
   void FindModule();

   TClassDoc*            fClassDoc; // class (or typedef) documentation represented by this info object
   TModuleDocInfo*       fModule; // module this class is in
   TString               fHtmlFileName; // name of the HTML doc file
   TList*                fSrcFiles; // typedefs to this class
   Bool_t                fSelected; // selected for doc output

   ClassDef(TClassDocInfo,0); // info cache for class documentation
};

//____________________________________________________________________
//
// Cache doc info for all known modules
//
class TModuleDocInfo: public TObject {
public:
   TModuleDocInfo(Doc::TModuleDoc* moddoc, TModuleDocInfo* super): 
      fModuleDoc(moddoc), fSuper(super), fSub(0), fSelected(kTRUE) {
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
   Doc::TModuleDoc* fModuleDoc; // module documentation
   TModuleDocInfo* fSuper; // module containing this module
   THashList   fSub; // modules contained in this module
   Bool_t      fSelected; // selected for doc output

   ClassDef(TModuleDocInfo,0); // documentation for a group of classes
};

//__________________________________________________________________________
//
// A library's documentation database:
// dependencies and sub-modules
//
class TLibraryDocInfo: public TObject {
 public:
   TLibraryDocInfo(Doc::TLibDocDoc* libdoc): fLibDoc(libdoc) {}

   std::set<std::string>& GetDependencies() {return fDependencies;}
   std::set<std::string>& GetModules() {return fModules;}
   void AddDependency(const std::string& lib) {fDependencies.insert(lib);}
   void AddModule(const std::string& module) {fModules.insert(module);}

 private:
   Doc::TLibDoc* fLibDoc; // library documentation
   std::set<std::string> fDependencies; // dependencies on other libraries
   std::set<std::string> fModules; // modules in the library

   ClassDef(TLibraryDocInfo,0); // documentation for a library
};

}

#endif // ROOT_TDocInfo
