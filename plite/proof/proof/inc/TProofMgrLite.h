// @(#)root/proofx:$Id$
// Author: G. Ganis, Apr 2008

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofMgrLite
#define ROOT_TProofMgrLite


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofMgrLite                                                        //
//                                                                      //
// Basic functionality implementtaion in the case of Lite sessions      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TProofMgr
#include "TProofMgr.h"
#endif
#ifndef ROOT_TUrl
#include "TUrl.h"
#endif

class TProofMgrLite : public TProofMgr {
private:
   TString    fSessionDir; // The client session directory

   void       ExpandPath(TString &path, Bool_t &sandbox);
#ifdef WIN32
   void       ShowFile(const char *what,
                       Int_t lines = 0, UInt_t opt = 0, const char *pattern = 0);
#endif

public:
   TProofMgrLite(const char *url, Int_t loglevel = -1, const char *alias = "");
   virtual ~TProofMgrLite() { }

   TProof     *CreateSession(const char * = 0, const char * = 0, Int_t = -1);
   TProofLog  *GetSessionLogs(Int_t ridx = 0, const char *stag = 0,
                              const char *pattern = "-v | SvcMsg");
   TObjString *ReadBuffer(const char *file, Long64_t ofs, Int_t len);
   TObjString *ReadBuffer(const char *file, const char *pattern);

   // File system actions
   Int_t       Cp(const char *src, const char *dst = 0, const char *opts = 0);
   void        Grep(const char *what, const char *how = 0, const char *where = 0);
   void        Ls(const char *what = "~/", const char *how = 0, const char *where = 0);
   void        More(const char *what, const char *how = 0, const char *where = 0);
   Int_t       Rm(const char *what, const char *how = 0, const char *where = 0);
   void        Tail(const char *what, const char *how = 0, const char *where = 0);
   Int_t       Md5sum(const char *what, TString &sum, const char *where = 0);
   Int_t       Stat(const char *what, FileStat_t &st, const char *where = 0);

   Int_t       GetFile(const char *remote, const char *local, const char *opt = 0);
   Int_t       PutFile(const char *local, const char *remote, const char *opt = 0);

   ClassDef(TProofMgrLite,0)  // XrdProofd PROOF manager interface
};

#endif
