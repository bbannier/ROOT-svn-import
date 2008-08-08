// Author: Jan Iwaszkiewicz    08/08/08

/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofProgressStatus
#define ROOT_TProofProgressStatus

#ifndef ROOT_TObject
#include "TObject.h"
#endif

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofProgressStatus                                                 //
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

class TProofProgressStatus : public TObject {
private:
   Long64_t fEntries;
   Long64_t fBytesRead;
   Double_t fProcTime; // the wallclock time
   Double_t fCPUTime;
public:
   TProofProgressStatus(Long64_t fEntries = 0, Long64_t fBytesRead = 0,
                        Double_t fProcTime = 0, Double_t fCPUTime = 0);
   void     Reset() { fEntries = 0, fBytesRead = 0,
   	      fProcTime = 0, fCPUTime = 0; }
   Long64_t GetEntries() { return fEntries; }
   Long64_t GetBytesRead() { return fBytesRead; }
   Double_t GetProcTime() { return fProcTime; }
   Double_t GetCPUTime() { return fCPUTime; }
   void     SetEntries(Long64_t entries) { fEntries = entries; }
   void     IncEntries(Long64_t entries = 1) { fEntries += entries; }
   void     SetBytesRead(Long64_t bytesRead) { fBytesRead = bytesRead; }
   void     SetProcTime(Double_t procTime) { fProcTime = procTime; }
   void     SetCPUTime(Double_t procTime) { fCPUTime = procTime; }
   TProofProgressStatus &operator+=(TProofProgressStatus *st);
   TProofProgressStatus &operator-=(TProofProgressStatus *st);
   ClassDef(TProofProgressStatus,1) // Proof progress status class
};

#endif
