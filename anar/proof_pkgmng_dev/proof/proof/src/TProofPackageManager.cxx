// @(#)root/proof:$Id$
// Author: Anar Manafov 16/09/2008

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofPackageManager                                                 //
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

// STD
#include <algorithm>
#include <string>
// ROOT
#include "TSlave.h"
#include "TProofPackageManager.h"

using namespace std;

//______________________________________________________________________________
template <class T>
void TProofPackageHelper<T>::BuildSlavesList(TList *uniqueSlaves)
{
   // The BuildSlavesList function collects a list of unique
   // slaves with the same architecture as the master.

   // Architecture of the Master
   GetMasterArchitecture(&fMasterArch);

   // Clean all internal lists
   fSlvArcMstr.Clear();
   fSlvArcUnq.Clear();

   // We loop over all selected nodes and check architecture.
   // At the end the fSlvArcMstr must contain a list of unique
   // slaves with the same architecture as the master.
   TIter iter(uniqueSlaves);
   for_each(iter, TIter::End(),
            std::bind1st(std::mem_fun(&TProofPackageHelper::AddSlave), this));
}

//______________________________________________________________________________
template <class T>
bool TProofPackageHelper<T>::NeedToUploadPkg(TSlave *slave) const
{
   // This function returns true if the package should be uploaded to the given slave.
   // It returns false in case when slave belongs to the list of slaves of the same architecture
   // as on master.
   return (fSlvArcUnq.FindObject(slave) != NULL);
}

//______________________________________________________________________________
template <class T>
void TProofPackageHelper<T>::BroadcastUnqArcSlv(const TMessage &msg) const
{
   // This method sends a broadcast message to all unique slaves,
   // which have a different architecture in compare to a master

   T *pThis = reinterpret_cast<T*>(this);
   if (!pThis)
      return;

   pThis->Broadcast(msg, &fSlvArcUnq);
}

//______________________________________________________________________________
template <class T>
bool TProofPackageHelper<T>::PrepareAndSendPkg() const
{
   // The PrepareAndSendPkg function tars and sends a pre-builded package from
   // a master to the slaves, which have the same architecture as on master.
   // returns: true upon success and false if failed
   return true;
}

//______________________________________________________________________________
template <class T>
void TProofPackageHelper<T>::AddSlave(TSlave *slave)
{
   // The AddSlave function collects internal lists of slaves

   if (!slave)
      return;

   if (slave->GetArchCompiler() == fMasterArch)
      fSlvArcMstr.Add(slave);
   else
      fSlvArcUnq.Add(slave);
}

//______________________________________________________________________________
template <class T>
void TProofPackageHelper<T>::GetMasterArchitecture(std::string *retVal)
{
   // a helper function, generates a build architecture string
   // example: linux-gcc412

   if (!retVal)
      return;

   std::ostringstream ss;
   ss << gSystem->GetBuildArch() << "-" << gSystem->GetBuildCompilerVersion();

   *retVal = ss.str();
}
