// @(#)root/proofx:$Id:$
// Author: 

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofBench
#define ROOT_TProofBench

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBench                                                          //
//                                                                      //
// TProofBench is a steering class for PROOF benchmark suite.           //
// The primary goal of benchmark suite is to determine the optimal      //
// configuration parameters for a set of machines to be used as PROOF   //
// cluster. The suite measures the performance of the cluster for a set //
// of standard tasks as a function of the number of effective processes.//
// From these results, indications about the optimal number of          //
// concurrent processes could be derived. for large facilities,         //
// the suite should also give indictions about the optimal number of    //
// of sub-masters into which the cluster should be partitioned.         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

class TFileCollection;
class TFile;
class TDSet;
class TDrawFeedback;
class TCanvas;
class TTree;
class TProof;
class TProofServ;

R__EXTERN TProof *gProof;

class TProofBench : public TObject {

public:

   //benchmark run type
   enum ERunType {kRunNotSpecified=-1, 
                  kRunNothing, 
                  kRunCPUTest, 
                  kRunGenerateFileBench,
                  kRunGenerateFileCleanup,
                  kRunCleanup,
                  kRunFullDataRead, 
                  kRunOptDataRead, 
                  kRunNoDataRead}; 

   //histogram types for CPU test
   enum EHistType {kHistNotSpecified=-1, 
                   kHist1D=1, //2^0
                   kHist2D=2, //2^1
                   kHist3D=4, //2^2
                   kHistAll=kHist1D | kHist2D | kHist3D}; 

   TProofBench(TProof* proof=gProof, 
               TString filename="", //output file where benchmark performance plot will be written to, 
                                    //user has to provide one
               TString basedir="",  //base directory where files to be used during the test are written
                                    //default is not to provide and it will be automatically decided
               ERunType whattorun=kRunNotSpecified, //type of run
               Long_t nhists=10, 
               Int_t maxnworkers=-1, //maximum number of workers to be tested. 
                                     //If not set (default), 2 times the number of total workers in the cluster available
               Int_t nnodes=-1, 
               Int_t nfiles=10, 
               Long64_t nevents=10000, 
               //Int_t niteration=10, 
               Int_t stepsize=1, 
               Int_t start=1, 
               Int_t draw=1,
               Int_t debug=0); //default constructor

   virtual ~TProofBench();     //destructor

   void RunBenchmarkAll(TString basedir="", 
                        Long_t nhists=-1, 
                        Int_t maxnworkers=-1, 
                        Int_t nnodes=-1,
                        Int_t nfiles=-1, 
                        Long64_t nevents=-1, 
                        //Int_t niter=-1, 
                        Int_t stepsize=1, 
                        Int_t start=1); //Do all benchmark tests

   void RunBenchmark(ERunType whattorun=kRunNotSpecified, 
                     TString basedir="", 
                     Long_t nhists=-1, 
                     Int_t maxnworkers=-1, 
                     Int_t nnodes=-1, 
                     Int_t nfiles=-1, 
                     Long64_t nevents=-1, 
                     //Int_t niter=-1, 
                     Int_t stepsize=1, 
                     Int_t start=1); //Do a benchmark test

   Int_t GenerateFiles(ERunType runtype,
                       TString basedir="", 
                       Int_t maxnworkers=-1, 
                       Int_t nfiles=-1, 
                       Long64_t nevents=-1 ); //Generate files for the test or cleanup run

   void MakeDataSet(TString basedir="", 
                    //Int_t nworkers=-1, 
                    //Int_t nnodes=-1, 
                    Int_t nfiles=-1, 
                    TString name_stem=""); //make dataset

   TFile* FileOpen(const char* filename="", 
                   Option_t* option="", 
                   Bool_t force=kFALSE, 
                   const char* ftitle = "", 
                   Int_t compress = 1);    //open a file for outputs 

   void Print(Option_t* option="")const;   //print status of an instance of this class 

   //void DrawTimeHists(TTree* t);

   //void DrawTimeHists(TString filename, 
   //                   TString perfstatsname);

   void DrawPerfProfiles(TString filename="", 
                         Int_t draw=1);

   //setters
   void SetRunType(ERunType runtype) {fRunType=runtype;}
   void SetNHists(Long_t nhists) {fNHists=nhists;}
   void SetHistType(EHistType histtype) {fHistType=histtype;}
   void SetNFiles(Int_t nfiles) {fNFiles=nfiles;}
   void SetNEvents(Long64_t nevents) {fNEvents=nevents;}
   void SetBaseDir(TString basedir) {fBaseDir=basedir;}
   void SetMaxNWorkers(Int_t maxnworkers) {fMaxNWorkers=maxnworkers;}
   void SetNNodes(Int_t nnodes) {fNNodes=nnodes;}
   void SetNTracksBench(Int_t ntracks) {fNTracksBench=ntracks;}
   void SetNTracksCleanup(Int_t ntracks) {fNTracksCleanup=ntracks;}
   void SetDraw(Int_t draw);
   void SetDebug(Int_t debug){fDebug=debug;}
   TFile* SetFile(TFile* ftmp);

   //getters
   ERunType GetRunType() const {return fRunType;}
   Long_t GetNHists() const {return fNHists;}
   EHistType GetHistType() const {return fHistType;}
   Int_t GetNFiles() const {return fNFiles;}
   Long64_t GetNEvents() const {return fNEvents;}
   TString GetBaseDir() const {return fBaseDir;}
   Int_t GetMaxNWorkers() const {return fMaxNWorkers;}
   Int_t GetNNodes() const {return fNNodes;}
   Int_t GetNTracksBench() const {return fNTracksBench;}
   Int_t GetNTracksCleanup() const {return fNTracksCleanup;}
   Int_t GetDraw() const {return fDraw;}
   Int_t GetDebug()const{return fDebug;}
   TFile* GetFile(){return fFile;}

   const TDSet* GetDataSet() const {return fDataSet;}

protected:

   TTree* BuildTimingTree(TString pattern, 
                          Int_t& max_slaves);

   Int_t SetParameters(TString &basedir, 
                            Long_t &nhists, 
                            Int_t &maxnworkers, 
                            Int_t& nnodes, 
                            Int_t &nfiles, 
                            Long64_t &nevents, 
                            //Int_t &niter, 
                            Int_t &stepsize, 
                            Int_t& start);

private:

   TProof* fProof;                 //pointer to proof
   TString fBaseDir;               //base directory for files
   ERunType fRunType;              //benchmark run type
   Long_t fNHists;                 //number of histograms for CPU test
   EHistType fHistType;
   Int_t fMaxNWorkers;             //number of maximum processes, 
                                   //this can be more than the number of total workers in the cluster
   Int_t fNNodes;                  //number of nodes to be included in the test 
   Int_t fNFiles;                  //number of files for I/O test
   Long64_t fNEvents;              //number of events per file for CPU test and/or I/O test
   Int_t fStepSize;                //test to be performed every fStepSize cpu cores
   Int_t fStart;                   //starting number of cpu cores

   Int_t fNFilesGenerated;         //number of files successfully generated to be used for the benchmark test
   Long64_t fNEventsGenerated;     //number of events per file successfully generated
   Bool_t fFilesGeneratedCleanup;  //true if clean up files generated
   Int_t fNTracksBench;            //number of tracks in an event for test files
   Int_t fNTracksCleanup;          //number of tracks in an event for cleanup files

   Int_t fDraw;                    //when true draw various plots on the canvas
   
   TDSet* fDataSet;                //Data set to be used for benchmark test
   TDSet* fDataSetGeneratedBench;  //Data set generated at worker nodes
   TDSet* fDataSetGeneratedCleanup;//Data set generated at worker nodes
   TFile* fFile;                   //output file to write performance histograms and trees on
   TCanvas *fCPerfProfiles;        //canvas for performance profile histograms
   TDirectory* fDirProofBench;     //directory for proof outputs
   Int_t fDebug;                   //debug switch, when on various debug plots will be saved to file

   ClassDef(TProofBench,0)         //PROOF benchmark suite steering
};

#endif
