// @(#)root/proof:$Id: TProofPackageManager.cxx 22828 2008-03-25 12:57:02Z anar $
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

/*template<class _T>
class CIterFrwdWrapper
{
   typedef _T* value_type;
   typedef _T& value_ref;

public:
   CIterFrwdWrapper():
         fNext((TList*)NULL),
         fCurrent(NULL) {
   }
   CIterFrwdWrapper(TList *_list):
         fNext(_list),
         fCurrent(NULL) {
   }
   CIterFrwdWrapper &operator ++() {
      fCurrent = reinterpret_cast<_T*>(fNext());
      return *this;
   }
   value_type operator*() const {
      return fCurrent;
   }
   bool operator !=(const CIterFrwdWrapper &_iter) {
      return (fCurrent != *_iter);
   }
private:
   TIter fNext;
   value_type fCurrent;
};
*/
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
   TIter iter_end( (TIterator*)NULL);
   for_each(iter, iter_end, STestFunctor());
   /* TIter next(_UniqueSlaves);
    TSlave *sl(dynamic_cast<TSlave*>(next()));
    while (sl) {
       if (!sl->IsValid())
          continue;

       sl = dynamic_cast<TSlave*>(next());
    }*/
}
