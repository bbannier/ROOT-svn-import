// @(#)root/html:$Id$
// Author: Axel Naumann 2009-10-26

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TClassDoc.h"

#include "TClass.h"

Doc::TClassDoc::TClassDoc(const char* name, TClass* cl, const char* module):
   TDocumented(name, cl), fModule(module), fKind(kUnknownKind)
{
   // Constructor initializing all members
   if (cl->Property() & kIsClass) {
      fKind = kClass;
   } else if (cl->Property() & kIsEnum) {
      fKind = kEnum;
   } else if (cl->Property() & kIsNamespace) {
      fKind = kNamespace;
   } else if (cl->Property() & kIsStruct) {
      fKind = kStruct;
   } else if (cl->Property() & kIsTypedef) {
      fKind = kTypedef;
   } else if (cl->Property() & kIsUnion) {
      fKind = kUnion;
   }
   fMembers.SetOwner();
   fTypes.SetOwner();
   fSeeAlso.SetOwner();
}

Doc::TClassDoc::~TClassDoc()
{
   // Destructor
}
