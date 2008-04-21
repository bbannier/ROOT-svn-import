// @(#)root/proof:$Id:$
// Author: G. Ganis Apr 2008

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofQueryOnHold                                                    //
//                                                                      //
// Handles queries put on-hold                                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
#include <errno.h>

#include "TProofQueryOnHold.h"

#include "TDSet.h"
#include "TList.h"
#include "TMacro.h"
#include "TMessage.h"
#include "TObjString.h"
#include "TSystem.h"

//______________________________________________________________________________
TProofQueryOnHold::TProofQueryOnHold(const char *path)
{
   // Constructor from file

   ResetBit(TObject::kInvalidObject);
   ResetBit(kOwnsMessage);
   ResetBit(kOwnsDSet);
   ResetBit(kOwnsOutputList);

   fPackages = 0;
   fLibPaths = "";
   fIncPaths = "";
   fMessage = 0;
   fSelecImp = 0;
   fSelecHdr = 0;
   fDSet = 0;
   fOutputList = 0;
   fProcessed = 0;
   fToProcess = -1;

   // Open the file in truncate mode
   FILE *f = fopen(path, "r");
   if (!f) {
      Info("TProofQueryOnHold", "cannot open file %s", path);
      return;
   }

   TMessage *m = 0;

   // Get messages from file
   TString type;
   while ((m = GetNextMessage(fileno(f)))) {
      if (m->What() == kPROOF_MESSAGE) {
         // Extract type
         (*m) >> type;
         if (type == "aux") {
            (*m) >> fTag >> fPackages >> fLibPaths >> fIncPaths;
         } else if (type == "entries") {
            (*m) >> fProcessed >> fToProcess;
         } else if (type == "selec") {
            (*m) >> fSelecImp >> fSelecHdr;
         } else if (type == "dset") {
            (*m) >> fDSet;
            if (fDSet) SetBit(kOwnsDSet);
         } else if (type == "outputlist") {
            (*m) >> fOutputList;
            if (fOutputList) {
               fOutputList->SetOwner();
               SetBit(kOwnsOutputList);
            }
         } else {
            Warning("TProofQueryOnHold", "unknown auxilliary message type found in file: %s - ignoring",
                    type.Data());
         }
         SafeDelete(m);
      } else if (m->What() == kPROOF_PROCESS) {
         fMessage = m;
         SetBit(kOwnsMessage);
      } else {
         Warning("TProofQueryOnHold","unknown message type found in file: %d - ignoring",
                 m->What());
      }
   }

   Info("TProofQueryOnHold", "proc: %lld, toproc: %ldd", fProcessed, fToProcess);


   // Did we got the minimal stuff?
   if (fTag.IsNull() || !fMessage) {
      Error("TProofQueryOnHold", "cannot fetch the minimal set of info from file %s - setting invalid", path);
      SetBit(TObject::kInvalidObject);
   }

   // Close the file
   fclose(f);
   return;
}

//______________________________________________________________________________
TProofQueryOnHold::TProofQueryOnHold(const char *n, TList *p, const char *l,
                                     const char *i, TMessage *m, const char *sel,
                                     TDSet *dset, TList *outlist)
                : fTag(n), fLibPaths(l), fIncPaths(i), fMessage(m), fDSet(dset),
                  fOutputList(outlist), fProcessed(0), fToProcess(-1)
{
   // Constructor

   ResetBit(TObject::kInvalidObject);
   ResetBit(kOwnsMessage);
   ResetBit(kOwnsDSet);
   ResetBit(kOwnsOutputList);

   fPackages = 0;
   if (p && p->GetSize() > 0) {
      fPackages = new TList;
      fPackages->SetOwner();
      TIter nxp(p);
      TObjString *os = 0;
      while ((os = (TObjString *)nxp())) {
         fPackages->Add(new TObjString(os->GetName()));
      }
   }

   // Init selector TMacros' from file 'selec'
   fSelecHdr = 0;
   fSelecImp = 0;
   TString selec(sel);
   Int_t ip = selec.Last('.');
   if (selec.IsNull() || ip == kNPOS) {
      // Selector file name not given or just in the form of the selector name: the latter
      // will be loaded from a package, so nothing to do here
      return;
   }
   // Extract the fine name first
   TString aclicMode, arguments, io;
   selec = gSystem->SplitAclicMode(selec, aclicMode, arguments, io);

   // Expand the path
   if (!selec.BeginsWith("/") && !selec.BeginsWith("~/")) {
      // Assume local directory
      selec.Insert(0, Form("%s/", gSystem->WorkingDirectory()));
   }
   gSystem->ExpandPathName(selec);

   // Create the macros
   if (gSystem->AccessPathName(selec)) {
      Error("TProofQueryOnHold", "selector file undefined, missing or not readable (%s) - setting invalid",
                                 selec.Data());
      SetBit(TObject::kInvalidObject);
   } else {
      // The implementation file
      fSelecImp = new TMacro;
      fSelecImp->ReadFile(selec);
      fSelecImp->SetName(gSystem->BaseName(selec));
      // The header file
      TString seleh(selec);
      ip = seleh.Last('.');
      if (ip != kNPOS) {
         seleh.Remove(ip);
         // Try ".h" and ".hh"
         seleh += ".h";
         if (gSystem->AccessPathName(seleh)) seleh += "h";
         if (!gSystem->AccessPathName(seleh)) {
            fSelecHdr = new TMacro;
            fSelecHdr->ReadFile(seleh);
            fSelecHdr->SetName(gSystem->BaseName(seleh));
         }
         selec.Remove(ip);
      }
      if (!fSelecHdr) {
         Error("TProofQueryOnHold", "coud not find selector header file %s.h, .hh", seleh.Data());
         SetBit(TObject::kInvalidObject);
      }
   }
}

//______________________________________________________________________________
TProofQueryOnHold::~TProofQueryOnHold()
{
   // Destructor

   SafeDelete(fPackages);

   if (TestBit(kOwnsMessage) && fMessage)
      delete fMessage;
   fMessage = 0;
   if (TestBit(kOwnsDSet) && fDSet)
      delete fDSet;
   fDSet = 0;
   if (TestBit(kOwnsOutputList) && fOutputList)
      delete fOutputList;
   fOutputList = 0;

   SafeDelete(fSelecImp);
   SafeDelete(fSelecHdr);
}

//______________________________________________________________________________
Int_t TProofQueryOnHold::PutMessage(TMessage *m, Int_t fd)
{
   // Write message to open file (descriptor fd)

   if (m && fd > 0) {
      Int_t n = -1;
      // Set the message length in first word of buffer
      m->SetLength();
      // Prepare the buffer
      char *mbuf = m->Buffer();
      Int_t mlen = m->Length();
      // Write out the length
      while ((n = write(fd, &mlen, sizeof(mlen))) < 0  && TSystem::GetErrno() == EINTR)
         TSystem::ResetErrno();
      // Check success
      if (n < 0) return -1;
      // Write out the buffer
      while ((n = write(fd, (const void *)mbuf, mlen)) < 0  && TSystem::GetErrno() == EINTR)
         TSystem::ResetErrno();
      // Check success
      if (n < 0) return -1;
      // Done
      return 0;
   }

   // Either the message or the file are invalid
   Error("PutMessage", "invalid inputs (message: %p, fd: %d)", m, fd);
   return -1;
}

//______________________________________________________________________________
TMessage *TProofQueryOnHold::GetNextMessage(Int_t fd)
{
   // Read next message from open file (descriptor fd)

   TMessage *m = 0;

   if (fd > 0) {
      // Check if there is still something to read
      off_t lnow = lseek(fd, (off_t) 0, SEEK_CUR);
      off_t ltot = lseek(fd, (off_t) 0, SEEK_END);
      if (lnow >= ltot) {
         if (gDebug > 0)
            Info("GetNextMessage", "EOF reached (%lld bytes)", Long64_t(ltot));
         return m;
      }
      // Re-position
      lseek(fd, (off_t) lnow, SEEK_SET);
      // Get the message
      Int_t  n;
      UInt_t len;
      char *buf = 0;
      // Read out the length
      while ((n = read(fd, (void *)&len, sizeof(len))) < 0 && TSystem::GetErrno() == EINTR)
         TSystem::ResetErrno();
      // Check success
      if (n < 0) return m;
      // Read out the buffer
      buf = new char[len + sizeof(UInt_t)];
      while ((n = read(fd, (void *)buf, len)) < 0 && TSystem::GetErrno() == EINTR)
         TSystem::ResetErrno();
      // Check success
      if (n < 0) {
         delete[] buf;
         return m;
      }
      // Create the output message
      m = new TMessage(buf, len+sizeof(UInt_t));
      // Done
      return m;
   }

   // The file  descriptor is invalid
   Error("GetNextMessage", "invalid input fd: %d", fd);
   return m;
}

//______________________________________________________________________________
Int_t TProofQueryOnHold::Save(const char *path)
{
   // Save info about the on-hold query into path

   if (!path || strlen(path) <= 0) {
      Info("Save", "invalid inputs");
      return -1;
   }

   // If the file existed already, warn and remove it
   if (!gSystem->AccessPathName(path))
      Warning("Save", "file '%s' already existing - removing", path);

   // Open the file in truncate mode
   FILE *f = fopen(path, "w");
   if (!f) {
      Error("Save", "cannot open file %s", path);
      return -1;
   }

   TMessage m(kPROOF_MESSAGE);

   // Create an auxilliary message with the package and path info
   m << TString("aux") << fTag << fPackages << fLibPaths << fIncPaths;
   if (PutMessage(&m, fileno(f)) != 0) {
      Error("Save", "cannot save auxilliary message");
      fclose(f);
      return -1;
   }

   Info("Save", "proc: %lld, toproc: %lld", fProcessed, fToProcess);

   // Entries
   m.Reset();
   m << TString("entries") << fProcessed << fToProcess;
   if (PutMessage(&m, fileno(f)) != 0) {
      Error("Save", "cannot save message with entries info");
      fclose(f);
      return -1;
   }

   // Selector info
   if (fSelecHdr && fSelecImp) {
      m.Reset();
      m << TString("selec") << fSelecHdr << fSelecImp;
      if (PutMessage(&m, fileno(f)) != 0) {
         Error("Save", "cannot save message with selector info");
         fclose(f);
         return -1;
      }
   }

   // Dataset
   if (fDSet) {
      m.Reset();
      m << TString("dset") << fDSet;
      if (PutMessage(&m, fileno(f)) != 0) {
         Error("Save", "cannot save message with dataset");
         fclose(f);
         return -1;
      }
   }

   // Output list
   if (fOutputList) {
      m.Reset();
      m << TString("outputlist") << fOutputList;
      if (PutMessage(&m, fileno(f)) != 0) {
         Error("Save", "cannot save message with the output list");
         fclose(f);
         return -1;
      }
   }

   // Main message
   if (PutMessage(fMessage, fileno(f)) != 0) {
      Error("Save", "cannot save main message");
      fclose(f);
      return -1;
   }

   // Close the file
   fclose(f);

   // Done
   return 0;
}
