// @(#)root/proof:$Id:$
// Author: G. Ganis Apr 2008

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/


#ifndef ROOT_TProofQueryOnHold
#define ROOT_TProofQueryOnHold

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofQueryOnHold                                                    //
//                                                                      //
// Handles queries put on-hold                                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif
#ifndef ROOT_TString
#include "TString.h"
#endif

class TDSet;
class TList;
class TMacro;
class TMessage;

class TProofQueryOnHold : public TObject {

public:
   // TProofQueryOnHold status bits
   enum EStatusBits {
      kOwnsMessage     = BIT(14),
      kOwnsDSet        = BIT(15),
      kOwnsOutputList  = BIT(16)
   };

private:
   TString     fTag;         // Unique tag
   TList      *fPackages;    // List of Packages to be enabled
   TString     fLibPaths;    // Library paths needed
   TString     fIncPaths;    // Include paths needed
   TMessage   *fMessage;     // Original message
   TDSet      *fDSet;        // TDSet to be processed if not from input list
   TList      *fOutputList;  // Current status of the output list
   TMacro     *fSelecHdr;    // Selector header file
   TMacro     *fSelecImp;    // Selector implementation file
   Long64_t    fProcessed;   // Number of Entries processed
   Long64_t    fToProcess;   // Number of Entries to process

   TMessage   *GetNextMessage(Int_t fd); // Read next message from file
   Int_t       PutMessage(TMessage *m, Int_t fd); // Write message to file

public:
   TProofQueryOnHold(const char *path);
   TProofQueryOnHold(const char *n, TList *p, const char *l, const char *i, TMessage *m,
                     const char *selec, TDSet *dset = 0, TList *outlist = 0);
   virtual ~TProofQueryOnHold();

   const char *Tag() const { return fTag; }
   TList      *Packages() const { return fPackages; }
   const char *LibPaths() const { return fLibPaths; }
   const char *IncPaths() const { return fIncPaths; }
   TMessage   *Message() const { return fMessage; }
   TDSet      *DSet() { ResetBit(kOwnsDSet); return fDSet; }
   TList      *OutputList() { ResetBit(kOwnsOutputList); return fOutputList; }
   TMacro     *SelecImp() const { return fSelecImp; }
   TMacro     *SelecHdr() const { return fSelecHdr; }
   Long64_t    Processed() const { return fProcessed; }
   Long64_t    ToProcess() const { return fToProcess; }

   Int_t       Save(const char *path);
   void        SetProcessed(Long64_t e) { fProcessed = e; }
   void        SetToProcess(Long64_t e) { fToProcess = e; }

   ClassDef(TProofQueryOnHold, 0) // Handles queries put on-hold
};

#endif
