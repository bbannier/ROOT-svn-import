// @(#)root/tree:$Id: TTreeCache.h 23014 2008-04-07 13:25:32Z lfranco $
// Author: Rene Brun   04/06/2006

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TTreeCacheUnzip
#define ROOT_TTreeCacheUnzip


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TTreeCacheUnzip                                                      //
//                                                                      //
// Specialization of TTreeCache for parallel Unzipping                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TTreeCache
#include "TTreeCache.h"
#endif

class TTree;
class TBranch;
class TThread;
class TCondition;
class TSortedList;
class TBasket;
class TMutex;
class TSemaphore;

class TTreeCacheUnzip : public TTreeCache {

protected:
   // TUnzipBufferInfo is a simple container to put the Info of each element in the
   // unzipping buffer.. it has nothing to do with TBuffer so I'm afraid the
   // name can be a bit misleading.
   class TUnzipBufferInfo : public TObject {
   private:
      Int_t      fNum;       // This is like the id of the element
      Bool_t     fRead;      // has it been already read? (to know if we can delete it)
      Long64_t   fPos;       // the offset of the buffer in the file
      Int_t      fLen;       // the len of the buffer in the file
      TBasket    *fBasket;   // the ref to the basket that is related to this buffer element

   public:
      TUnzipBufferInfo(Int_t i = 0) : fNum(i), fRead(kFALSE), fPos(0), fLen(0), fBasket(0) { }
      ~TUnzipBufferInfo() {}
      void     SetNum(Int_t i) { fNum = i; }
      Int_t    GetNum() { return fNum; }

      void     SetRead(Bool_t r) { fRead = r; }
      Bool_t   GetRead() { return fRead; }

      void     SetPos(Long64_t p) { fPos = p; }
      Long64_t GetPos() { return fPos; }

      void     SetLen(Int_t l) { fLen = l; }
      Int_t    GetLen() { return fLen; }

      void     SetBasket(TBasket *b) { fBasket = b; }
      TBasket  *GetBasket() { return fBasket; }

      void     Print(Option_t *) const { Info("TUnzipBufferInfo","fNum = %d, fPos:%lld, fLen:%d, fRead?:%d, fBasket:%p", fNum, fPos, fLen, fRead, fBasket); }
      ULong_t  Hash() const { return fNum; }
      Bool_t   IsEqual(const TObject *obj) const { return fNum == ((TUnzipBufferInfo*)obj)->GetNum(); }
      Bool_t   IsSortable() const { return kTRUE; }
      Int_t    Compare(const TObject *obj) const { if (fNum > ((TUnzipBufferInfo*)obj)->GetNum())
                                                      return 1;
                                                   else if (fNum < ((TUnzipBufferInfo*)obj)->GetNum())
                                                      return -1;
                                                   else
                                                      return 0; 
                                                 }
   };
 
   TMutex         *fMutexCache;

   // Members for paral. managing
   TThread    *fUnzipThread;
   Bool_t      fActiveThread;
   TCondition *fUnzipCondition; 
   //TSemaphore *fUnzipCondition; 
   Bool_t      fNewTransfer;    // Used to indicate the second thread taht a new transfer is in progress
   Bool_t      fParallel;       // Indicate if we want to activate the parallelism (for this instance)

   TMutex      *fMutexUnzipBuffer; // Dont access the same method of the same object at the same time
   TMutex      *fMutexBuffer;      // Mutex to protect the unzipping buffer 'fUnzipBuffer'
   TMutex      *fMutexList;        // Mutex to protect the list of inflated buffer
   TSortedList *fUnzipList;         // list of aleady inflated buffers
   TCondition  *fBufferCond;

   Int_t      fTmpBufferSz;    //!  Size for the fTmpBuffer (default is 10KB... used to unzip a buffer)
   char       *fTmpBuffer;     //! [fTmpBufferSz] buffer of contiguous unzipped blocks
   
   static TString   fgParallel;       // Indicate if we want to activate the parallelism

   // Members to keep track of the unzipping buffer
   Long64_t   fPosRead;
   Long64_t   fPosWrite;

   // Unzipping related member
   Bool_t     fUnzipped;           //! true if cache has been unzipped
   Int_t      *fUnzipLen;          //! [fNseek] Length of buffers to be unzipped
   Int_t      *fUnzipPos;          //! [fNseek] Position of sorted blocks in fUnzipBuffer
   Int_t      fNseekMax;           //!  fNseek can change so we need to know its max size
   Long64_t   fUnzipBufferSize;    //!  Size for the fUnzipBuffer (default is 2*fBufferSize)
   char       *fUnzipBuffer;       //! [fTotBytes] buffer of contiguous unzipped blocks
   Bool_t     fSkipZip;            //  say if we should skip the uncompression of all buffers
   Int_t      fLastPos;            //!  offset of the last buffer that was unzipped.
   static Double_t fgRelBuffSize;  // This is the percentage of the TTreeCacheUnzip that will be used
 
   // Members use to keep statistics
   Int_t      fNUnzip;           //! number of blocks that were unzipped
   Int_t      fNFound;           //! number of blocks that were found in the cache
   Int_t      fNMissed;          //! number of blocks that were not found in the cache and were unzipped

   Int_t      fUnzipStart;
   Int_t      fUnzipEnd;
   Int_t      fUnzipNext;

private:
   TTreeCacheUnzip(const TTreeCacheUnzip &);            //this class cannot be copied
   TTreeCacheUnzip& operator=(const TTreeCacheUnzip &);

   // Private methods
   void  Init();
   Int_t StartThreadUnzip();
   Int_t StopThreadUnzip();

public:
   TTreeCacheUnzip();
   TTreeCacheUnzip(TTree *tree, Int_t buffersize=0);
   virtual ~TTreeCacheUnzip();
   virtual void        AddBranch(TBranch *b, Bool_t subbranches = kFALSE);
   virtual void        AddBranch(const char *branch, Bool_t subbranches = kFALSE);
   Bool_t              FillBuffer();
   void                SetEntryRange(Long64_t emin,   Long64_t emax);
   virtual void        StopLearningPhase();
   void                UpdateBranches(TTree *tree, Bool_t owner = kFALSE);

   // Methods related to the thread
   static Option_t *GetParallelUnzip();   
   static Bool_t    IsParallelUnzip();
   Bool_t           IsActiveThread();
   Bool_t           IsQueueEmpty();
   Int_t            ProcessQueue();   
   void             SendSignal();
   static Int_t     SetParallelUnzip(Option_t* option = "");
   void             WaitForSignal();

   // Methods to get stats
   Int_t  GetNUnzip() { return fNUnzip; }
   Int_t  GetNFound() { return fNFound; }
   Int_t  GetNMissed(){ return fNMissed; }

   // Unzipping related methods
   Int_t          GetRecordHeader(char *buf, Int_t maxbytes, Int_t &nbytes, Int_t &objlen, Int_t &keylen);
   virtual Bool_t GetSkipZip() { return fSkipZip; }
   virtual void   ResetCache();
   Int_t          GetUnzipBuffer(char **buf, Long64_t pos, Int_t len, Bool_t *free, TBasket *basRef);
   void           SetBufferRead(Long64_t pos, Int_t len, TBasket *basket);
   void           SetUnzipBufferSize(Long64_t bufferSize);
   virtual void   SetSkipZip(Bool_t skip = kTRUE) { fSkipZip = skip; }
   Int_t          UnzipBuffer(char **dest, char *src);
   Int_t          UnzipCache();
   
   // static members
   static void* UnzipLoop(void *arg);
   ClassDef(TTreeCacheUnzip,0)  //Specialization of TTreeCache for parallel unzipping
};

#endif
