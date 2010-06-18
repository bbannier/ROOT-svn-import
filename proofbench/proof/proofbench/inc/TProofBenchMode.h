// @(#)root/proofx:$Id:$
// Author:

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBenchMode                                                      //
//                                                                      //
// TProofBenchMode is ...                                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TProofBenchMode
#define ROOT_TProofBenchMode

#ifndef ROOT_TObject
#include "TObject.h"
#endif

class TProof;
class TDSet;
class TMap;

R__EXTERN TProof *gProof;

class TProofBenchMode: public TObject{

public:

   enum EFileType {kFileNotSpecified=0,  //type not specified
                   kFileBenchmark,       //file for benchmark test
                   kFileCleanup};        //file for cleaning up memory between runs

   virtual ~TProofBenchMode();          //destructor

   virtual TMap* FilesToProcess(Int_t nf)=0;

   virtual Int_t MakeDataSets(Int_t nf=-1,   //when ==-1, use data member fNFiles in the derived class
                              Int_t start=1, //start number of workers
                              Int_t stop=-1,  //when ==-1, end number of workers are set to max workers
                              Int_t step=1,
                              const TDSet* tdset=0,
                              const char* option="",
                              TProof* proof=gProof)=0;  //

   virtual Int_t MakeDataSets(Int_t nf,
                              Int_t np,
                              const Int_t *wp,
                              const TDSet* tdset=0,
                              const char *option="",
                              TProof* proof=gProof)=0;


   virtual EFileType GetFileType()=0;

   virtual void Print(Option_t* option="")const=0;   //print status of an instance of this class 

   virtual void SetNFiles(Int_t nfiles)=0;
   virtual Int_t GetNFiles()const=0;

private:

   ClassDef(TProofBenchMode,0)         //Abstract class for Proof benchmark
};

#endif
