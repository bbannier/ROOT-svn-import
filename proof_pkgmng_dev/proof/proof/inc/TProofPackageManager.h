// @(#)root/proof:$Id$
// Author: Anar Manafov 16/09/2008

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
#ifndef ROOT_TProofPackageManager
#define ROOT_TProofPackageManager

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofPackageManager                                                 //
//                                                                      //
//   A strategy:                                                        //
//                                                                      //
//   On Master:                                                         //
//          - check architecture of unique workers                      //
//          - upload packages to workers of different architecture      //
//              (different in compare to master)                        //
//          - compile packages on workers of different architecture     //
//          - compile package on master                                 //
//          - tar package on master and push (SendFile) to unique       //
//            workers of the same architecture as the master            //
//  On Workers (of the same architecture):                              //
//          - untar compiled package                                    //
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

// STD
#include <sstream>
//ROOT
#include "TSystem.h"
#include "TList.h"
#include "TMessage.h"

class TSlave;

template<class T>
class TProofPackageHelper
{
protected:
   void BuildSlavesList(TList *uniqueSlaves);
   bool NeedToUploadPkg(TSlave *slave) const;
   void BroadcastUnqArcSlv(const TMessage &msg) const;
   bool PrepareAndSendPkg() const;

private:
   void AddSlave(TSlave *slave);
   //TODO: maybe we want to exclude this function from the class
   //      and make it a free inline function
   void GetMasterArchitecture(std::string *retVal);

private:
   TList fSlvArcMstr; // a list of Slaves with the same architecture as on Master
   TList fSlvArcUnq;  // a list of Slaves with the unique architecture
   std::string fMasterArch; // a build architecture string of the Master
};

#endif
