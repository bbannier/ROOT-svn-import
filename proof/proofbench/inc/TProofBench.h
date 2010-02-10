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
#ifndef ROOT_TNamed
#include "TNamed.h"
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
class TProfile;

R__EXTERN TProof *gProof;

class TProofBench : public TObject {

public:

   //benchmark run type
   enum ERunType {kRunNotSpecified=0, 
                  //kRunNothing, 
                  kRunCPUTest=1, 
                  kRunFullDataRead=2, 
                  kRunOptDataRead=4, 
                  kRunNoDataRead=8,
                  kRunGenerateFileBench=0x10,
                  kRunGenerateFileCleanup=0x20,
                  kRunCleanup=0x40,
                  kRunAll=kRunCPUTest | kRunFullDataRead | kRunOptDataRead | kRunNoDataRead}; 

   //histogram types for CPU test
   enum EHistType {kHistNotSpecified=0, 
                   kHist1D=1, 
                   kHist2D=2, 
                   kHist3D=4,
                   kHistAll=kHist1D | kHist2D | kHist3D}; 

   TProofBench(TString fFilename="",//output file where benchmark performance plot will be written to, 
                                    //user has to provide one
               TProof* proof=gProof,
               TString basedir="",  //base directory where files to be used during the test are written
                                    //default is not to provide and it will be automatically decided
               ERunType runtype=kRunNotSpecified, //type of run
               Long_t nhists=10,
               EHistType histtype=kHistAll,
               Int_t maxnworkers=-1,//maximum number of workers to be tested. 
                                    //If not set (default), 2 times the number of total workers in the cluster available
               Int_t nnodes=-1,
               Int_t ntries=10,
               Long64_t nevents=10000,
               Int_t stepsize=-1,
               Int_t start=1,
               Int_t ntracksbench=3,
               Int_t ntrackscleanup=100,
               Int_t draw=0,
               Int_t debug=0); //default constructor

   virtual ~TProofBench();     //destructor

   void RunBenchmarkAll(); //Do all benchmark tests

   void RunBenchmark(ERunType whattorun=kRunNotSpecified); //Do a benchmark test

   Int_t GenerateFiles();//Generate files for the test or cleanup run

   Int_t GenerateFilesN(Int_t nf, Long64_t fileent = 100000);
   Int_t CreateDataSetsN(const char *basedir, Int_t np, Int_t *wp, Int_t nr, Int_t nfw);

   TFile* FileOpen(const char* filename="", 
                   Option_t* option="", 
                   Bool_t force=kFALSE, 
                   const char* ftitle = "", 
                   Int_t compress = 1);    //open a file for outputs 

   void Print(Option_t* option="")const;   //print status of an instance of this class 

   //void DrawTimeHists(TTree* t);

   //void DrawTimeHists(TString filename, 
   //                   TString perfstatsname);

   //void DrawPerfProfiles(TString filename="", 
    //                     Int_t draw=1);

   //setters
   TFile* SetFile(TFile* ftmp);
   void SetBaseDir(TString basedir);
   void SetRunType(TString where, ERunType runtype);
   void SetNHists(Long_t nhists);
   void SetHistType(EHistType histtype) {fHistType=histtype;}
   void SetMaxNWorkers(Int_t maxnworkers);
   void SetMaxNWorkers(TString sworkers);
   void SetNNodes(Int_t nnodes) {fNNodes=nnodes;}
   void SetNTries(Int_t ntries);
   void SetNEvents(Long64_t nevents);
   void SetStepSize(Int_t stepsize);
   void SetStart(Int_t start);
   void SetNTracksBench(Int_t ntracks);
   void SetNTracksCleanup(Int_t ntracks);
   void SetDraw(Int_t draw);
   void SetDebug(Int_t debug);

   //getters
   TFile* GetFile(){return fFile;}
   TString GetBaseDir() const {return fBaseDir;}
   ERunType GetRunType() const {return fRunType;}
   TString GetNameStem()const{return fNameStem;}
   Long_t GetNHists() const {return fNHists;}
   EHistType GetHistType() const {return fHistType;}
   Int_t GetMaxNWorkers() const {return fMaxNWorkers;}
   Int_t GetNNodes() const {return fNNodes;}
   Int_t GetNTries() const {return fNTries;}
   Long64_t GetNEvents() const {return fNEvents;}
   Int_t GetStepSize() const {return fStepSize;}
   Int_t GetStart() const {return fStart;}
   Int_t GetNTracksBench() const {return fNTracksBench;}
   Int_t GetNTracksCleanup() const {return fNTracksCleanup;}
   Int_t GetDraw() const {return fDraw;}
   Int_t GetDebug()const{return fDebug;}

   Long64_t GetNEventsGenerated()const{return fNEventsGenerated;}
   Int_t GetNFilesGeneratedBench()const{return fNFilesGeneratedBench;}
   Bool_t GetFilesGeneratedCleanup()const{return fFilesGeneratedCleanup;}
   const TDSet* GetDataSet() const {return fDataSet;}
   const TDSet* GetDataSetGeneratedBench() const {return fDataSetGeneratedBench;}
   const TDSet* GetDataSetGeneratedCleanup() const {return fDataSetGeneratedCleanup;}
   TCanvas* GetCPerfProfiles() const {return fCPerfProfiles;}
   TDirectory* GetDirProofBench() const {return fDirProofBench;}

   TTree* BuildPerfProfiles(ERunType runtype=kRunNotSpecified);
protected:


   /* Int_t SetParameters(TString &basedir, 
                            Long_t &nhists, 
                            Int_t &maxnworkers, 
                            Int_t& nnodes, 
                            Int_t &ntries, 
                            Long64_t &nevents, 
                            Int_t &stepsize, 
                            Int_t& start);
*/
   void ClearInputParameters(); //clear input parameters to selector
   void SetInputParameters(); //set input parameters to selector
   Int_t CheckParameters(TString where);
   Int_t FillNodeInfo();
   void MakeDataSet(//Int_t nworkers=-1, 
                    //Int_t nnodes=-1, 
                    Int_t ntries=-1); //make dataset


private:
   //TProofBench(const TProofBench&){} //not implemented

   TFile* fFile;                   //output file to write performance histograms and trees on
   TProof* fProof;                 //pointer to proof
   TString fBaseDir;               //base directory for files
   ERunType fRunType;              //benchmark run type
   TString fNameStem;              //name stem for the run type
   Long_t fNHists;                 //number of histograms for CPU test
   EHistType fHistType;
   Int_t fMaxNWorkers;             //number of maximum processes, 
                                   //this can be more than the number of total workers in the cluster
   Int_t fNNodes;                  //number of nodes to be included in the test 
   Int_t fNTries;                  //number of files for I/O test
   Long64_t fNEvents;              //number of events per file for CPU test and/or I/O test
   Int_t fStepSize;                //test to be performed every fStepSize cpu cores
   Int_t fStart;                   //starting number of cpu cores
   Int_t fNTracksBench;            //number of tracks in an event for test files
   Int_t fNTracksCleanup;          //number of tracks in an event for cleanup files
   Int_t fDraw;                    //when true draw various plots on the canvas
   Int_t fDebug;                   //debug switch, when on various debug plots will be saved to file

   Long64_t fNEventsGenerated;     //number of events per file successfully generated
   Int_t fNFilesGeneratedBench;         //number of files successfully generated to be used for the benchmark test
   Bool_t fFilesGeneratedCleanup;  //true if clean up files generated
   TDSet* fDataSet;                //Data set to be used for benchmark test
   TDSet* fDataSetGeneratedBench;  //Data set generated at worker nodes
   TDSet* fDataSetGeneratedCleanup;//Data set generated at worker nodes
   TCanvas *fCPerfProfiles;        //canvas for performance profile histograms
   TDirectory* fDirProofBench;     //directory for proof outputs

   TProfile* fProfCPUTestEvent;
   TProfile* fProfOptDataReadEvent;
   TProfile* fProfOptDataReadIO;
   TProfile* fProfFullDataReadEvent;
   TProfile* fProfFullDataReadIO;

   TList    *fNodes;               // List of worker nodes info
   //TList* fListRunType;       //map of <ERunType, namestem>

   ClassDef(TProofBench,0)         //PROOF benchmark suite steering
};

class TProofNode : public TNamed
{
private:
   Int_t   fPhysRam;   // Physical RAM of this worker node
   Int_t   fNWrks;     // Number of workers on this node
public:
   TProofNode(const char *wn, Int_t ram) : TNamed(wn,""), fPhysRam(ram), fNWrks(1) { }
   virtual ~TProofNode() { }

   void AddWrks(Int_t n = 1) { fNWrks += n; }
   Int_t GetPhysRam() const { return fPhysRam; }
   Int_t GetNWrks() const { return fNWrks; }
   void SetNWrks(Int_t n) { fNWrks = n; }
 };
#endif
