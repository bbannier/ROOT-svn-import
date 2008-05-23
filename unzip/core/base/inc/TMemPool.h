// @(#)root/base:$Id: TMemPool.h 23013 2008-04-07 12:50:48Z lfranco $
// Author: Rene Brun   02/09/2000

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
 
#ifndef ROOT_TMemPool
#define ROOT_TMemPool

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TMemPool                                                             //
//                                                                      //
//                 //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


class TMemPool {

private:
   class TMemBlock {
   private:
      char*      fData;  // Pointer to the requested buffer
      Long_t     fSize;  // Total size of the buffer
      Long_t     fUsed;  // Used size of the buffer
      TMemBlock* fNext;  // We will have a linked list of blocks
   
   public:
      TMemBlock();
      TMemBlock(Long_t size);
      virtual ~TMemBlock();

      Long_t AddUsed(Long_t used){ fUsed+=used; return fUsed;}
      
      char*      GetData(){ return fData; }
      Long_t     GetFree(){ return fSize-fUsed; }
      char*      GetPtr(){  return fData+fUsed; }
      TMemBlock* GetNext(){ return fNext; }
      Long_t     GetSize(){ return fSize; }
      Long_t     GetUsed(){ return fUsed; }

      void SetNext(TMemBlock* next){ fNext=next; }
      void SetUsed(Long_t used){ fUsed=used; }
   };
   
   TMemBlock* fFirst;   // Pointer to the first TMemBlock (it's a linked list)
   TMemBlock* fLast;    // Pointer to the last TMemBlock (it's a linked list)
   TMemBlock* fCurrent; // Pointer to the current TMemBlock (it's a linked list)
   Long_t     fSize;    // Total size in the pool (all the blocks)
   Long_t     fUsed;    // Used size in the pool (all the blocks)
   
   
   TMemPool(const TMemPool &pool);  //pools cannot be copied
   void operator=(const TMemPool &);

public:

   TMemPool();
   TMemPool(Long_t size);
   virtual ~TMemPool();
   
   void Delete();
   char *GetMem(Long_t size);
   void Print();
   void Reset();
   
   ClassDef(TMemPool,0)
};

#endif

