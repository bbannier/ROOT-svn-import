// @(#)root/proofx:$Id:$
// Author: Sangsu Ryu 22/06/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofBenchMode
#define ROOT_TProofBenchMode

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBenchMode                                                      //
//                                                                      //
// An abstract class for modes for PROOF benchmark test.                //
// A mode determines how files are generated in the cluster and used    //
// during the PROOF benchmark test.                                     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif

class TProof;
class TDSet;
class TMap;

R__EXTERN TProof *gProof;

class TProofBenchMode: public TObject{

private:

public:

   enum EFileType {kFileNotSpecified=0,  //type not specified
                   kFileBenchmark,       //file for benchmark test
                   kFileCleanup};        //file for cleaning up memory between runs

   virtual ~TProofBenchMode();

   virtual TMap* FilesToProcess(Int_t nf)=0;

   virtual Int_t MakeDataSets(Int_t nf=-1,
                              Int_t start=1,
                              Int_t stop=-1,
                              Int_t step=1,
                              const TDSet* tdset=0,
                              const char* option="",
                              TProof* proof=gProof)=0;

   virtual Int_t MakeDataSets(Int_t nf,
                              Int_t np,
                              const Int_t *wp,
                              const TDSet* tdset=0,
                              const char *option="",
                              TProof* proof=gProof)=0;


   virtual EFileType GetFileType()=0;

   virtual void Print(Option_t* option="")const=0;

   virtual void SetNFiles(Int_t nfiles)=0;
   virtual Int_t GetNFiles()const=0;

   ClassDef(TProofBenchMode,0)         //Abstract class for modes for Proof benchmark test
};

#endif
