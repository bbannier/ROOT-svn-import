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
struct SFindSameArch: public binary_function<TObject*, const string&, bool> {
   bool operator()(TObject *_obj, const string &_master_arch) const {
      TSlave *slave(dynamic_cast<TSlave*>(_obj));
      return(slave->GetArchCompiler() == _master_arch);
   }
};

//______________________________________________________________________________
struct SGetSlave {
   TSlave* operator()(TObject *_obj) const {
      return(dynamic_cast<TSlave*>(_obj));
   }
};

//______________________________________________________________________________
void TProofPackageManager::BuildSlavesList(TList *_UniqueSlaves)
{
   // The BuildSlavesList function collects a list of unique
   // slaves with the same architecture as the master.

   // Architecture of the Master
   string master_arch;
   get_architecture(&master_arch);

   // We loop over all selected nodes and check architecture.
   // At the end the fSlvArcMstr must contain a list of unique
   // slaves with the same architecture as the master.
   typedef TIterCategory<TList> iterator_t;
   iterator_t iter(_UniqueSlaves);
   transform(find_if(iter, iterator_t::End(), bind2nd(SFindSameArch(), master_arch)), iterator_t::End(),
             inserter(fSlvArcMstr, fSlvArcMstr.begin()),
             SGetSlave());
}
