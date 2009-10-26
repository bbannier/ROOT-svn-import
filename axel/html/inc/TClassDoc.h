// @(#)root/html:$Id$
// Author: Axel Naumann 2009-10-26

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TClassDoc
#define ROOT_TClassDoc

////////////////////////////////////////////////////////////////////////////
//                                                                        //
// TClassDoc                                                              //
//                                                                        //
// Documentation for a clas / struct / union / enum / namespace           //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TDocumented
#include "TDocumented.h"
#endif
#ifndef ROOT_TDocStringTable
#include "TDocStringTable.h"
#endif
#ifndef ROOT_TList
#include "TList.h"
#endif

namespace Doc {
class TDataDoc;
class TFunctionDoc;
class TTypedefDoc;

class TClassDoc: public TDocumented {
public:

   // Kind of C++ record this TClassDoc is describing
   enum EKind {
      kClass = BIT(16), // it's a class
      kStruct, // it's a struct
      kUnion, // it's a union
      kEnum, // it's an enum
      kNamespace, // it's a namespace
      kUnknownKind, // not set
      kNumKinds = kUnknownKind, // number of kinds
      kKindBitMask = BIT(16) | BIT(17) | BIT(18)
   };

   TClassDoc() {}
   TClassDoc(const char* name, EKind kind, const char* module, TClass* cl);

   virtual ~TClassDoc();

   const TString& GetModule() const { return fModule; }
   const TCollection* GetMembers() const { return &fMembers; }
   const TCollection* GetTypes() const { return &fTypes; }
   const TCollection* GetSeeAlso() const { return &fSeeAlso; }
   EKind  GetKind() const { return (EKind) (TestBits(kKindBitMask) - BIT(16)); }

   const char* GetURL() const;
   Int_t Compare(const TObject* obj) const;

   TDataDoc* AddDataMember(const char* name, const char* type);
   TFunctionDoc* AddFunctionMember(const char* name, const char* type, const char* signature);
   TTypedefDoc* AddTypedef(const char* name, const TDocString& underlying);

private:
   TDocString fShortDoc; // short documentation
   TString    fModule; // module that this class belongs to
   EKind fKind; // kind of element

   TDocStringTable fRefTypes; // strings referenced by the struct and its members
   TDocStringTable fRefFiles; // files containing the struct and its members
   TList fMembers; // data (TDataDoc) of function (TFunctionDoc) members
   TList fTypes; // types contained in this class, list of TTypedefDoc
   TList fSeeAlso; // types referenced by this class; list of TObjString

   ClassDef(TClassDoc, 1); // Documentation for a class
};
} // namespace Doc

#endif // ROOT_TClassDoc

