// @(#)root/proof:$Id$
// Author: Anar Manafov 28/03/2008

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

#include "TProofPackageManager.h"

using namespace std;

struct STestFunctor {
   bool operator()(TObject *_obj) {
	// TSlave * slave( dynamic_cast<TSlave*>(_obj));
      return true;
   }
};
//______________________________________________________________________________
void TProofPackageManager::BuildSlavesList(TList *_UniqueSlaves)
{
   // architectur of the Master
   string master_arch;
   get_architecture(&master_arch);

   // loop over all selected nodes and check architecture
   TIter iter(_UniqueSlaves);   
   for_each(iter, TIter::End(), STestFunctor());   
}
