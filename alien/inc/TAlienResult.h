// @(#)root/alien:$Id$
// Author: Fons Rademakers   3/1/2002

/*************************************************************************
 * Copyright (C) 1995-2002, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TAlienResult
#define ROOT_TAlienResult

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlienResult                                                         //
//                                                                      //
// Class defining interface to a Alien result set.                      //
// Objects of this class are created by TGrid methods.                  //
//                                                                      //
// Related classes are TAlien.                                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TGridResult
#include "TGridResult.h"
#endif

class TFileCollection;


class TAlienResult : public TGridResult {

private:
   mutable TString fFilePath;  // file path

public:
   virtual ~TAlienResult();

   virtual void DumpResult();
   virtual const char       *GetFileName(UInt_t i) const;      // file name of item i
   virtual const char       *GetFileNamePath(UInt_t i) const;  // full path + file name of item i
   virtual const TEntryList *GetEntryList(UInt_t i) const;     // entry list, if it is defined
   virtual const char       *GetPath(UInt_t i) const;          // file path of item i
   virtual const char       *GetKey(UInt_t i, const char *key) const; // key value of item i
   virtual Bool_t            SetKey(UInt_t i, const char *key, const char *value); // set key value of item i
   virtual TFileCollection  *GetFileInfoList() const;          // new allocated List of TFileInfo Objects
   void                      Print(Option_t *option = "") const;
   void                      Print(Option_t *wildcard, Option_t *option) const;

   ClassDef(TAlienResult,0)  // Alien query result set
};

#endif
