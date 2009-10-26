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

Doc::TClassDoc::TClassDoc(const char* name, EKind kind, const char* module, TClass* cl):
   TDocumented(name, cl), fModule(module), fKind(kind)
{
   // Constructor initializing all members
   fMembers.SetOwner();
   fTypes.SetOwner();
   fSeeAlso.SetOwner();
}

Doc::TClassDoc::~TClassDoc()
{
   // Destructor
}
