// @(#)root/proofx:$Id:$
// Author:

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofBenchRunCleanup
#define ROOT_TProofBenchRunCleanup

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBenchRunCleanup                                                //
//                                                                      //
// TProofBenchRunCleanup is ...                                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TString
#include "TString.h"
#endif

#ifndef ROOT_TProofBenchRun
#include "TProofBenchRun.h"
#endif

class TProof;
class TFile;
class TCanvas;
class TProfile;

R__EXTERN TProof *gProof;

class TProofBenchRunCleanup : public TProofBenchRun{

private:

   TProof* fProof;                 //pointer to proof

   TProofBenchRun::ECleanupType fCleanupType;
   Long64_t fNEvents;            //number of events per file for CPU test and/or I/O test
   Int_t fMaxNWorkers;           //number of maximum processes, 
                                 //this can be more than the number of total workers in the cluster
   TString fDataSetCleanup;      //data set to be cleaned up at nodes using fadvice
   Int_t fDraw;
   Int_t fDebug;

   TFile* fFile;                 //output file to write performance histograms and trees on
   TDirectory* fDirProofBench;   //directory for proof outputs
   Bool_t fWritable;

   TString fName;

protected:

   Int_t SetParameters();
   Int_t DeleteParameters();

   const char* BuildPatternName(const char* objname, const char* delimiter="_");

public:

   TProofBenchRunCleanup(TProofBenchRun::ECleanupType cleanuptype=TProofBenchRun::kCleanupNotSpecified,
                         TString filename="",//output file where benchmark performance plot will be written to, 
                                             //user has to provide one
                         Option_t* foption="", //option to TFile() 
                         TProof* proof=gProof,
                         Int_t maxnworkers=-1,//maximum number of workers to be tested. 
                                              //If not set (default), 2 times the number of total workers in the cluster available
                         Long64_t nevents=-1,
                         Int_t draw=0,
                         Int_t debug=0); //default constructor

   virtual ~TProofBenchRunCleanup();          //Destructor

   void Run(Long64_t nevents,
            Int_t,
            Int_t,
            Int_t,
            Int_t,
            Int_t debug,
            Int_t draw);

   void BuildPerfProfiles(Int_t,
                          Int_t,
                          Int_t,
                          Int_t);

   void DrawPerfProfiles();

   void Print(Option_t* option="")const;   //Print status of an instance of this class 

   void SetCleanupType(TProofBenchRun::ECleanupType cleanuptype);
   void SetNEvents(Long64_t nevents);
   void SetMaxNWorkers(Int_t maxnworkers);
   void SetMaxNWorkers(TString sworkers);
   void SetDataSetCleanup(const TString& dataset);
   void SetDraw(Int_t draw);
   void SetDebug(Int_t debug);
   TFile* OpenFile(const char* filename="",
                           Option_t* option="",
                           const char* ftitle = "",
                           Int_t compress = 1);    //open a file for outputs 
   void SetDirProofBench(TDirectory* dir);
   void SetName(TString name);

   TProofBenchRun::ECleanupType GetCleanupType()const;
   Long64_t GetNEvents()const;
   Int_t GetMaxNWorkers()const;
   TString GetDataSetCleanup()const;
   Int_t GetDraw()const;
   Int_t GetDebug()const;
   TFile* GetFile()const;
   TDirectory* GetDirProofBench() const;
   const char* GetName()const;

   TString GetNameStem()const;

   ClassDef(TProofBenchRunCleanup,0)         //PROOF benchmark memory cleaning-up run 
};

#endif
