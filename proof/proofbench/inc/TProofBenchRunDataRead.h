// @(#)root/proofx:$Id:$
// Author:

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
// TProofBenchRunDataRead is ...                                        //
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
   TProof* fProof;                 //pointer to proof

   TProofBenchRun::EReadType fReadType;
   TProofBenchMode* fMode;
   TProofBenchRunCleanup* fRunCleanup;

   Long64_t fNEvents;            //number of events per file for CPU test and/or I/O test
   Int_t fNTries;                //number of files for I/O test
   Int_t fMaxNWorkers;           //number of maximum processes, 
                                 //this can be more than the number of total workers in the cluster
   Int_t fStart;                 //number of workers the test starts with
   Int_t fStop;                  //number of workers the test ends with
   Int_t fStep;                  //number of workers to increase for each step
   Int_t fDraw;                  //various plots on the canvas when true
   Int_t fDebug;                 //debug switch, various debug plots will be saved to file when true

   TFile* fFile;                 //output file to write performance histograms and trees on
   TDirectory* fDirProofBench;   //directory for proof outputs
   Bool_t fWritable;             //true when file is writable

   TList* fNodes;                // List of worker nodes info, fNodes is the owner of its members

   TList* fPerfStats;            //List of PROOF_PerfStats
   TProfile* fProfEvent;         //profile histogram (number of events processed per second)
   TProfile* fProfIO;            //profile histogram (data size read per second)

   TList* fListPerfProfiles;       //List of performance profiles

   TCanvas* fCPerfProfiles;      //canvas for performance profile histograms

   TString fName;                

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
                          TString filename="",  //output file where benchmark performance plot will be written to, 
                                                //user has to provide one
                          Option_t* foption="", //option to TFile() 
                          TProof* proof=gProof,
                          Int_t maxnworkers=-1, //maximum number of workers to be tested. 
                                                //If not set (default), 2 times the number of total workers in the cluster available
                          Long64_t nevents=-1,  //process all events
                          Int_t ntries=2,
                          Int_t start=1,
                          Int_t stop=-1,        //stop=maxnowkers when stop=-1
                          Int_t step=1,
                          Int_t draw=0,
                          Int_t debug=0);       //default constructor

   virtual ~TProofBenchRunDataRead();           //Destructor

   void Run(Long64_t nevents,
            Int_t ntries,
            Int_t start,
            Int_t stop,
            Int_t step,
            Int_t debug,
            Int_t draw);

   void DrawPerfProfiles();

   void Print(Option_t* option="")const;   //Print status of an instance of this class 

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
                   Int_t compress = 1);    //open a file for outputs 
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

   ClassDef(TProofBenchRunDataRead,0)         //PROOF benchmark run type data read
};

#endif
