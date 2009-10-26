// @(#)root/html:$Id$
// Author: Axel Naumann 2007-01-09

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TFunctionDoc
#define ROOT_TFunctionDoc

////////////////////////////////////////////////////////////////////////////
//                                                                        //
// TFunctionDoc                                                           //
//                                                                        //
// Documentation for a function                                           //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TMemberDoc
#include "TMemberDoc.h"
#endif

namespace Doc {
class TFunctionDoc: public TMemberDoc {
public:
   TFunctionDoc();
   TFunctionDoc(TClassDoc* owner, const char* name, const TDocString& type, const TDocString& signature):
      TMemberDoc(owner, name, type), fSignature(signature) {}

   virtual ~TFunctionDoc();

   const char* GetSignature() const { return fSignature; }
   virtual const char* GetAnchor() const;
   virtual const char* GetURL() const;

protected:
   virtual TDictionary* FindDictionary() const;

private:
   TFunctionDoc(const TFunctionDoc&); // intentionally not implemented

   TDocString fSignature; // the function's signature
   ClassDef(TFunctionDoc, 1); // Documentation for a function
};
} // namespace Doc

#endif // ROOT_TFunctionDoc

