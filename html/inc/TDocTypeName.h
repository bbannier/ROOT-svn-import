// @(#)root/html:$Id$
// Author: Axel Naumann 2009-11-03

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TDocTypeName
#define ROOT_TDocTypeName

////////////////////////////////////////////////////////////////////////////
//                                                                        //
// TDocTypeName                                                           //
//                                                                        //
// Placeholder for a documented type                                      //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TList
#include "TList.h"
#endif
#ifndef ROOT_TString
#include "TString.h"
#endif

class TKey;
class TDictionary;

namespace Doc {
class TDocumented;
class TModuleDoc;

class TDocTypeName: public TObject {
public:

   // Kind of C++ record this TDocTypeName is describing
   enum EKind {
      kClass = BIT(16), // it's a class
      kStruct, // it's a struct
      kUnion, // it's a union
      kEnum, // it's an enum
      kNamespace, // it's a namespace
      kTypedef, // it's a typedef to a class - should that be here?
      kUnknownKind, // not set
      kNumKinds = kUnknownKind - kClass, // number of kinds
      kKindBitMask = BIT(16) | BIT(17) | BIT(18)
   };

   TDocTypeName(): fModule(0), fKey(0), fDict(0), fDoc(0) {}
   TDocTypeName(TKey* key, TModuleDoc* module);
   TDocTypeName(TDictionary* dict, TDocumented* doc = 0, TModuleDoc* module = 0);

   virtual ~TDocTypeName();

   const char* GetName() const;

   TModuleDoc* GetModule() const { return fModule; }
   void SetModule(TModuleDoc* module) { fModule = module; }

   EKind GetKind() const { return (EKind) (TestBits(kKindBitMask)); }
   void SetKind(EKind kind) { SetBit(kind); }

   TList& GetFiles() { return fFiles; }
   Bool_t HaveSource() const { return !fFiles.IsEmpty(); }
   TDocumented* GetDoc() const { return fDoc; }
   void SetDoc(TDocumented* doc);
   TKey* GetKey() const { return fKey; }
   void SetKey(TKey* key) { fKey = key; }
   TDictionary* GetDictionary() const { return fDict; }
   const TString& GetDeclFileName() const { return fDeclFileName; }
   void SetDeclFileName(const char* name) { fDeclFileName = name; }
   const TString& GetImplFileName() const { return fImplFileName; }
   void SetImplFileName(const char* name) { fImplFileName = name; }
   const TString& GetHtmlFileName() const { return fHtmlFileName; }
   void SetHtmlFileName(const char* name) { fHtmlFileName = name; }

   void Select(Bool_t selected = kTRUE) { SetBit(BIT(19), selected); }
   Bool_t IsSelected() const { return TestBit(BIT(19)); }

   Int_t Compare(const TObject* obj) const;

   Bool_t IsNewerThan(TKey* other) const;

private:
   TModuleDoc*  fModule; // module that this class belongs to
   TKey*        fKey; // key holding the TClassDoc for this type name
   TDictionary* fDict; // ROOT dictionary object for this type name
   TDocumented* fDoc; // documentation object for this type name (TClassDoc or TTypedefDoc)
   TString      fDeclFileName; // user-provided declaration file
   TString      fImplFileName; // user-provided implementation file
   TString      fHtmlFileName; // output file name
   TList        fFiles; // declaration and implementation files (TFileSysEntry)

   ClassDef(TDocTypeName, 0); // Documented type
};
} // namespace Doc

#endif // ROOT_TDocTypeName

