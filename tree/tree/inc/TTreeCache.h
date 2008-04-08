// @(#)root/tree:$Id$
// Author: Rene Brun   04/06/2006

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TTreeCache
#define ROOT_TTreeCache


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TTreeCache                                                           //
//                                                                      //
// Specialization of TFileCacheRead for a TTree                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TFileCacheRead
#include "TFileCacheRead.h"
#endif
#ifndef ROOT_TObjArray
#include "TObjArray.h"
#endif

class TTree;
class TBranch;
class TThread;
class TCondition;
class TSortedList;
class TBasket;
class TMutex;

 // TBufferInfo is a simple container to put the Info of each element in the
 // unzipping buffer.. it has nothing to do with TBuffer so I'm afraid the
 // name can be a bit misleading.
 class TBufferInfo : public TObject {
 private:
    Int_t      fNum;       // This is like the id of the element
    Bool_t     fRead;      // has it been already read? (to know if we can delete it)
    Long64_t   fPos;       // the offset of the buffer in the file
    Int_t      fLen;       // the len of the buffer in the file
    TBasket    *fBasket;   // the ref to the basket that is related to this buffer element

 public:
    TBufferInfo(Int_t i = 0) : fNum(i), fRead(kFALSE), fPos(0), fLen(0), fBasket(0) { }
    ~TBufferInfo() {}
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

    void     Print(Option_t *) const { Info("TBufferInfo","fNum = %d, fPos:%lld, fLen:%d, fRead?:%d, fBasket:%p", fNum, fPos, fLen, fRead, fBasket); }
    ULong_t  Hash() const { return fNum; }
    Bool_t   IsEqual(const TObject *obj) const { return fNum == ((TBufferInfo*)obj)->GetNum(); }
    Bool_t   IsSortable() const { return kTRUE; }
    Int_t    Compare(const TObject *obj) const { if (fNum > ((TBufferInfo*)obj)->GetNum())
                                                    return 1;
                                                 else if (fNum < ((TBufferInfo*)obj)->GetNum())
                                                    return -1;
                                                 else
                                                    return 0; 
                                               }
 };
 

class TTreeCache : public TFileCacheRead {

protected:
   Long64_t        fEntryMin;    //! first entry in the cache
   Long64_t        fEntryMax;    //! last entry in the cache
   Long64_t        fEntryNext;   //! next entry number where cache must be filled
   Long64_t        fZipBytes;    //! Total compressed size of branches in cache
   Int_t           fNbranches;   //! Number of branches in the cache
   Int_t           fNReadOk;     //Number of blocks read and found in the cache
   Int_t           fNReadMiss;   //Number of blocks read and not found in the chache
   Int_t           fNReadPref;   //Number of blocks that were prefetched
   TObjArray      *fBranches;    //! List of branches to be stored in the cache
   TList          *fBrNames;     //! list of branch names in the cache
   TTree          *fOwner;       //! pointer to the owner Tree/chain
   TTree          *fTree;        //! pointer to the current Tree
   Bool_t          fIsLearning;  //! true if cache is in learning mode
   Bool_t          fIsManual;    //! true if cache is StopLearningPhase was used
   static  Int_t fgLearnEntries; //Number of entries used for learning mode

   TMutex         *fMutexCache;

   // Members for paral. managing
   TThread    *fUnzipThread;
   Bool_t      fActiveThread;
   TCondition *fUnzipCondition; 
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
   static Double_t fgRelBuffSize;  // This is the percentage of the TTreeCache that will be used
 
   // Members use to keep statistics
   Int_t      fNUnzip;           //! number of blocks that were unzipped
   Int_t      fNFound;           //! number of blocks that were found in the cache
   Int_t      fNMissed;          //! number of blocks that were not found in the cache and were unzipped

private:
   TTreeCache(const TTreeCache &);            //this class cannot be copied
   TTreeCache& operator=(const TTreeCache &);

   // Private methods related to the thread
   Int_t StartThreadUnzip();
   Int_t StopThreadUnzip();

public:
   TTreeCache();
   TTreeCache(TTree *tree, Int_t buffersize=0);
   virtual ~TTreeCache();
   void                AddBranch(TBranch *b, Bool_t subbranches = kFALSE);
   void                AddBranch(const char *branch, Bool_t subbranches = kFALSE);
   Double_t            GetEfficiency();
   Double_t            GetEfficiencyRel();
   static Int_t        GetLearnEntries();
   Bool_t              FillBuffer();
   TTree              *GetOwner() const;
   TTree              *GetTree() const;
   void                Init();
   Bool_t              IsLearning() const {return fIsLearning;}
   virtual Int_t       ReadBuffer(char *buf, Long64_t pos, Int_t len);
   void                SetEntryRange(Long64_t emin,   Long64_t emax);
   static void         SetLearnEntries(Int_t n = 100);
   void                StartLearningPhase();
   void                StopLearningPhase();
   void                UpdateBranches(TTree *tree, Bool_t owner = kFALSE);

   // Methods related to the thread
   static Option_t *GetParallelUnzip();   
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
   Int_t  GetRecordHeader(char *buf, Int_t maxbytes, Int_t &nbytes, Int_t &objlen, Int_t &keylen);
   Bool_t GetSkipZip() { return fSkipZip; }
   void   ResetCache();
   Int_t  GetUnzipBuffer(char **buf, Long64_t pos, Int_t len, Bool_t *free, TBasket *basRef);
   void   SetBufferRead(Long64_t pos, Int_t len, TBasket *basket) ;
   void   SetUnzipBufferSize(Long64_t bufferSize);
   void   SetSkipZip(Bool_t skip = kTRUE) { fSkipZip = skip; }
   Int_t  UnzipBuffer(char **dest, char *src);
   Int_t  UnzipCache();
   
   // static members related to the thread
   static void* UnzipLoop(void *arg);

   ClassDef(TTreeCache,3)  //Specialization of TFileCacheRead for a TTree
};

#endif
