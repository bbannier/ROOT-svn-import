// @(#)root/base:$Id$
// Author: Fons Rademakers   04/05/96

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TBuffer                                                              //
//                                                                      //
// Buffer base class used for serializing objects.                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TBuffer.h"
#include "TClass.h"
#include "TProcessID.h"
#include "TMemPool.h"

const Int_t  kExtraSpace        = 8;   // extra space at end of buffer (used for free block count)

ClassImp(TBuffer)

//______________________________________________________________________________
static inline ULong_t Void_Hash(const void *ptr)
{
   // Return hash value for this object.

   return TString::Hash(&ptr, sizeof(void*));
}


//______________________________________________________________________________
TBuffer::TBuffer(EMode mode, TMemPool *mempool /*= 0*/)
{
   // Create an I/O buffer object. Mode should be either TBuffer::kRead or
   // TBuffer::kWrite. By default the I/O buffer has a size of
   // TBuffer::kInitialSize (1024) bytes.
   // If mempool is valid then the user has passed a valid mempool and the
   // buffer can request a chunk of memory from it instead of doing the
   // allocation.

   fBufSize      = kInitialSize;
   fMode         = mode;
   fVersion      = 0;
   fParent       = 0;

   SetBit(kIsOwner);

   fMemPool = mempool;
   if(fMemPool) {
      fBuffer = (char *) fMemPool->GetMem(fBufSize+kExtraSpace);
      ResetBit(kIsOwner);
   }
   else {
      fBuffer = new char[fBufSize+kExtraSpace];
   }

   fBufCur = fBuffer;
   fBufMax = fBuffer + fBufSize;
}

//______________________________________________________________________________
TBuffer::TBuffer(EMode mode, Int_t bufsiz, TMemPool *mempool /*= 0*/)
{
   // Create an I/O buffer object. Mode should be either TBuffer::kRead or
   // TBuffer::kWrite.

   if (bufsiz < kMinimalSize) bufsiz = kMinimalSize;
   fBufSize  = bufsiz;
   fMode     = mode;
   fVersion  = 0;
   fParent   = 0;

   SetBit(kIsOwner);

   fMemPool = mempool;
   if(fMemPool) {
      fBuffer = (char *) fMemPool->GetMem(fBufSize+kExtraSpace);
      ResetBit(kIsOwner);
   }
   else {
      fBuffer = new char[fBufSize+kExtraSpace];
   }

   fBufCur = fBuffer;
   fBufMax = fBuffer + fBufSize;
}

//______________________________________________________________________________
TBuffer::TBuffer(EMode mode, Int_t bufsiz, void *buf, Bool_t adopt)
{
   // Create an I/O buffer object. Mode should be either TBuffer::kRead or
   // TBuffer::kWrite. By default the I/O buffer has a size of
   // TBuffer::kInitialSize (1024) bytes. An external buffer can be passed
   // to TBuffer via the buf argument. By default this buffer will be adopted
   // unless adopt is false.

   if (!buf && bufsiz < kMinimalSize) bufsiz = kMinimalSize;
   fBufSize  = bufsiz;
   fMode     = mode;
   fVersion  = 0;
   fParent   = 0;

   SetBit(kIsOwner);

   if (buf) {
      fBuffer = (char *)buf;
      if (!adopt) ResetBit(kIsOwner);
   } else
      fBuffer = new char[fBufSize+kExtraSpace];
   fBufCur = fBuffer;
   fBufMax = fBuffer + fBufSize;

   fMemPool = 0; // There is no need to set up the pool here
}

//______________________________________________________________________________
TBuffer::~TBuffer()
{
   // Delete an I/O buffer object.

   if (TestBit(kIsOwner)) {
      //printf("Deleting fBuffer=%lx\n", fBuffer);
      delete [] fBuffer;
   }
   fBuffer = 0;
   fParent = 0;
}

//______________________________________________________________________________
void TBuffer::SetBuffer(void *buf, UInt_t newsiz, Bool_t adopt)
{
   // Sets a new buffer in an existing TBuffer object. If newsiz=0 then the
   // new buffer is expected to have the same size as the previous buffer.
   // The current buffer position is reset to the start of the buffer.
   // If the TBuffer owned the previous buffer, it will be deleted prior
   // to accepting the new buffer. By default the new buffer will be
   // adopted unless adopt is false.

   if (fBuffer && TestBit(kIsOwner))
      delete [] fBuffer;

   if (adopt)
      SetBit(kIsOwner);
   else
      ResetBit(kIsOwner);

   fBuffer = (char *)buf;
   fBufCur = fBuffer;
   if (newsiz > 0) fBufSize = newsiz;
   fBufMax = fBuffer + fBufSize;
}

//______________________________________________________________________________
void TBuffer::Expand(Int_t newsize)
{
   // Expand the I/O buffer to newsize bytes.

   Int_t l  = Length();

   if(fMemPool) {
      if (fBuffer == 0) {
         fBuffer = (char *) fMemPool->GetMem(newsize+kExtraSpace);
      }
      else if(newsize == fBufSize) {
         // Do nothing... leave it here to follow ReAllocChar impl.
      }
      else{
         // Normal case newsize>fBufSize
         char* vp = (char *) fMemPool->GetMem(newsize+kExtraSpace);
         if(newsize>fBufSize) {
            memcpy(vp, fBuffer, fBufSize+kExtraSpace);
            memset((char*) vp+fBufSize+kExtraSpace, 0, newsize - fBufSize);
         }
         else {
            memcpy(vp, fBuffer, newsize+kExtraSpace);
         }
         fBuffer = vp;
      }
   }
   else {
      fBuffer  = TStorage::ReAllocChar(fBuffer, newsize+kExtraSpace,
                                    fBufSize+kExtraSpace);
   }

   fBufSize = newsize;
   fBufCur  = fBuffer + l;
   fBufMax  = fBuffer + fBufSize;
}

//______________________________________________________________________________
TObject *TBuffer::GetParent() const
{
   // Return pointer to parent of this buffer.

   return fParent;
}

//______________________________________________________________________________
void TBuffer::SetParent(TObject *parent)
{
   // Set parent owning this buffer.

   fParent = parent;
}

//______________________________________________________________________________
void TBuffer::SetReadMode()
{
   // Set buffer in read mode.

   fMode = kRead;
}

//______________________________________________________________________________
void TBuffer::SetWriteMode()
{
   // Set buffer in write mode.

   fMode = kWrite;
}

//______________________________________________________________________________
TClass *TBuffer::GetClass(const type_info &typeinfo)
{
   // Forward to TROOT::GetClass().

   return TClass::GetClass(typeinfo);
}

//______________________________________________________________________________
TClass *TBuffer::GetClass(const char *className)
{
   // Forward to TROOT::GetClass().

   return TClass::GetClass(className);
}

//______________________________________________________________________________
TProcessID *TBuffer::ReadProcessID(UShort_t pidf)
{
   // Return the current PRocessID.

   if (!pidf) return TProcessID::GetPID(); //may happen when cloning an object
   return 0;
}

//______________________________________________________________________________
UShort_t TBuffer::WriteProcessID(TProcessID *)
{
   // Always return 0 (current processID).

   return 0;
}
