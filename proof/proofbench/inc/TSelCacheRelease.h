// @(#)root/proofx:$Id$
// Author: Sangsu Ryu 22/06/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TSelCacheRelease
#define ROOT_TSelCacheRelease

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TSelCacheRelease                                                     //
//                                                                      //
// PROOF selector for file cache release.                               //
// List of files to be cleaned for each node is provided by client.     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TSelector
#include <TSelector.h>
#endif
#ifndef ROOT_TString
#include <TString.h>
#endif

class TList;

class TSelCacheRelease : public TSelector {

private:

public :

   TSelCacheRelease() { }
   virtual ~TSelCacheRelease() { }
   virtual Int_t   Version() const {return 2;}
   virtual void    Begin(TTree *) { }
   virtual void    SlaveBegin(TTree *) { }
   virtual void    Init(TTree *) { }
   virtual Bool_t  Notify() { return kTRUE; }
   virtual Bool_t  Process(Long64_t entry);
   virtual void    SetOption(const char *option) { fOption = option; }
   virtual void    SetObject(TObject *obj) { fObject = obj; }
   virtual void    SetInputList(TList *input) {fInput = input;}
   virtual TList  *GetOutputList() const { return fOutput; }
   virtual void    SlaveTerminate() { }
   virtual void    Terminate() { }

   ClassDef(TSelCacheRelease,0)     //PROOF selector for event file generation
};

#endif

