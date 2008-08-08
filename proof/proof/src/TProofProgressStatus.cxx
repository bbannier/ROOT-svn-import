// Author: Jan Iwaszkiewicz    08/08/08

/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TProofProgressStatus.h"
#include "TObject.h"

//______________________________________________________________________________
//

ClassImp(TProofProgressStatus)

//______________________________________________________________________________
TProofProgressStatus::TProofProgressStatus(Long64_t entries,
                                           Long64_t bytesRead,
                                           Double_t procTime,
                                           Double_t cpuTime): TObject()
{
   fEntries = entries;
   fBytesRead = bytesRead;
   fProcTime = procTime;
   fCPUTime = cpuTime;
}

//______________________________________________________________________________
TProofProgressStatus& TProofProgressStatus::operator+=(TProofProgressStatus *st)
{
   fEntries += st->GetEntries();
   fBytesRead += st->GetBytesRead();
   fProcTime += st->GetProcTime();
   fCPUTime += st->GetCPUTime();
   return *this;
}

//______________________________________________________________________________
TProofProgressStatus& TProofProgressStatus::operator-=(TProofProgressStatus *st)
{
   fEntries -= st->GetEntries();
   fBytesRead -= st->GetBytesRead();
   fProcTime -= st->GetProcTime();
   fCPUTime -= st->GetCPUTime();
   return *this;
}
