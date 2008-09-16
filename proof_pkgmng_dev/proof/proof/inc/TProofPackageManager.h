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
//                                                                      //
//////////////////////////////////////////////////////////////////////////

// STD
#include <set>
#include <sstream>
//ROOT
#include "TSystem.h"

class TSlave;
class TList;

class TProofPackageManager
{
   typedef std::set<const TSlave *> SlavesList_t;

public:
   void BuildSlavesList(TList *_UniqueSlaves);

private:
   SlavesList_t fSlvArcMstr; // a list of Slaves with the same architecture as on Master
   SlavesList_t fSlvArcUnq;  // a list of Slaves with the unique architecture
};

// a helper function, generates a build architecture string
// example: linux-gcc412
inline void get_architecture(std::string *_RetVal)
{
   if (!_RetVal)
      return;

   std::ostringstream ss;
   ss << gSystem->GetBuildArch() << "-" << gSystem->GetBuildCompilerVersion();

   *_RetVal = ss.str();
}

#endif
