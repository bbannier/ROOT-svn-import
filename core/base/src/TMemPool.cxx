// @(#)root/base:$Id: TMemPool.cxx 23013 2008-04-07 12:50:48Z lfranco $
// Author: Leandro Franco  23/05/2008

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
//  Create a simple memory pool to request space for temporary buffers  //
//  the idea is to avoid as many dew/delete as possible and the         //
//  principal targets are the buffers needed to process data when       //
//  reading a tree (entry by entry)                                     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TMemPool.h"
#include "TROOT.h"
#include <stdlib.h> // malloc

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  TMemPool::TMemBlock is the basic block of our memory pool, it's jsut      //
//  a node in a linked list where in addition to the data pointer and the     //
//  nwxt link, we have some variable to keep track of the used memory         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
Int_t TMemPool::TMemBlock::fgBlockSize = 4096;

//______________________________________________________________________________
TMemPool::TMemBlock::TMemBlock()
{
   fData=0;
   fSize=0;
   fUsed=0;
   fNext=0;
}

//______________________________________________________________________________
TMemPool::TMemBlock::TMemBlock(Long_t size)
{
   Int_t nblocks = ((Int_t)(size/fgBlockSize)) + ((size%fgBlockSize)?1:0);

   size = nblocks*fgBlockSize;
   
   fData=(char*)malloc(size);
   fUsed=0;
   fNext=0;
   fSize=size;
   fFree=fSize;
}

//______________________________________________________________________________
TMemPool::TMemBlock::~TMemBlock()
{ 
   fFree=0;
   fSize=0;
   fUsed=0;
   free(fData);
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  TMemPool is the actual memory pool that is a container for the linked     //
//  list of TMemBlocks (with pointers to the first, last and current blocks)  //
//  plus some additional info about the used memory                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
ClassImp(TMemPool);

//______________________________________________________________________________
TMemPool::TMemPool()
{
   // Create the memory pool, this will set all the variables to 0 so be careful
   if (gDebug == -1) 
      Printf("TMemPool -- New mem pool (everything is zero) p:%p", this);

   fFirst   = 0;
   fLast    = 0;
   fCurrent = 0;
   
   fSize = 0;
   fUsed = 0;
}

//______________________________________________________________________________
TMemPool::TMemPool(Long_t size)
{
   // Create the memory pool with a first block of size "size"
   if (gDebug == -1)
      Printf("TMemPool New mem pool size: %d p:%p", size, this);

   fFirst   = new TMemBlock(size);
   fLast    = fFirst;
   fCurrent = fFirst;
   
   fSize = size;
   fUsed = 0;
}

//______________________________________________________________________________
TMemPool::~TMemPool()
{
   // Delete our memory pool 
   Delete();
}

//______________________________________________________________________________
void TMemPool::Delete()
{
   // Delete the list of blocks reserved for this pool (each one of them)
   // and change the information about the memory accordingly
   if (gDebug == -1)
      Printf("~TMemPool -- Deleting mem pool p:%p", this);
   
   TMemBlock* btmp;  // rem the next pointer
   for (TMemBlock* block=fFirst; block!=0; block=btmp) {
      btmp = block->GetNext();  // since we delete it
      
      fSize -= block->GetSize();
      fUsed -= block->GetUsed();
      delete block;
   }
   fFirst   = 0;
   fLast    = 0;
   fCurrent = 0;
}

//______________________________________________________________________________
char* TMemPool::GetMem(Long_t size)
{
   // Do what you are suppose to do... return a valid pointer to a buffer of
   // size "size". For the moment, we are using a next-fit strategy to find
   // to block from which we will take the memory. This is probably the
   // simplest (and fastest) algorithm if we don't have to garbage collect the
   // memory.

   char *buff = 0;
   for (TMemBlock* block=fCurrent; block!=0; block=block->GetNext()) {
      // Do we have enough space in this buffer?
      if( block->GetFree()>=size) {
         buff = block->GetPtr();
         block->AddUsed(size);
         fUsed += size;
	 if(fCurrent!=block) fCurrent = block;
         if (gDebug == -1)
            Printf("GetMem -- Returning a Block:%d p:%p size:%d used:%d", size, block, block->GetSize(), block->GetUsed() );
         return buff;
      }
   }
   
   // Maybe we don't have enough memory in our pool...
   // create a new memory block
   if(!buff) {
      if(!fFirst && !fLast) {
         fFirst = new TMemBlock(size);
         fLast = fCurrent = fFirst;
      }
      else {
         fLast->SetNext(new TMemBlock(fLast->GetSize()+size));
         fLast = fLast->GetNext();
         fCurrent = fLast;
      }
      fSize += fLast->GetSize();

      if (gDebug == -1)
         Printf("GetMem -- Allocating a new Block fLast->fSize:%d size:%d", fLast->GetSize(), size );
      if( fCurrent->GetFree()>=size) {
         buff = fCurrent->GetPtr();
         fCurrent->AddUsed(size);
         fUsed += size;
         Print();
         return buff;
      }
   }
   return 0;
}

//______________________________________________________________________________
void TMemPool::Print()
{
   // Print the Info about the mem pool...
   if (gDebug == -1)
      Printf("Print -- Total p:%p fSize:%d fUsed:%d", this, fSize, fUsed);

   Int_t n=0;
   for (TMemBlock* block=fFirst; block!=0; block=block->GetNext(), n++) {
      if (gDebug == -1)
         Printf("Print -- p:%p Block n.:%d block->fSize:%d block->fUsed:%d", this, n, block->GetSize(), block->GetUsed() );
   }   
}

//______________________________________________________________________________
void TMemPool::Reset()
{
   // Clear up this memory pool... don't delete anything, just put the
   // pointers to zero
   if(fUsed==0)
      return;
   
   if (gDebug == -1)
      Printf("Reset -- Cleaning up all the blocks p:%p", this);
   
   for (TMemBlock* block=fFirst; block!=0; block=block->GetNext()) {
      block->SetUsed(0);
   }
   fUsed = 0;
}

