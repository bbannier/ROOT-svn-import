// @(#)root/html:$Id$
// Author: Axel Naumann 2007-01-09

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TDataDoc
#define ROOT_TDataDoc

////////////////////////////////////////////////////////////////////////////
//                                                                        //
// TDataDoc                                                               //
//                                                                        //
// Documentation for a global variable / data member                      //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TMemberDoc
#include "TMemberDoc.h"
#endif

namespace Doc {
class TDataDoc: public TMemberDoc {
public:
   TDataDoc();
   TDataDoc(TClassDoc* owner, const char* name, const TDocString& type):
      TMemberDoc(owner, name, type) {}
   virtual ~TDataDoc();

   const char* GetAnchor() const;
   const char* GetURL() const;

protected:
   virtual TDictionary* FindDictionary() const;

   ClassDef(TDataDoc, 1); // Documentation for a data member
};
} // namespace Doc

#endif // ROOT_TDataDoc
