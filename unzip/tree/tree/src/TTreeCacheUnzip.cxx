// @(#)root/tree:$Id: TTreeCache.cxx 23030 2008-04-07 17:25:47Z lfranco $
// Author: Leandro Franco   10/04/2008

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
// Parallel Unzipping                                                   //
//                                                                      //
// TTreeCache has been modified to create an additional thread to unzip //
// the buffers that are sitting on the cache, in that way we could      //
// improve the overall ROOT performance by executing two tasks in       //
// parallel.                                                            //
// The order is important and for the moment we will do it following    //
// the sorting algorithm in TTreeCache since they are usually read by   //
// entry.                                                               //
// By default the unzipping cache will need only 10% of the buffer for  //
// TTreCache so be careful with the memory (in a normal case you need   //
// 10MB for TTreeCache and 1MB for the unizp. buffer), to change it use //
// TTreeCache::SetUnzipBufferSize(Long64_t bufferSize)                  //
// where bufferSize must be passed in bytes.                            //
//                                                                      //
// This is a implemented in a similar way to a consumer-producer model  //
// where the the producer will be TTreCache by transfering the data     //
// and the consumer will be additional Thread trying to unzip it.       //
//////////////////////////////////////////////////////////////////////////

#include "TTreeCacheUnzip.h"
#include "TChain.h"
#include "TList.h"
#include "TBranch.h"
#include "TEventList.h"
#include "TObjString.h"
#include "TRegexp.h"
#include "TLeaf.h"
#include "TFriendElement.h"
#include "TVirtualMutex.h"
#include "TThread.h"
#include "TCondition.h"
#include "TSortedList.h"
#include "TBasket.h"
#include "TFile.h"
#include "TMath.h"
#include "Bytes.h"
#include "TEnv.h"

extern "C" void R__unzip(Int_t *nin, UChar_t *bufin, Int_t *lout, char *bufout, Int_t *nout);

TString  TTreeCacheUnzip::fgParallel = "e";

// Unzip cache is 10% of TTreeCache.
// if by default fBufferSize = 10MB
// then 0.1=1MB 0.01=100KB 0.001=10KB 0.0001=1KB
Double_t TTreeCacheUnzip::fgRelBuffSize = 0.1;

ClassImp(TTreeCacheUnzip)

//______________________________________________________________________________
TTreeCacheUnzip::TTreeCacheUnzip() : TTreeCache(),
   fUnzipThread(0),
   fActiveThread(kFALSE),
   fNewTransfer(kFALSE),
   fTmpBufferSz(0),
   fTmpBuffer(0),
   fPosRead(0),
   fPosWrite(0),
   fUnzipped(kFALSE),
   fUnzipLen(0),
   fUnzipPos(0),
   fNseekMax(0),
   fUnzipBufferSize(0),
   fUnzipBuffer(0),
   fSkipZip(0),
   fLastPos(0),
   fNUnzip(0),
   fNFound(0),
   fNMissed(0)
{
   // Default Constructor.

   Init();
}

//______________________________________________________________________________
TTreeCacheUnzip::TTreeCacheUnzip(TTree *tree, Int_t buffersize) : TTreeCache(tree,buffersize),
   fUnzipThread(0),
   fActiveThread(kFALSE),
   fNewTransfer(kFALSE),
   fTmpBufferSz(10000),
   fTmpBuffer(new char[fTmpBufferSz]),
   fPosRead(0),
   fPosWrite(0),
   fUnzipped(kFALSE),
   fUnzipLen(0),
   fUnzipPos(0),
   fNseekMax(0),
   fUnzipBufferSize(0),
   fUnzipBuffer(0),
   fSkipZip(0),
   fLastPos(0),
   fNUnzip(0),
   fNFound(0),
   fNMissed(0)
{
   // Constructor.

   Init();
}

//______________________________________________________________________________
void TTreeCacheUnzip::Init()
{
   // Initialization procedure common to all the constructors

   fMutexCache       = new TMutex(kTRUE);
   fMutexUnzipBuffer = new TMutex();
   fMutexBuffer      = new TMutex();
   fMutexList        = new TMutex();
   fUnzipCondition   = new TCondition();
   fBufferCond       = new TCondition();

   fUnzipList = new TSortedList();

   if (fgParallel.Contains("d")) {
      fParallel = kFALSE;
   }
   else if(fgParallel.Contains("e") || fgParallel.IsNull() || fgParallel.Contains("f")){
      SysInfo_t info;
      gSystem->GetSysInfo(&info);
      Int_t ncpus = info.fCpus;

      if(ncpus > 1 || fgParallel.Contains("f")) {
         if(gDebug > 0)
            Info("TTreeCacheUnzip", "Enabling Parallel Unzipping, number of cpus:%d", ncpus);

         fParallel = kTRUE;
         StartThreadUnzip();
      }
      else {
         fParallel = kFALSE;
      }
   }
   else {
      Warning("TTreeCacheUnzip", "Parallel Option unknown");
   }
}

//______________________________________________________________________________
TTreeCacheUnzip::~TTreeCacheUnzip()
{
   // destructor. (in general called by the TFile destructor
   // destructor. (in general called by the TFile destructor)

   ResetCache();
   fBufferCond->Signal();

   if (IsActiveThread())
      StopThreadUnzip();

   delete fUnzipCondition;
   delete [] fUnzipBuffer;
   delete [] fTmpBuffer;
   delete [] fUnzipLen;
   delete [] fUnzipPos;

   delete fMutexCache;
   delete fMutexUnzipBuffer;
   delete fMutexBuffer;
   delete fMutexList;
   delete fBufferCond;
}

//_____________________________________________________________________________
void TTreeCacheUnzip::AddBranch(TBranch *b, Bool_t subbranches /*= kFALSE*/)
{
   //add a branch to the list of branches to be stored in the cache
   //this function is called by TBranch::GetBasket
   R__LOCKGUARD(fMutexCache);

   TTreeCache::AddBranch(b, subbranches);
}

//_____________________________________________________________________________
void TTreeCacheUnzip::AddBranch(const char *branch, Bool_t subbranches /*= kFALSE*/)
{
   //add a branch to the list of branches to be stored in the cache
   //this function is called by TBranch::GetBasket
   R__LOCKGUARD(fMutexCache);

   TTreeCache::AddBranch(branch, subbranches);
}

//_____________________________________________________________________________
Bool_t TTreeCacheUnzip::FillBuffer()
{
   // Fill the cache buffer with the branches in the cache.
   R__LOCKGUARD(fMutexCache);

   //return TTreeCache::FillBuffer();

   if (fNbranches <= 0) return kFALSE;
   TTree *tree = ((TBranch*)fBranches->UncheckedAt(0))->GetTree();
   Long64_t entry = tree->GetReadEntry();

   if (!fIsManual && entry < fEntryNext) return kFALSE;

   // Triggered by the user, not the learning phase
   if (entry == -1)  entry=0;

   // Estimate number of entries that can fit in the cache compare it
   // to the original value of fBufferSize not to the real one
   if (fZipBytes==0) {
      fEntryNext = entry + tree->GetEntries();
   } else {
      fEntryNext = entry + tree->GetEntries()*fBufferSizeMin/fZipBytes;
   }
   if (fEntryMax <= 0) fEntryMax = tree->GetEntries();
   if (fEntryNext > fEntryMax) fEntryNext = fEntryMax+1;

   //check if owner has a TEventList set. If yes we optimize for this special case
   //reading only the baskets containing entries in the list
   Long64_t chainOffset = 0;
   if (fOwner->GetEventList()) {
      if (fOwner->IsA() == TChain::Class()) {
         TChain *chain = (TChain*)fOwner;
         Int_t t = chain->GetTreeNumber();
         chainOffset = chain->GetTreeOffset()[t];
      }
   }

   fMutexCache->UnLock();
   //clear cache buffer
   ResetCache();
   TFileCacheRead::Prefetch(0,0);
   fMutexCache->Lock();
   //store baskets
   Bool_t mustBreak = kFALSE;
   for (Int_t i=0;i<fNbranches;i++) {
      if (mustBreak) break;
      TBranch *b = (TBranch*)fBranches->UncheckedAt(i);
      Int_t nb = b->GetMaxBaskets();
      Int_t *lbaskets   = b->GetBasketBytes();
      Long64_t *entries = b->GetBasketEntry();
      if (!lbaskets || !entries) continue;
      //we have found the branch. We now register all its baskets
      //from the requested offset to the basket below fEntrymax
      for (Int_t j=0;j<nb;j++) {
         Long64_t pos = b->GetBasketSeek(j);
         Int_t len = lbaskets[j];
         if (pos <= 0 || len <= 0) continue;
         if (entries[j] > fEntryNext) continue;
         if (entries[j] < entry && (j<nb-1 && entries[j+1] < entry)) continue;
         if (fOwner->GetEventList()) {
            Long64_t emax = fEntryMax;
            if (j<nb-1) emax = entries[j+1]-1;
            if (!(fOwner->GetEventList())->ContainsRange(entries[j]+chainOffset,emax+chainOffset)) continue;
         }
         fNReadPref++;
         TFileCacheRead::Prefetch(pos,len);
         //we allow up to twice the default buffer size. When using eventlist in particular
         //it may happen that the evaluation of fEntryNext is bad, hence this protection
         if (fNtot > 2*fBufferSizeMin) {TFileCacheRead::Prefetch(0,0);mustBreak = kTRUE; break;}
      }
      if (gDebug > 0) printf("Entry: %lld, registering baskets branch %s, fEntryNext=%lld, fNseek=%d, fNtot=%d\n",entry,((TBranch*)fBranches->UncheckedAt(i))->GetName(),fEntryNext,fNseek,fNtot);
   }
   fIsLearning = kFALSE;
   if (mustBreak) return kFALSE;
   return kTRUE;
}

//_____________________________________________________________________________
void TTreeCacheUnzip::SetEntryRange(Long64_t emin, Long64_t emax)
{
   // Set the minimum and maximum entry number to be processed
   // this information helps to optimize the number of baskets to read
   // when prefetching the branch buffers.
   R__LOCKGUARD(fMutexCache);

   TTreeCache::SetEntryRange(emin, emax);
}

//_____________________________________________________________________________
void TTreeCacheUnzip::UpdateBranches(TTree *tree, Bool_t owner)
{
   //update pointer to current Tree and recompute pointers to the branches in the cache
   R__LOCKGUARD(fMutexCache);

   TTreeCache::UpdateBranches(tree, owner);
}

//_____________________________________________________________________________
//_____________________________________________________________________________

//_____________________________________________________________________________
Option_t *TTreeCacheUnzip::GetParallelUnzip()
{
   // Static function that returns the parallel option
   // (to indicate an additional thread)

   return fgParallel;
}

//_____________________________________________________________________________
Bool_t TTreeCacheUnzip::IsParallelUnzip()
{
   // Static function that tells wether the multithreading unzipping
   // is activated

   TString opt(fgParallel);
   opt.ToLower();

   if (opt.Contains("d"))
      return kFALSE;

   if (opt.Contains("f") || opt.Contains("e"))
      return kTRUE;

   return kFALSE;
}

//_____________________________________________________________________________
Bool_t TTreeCacheUnzip::IsActiveThread()
{
   // This indicates if the thread is active in this moment...
   // this variable is very important because if we change it from true to
   // false the thread will stop... ( see StopThreadTreeCacheUnzip() )

   return fActiveThread;
}

//_____________________________________________________________________________
Bool_t TTreeCacheUnzip::IsQueueEmpty()
{
   // It says if the queue is empty... useful to see if we have to process
   // it.
   R__LOCKGUARD(fMutexCache);

   if ( fIsLearning )
      return kTRUE;

   return kFALSE;
}

//_____________________________________________________________________________
Int_t TTreeCacheUnzip::ProcessQueue()
{
   // This will traverse the queue and read all the buffers to put them
   // in the cache... after reading each buffer it will send a signal and
   // if someone is waiting for it, it will wake up and will check the queue
   // again to see if the buffer it was waiting for was the buffer that was
   // just processed.
   // It works that way because I can not have one signal per-buffer...
   // instead there is only one signal and all the methods waiting for buffer
   // will know that a "new" buffer has been processed (even if that particular
   // method is not the one waiting for the buffer)

   if ( IsQueueEmpty() )
      return 0;

   if (gDebug > 0) Info("ProcessQueue", " Calling UnzipCache() ");

   return UnzipCache();
}

//_____________________________________________________________________________
void TTreeCacheUnzip::SendSignal()
{
   // This will send the signal corresponfing to the queue... normally used
   // when we want to start processing the list of buffers.

   if (gDebug > 0) Info("SendSignal", " fUnzipCondition->Signal()");

   fUnzipCondition->Signal();
}

//_____________________________________________________________________________
Int_t TTreeCacheUnzip::SetParallelUnzip(Option_t* option)
{
   // Static function that(de)activates multithreading unzipping
   // The possible options are:
   // "E" _Enable_ it, which causes an automatic detection and launches the
   //  additional thread if the number of cores in the machine is greater than one.
   // "D" _Disable_ will not activate the additional thread.
   // "F" _Force_ will start the additional thread even if there is only one core.
   // "" will be taken as "E".
   // returns 0 if there was an error, 1 otherwise.

   TString opt(option);
   opt.ToLower();
   if (opt.Contains("d")) {
      fgParallel = "d";
      return 1;
   }
   if (opt.Contains("f")) {
      fgParallel = "f";
      return 1;
   }
   if (opt.Contains("e") || opt.IsNull()) {
      fgParallel = "e";
      return 1;
   }
   return 0;
}

//_____________________________________________________________________________
Int_t TTreeCacheUnzip::StartThreadUnzip()
{
   // The Thread is only a part of the TTreeCache but it is the part that
   // waits for info in the queue and process it... unfortunatly, a Thread is
   // not an object an we have to deal with it in the old C-Style way
   // Returns 0 if the thread was initialized or 1 if it was already running

   if(!fUnzipThread) {
      fActiveThread=kTRUE;
      fUnzipThread= new TThread("UnzipLoop", UnzipLoop, (void*) this);
      fUnzipThread->Run();
      return 0;
   }
   return 1;
}

//_____________________________________________________________________________
Int_t TTreeCacheUnzip::StopThreadUnzip()
{
   // To stop the thread we only need to change the value of the variable
   // fActiveThread to false and the loop will stop (of course, we will have)
   // to do the cleaning after that.
   // Note: The syncronization part is important here or we will try to delete
   //       teh object while it's still processing the queue

   if(fUnzipThread){
      fActiveThread = kFALSE;
      SendSignal();
      if (fUnzipThread->Exists()) {
         SendSignal();
         fUnzipThread->Join();
      }
      fUnzipThread = 0;
      return 0;
   }
   return 1;
}

//_____________________________________________________________________________
void TTreeCacheUnzip::WaitForSignal()
{
   // This is the counter part of SendSignal() and is used to wait for a buffer
   // that is in the queue and will be processed soon (instead of making a new
   // call)

   fUnzipCondition->Wait();
}

//_____________________________________________________________________________
void* TTreeCacheUnzip::UnzipLoop(void *arg)
{
   // This is a static function.
   // This is the call that will be executed in the Thread generated by
   // StartThreadTreeCacheUnzip... what we want to do is to inflate the next
   // series of buffers leaving them in the second cache.
   // Returns 0 when it finishes

   TTreeCacheUnzip *unzipMng = (TTreeCacheUnzip *)arg;
   TThread::SetCancelOn();
   TThread::SetCancelDeferred();

   while( unzipMng->IsActiveThread() ) {
      unzipMng->ProcessQueue();
      if(!unzipMng->IsActiveThread()) break;
      TThread::CancelPoint();
      unzipMng->WaitForSignal();
   }
   return (void *)0;
}

//_____________________________________________________________________________
//_____________________________________________________________________________

//_____________________________________________________________________________
Int_t TTreeCacheUnzip::GetRecordHeader(char *buf, Int_t maxbytes, Int_t &nbytes, Int_t &objlen, Int_t &keylen)
{
   // Read the logical record header from the buffer buf.
   // That must be the pointer tho the header part not the object by itself and
   // must contain data of at least maxbytes
   // Returns nread;
   // In output arguments:
   //    nbytes : number of bytes in record
   //             if negative, this is a deleted record
   //             if 0, cannot read record, wrong value of argument first
   //    objlen : uncompressed object size
   //    keylen : length of logical record header
   // Note that the arguments objlen and keylen are returned only
   // if maxbytes >=16
   // Note: This was adapted from TFile... so some things dont apply

   Version_t versionkey;
   Short_t klen;
   UInt_t datime;
   Int_t nb = 0,olen;
   Int_t nread = maxbytes;
   frombuf(buf,&nb);
   nbytes = nb;
   if (nb < 0) return nread;
   //   const Int_t headerSize = Int_t(sizeof(nb) +sizeof(versionkey) +sizeof(olen) +sizeof(datime) +sizeof(klen));
   const Int_t headerSize = 16;
   if (nread < headerSize) return nread;
   frombuf(buf, &versionkey);
   frombuf(buf, &olen);
   frombuf(buf, &datime);
   frombuf(buf, &klen);
   if (!olen) olen = nbytes-klen;
   objlen = olen;
   keylen = klen;
   return nread;
}

//_____________________________________________________________________________
void TTreeCacheUnzip::ResetCache()
{
   // This will delete the list of buffers that are in the unzipping cache
   // and will reset certain values in the cache.
   // This name is ambiguos because the method doesn't reset the whole cache,
   // only the part related to the unzipping
   // Note: This method is completely different from TTreeCache::ResetCache(),
   // in that method we were cleaning the prefetching buffer while here we 
   // delete the information about the unzipping buffers
   R__LOCKGUARD(fMutexList);

   TUnzipBufferInfo *ind  = 0;
   TIter next(fUnzipList);
   while ((ind = (TUnzipBufferInfo*)next())) {
      TBasket *basket=ind->GetBasket();
      if(basket && !ind->GetRead()) {
	 // dont use basket->DeleteFromBranch to avoid a possible (although not probable)
	 // deadlock since that function will call TBranch::DropBasket which will lock
         // fgMutexBranch...
         // and in a second thread we would lock the mutexes in the inverse order.
         // this will work as an invalidation
         // basket->DeleteFromBranch();
         ind->SetRead(kTRUE);
      }
   }
   // We invalidate the cache buffer but remove the pointers from the list
   fUnzipList->Delete();

   fLastPos     = 0;
   fPosRead     = 0;
   fPosWrite    = 0;
   fNewTransfer = kTRUE;
}

//_____________________________________________________________________________
Int_t TTreeCacheUnzip::GetUnzipBuffer(char **buf, Long64_t pos, Int_t len, Bool_t *free, TBasket *basRef)
{
   // We try to read a buffer that has already been unzipped
   // Returns -1 in case of read failure, 0 in case it's not in the
   // cache and n>0 in case read from cache (number of bytes copied).
   // pos and len are the original values as were passed to ReadBuffer
   // but instead we will return the inflated buffer.
   // Note!! : If *buf == 0 we will allocate the buffer and it will be the
   // responsability of the caller to free it... it is useful for example
   // to pass it to the creator of TBuffer

   if (fParallel){
      if ( fIsLearning ) {
         // We need to reset it for new transferences...
         ResetCache();
         TFileCacheRead::Prefetch(0,0);
      }
      // The modify the cache if it's in de middle of something (unzipping for example)
      R__LOCKGUARD(fMutexCache);

      // If this is the first time we get here since the last FillBuffer
      // it's probable that the information about the prefetched buffers is there
      // but it hasn't actually been transfered... Is this the best place to put it??
      if (fNseek > 0 && !fIsSorted) {
         Sort();

         // Then we use the vectored read to read everything now
         if (fFile->ReadBuffers(fBuffer,fPos,fLen,fNb)) {
	    return -1;
         }
      }

      // be careful.. can be -1
      Int_t loc = (Int_t)TMath::BinarySearch(fNseek,fSeekSort,pos);

      if (loc >= 0) {
         // We shouldn't have to "wait" so a long Lock is not be very bad for the performance
         R__LOCKGUARD(fMutexList);

         // Look for the index in the list of unzipped buffers...
         TUnzipBufferInfo *ind  = 0;
         TUnzipBufferInfo *last = 0;
         Int_t i = 0;
         Int_t n = fUnzipList->GetSize();
         Bool_t oneLeft = kFALSE;
         TIter next(fUnzipList);
         ind = (TUnzipBufferInfo*)next();
         while (ind) {
            // it was found in the list
            if (ind->GetNum() == loc)
               break;

            ind = (TUnzipBufferInfo*)next();
            if ( i==n-3 ){
               if(( ind && (ind->GetNum() == loc) )) {
                  oneLeft = kTRUE;
               }
            }
            i++;
            if (ind) last = ind;
         }

         // not found in the unzipped range... inflate all following buffers
         // Trying the unzipping part if:
         // 1 - It is the first buffer requested
         // 2 - There is only one left (try to unzip next batch)
         // 3 - If it was not found in the list and it's a "future" buffer
         if ( ( !fUnzipList->GetSize() || oneLeft ||  ( (n == i) && last && (last->GetNum() < loc) ) )  && (!fIsLearning) ) {
            fLastPos = loc + 1;
            if (oneLeft)
               fLastPos = loc + 3;

            if (gDebug > 0)
               Info("GetUnzipBuffer", " Sending Signal found in the list loc:%d, i:%d, fUnzipList->GetSize():%d, fLastPos:%d", 
                    loc, i, fUnzipList->GetSize(), fLastPos);
            this->SendSignal();

            //maybe it was unzipped with the last signal
            if(!ind) {
               i = 0;
               n = fUnzipList->GetSize();
               oneLeft = kFALSE;
               TIter next2(fUnzipList);
               while ((ind = (TUnzipBufferInfo*)next2())) {
                  // it was found in the list
                  if (ind->GetNum() == loc)
                     break;
                  i++;
               }
            }
         }

         // is this buffer in the range unziped by the last unzipping call?
         if ( ind && (ind->GetNum() == loc) && (pos == fSeekSort[loc]) ) {
            // The second thread can modify this values after the signal fBufferCond->Signal() ... be careful
            Long64_t locPos = fUnzipPos[loc]; // Gives the pos in the buffer
            Int_t    locLen = fUnzipLen[loc]; // Gives the size in the buffer

            if (gDebug > 0)
               Info("GetUnzipBuffer", "FOUND in the list loc:%d, pos:%lld ind:%p, ind->GetNum():%d", loc, pos, ind, ind->GetNum());  

            fMutexBuffer->Lock();
            //memcpy(*buf,&fUnzipBuffer[locPos], locLen);
            *buf = &fUnzipBuffer[locPos];
            *free = kFALSE;
            fMutexBuffer->UnLock();
            ind->SetBasket(basRef);
            fNFound++;
            return locLen;
         }
      }
      if (gDebug > 0)
         Info("GetUnzipBuffer", " loc:%d, Buffer pos: %lld,  len: %d, Was NOT FOUND in the cache...", 
              loc, pos, len);
      fBufferCond->Signal();
   }

   char *comp = new char[len];
   Bool_t found = kFALSE;

   if (fNseek > 0 && !fIsSorted) {
      if (gDebug > 0)
         Info("GetUnzipBuffer", "This is a new transfer... must clean things up fNSeek:%d", fNseek);
      ResetCache();
   }

   //fMutexList->Lock();  // *** fMutexList  Lock
   if (TFileCacheRead::ReadBuffer(comp,pos,len) == 1){
      found = kTRUE;
   }
   if(!found) {
      //not found in cache. Do we need to fill the cache?
      Bool_t bufferFilled = FillBuffer();
      if (bufferFilled) {
         if (TFileCacheRead::ReadBuffer(comp,pos,len) == 1){
            found = kTRUE;
         }
      }
   }

   if (!found) {
      fFile->Seek(pos);
      if(fFile->ReadBuffer(comp, len)){
         Error("GetUnzipBuffer", " Error reading from TFile ... must go out");
         delete [] comp;
         return -1;
      }
   }
   //fMutexList->UnLock();  // *** fMutexList  UnLock

   Int_t res = UnzipBuffer(buf, comp);
   *free = kTRUE;
   if (!fIsLearning) fNMissed++;

   if (comp) delete [] comp;
   return res;
}

//_____________________________________________________________________________
void TTreeCacheUnzip::SetBufferRead(Long64_t pos, Int_t len, TBasket *basket)
{
   // inform the cache that a buffer contained in it's memory has been read
   // and is ready to be reused... a kind of trcky algorithm to avoid
   R__LOCKGUARD(fMutexList);

   (void) len;
   TUnzipBufferInfo *ind  = 0;
   TIter next(fUnzipList);
   while ((ind = (TUnzipBufferInfo*)next())) {
      // it was found in the list
      if ((ind->GetPos()) == pos && (ind->GetBasket()) == basket){
         // mark as read and go out of the list
         Int_t index = ind->GetNum();
         Long64_t locPos = fUnzipPos[index]; // Gives the pos in the buffer
         Int_t    locLen = fUnzipLen[index]; // Gives the size in the buffer

         ind->SetRead(kTRUE);
         if (gDebug > 0)
            Info("SetBufferRead", "Marking as READ ind:%p, pos:%lld, num:%d, basket:%p", ind, ind->GetPos(), index, basket);

         // If the second thread is waiting fr this buffer to be read... inform it
         if( (fPosRead <= fPosWrite) && (locPos + locLen >= fPosWrite)) {
            fBufferCond->Signal();
         }
         fPosRead = locPos + locLen;
      }
   }
}

//_____________________________________________________________________________
void TTreeCacheUnzip::SetUnzipBufferSize(Long64_t bufferSize)
{
   // Sets the size for the unzipping cache... by default it should be
   // two times the size of the prefetching cache

   fUnzipBufferSize = bufferSize;
}

//_____________________________________________________________________________
Int_t TTreeCacheUnzip::UnzipBuffer(char **dest, char *src)
{
   // UNzips a ROOT specific buffer... by reading the header at the beginning.
   // returns the size of the inflated buffer or -1 if error
   // Note!! : If *dest == 0 we will allocate the buffer and it will be the
   // responsability of the caller to free it... it is useful for example
   // to pass it to the creator of TBuffer
   // src is the original buffer with the record (header+compressed data)
   // *dest is the inflated buffer (including the header)
   R__LOCKGUARD(fMutexUnzipBuffer);

   Int_t  uzlen = 0;
   Bool_t alloc = kFALSE;

   // Here we read the header of the buffer
   const Int_t hlen=128;
   Int_t nbytes=0, objlen=0, keylen=0;
   GetRecordHeader(src, hlen, nbytes, objlen, keylen);

   if (gDebug > 1)
      Info("UnzipBuffer", "nbytes:%d, objlen:%d, keylen:%d  ", nbytes, objlen, keylen); 

   if (!(*dest)) {
      *dest = new char[keylen+objlen];
      alloc = kTRUE;
   }
   // Must unzip the buffer
   // fSeekPos[ind]; adress of zipped buffer
   // fSeekLen[ind]; len of the zipped buffer
   // &fBuffer[fSeekPos[ind]]; memory address

   // This is similar to TBasket::ReadBasketBuffers
   Bool_t oldCase = objlen==nbytes-keylen
      && ((TBranch*)fBranches->UncheckedAt(0))->GetCompressionLevel()!=0
      && fFile->GetVersion()<=30401;

   if (objlen > nbytes-keylen || oldCase) {
      // /**/ Question? ... do we have to ask this
      //      for every buffer or the global fSkipZip is enough?
      //      can somebody else set it up?

      if (fSkipZip) {
         // Copy without unzipping
         memcpy(*dest, src, keylen);
         uzlen += keylen;

         memcpy(*dest, src + keylen, objlen);
         uzlen += objlen;

         return nbytes;
      }

      // Copy the key
      if (gDebug > 2)
         Info("UnzipBuffer", "Copy the key keylen:%d from src:%p to *dest:%p", keylen, src, *dest);     

      memcpy(*dest, src, keylen);
      uzlen += keylen;

      char *objbuf = *dest + keylen;
      UChar_t *bufcur = (UChar_t *) (src + keylen);
      Int_t nin, nout, nbuf;
      Int_t noutot = 0;

      while (1) {
         nin  = 9 + ((Int_t)bufcur[3] | ((Int_t)bufcur[4] << 8) | ((Int_t)bufcur[5] << 16));
         nbuf = (Int_t)bufcur[6] | ((Int_t)bufcur[7] << 8) | ((Int_t)bufcur[8] << 16);

         if (gDebug > 2)
            Info("UnzipBuffer", " nin:%d, nbuf:%d, bufcur[3] :%d, bufcur[4] :%d, bufcur[5] :%d ", 
                 nin, nbuf, bufcur[3], bufcur[4], bufcur[5]);

         if (oldCase && (nin > objlen || nbuf > objlen)) {
            if (gDebug > 2)
               Info("UnzipBuffer", "oldcase objlen :%d ", objlen);                    

            //buffer was very likely not compressed in an old version
            memcpy( *dest + keylen, src + keylen, objlen);
            uzlen += objlen;
            return uzlen;
         }

         R__unzip(&nin, bufcur, &nbuf, objbuf, &nout);
         if (gDebug > 2)
            Info("UnzipBuffer", "R__unzip nin:%d, bufcur:%p, nbuf:%d, objbuf:%p, nout:%d", 
                 nin, bufcur, nbuf, objbuf, nout);

         if (!nout) break;
         noutot += nout;
         if (noutot >= objlen) break;
         bufcur += nin;
         objbuf += nout;
      }

      if (noutot != objlen) {
         Error("UnzipBuffer", "nbytes = %d, keylen = %d, objlen = %d, noutot = %d, nout=%d, nin=%d, nbuf=%d", 
               nbytes,keylen,objlen, noutot,nout,nin,nbuf);
         uzlen = -1;
         if(alloc) delete [] *dest;
         return uzlen;
      }
      uzlen += objlen;
   } else {
      memcpy(*dest, src, keylen);
      uzlen += keylen;
      memcpy(*dest + keylen, src + keylen, objlen);
      uzlen += objlen;
   }
   return uzlen;
}

//_____________________________________________________________________________
Int_t TTreeCacheUnzip::UnzipCache()
{
   // This inflates all the buffers in the cache.. passing the data to a new
   // buffer that will only wait there to be read...
   // We can not inflate all the buffers in the cache so we will try to do
   // it by parts... there is a member calle fUnzipBufferSize which will
   // tell us the size we can allocate for this cache so we will divide
   // the prefeteched cche in chunks of this size and we will try to unzip then
   // note that we will  unzip in the order they were put into the cache not
   // the order of the transference so it has to be read in that order or the
   // pre-unzipping will be useless.
   // pos and len are used to see where we have to start unzipping...
   // and for how long..
   // returns 0 in normal conditions or -1 if error

   if(!fIsSorted) {
      if (gDebug > 0)
         Info("UnzipCache", "It is still in the learning phase");
      return 0;
   }

   // This is the first time we unzip the cache
   if (fUnzipBufferSize == 0) {
      // creating a new buffer
      SetUnzipBufferSize((Long64_t)(fgRelBuffSize*fBufferSize));

      fMutexBuffer->Lock();   //*** fMutexBuffer Lock
      fUnzipBuffer = new char[fUnzipBufferSize];
      fMutexBuffer->UnLock(); //*** fMutexBuffer Lock
      if (gDebug > 0)
         Info("UnzipCache", "Creating a buffer of %lld bytes ", fUnzipBufferSize);
   }
   if(fNseekMax < fNseek){
      if (gDebug > 0)
         Info("UnzipCache", "Changing fNseekMax from:%d to:%d", fNseekMax, fNseek);

      fMutexList->Lock();  //*** fMutexList Lock
      Int_t *aUnzipPos = new Int_t[fNseek];
      Int_t *aUnzipLen = new Int_t[fNseek];

      for (Int_t i=0;i<fNseekMax;i++) {
         aUnzipPos[i] = fUnzipPos[i];
         aUnzipLen[i] = fUnzipLen[i];
      }

      if (fUnzipPos) delete [] fUnzipPos;
      if (fUnzipLen) delete [] fUnzipLen;

      fUnzipPos  = aUnzipPos;
      fUnzipLen  = aUnzipLen;
      fNseekMax  = fNseek;
      fMutexList->UnLock(); //*** fMutexList UnLock
   }

   // fLastPosf may change in the middle when two signals are sent.
   Int_t localLastPos = fLastPos;
   Int_t localNseek   = fNseek;

   Long64_t locPos = 0; // Local values for each buffer... change them together to ease sync.
   Int_t    locLen = 0;
   Int_t badread = 0;
   fNewTransfer = kFALSE;
   for (Int_t i = localLastPos; i<localNseek; i++) {
      // Always for a change before continuing
      if (!IsActiveThread() || !fNseek || fIsLearning || fNewTransfer){
         return 0;
      }

      // This must have this lock because UnzipBuffer can access GetRecordHeader also
      fMutexUnzipBuffer->Lock();  //*** fMutexUnzipBuffer Lock
      const Int_t hlen=128;
      Int_t objlen=0, keylen=0;
      Int_t nbytes=0;
      GetRecordHeader(&fBuffer[fSeekPos[i]], hlen, nbytes, objlen, keylen);
      Int_t len = (objlen > nbytes-keylen)? keylen+objlen : nbytes;
      fMutexUnzipBuffer->UnLock();  //*** fMutexUnzipBuffer UnLock

      // We need a protection here in case a buffer is bigger than
      // the whole unzipping cache... do it only at the first iteration
      // to guarantee that it indeed doesnt fit the cache...
      if ((i == localLastPos) && (len > fUnzipBufferSize)) {
         if (gDebug > 0)
            Info("UnzipCache", "One buffer is too big resizing from:%d to len*2:%d", fUnzipBufferSize, len*2);  

         fMutexBuffer->Lock();  //*** fMutexBuffer Lock
         char *newBuffer = new char[len*2];
         memcpy(newBuffer, fUnzipBuffer, fPosWrite);
         delete [] fUnzipBuffer;

         SetUnzipBufferSize((Long64_t)(len*2));
         fUnzipBuffer = newBuffer;
         fMutexBuffer->UnLock(); //*** fMutexBuffer UnLock
      }

      if (i > localLastPos) {
         fMutexList->Lock();   //*** fMutexList Lock
         locPos = fUnzipPos[i-1] + fUnzipLen[i-1];
         locLen = 0; // just in case
         fMutexList->UnLock(); //*** fMutexList UnLock
      }

      // if it is going to exceed the buffer size then stop
      if( (locPos + len) > fUnzipBufferSize ){
         if (gDebug > 0)
            Info("UnzipCache", "Cache is full.. breaking i:%d, fUnzipBufferSize: %lld, locPos: %lld", 
                 i, fUnzipBufferSize, locPos);
         break;
      }

      // I know this is very very ugly but we need the scope for the mutex
      {
         R__LOCKGUARD(fMutexList);

         TUnzipBufferInfo *ind = 0;
         TIter next(fUnzipList);
         while ( (fUnzipList->GetSize()>0) && (ind=(TUnzipBufferInfo*)next())) {
            if (!IsActiveThread() || !fNseek || fIsLearning || fNewTransfer)
               return 0;

            Int_t index = ind->GetNum();

            //delete if it was already read
            if( ind->GetRead() ) {
               if ( fUnzipList->Remove(ind) )
                  delete ind;
               fUnzipPos[index] = 0;
               fUnzipLen[index] = 0;
               continue;
            }

            // if the new buffer overlaps an old one
            if( ((fUnzipPos[index] <= locPos) && (locPos < (fUnzipPos[index] + fUnzipLen[index])))
                || ((fUnzipPos[index] >= locPos) && ((locPos + len) > fUnzipPos[index] )) ) {
               if (gDebug > 0)
                  Info("UnzipCache", "Removing from the list fUnzipList->Remove(ind) ind:%p, index:%d", ind, index);

               if ( !ind->GetRead() ) {
                  // We will get a deadlock in the next wait is this mutex is still locked and the other thread
                  // tries to get a different buffer so we have to unlock it, but we it's risky since a ResetCache
                  // can change our list... that's why we need to check the state exactly after obtaining the mutex 
                  // again

                  if (gDebug > 0)
                     Info("UnzipCache", "Waiting 1 !!!");

                  fMutexList->UnLock();  //*** fMutexList UnLock
                  TThread::CancelPoint();
                  fBufferCond->Wait();
                  fMutexList->Lock();   //*** fMutexList Lock

                  if (!IsActiveThread() || !fNseek || fIsLearning || fNewTransfer)
                     return 0;

                  //delete if it was already read (while we were waiting)
                  Bool_t read = ind->GetRead();
                  TBasket *basket=ind->GetBasket();
                  if ( fUnzipList->Remove(ind) )
                     delete ind;
                  fUnzipPos[index] = 0;
                  fUnzipLen[index] = 0;

                  if(basket && !read) {
                     // We will get a very nasty deadlock if we call DeleteFromBranch
                     // while locking fMutexList.
                     //
                     // UnzipCache locks _fMutexList_
                     // UnzipCache calls TBasket::DeleteFromBranch
                     // TBasket::DeleteFromBranch calls TBranch::DropBasket
                     // TBranch::DropBasket locks _fMutexBranch_
                     //
                     // and from the other thread
                     // TBranch::GetEntry locks _fMutexBranch_
                     //
                     // and we wait to dead...
                     fMutexList->UnLock(); //*** fMutexList UnLock
                     basket->DeleteFromBranch();

                     fMutexList->Lock();   //*** fMutexList Lock

                     if (!IsActiveThread() || !fNseek || fIsLearning || fNewTransfer)
                        return 0;
                  }
               }
            }
         }
      }

      // Initially we had fBuffer with all the buffers to be unzipped... we took
      // one of then, we unzipped it and we put it in fUnzipBuffer. But the first
      // thread can start a transference at the same moment changing the content of fBuffer,
      // putting a lock on it is inneficient because we have to wait for the unzip to
      // start the transfer and I have seen that is better to just copy the data to a 
      // "local variable"... but doing _new_ here wouldn't be very good also so
      // I use a preallocated buffer.
      if(fTmpBufferSz < fSeekSortLen[i]) {
         delete [] fTmpBuffer;
         fTmpBufferSz = fSeekSortLen[i]*2;
         fTmpBuffer = new char[fTmpBufferSz];
      }

      fMutexList->Lock();   //*** fMutexList Lock
      memcpy(fTmpBuffer, &fBuffer[fSeekPos[i]], fSeekSortLen[i]);
      fMutexList->UnLock(); //*** fMutexList UnLock

      char *ptr = &fUnzipBuffer[locPos];
      locLen = UnzipBuffer(&ptr, fTmpBuffer);

      R__LOCKGUARD(fMutexList); //*** fMutexList LOCK until gong out of scope

      if (!IsActiveThread() || !fNseek || fIsLearning || fNewTransfer)
         return 0;

      fPosWrite    = locPos + locLen;
      fUnzipPos[i] = locPos;
      fUnzipLen[i] = locLen;
      if (gDebug > 0)
         Info("UnzipCache", "i:%d fUnzipPos[i]: %d,  fUnzipLen[i]: %d",
              i, fUnzipPos[i], fUnzipLen[i]);

      // add it to the list of unzipped buffers
      TUnzipBufferInfo *elem = new TUnzipBufferInfo(i);
      elem->SetPos(fSeekSort[i]);
      elem->SetLen(fSeekSortLen[i]);
      fUnzipList->AddLast(elem);

      if(gDebug > 0)
         Info("UnzipCache", "Adding elem:%p, i:%d, pos:%lld, len:%d", elem, i, elem->GetPos(), elem->GetLen());

      fNUnzip++;
      localLastPos = i;
   }
   R__LOCKGUARD(fMutexList); //*** fMutexList LOCK until gong out of scope

   fLastPos = localLastPos;

   if(!badread) fUnzipped = kTRUE;
   return badread;
}

//_____________________________________________________________________________
//_____________________________________________________________________________
