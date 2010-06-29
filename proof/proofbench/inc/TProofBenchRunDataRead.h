// @(#)root/proofx:$Id:$
// Author: Sangsu Ryu 22/06/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofBenchRunDataRead
#define ROOT_TProofBenchRunDataRead

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBenchRunDataRead                                               //
//                                                                      //
// I/O-intensive PROOF benchmark test reads in event files distributed  //
// on the cluster. Number of events processed per second and size of    //
// events processed per second are plotted against number of active     //
// workers. Performance rate for unit packets and performance rate      //
// for query are plotted.                                               //
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
class TTree;

class TProofBenchMode;
class TProofBenchRunCleanup;

R__EXTERN TProof *gProof;

class TProofBenchRunDataRead : public TProofBenchRun{

private:
   TProof* fProof;               //pointer to proof

   TProofBenchRun::EReadType fReadType; //read type
   TProofBenchMode* fMode;              //mode
   TProofBenchRunCleanup* fRunCleanup;  //clean-up run

   Long64_t fNEvents;            //number of events per file
   Int_t fNTries;                //number of tries
   Int_t fMaxNWorkers;           //number of maximum processes
   Int_t fStart;                 //start number of workers
   Int_t fStop;                  //stop number of workers
   Int_t fStep;                  //test to be performed every fStep workers
   Int_t fDraw;                  //draw switch
   Int_t fDebug;                 //debug switch

   TFile* fFile;                 //output file to write performance histograms and trees on
   TDirectory* fDirProofBench;   //directory for proof outputs
   Bool_t fWritable;             //file writable

   TList* fNodes;                //list of nodes information

   TList* fPerfStats;            //list of PROOF_PerfStats
   TList* fListPerfProfiles;     //list of performance profiles

   TCanvas* fCPerfProfiles;      //canvas for performance profile histograms

   TString fName;                //name of this run

protected:

   void FillPerfStatProfiles(TTree* t, TProfile* profile_event, TProfile* profile_IO, Int_t nactive);

   Int_t FillNodeInfo();

   Int_t SetParameters();
   Int_t DeleteParameters();

   TString BuildPatternName(const TString& objname, const TString& delimiter="_");
   TString BuildNewPatternName(const TString& objname, Int_t nactive, Int_t tries, const TString& delimiter="_");
   TString BuildProfileName(const TString& objname, const TString& type, const TString& delimiter="_");
   TString BuildProfileTitle(const TString& objname, const TString& type, const TString& delimiter=" ");

public:

   TProofBenchRunDataRead(TProofBenchMode* mode,
                          TProofBenchRunCleanup* runcleanup,
                          TProofBenchRun::EReadType readtype=TProofBenchRun::kReadNotSpecified,
                          TString filename="",
                          Option_t* foption="",
                          TProof* proof=gProof,
                          Int_t maxnworkers=-1,
                          Long64_t nevents=-1,
                          Int_t ntries=2,
                          Int_t start=1,
                          Int_t stop=-1,
                          Int_t step=1,
                          Int_t draw=0,
                          Int_t debug=0);

   virtual ~TProofBenchRunDataRead();

   void Run(Long64_t nevents,
            Int_t ntries,
            Int_t start,
            Int_t stop,
            Int_t step,
            Int_t debug,
            Int_t draw);

   void DrawPerfProfiles();

   void Print(Option_t* option="")const;

   void SetReadType(TProofBenchRun::EReadType readtype);
   void SetMode(TProofBenchMode* mode);
   void SetRunCleanup(TProofBenchRunCleanup* runcleanup);
   void SetNEvents(Long64_t nevents);
   void SetNTries(Int_t ntries);
   void SetMaxNWorkers(Int_t maxnworkers);
   void SetMaxNWorkers(TString sworkers);
   void SetStart(Int_t start);
   void SetStop(Int_t stop);
   void SetStep(Int_t step);
   void SetDraw(Int_t draw);
   void SetDebug(Int_t debug);
   TFile* OpenFile(const char* filename="",
                   Option_t* option="",
                   const char* ftitle = "",
                   Int_t compress = 1);
   void SetDirProofBench(TDirectory* dir);

   TProofBenchRun::EReadType GetReadType()const;
   TProofBenchMode* GetMode()const;
   TProofBenchRunCleanup* GetRunCleanup()const;
   Long64_t GetNEvents()const;
   Int_t GetNTries()const;
   Int_t GetMaxNWorkers()const;
   Int_t GetStart()const;
   Int_t GetStop()const;
   Int_t GetStep()const;
   Int_t GetDraw()const;
   Int_t GetDebug()const;
   TFile* GetFile()const;
   TDirectory* GetDirProofBench()const;
   TList* GetListOfPerfStats()const;
   TList* GetListPerfProfiles()const;
   TCanvas* GetCPerfProfiles()const;
   const char* GetName()const;

   TString GetNameStem()const;

   ClassDef(TProofBenchRunDataRead,0)         //I/O-intensive PROOF benchmark
};

#endif
