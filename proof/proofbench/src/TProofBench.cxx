// @(#)root/proofx:$Id$
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
// TProofBench                                                          //
//                                                                      //
// TProofBench is ...                                                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofBench.h"
#include "TFile.h"
#include "TFileCollection.h"
#include "TFileInfo.h"
#include "TProof.h"
#include "TString.h"
#include "TDSet.h"
#include "Riostream.h"
#include "THashList.h"
#include "TMap.h"
#include "TEnv.h"
#include "TTree.h"
#include "TH1.h"
#include "TLeaf.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPaveText.h"
#include "TProfile.h"
#include "TLegend.h"
#include "TKey.h"
#include "TRegexp.h"
#include "TPerfStats.h"
#include "TParameter.h"
#include "TSelectorList.h"
#include "TDrawFeedback.h"

#include <stdlib.h>

ClassImp(TProofBench)

TProofBench::TProofBench(TProof* proof, 
                         TString fFilename, 
                         TString basedir, 
                         ERunType whattorun, 
                         Long_t nhists, 
                         Int_t maxnworkers, 
                         Int_t nnodes, 
                         Int_t nfiles, 
                         Long64_t nevents, 
                         //Int_t niter, 
                         Int_t stepsize, 
                         Int_t start, 
                         Int_t draw,
                         Int_t debug):
fProof(proof), 
fBaseDir(basedir), 
fRunType(whattorun), 
fNHists(nhists),
fHistType(kHistAll),
fMaxNWorkers(maxnworkers), 
fNNodes(nnodes), 
fNFiles(nfiles),
fNEvents(nevents), 
//fNIterations(niter), 
fStepSize(stepsize),
fStart(start), 
fNFilesGenerated(0), 
fNEventsGenerated(0),
fFilesGeneratedCleanup(kFALSE),
fNTracksBench(3),
fNTracksCleanup(100),
fDraw(draw), 
fDataSet(0), 
fDataSetGeneratedBench(0), 
fDataSetGeneratedCleanup(0), 
fFile(0), 
fCPerfProfiles(0), 
fDirProofBench(0),
fDebug(debug)
{

//Default constructor
   
   FileOpen(fFilename, "new");
   SetDraw(draw);
   SetParameters(basedir, nhists, maxnworkers, nnodes, nfiles, nevents, stepsize, start);
   // switch logging on
   // Probably changing environment variable is not a good idea. This should be fixed
   gEnv->SetValue("Proof.StatsHist",0);
   gEnv->SetValue("Proof.StatsTrace",1);
   gEnv->SetValue("Proof.SlaveStatsTrace",0);
}

void TProofBench::SetDraw(Int_t draw)
{

   fDraw=draw;

   if (!fProof){
      Error("SetDraw", "Proof not set");
      return;
   }
}

TProofBench::~TProofBench()
{
//destructor

   if (fDataSet){ delete fDataSet;}
   if (fDataSetGeneratedBench){ delete fDataSetGeneratedBench;}
   if (fDataSetGeneratedCleanup){ delete fDataSetGeneratedCleanup;}
   if (fFile){ 
      TString soption=fFile->GetOption(); 
      soption.ToLower(); 
      if (soption.Contains("create") || soption.Contains("update")){
         fFile->Write(0, TFile::kWriteDelete);
      }
      fFile->Close(); 
      delete fFile;
   }
   fProof=0;
} 

void TProofBench::RunBenchmarkAll(TString basedir, 
                                  Long_t nhists, 
                                  Int_t maxnworkers, 
                                  Int_t nnodes, 
                                  Int_t nfiles, 
                                  Long64_t nevents, 
                                  //Int_t niter, 
                                  Int_t stepsize, 
                                  Int_t start)
{

   if (!fProof){
      Error("RunBenchmarkAll", "Proof not set");
      return;
   }

   /*RunBenchmark(kRunCPUTest, basedir, nhists, maxnworkers, nnodes, nfiles, nevents, niter, stepsize, start);
   RunBenchmark(kRunFullDataRead, basedir, nhists, maxnworkers, nnodes, nfiles, nevents, niter, stepsize, start);
   RunBenchmark(kRunOptDataRead, basedir, nhists, maxnworkers, nnodes, nfiles, nevents, niter, stepsize, start);
   */
   RunBenchmark(kRunCPUTest, basedir, nhists, maxnworkers, nnodes, nfiles, nevents, stepsize, start);
   RunBenchmark(kRunFullDataRead, basedir, nhists, maxnworkers, nnodes, nfiles, nevents, stepsize, start);
   RunBenchmark(kRunOptDataRead, basedir, nhists, maxnworkers, nnodes, nfiles, nevents, stepsize, start);

   DrawPerfProfiles("", fDraw);
}

void TProofBench::RunBenchmark(ERunType whattorun, 
                               TString basedir, 
                               Long_t nhists, 
                               Int_t maxnworkers, 
                               Int_t nnodes, 
                               Int_t nfiles, 
                               Long64_t nevents, 
                               //Int_t niter, 
                               Int_t stepsize, 
                               Int_t start)
{
   if (!fProof){
      Error("RunBenchmark", "Proof not set");
      return;
   }

   //SetParameters(basedir, nhists, maxnworkers, nnodes, nfiles, nevents, niter, stepsize, start);
   SetParameters(basedir, nhists, maxnworkers, nnodes, nfiles, nevents, stepsize, start);

   TString perfstats_name = "PROOF_PerfStats";
   TString name_stem;

   switch (whattorun){
   case kRunNotSpecified:
      Info("RunBenchmark", "Run not specified, returning");
      return;
      break;
   case kRunNothing:
      Info("RunBenchmark", "kRunNothing requested; doing nothing and returning");
      return;
      break;
   case kRunCPUTest:
      name_stem="CPUTest";
      break;
   case kRunGenerateFileBench:
      name_stem="GenerateFileBench";
      break;
   case kRunGenerateFileCleanup:
      name_stem="GenerateFileCleanup";
      break;
   case kRunCleanup:
      name_stem="Cleanup";
      break;
   case kRunFullDataRead:
      name_stem="FullDataRead";
      break;
   case kRunOptDataRead:
      name_stem="OptDataRead";
      break;
   case kRunNoDataRead:
      name_stem="NoDataRead";
      break;
   default:
      Error("RunBenchmark", "Invalid run type: use one of (TProofBench::kRunNothing,"
                           "TProofBench::kRunCPUTest, TProofBench::kRunGenerateFileBench,"
                           "TProofBench::kRunGenerateFileCleanup, TProofBench::kRunCleanup,"
                           "TProofBench::kRunFullDataRead, TProofBench::kRunOptDataRead,"
                           " or TProofBench::kRunNoDataRead)");
      return;
      break;
   }

   switch (whattorun){
   case kRunNothing:
      Info("RunBenchmark", "kRunNothing requested; doing nothing and returning");
      return;
      break;
   case kRunCPUTest:
      fRunType=kRunCPUTest;

      for (Int_t nactive=fStart; nactive<=fMaxNWorkers; nactive+=fStepSize) {
         fProof->SetParallel(nactive);
         for (Int_t j=0; j<fNFiles; j++) {

            fProof->SetParameter("fRunType", fRunType);
            fProof->SetParameter("fNHists", fNHists);
            fProof->SetParameter("fHistType", fHistType);
            fProof->SetParameter("fNEvents", fNEvents);
            fProof->SetParameter("fDraw", Int_t(fDraw));
         
            TTime starttime = gSystem->Now();
            fProof->Process("TSelHist", fNEvents);
            TTime endtime = gSystem->Now();

            //TList* l = fDataSet->GetOutputList();
            TList* l = fProof->GetOutputList();

            //save perfstats
            TTree* t = dynamic_cast<TTree*>(l->FindObject(perfstats_name.Data()));
            if (t) {
               //TDirectory* trdir = t->GetDirectory();
               //TDirectory* dirsav = gDirectory;

               t->SetDirectory(fDirProofBench);

               TString origname = t->GetName();

               //build up new name
               TString newname = perfstats_name;
               newname+="_";
               newname+=name_stem;
               newname+="_";
               newname+=nactive;
               newname+="slaves_run";
               newname+=j;
               t->SetName(newname);

               if (fDebug){
                  if (fFile && !fFile->IsZombie()){
                     fDirProofBench->cd();
                     t->Write();
                     //t->SetName(origname);
                     //t->SetDirectory(trdir);
                     //t->Delete("");
                  }
               }
                  //dirsav->cd();
            } else {
               Error("RunBenchmark", "tree %s not found", perfstats_name.Data());
            }
         }//for iterations
      }//for number of workers

      break;
   case kRunGenerateFileBench:
      fRunType=whattorun;
      GenerateFiles(fRunType, fBaseDir,fMaxNWorkers, fNFiles, fNEvents);
      break;
   case kRunGenerateFileCleanup:
      fRunType=whattorun;
      GenerateFiles(fRunType, fBaseDir,fMaxNWorkers, fNFiles, fNEvents);
      break;
   case kRunCleanup:
      {
         Int_t nactive_sav=fProof->GetParallel();
         fProof->SetParallel(fMaxNWorkers);

         fProof->SetParameter("fRunType", fRunType);
         fProof->SetParameter("fNFiles", fNFiles);
         fProof->SetParameter("fNEvents", fNEvents);
         //fProof->SetParameter("fBaseDir", fBaseDir.Data());
         fProof->SetParameter("fDraw", Int_t(fDraw));

         fDataSetGeneratedCleanup->Print("a");

         TTime starttime = gSystem->Now();

         fProof->Process(fDataSetGeneratedCleanup, "TSelEvent", "", -1);

         if (fDebug){

            TList* l = fDataSetGeneratedCleanup->GetOutputList();

            //Info("RunBenchmark", "Printing output list");
            //l->Print("a");
            //Info("RunBenchmark", "Printing output list done");

            //save perfstats
            TTree* t = dynamic_cast<TTree*>(l->FindObject(perfstats_name.Data()));

            static int ncalls=0;
            if (t) {
               //TDirectory* trdir = t->GetDirectory();
               //TDirectory* dirsav = gDirectory;
               //fFile->cd();
               t->SetDirectory(fDirProofBench);
               //TString origname = t->GetName();

               //build up new name
               TString newname = perfstats_name;
               newname+="_";
               newname+=name_stem;
               newname+=Form("%d", ncalls);
               t->SetName(newname);

               if (fFile && !fFile->IsZombie()) {
                  fDirProofBench->cd();
                  t->Write();
                  //tnew->SetName(origname);
                  //tnew->SetDirectory(trdir);
                  //t->Delete("");
               }
            } else {
               Error("RunBenchmark", "tree %s not found",  perfstats_name.Data());
            }

            //save outputhistos 
            TString ptdist_name = "pt_dist";
            TH1* h = dynamic_cast<TH1*>(l->FindObject(ptdist_name.Data()));
            if (h) {
               //TDirectory* hdir = h->GetDirectory();
               //TDirectory* dirsav = gDirectory;
               //fFile->cd();
               TH1 *hnew = (TH1*)h->Clone("hnew");

               hnew->SetDirectory(fDirProofBench);
               TString origname = h->GetName();
               TString newname = ptdist_name;
               newname+="_";
               newname+=name_stem;
               newname+=Form("%d", ncalls);
               hnew->SetName(newname);

               if (fFile && !fFile->IsZombie()) {
                  fDirProofBench->cd();
                  hnew->Write();
                  delete hnew;
               }
            } else {
               Error("RunBenchmark", "histogram %s not found",  ptdist_name.Data());
            }

            TString tracksdist_name = "ntracks_dist";
            TH1* h2 = dynamic_cast<TH1*>(l->FindObject(tracksdist_name.Data()));
            if (h2) {
               //TDirectory* hdir = h2->GetDirectory();
               //TDirectory* dirsav = gDirectory;
               //fFile->cd();
               TH1 *hnew = (TH1*)h2->Clone("hnew");
               hnew->SetDirectory(fDirProofBench);
               //TString origname = h2->GetName();
               TString newname = tracksdist_name;
               newname+="_";
               newname+=name_stem;
               newname+=Form("%d", ncalls);
               hnew->SetName(newname);
               if (fFile && !fFile->IsZombie()) {
                  fDirProofBench->cd();
                  hnew->Write();
                  delete hnew;
               }
            } 
            else {
               Error("RunBenchmark", "histogram %s not found",  tracksdist_name.Data());
            }
            ncalls++;
            fProof->SetParallel(nactive_sav);
         }
      } 
      break;
   case kRunFullDataRead:
   case kRunOptDataRead:
   case kRunNoDataRead:

      fRunType=whattorun;

      GenerateFiles(kRunGenerateFileCleanup, fBaseDir, fMaxNWorkers, fNFiles, fNEvents);
      GenerateFiles(kRunGenerateFileBench, fBaseDir, fMaxNWorkers, fNFiles, fNEvents);

      for (Int_t nactive=fStart; nactive<=fMaxNWorkers; nactive+=fStepSize) {
         for (Int_t j=0; j<fNFiles; j++) {

            //cleanup run
            fProof->SetParallel(fMaxNWorkers);

            RunBenchmark(kRunCleanup);

            fProof->SetParallel(nactive);

            fProof->SetParameter("fRunType", fRunType);
            fProof->SetParameter("fNFiles", fNFiles);
            fProof->SetParameter("fNEvents", fNEvents);
            //fProof->SetParameter("fBaseDir", fBaseDir.Data());
            fProof->SetParameter("fDraw", Int_t(fDraw));

            //MakeDataSet(fBaseDir, nactive, fNNodes, j, name_stem);
            //MakeDataSet(fBaseDir, nactive, j, name_stem);
            MakeDataSet(fBaseDir, j, name_stem);

            fDataSet->Print("a");

            TTime starttime = gSystem->Now();

            fProof->Process(fDataSet, "TSelEvent", "", -1);

            TTime endtime = gSystem->Now();

            TList* l = fDataSet->GetOutputList();

            //Info("RunBenchmark", "Printing output list");
            //l->Print("a");
            //Info("RunBenchmark", "Printing output list done");

            //save perfstats
            TTree* t = dynamic_cast<TTree*>(l->FindObject(perfstats_name.Data()));
            if (t) {
               //TDirectory* trdir = t->GetDirectory();
               //TDirectory* dirsav = gDirectory;
               //fFile->cd();
               t->SetDirectory(fDirProofBench);
               //TString origname = t->GetName();

               //build up new name
               TString newname = perfstats_name;
               newname+="_";
               newname+=name_stem;
               newname+="_";
               newname+=nactive;
               newname+="slaves_run";
               newname+=j;
               t->SetName(newname);
               if (fDebug){
                  if (fFile && !fFile->IsZombie()) {
                     fDirProofBench->cd();
                     t->Write();
                     //tnew->SetName(origname);
                     //tnew->SetDirectory(trdir);
                     //t->Delete("");
                  }
               }
               //dirsav->cd();
            } else {
               cout << perfstats_name.Data() << " tree not found" << endl << flush;
            }

            //save outputhistos 
            if (fDebug){
               TString ptdist_name = "pt_dist";
               TH1* h = dynamic_cast<TH1*>(l->FindObject(ptdist_name.Data()));
               if (h) {
                  //TDirectory* hdir = h->GetDirectory();
                  //TDirectory* dirsav = gDirectory;
                  //fFile->cd();
                  TH1 *hnew = (TH1*)h->Clone("hnew");
   
                  hnew->SetDirectory(fDirProofBench);
                  TString origname = h->GetName();
                  TString newname = ptdist_name;
                  newname+="_";
                  newname+=name_stem;
                  newname+="_";
                  newname+=nactive;
                  newname+="slaves_run";
                  newname+=j;
                  hnew->SetName(newname);
                  if (fFile && !fFile->IsZombie()) {
                     fDirProofBench->cd();
                     hnew->Write();
                     //hnew->SetName(origname);
                     //hnew->SetDirectory(hdir);
                     delete hnew;
                  }
               
                  //dirsav->cd();
               } else {
                  Error("RunBenchmark", "histogram %s not found", ptdist_name.Data());
               }
   
               TString tracksdist_name = "ntracks_dist";
               TH1* h2 = dynamic_cast<TH1*>(l->FindObject(tracksdist_name.Data()));
               if (h2) {
                  //TDirectory* hdir = h2->GetDirectory();
                  //TDirectory* dirsav = gDirectory;
                  //fFile->cd();
                  TH1 *hnew = (TH1*)h2->Clone("hnew");
                  hnew->SetDirectory(fDirProofBench);
                  //TString origname = h2->GetName();
                  TString newname = tracksdist_name;
                  newname+="_";
                  newname+=name_stem;
                  newname+="_";
                  newname+=nactive;
                  newname+="slaves_run";
                  newname+=j;
                  hnew->SetName(newname);
                  if (fFile && !fFile->IsZombie()) {
                    fDirProofBench->cd();
                    hnew->Write();
                    //h2->SetName(origname);
                    //h2->SetDirectory(hdir);
                    delete hnew;
                  }
                  //dirsav->cd();
               } 
               else {
                  Error("RunBenchmark", "histogram %s not found", tracksdist_name.Data());
               }
            }
         }//for nfiles
      }//for number of workers
      break;
   default:
      Error("RunBenchmark", "Invalid run type: use one of kRunFullDataRead, kRunOptDataRead or kRunNoDataRead");
      return;
      break;
   }
}

Int_t TProofBench::GenerateFiles(ERunType runtype,
                                 TString basedir, 
                                 Int_t maxnworkers, 
                                 Int_t nfiles, 
                                 Long64_t nevents)
{

//Generate files on worker nodes for I/O test or for cleanup run
//basedir: base directory for the files to be generated on the worker node. 
//Use "" (empty string) not to change the current base directory
//nfiles: number of files per node to be generated. Use negative number not to change the current number
//nevents: number of events to generate per file. Use negative number not to change the current number
//Returns number of successfully generated files

   if (!fProof){
      Error("GenerateFiles", "Proof not set");
      return 0;
   }

   Long_t nhists=-1;
   //Int_t niter=-1;
   Int_t nnodes=-1;
   Int_t stepsize=-1;
   Int_t start=-1;

   if (runtype==kRunGenerateFileBench){
      //if (!SetParameters(basedir, nhists, maxnworkers, nnodes, nfiles, nevents, niter, stepsize, start)){
      if (!SetParameters(basedir, nhists, maxnworkers, nnodes, nfiles, nevents, stepsize, start)){
         Info("GenerateFiles", "Files already generated, returning");
         return 0;
      }

      if (fDataSetGeneratedBench) delete fDataSetGeneratedBench;
      fDataSetGeneratedBench = new TDSet("TTree","EventTree");
   }
   else if (runtype==kRunGenerateFileCleanup){
      if (fFilesGeneratedCleanup){
         return 0;  
      }
      if (fDataSetGeneratedCleanup) delete fDataSetGeneratedCleanup;
      fDataSetGeneratedCleanup = new TDSet("TTree","EventTree");
   }
   
   Int_t nactive_sav;

   //Generate files on all nodes, active or inactive
   nactive_sav=fProof->GetParallel();
   fProof->SetParallel(99999);

   if (runtype==kRunGenerateFileBench){
      fProof->SetParameter("fRunType", kRunGenerateFileBench);
   }
   else if (runtype==kRunGenerateFileCleanup){
      fProof->SetParameter("fRunType", kRunGenerateFileCleanup);
   }

   fProof->SetParameter("fBaseDir", fBaseDir.Data());
   fProof->SetParameter("fMaxNWorkers", fMaxNWorkers);
   fProof->SetParameter("fNFiles", fNFiles);
   fProof->SetParameter("fNEvents", fNEvents);
   fProof->SetParameter("fNTracksBench", fNTracksBench);
   fProof->SetParameter("fNTracksCleanup", fNTracksCleanup);

   TList* wl=fProof->GetListOfSlaveInfos();
   wl->SetName("listofslaveinfos");
   fProof->AddInputData(wl, kTRUE);

   if (runtype==kRunGenerateFileBench){
      Info("GenerateFiles", "Generating %d * %d file(s) with %lld event(s)/file at %s on the cluster. "
           "It may take a while...", 
        fMaxNWorkers, fNFiles, fNEvents, fBaseDir.Data());
   }
   else if (runtype==kRunGenerateFileCleanup){
      Info("GenerateFiles", "Generating file(s) at each node for cleaning up memory. "
           "It may take a while...");
   }

   fProof->Process("TSelEventGen", Long64_t(0));


   TList* l = fProof->GetOutputList();

   if (!l){
      Error("GenerateFiles", "list of output not found");
      return 0;
   } 


   Info("GenerateFiles", "Printing out list of outputs");
   l->Print("a");
   Info("GenerateFiles", "Printing out list of outputs; done");

   //build up dataset out of outputs of workers
   TObject* obj;
   TList* lfilesgenerated;
   TList* ltdelement;
   TDSet* tdset;
   TString outputname, hostname, filename, newfilename;
   TDSetElement* tdelement;
 
   TIter nxt(l);

   while((obj=nxt())){
      outputname=obj->GetName(); 
      if (outputname.Contains("listoffilesgenerated")){
         lfilesgenerated=dynamic_cast<TList*>(obj);
         if (lfilesgenerated){
            TObjArray* token=outputname.Tokenize("_"); //filename=listoffilesgenerated_hostname_ordinal
            hostname=((*token)[1])->GetName();

            tdset=dynamic_cast<TDSet*>(lfilesgenerated->At(0)); //lfilesgenerated is 1-element list
            ltdelement=tdset->GetListOfElements();
            TIter nxtelement(ltdelement); 
            while((tdelement=(TDSetElement*)nxtelement())){
               filename=tdelement->GetName();
               //printf("filename=%s\n", filename.Data());
               newfilename="root://"+hostname+"/"+filename;
               tdelement->SetName(newfilename.Data());
            }
            //tdset->Print("a");
            if (runtype==kRunGenerateFileBench){
               fDataSetGeneratedBench->Add(tdset);  
            }
            else if (runtype==kRunGenerateFileCleanup){
               fDataSetGeneratedCleanup->Add(tdset);  
            }
         }
         else{
            Error("GenerateFiles", "%s not type TList*", outputname.Data());
            return 0; 
         }
      }
   }

   if (runtype==kRunGenerateFileBench){
      //fDataSetGenerated->Validate();
      fDataSetGeneratedBench->Print("a"); 
      fNFilesGenerated=fNFiles;
      fNEventsGenerated=fNEvents;
   }
   else if (runtype==kRunGenerateFileCleanup){
      fDataSetGeneratedCleanup->Print("a"); 
      fFilesGeneratedCleanup=kTRUE;
   }

   //Put it back to old configuration
   fProof->SetParallel(nactive_sav);

   return fNFilesGenerated;
}

void TProofBench::MakeDataSet(TString basedir, 
                              //Int_t nworkers, 
                              //Int_t nnodes, 
                              Int_t nfile, 
                              TString name_stem)
{
// Create a TDSet object that will be used to process
// the files generated with GenerateFiles function.
// basedir: location of files local to proof slaves
// nfile: file number

   if (!fProof){
      Error("MakeDataSet", "Proof not set");
      return;
   }

   basedir.Remove(TString::kBoth, ' '); //remove leading and trailing white space(s)
   basedir.Remove(TString::kBoth, '\t');//remove leading and trailing tab character(s)
   basedir.Remove(TString::kTrailing, '/'); //remove trailing /

   if (basedir.Length()>0) {
     fBaseDir=basedir; 
   }

   if (fDataSet){
      delete fDataSet;
      fDataSet=0;
   }

   fDataSet = new TDSet("TTree","EventTree");

   TList* l = fProof->GetListOfSlaveInfos();
   if (!l) {
      cout << "No list of workers received!" << endl;
      return;
   }

   TIter nxw(l);
   //TSlaveInfo *si = 0;

   //nodelist.SetOwner(kFALSE);
   TList nodelist;
   TString s, nodename, ordinal, ordinalchopped;
   //TObjString* nobj;
   //Ssiz_t from, to;
   //Int_t iordinalchopped;
   //Int_t ntotalworkers=l->GetSize();

   //Int_t nodenumber=0;
   Int_t nfilesadded=0;

   TList* lelement=fDataSetGeneratedBench->GetListOfElements();
   TIter nxtelement(lelement);
   TDSetElement *tdelement;
   TFileInfo* fileinfo; 
   TUrl* url; 
   TString hostname, filename, tmpstring;
   Int_t nworkers_file;

   while ((tdelement=(TDSetElement*)nxtelement())){
      fileinfo=tdelement->GetFileInfo();
      url=fileinfo->GetCurrentUrl();
      hostname=url->GetHost();
      filename=url->GetFile();

      //printf("filename=%s\n", filename.Data());

      if (!filename.Contains(name_stem)) continue;

      //remove upto name_stem and leading "_"
      tmpstring=filename;

      tmpstring.Remove(0, filename.Index(name_stem)+name_stem.Length()+1);
      //printf("tmpstring=%s\n", tmpstring.Data());

      TObjArray* token=tmpstring.Tokenize("_"); //filename=...namestem_nworkers_nfiles.root

      //printf("token[%d]=%s\n", 0, (*token)[0]->GetName()); //should be nworkers
      //printf("token[%d]=%s\n", 1, (*token)[1]->GetName()); //should be nfiles.root
      
      nworkers_file=TString((*token)[0]->GetName()).Atoi();
      //printf("nworkers_file=%d\n", nworkers_file);

      //if (nworkers_file!=nworkers){
      //   continue;
      //}

      token=TString((*token)[1]->GetName()).Tokenize(".");
      Int_t nfile_file=TString((*token)[0]->GetName()).Atoi();
      //printf("nfile_tmp=%d\n", nfile_tmp);

      if (nfile_file!=nfile){
         continue;
      }

      //found
      fDataSet->Add(fileinfo);
      nfilesadded++;
   }
   fDataSet->Lookup();
   fDataSet->Validate();
   return;
}

void TProofBench::DrawPerfProfiles(TString filename, Int_t draw) {
// Plots total processing time as a function of number of slaves
// using each of the 3 selectors.
// filename: input file name with performance statistics

   fDraw=draw>=0?draw:fDraw;

   TDirectory* dirsav=gDirectory;

   fDirProofBench->cd();

   //TFile* fFilelocal;

   filename.Remove(TString::kBoth, ' '); //remove leading and trailing white space(s)
   filename.Remove(TString::kBoth, '\t');//remove leading and trailing tab character(s)

   //FileOpen(filename, "READ");
   //fFilelocal->cd();

   TString perfstats_name = "PROOF_PerfStats";

   Int_t ns_holder;
   Int_t run_holder;
   Int_t nevents_holder;
   Int_t bytes_holder;
   Float_t time_holder;

   Int_t procmax_slaves = 0;

#if 0
   TTree* tt_cpu    = BuildTimingTree(perfstats_name+"_CPUTest",
                                      procmax_slaves);
#else
   TTree* tt_cpu    = BuildTimingTree(TString("PROOF_PerfStats_CPUTest"),
                                      procmax_slaves);
#endif
   //tt_proc->SetMarkerStyle(4);
   //set branch addresses
   tt_cpu->GetBranch("perfproctime")->GetLeaf("nslaves")->SetAddress(&ns_holder);
   tt_cpu->GetBranch("perfproctime")->GetLeaf("run")->SetAddress(&run_holder);
   tt_cpu->GetBranch("perfproctime")->GetLeaf("nevents")->SetAddress(&nevents_holder);
   tt_cpu->GetBranch("perfproctime")->GetLeaf("bytes")->SetAddress(&bytes_holder);
   tt_cpu->GetBranch("perfproctime")->GetLeaf("time")->SetAddress(&time_holder);

   tt_cpu->Print("a");

   TTree* tt_proc    = BuildTimingTree(perfstats_name+"_FullDataRead",
                                       procmax_slaves);
   //tt_proc->SetMarkerStyle(4);
   //set branch addresses
   tt_proc->GetBranch("perfproctime")->GetLeaf("nslaves")->SetAddress(&ns_holder);
   tt_proc->GetBranch("perfproctime")->GetLeaf("run")->SetAddress(&run_holder);
   tt_proc->GetBranch("perfproctime")->GetLeaf("nevents")->SetAddress(&nevents_holder);
   tt_proc->GetBranch("perfproctime")->GetLeaf("bytes")->SetAddress(&bytes_holder);
   tt_proc->GetBranch("perfproctime")->GetLeaf("time")->SetAddress(&time_holder);

   tt_proc->Print("a");

   Int_t procoptmax_slaves = 0;
   TTree* tt_procopt = BuildTimingTree(perfstats_name+"_OptDataRead",
                                       procoptmax_slaves);
   //tt_procopt->SetMarkerStyle(5);
   //set branch addresses
   tt_procopt->GetBranch("perfproctime")->GetLeaf("nslaves")->SetAddress(&ns_holder);
   tt_procopt->GetBranch("perfproctime")->GetLeaf("run")->SetAddress(&run_holder);
   tt_procopt->GetBranch("perfproctime")->GetLeaf("nevents")->SetAddress(&nevents_holder);
   tt_procopt->GetBranch("perfproctime")->GetLeaf("bytes")->SetAddress(&bytes_holder);
   tt_procopt->GetBranch("perfproctime")->GetLeaf("time")->SetAddress(&time_holder);

   tt_procopt->Print("a");

   Int_t nslaves = procmax_slaves>procoptmax_slaves?procmax_slaves:procoptmax_slaves;
   //if (nslaves<noprocmax_slaves) nslaves=noprocmax_slaves;
   
   TProfile* cpuprof_event = new TProfile("cpuprof_event", "CPU Test Event Rate", nslaves, 0.5, nslaves+0.5);

   Double_t event_rate, IO_rate;
   Int_t entries=tt_cpu->GetEntries();
   for (int i=0; i<entries; i++){
      tt_cpu->GetEntry(i); 
      //printf("ns_holder=%d, run_holder=%d time_holder=%f\n", ns_holder, run_holder, time_holder);
      event_rate=nevents_holder/time_holder; 
      cpuprof_event->Fill(Double_t(ns_holder), event_rate);
   }
   
   cpuprof_event->GetXaxis()->SetTitle("Number of Slaves");
   cpuprof_event->GetYaxis()->SetTitle("Events/sec");

   cpuprof_event->SetMarkerStyle(20);

   TProfile* procprof_event = new TProfile("procprof_event", "Event Rate", nslaves, 0.5, nslaves+0.5);
   TProfile* procprof_IO = new TProfile("procprof_IO", "I/O Rate", nslaves, 0.5, nslaves+0.5);

   entries=tt_proc->GetEntries();
   for (int i=0; i<entries; i++){
      tt_proc->GetEntry(i); 
      //printf("ns_holder=%d, run_holder=%d time_holder=%f\n", ns_holder, run_holder, time_holder);
      event_rate=nevents_holder/time_holder; 
      IO_rate=bytes_holder/time_holder/(1024.*1024.); 
      procprof_event->Fill(Double_t(ns_holder), event_rate);
      procprof_IO->Fill(Double_t(ns_holder), IO_rate);
   }
   
   Double_t ymax=procprof_event->GetMaximum();
   procprof_event->GetYaxis()->SetRangeUser(0, ymax*1.2);

   procprof_event->GetXaxis()->SetTitle("Number of Slaves");
   procprof_event->GetYaxis()->SetTitle("Events/sec");
   procprof_IO->GetXaxis()->SetTitle("Number of Slaves");
   procprof_IO->GetYaxis()->SetTitle("MB/sec");

   procprof_event->SetMarkerStyle(21);
   procprof_IO->SetMarkerStyle(22);

   TProfile* procoptprof_event = new TProfile("procoptprof_event", "Event Rate", nslaves, 0.5, nslaves+0.5);
   TProfile* procoptprof_IO = new TProfile("procoptprof_IO", "I/O Rate", nslaves, 0.5, nslaves+0.5);

   procoptprof_event->GetXaxis()->SetTitle("Number of Slaves");
   procoptprof_event->GetYaxis()->SetTitle("Events/sec");
   procoptprof_IO->GetXaxis()->SetTitle("Number of Slaves");
   procoptprof_IO->GetYaxis()->SetTitle("MB/sec");

   procoptprof_event->SetMarkerStyle(23);
   procoptprof_IO->SetMarkerStyle(24);

   entries=tt_procopt->GetEntries();
   for (int i=0; i<entries; i++){
      tt_procopt->GetEntry(i); 
      event_rate=nevents_holder/time_holder; 
      IO_rate=bytes_holder/time_holder/(1024.*1024.); 
      procoptprof_event->Fill(Double_t(ns_holder), event_rate);
      procoptprof_IO->Fill(Double_t(ns_holder), IO_rate);
   }

   if (fFile && !fFile->IsZombie() && !TString(fFile->GetOption()).Contains("READ")){
       fDirProofBench->cd();
       cpuprof_event->Write();
       procprof_event->Write();
       procprof_IO->Write();
       procoptprof_event->Write();
       procoptprof_IO->Write();
       if (gROOT->IsBatch() || !fDraw){
          delete cpuprof_event;
          delete procprof_event;
          delete procprof_IO;
          delete procoptprof_event;
          delete procoptprof_IO;
       }
   }

   if (!gROOT->IsBatch() && fDraw){
      //save current pad
      //TVirtualPad* pad_sav=gPad;

      /*if (!fCPerfProfiles){
         fCPerfProfiles=new TCanvas("Performance_Profiles", "Performance Profiles");
      }
      else{
         fCPerfProfiles->cd();
      }*/

      fCPerfProfiles=dynamic_cast<TCanvas*>(gROOT->FindObject("Performance_Profiles"));

      if (!fCPerfProfiles){
         fCPerfProfiles = new TCanvas("Performance_Profiles","Performance Profiles",800,600);
         fCPerfProfiles->SetBorderMode(0);
      }

      fCPerfProfiles->Clear();
      fCPerfProfiles->Divide(2, 2);
     

      TStyle* style_sav=gStyle;

      gROOT->SetStyle("Plain");
      gStyle->SetOptStat(0);
      gStyle->SetNdivisions(505);
      gStyle->SetTitleFontSize(0.07);

      fCPerfProfiles->cd(1);
      cpuprof_event->Draw();
      gPad->Update();

      fCPerfProfiles->cd(2);
      procoptprof_event->GetYaxis()->SetRangeUser(0, procoptprof_event->GetMaximum()*1.2);
      procoptprof_event->Draw();
      procprof_event->Draw("same");
      gPad->Update();

      fCPerfProfiles->cd(3);
      procoptprof_IO->GetYaxis()->SetRangeUser(0, procoptprof_IO->GetMaximum()*1.2);
      procoptprof_IO->Draw();
      procprof_IO->Draw("same");
      gPad->Update();

      //legend for pad 2
      fCPerfProfiles->cd(2);

      Float_t legxoffset = 0.1;
      Float_t legwidth = 0.2;
      Float_t legyoffset = 0.02;
      Float_t legheight = 0.15;
   
      Float_t lm = gPad->GetLeftMargin();
      Float_t rm = gPad->GetRightMargin();
      Float_t tm = gPad->GetTopMargin();
      Float_t bm = gPad->GetBottomMargin();
   
      TLegend* leg_event = new TLegend(lm+legxoffset*(1.0-lm-rm),
                                 1.0-tm-(legyoffset+legheight)*(1.0-tm-bm),
                                 lm+(legxoffset+legwidth)*(1.0-lm-rm),
                                 1.0-tm-legyoffset*(1.0-tm-bm));
      leg_event->SetBorderSize(1); 
      leg_event->SetFillColor(0);
      leg_event->AddEntry(procoptprof_event,"Partial Event","p");
      leg_event->AddEntry(procprof_event,"Full Event","p");

      leg_event->Draw();
      gPad->Update();

      //legends for pad 3
      fCPerfProfiles->cd(3);

      lm = gPad->GetLeftMargin();
      rm = gPad->GetRightMargin();
      tm = gPad->GetTopMargin();
      bm = gPad->GetBottomMargin();
   
      TLegend* leg_IO = new TLegend(lm+legxoffset*(1.0-lm-rm),
                                 1.0-tm-(legyoffset+legheight)*(1.0-tm-bm),
                                 lm+(legxoffset+legwidth)*(1.0-lm-rm),
                                 1.0-tm-legyoffset*(1.0-tm-bm));
      leg_IO->SetBorderSize(1); 
      leg_IO->SetFillColor(0);
      leg_IO->AddEntry(procoptprof_IO,"Partial Event","p");
      leg_IO->AddEntry(procprof_IO,"Full Event","p");

      leg_IO->Draw();
      gPad->Update();
   
      fCPerfProfiles->Update();

/*      TPaveText* titlepave = dynamic_cast<TPaveText*>(gPad->GetListOfPrimitives()->FindObject("title"));

      if (titlepave) {
         Double_t x1ndc = titlepave->GetX1NDC();
         Double_t x2ndc = titlepave->GetX2NDC();
         titlepave->SetX1NDC((1.0-x2ndc+x1ndc)/2.);
         titlepave->SetX2NDC((1.0+x2ndc-x1ndc)/2.);
         titlepave->SetBorderSize(0);

         gPad->Update();
      }
      gPad->Modified();
      fCPerfProfiles->Update();
      //pad_sav->cd();
*/

      gStyle=style_sav;
   }
   dirsav->cd();
}

TTree* TProofBench::BuildTimingTree(TString pattern, Int_t& max_slaves) {

   fDirProofBench->cd();

   TTree* timing_tree = new TTree("Timing Tree", "Timing Tree");
   timing_tree->SetDirectory(0);

   Int_t ns_holder;
   Int_t run_holder;
   Int_t nevents_holder;
   Int_t bytes_holder;
   Float_t time_holder;

   TBranch* br = timing_tree->Branch("perfproctime", &ns_holder,
                                     "nslaves/I:run/I:nevents/I:bytes/I:time/F");
   br->GetLeaf("nslaves")->SetAddress(&ns_holder);
   br->GetLeaf("run")->SetAddress(&run_holder);
   br->GetLeaf("nevents")->SetAddress(&nevents_holder);
   br->GetLeaf("bytes")->SetAddress(&bytes_holder);
   br->GetLeaf("time")->SetAddress(&time_holder);

   // extract timing info
   max_slaves = 0;
/*   TIter NextKey(fDirProofBench->GetListOfKeys());
   TKey* key = 0;
   while ((key = dynamic_cast<TKey*>(NextKey()))) {
       printf("key in the list: %s\n", key->GetName());
      if(!TString(key->GetName()).Contains(TRegexp(pattern)))
         continue;

      TObject* obj = key->ReadObj();
*/

   Int_t isfile=0;
   TList* list=0;
   TString sdir(fDirProofBench->GetPath()); 
   if (sdir.Contains("Rint:")){//memory
      list=fDirProofBench->GetList(); //list of object
      isfile=0;//memory
   }
   else{//file
      if (fDebug){ //it is written to the disk
         list=fDirProofBench->GetListOfKeys();
         isfile=1;//file
      }
      else{
         list=fDirProofBench->GetList(); //list of object
         isfile=0;//memory
      }
   }

   TIter nxt(list);
   TObject* obj; 
   TKey* key;
   while ((obj=nxt())){
       if (TString(obj->GetName()).Contains(TRegexp(pattern))){
          printf("key in the list: %s\n", obj->GetName());
       }

      if (!TString(obj->GetName()).Contains(TRegexp(pattern))) continue;

      if (isfile){
         key=dynamic_cast<TKey*>(obj);
         obj = key->ReadObj();
      }

      TTree* t = dynamic_cast<TTree*>(obj);
      if (!t) {
         if (isfile) delete obj;
         continue;
      }

      //parse name to get number of slaves and run
      UInt_t Index = 0;
      const Char_t *name = t->GetName();
      while (Index<strlen(name)) {
        if ( name[Index]>='0' && name[Index]<='9')
        break;
        Index++;
      }

      if (Index == strlen(name)) {
         if (isfile) delete t;
         continue;
      } else {
         // this should be the number of slaves
         ns_holder = atoi(name+Index);
      }

      // get past number of slaves
      while (Index<strlen(name)) {
        if ( name[Index]<'0' || name[Index]>'9')
        break;
        Index++;
      }

      if (Index == strlen(name)) {
         if (isfile) delete t;
         continue;
      }

      while (Index<strlen(name)) {
        if ( name[Index]>='0' && name[Index]<='9')
        break;
        Index++;
      }

      if (Index == strlen(name)) {
         if (isfile) delete t;
         continue;
      } else {
         // this should be the run number
         run_holder = atoi(name+Index);
      }

      if(!t->FindBranch("PerfEvents")) {
         if (isfile) delete t;
         continue;
      }

      //printf("name=%s\n", name);
      // extract timing information
      TPerfEvent pe;
      TPerfEvent* pep = &pe;
      t->SetBranchAddress("PerfEvents",&pep);
      Long64_t entries = t->GetEntries();
      Double_t start(0), end(0);
      //Bool_t started=kFALSE;
      
      Long64_t nevents_kPacket=0;
      Long64_t nevents_kRate=0;
      Long64_t bytesread_kPacket=0;
      Long64_t bytesread_kRate=0;

      for (Long64_t k=0; k<entries; k++) {
         t->GetEntry(k);

         printf("k:%lld fTimeStamp=%lf fEvtNode=%s pe.fType=%d fSlaveName=%s fNodeName=%s fFileName=%s fFileClass=%s fSlave=%s fEventsProcessed=%lld fBytesRead=%lld fLen=%lld fLatency=%lf fProcTime=%lf fCpuTime=%lf fIsStart=%d fIsOk=%d\n",k, pe.fTimeStamp.GetSec() + 1e-9*pe.fTimeStamp.GetNanoSec(), pe.fEvtNode.Data(), pe.fType, pe.fSlaveName.Data(), pe.fNodeName.Data(), pe.fFileName.Data(), pe.fFileClass.Data(), pe.fSlave.Data(), pe.fEventsProcessed, pe.fBytesRead, pe.fLen, pe.fLatency, pe.fProcTime, pe.fCpuTime, pe.fIsStart, pe.fIsOk);

         if (pe.fType==TVirtualPerfStats::kPacket){
            nevents_kPacket+=pe.fEventsProcessed;
            bytesread_kPacket+=pe.fBytesRead;
         }
         if (pe.fType==TVirtualPerfStats::kRate){
            //printf("adding pe.fEventsProcessed=%lld\n", pe.fEventsProcessed);
            nevents_kRate+=pe.fEventsProcessed;
            bytesread_kRate+=pe.fBytesRead;
         }
        
         /*if (!started) {
            if (pe.fType==TVirtualPerfStats::kPacket) {
               start = pe.fTimeStamp.GetSec()
                       + 1e-9*pe.fTimeStamp.GetNanoSec()
                       - pe.fProcTime;
               started=kTRUE;
            }
         } else {
            if (pe.fType==TVirtualPerfStats::kPacket) {
               end = pe.fTimeStamp.GetSec()
                     + 1e-9*pe.fTimeStamp.GetNanoSec();
            }
         }*/
         //skip information from workers
         if (pe.fEvtNode.Contains(".")) continue;
         if (pe.fType==TVirtualPerfStats::kStart) start= pe.fTimeStamp.GetSec()+1e-9*pe.fTimeStamp.GetNanoSec();
         if (pe.fType==TVirtualPerfStats::kStop) end= pe.fTimeStamp.GetSec()+1e-9*pe.fTimeStamp.GetNanoSec();
      }
     
      //printf("nevents_kPacket=%lld, nevents_kRate=%lld\n", nevents_kPacket, nevents_kRate);
      //printf("bytesread_kPacket=%lld, bytesread_kRate=%lld\n", bytesread_kPacket, bytesread_kRate);

      //if (nevents_kPacket!=fNEvents){
       //  Error("BuildTimingTree", "Number of events processed is different from the number of events in the file");
        // return 0;
      //}

      nevents_holder=nevents_kPacket;
      bytes_holder=bytesread_kPacket;
      time_holder = end-start;

      timing_tree->Fill();

      if (max_slaves<ns_holder) max_slaves=ns_holder;

      if (isfile) delete t;
//printf("printing gDirectory\n");
//      gDirectory->ls();
//printf("printing gDirectory, done\n");
   }

   return timing_tree;
}

void TProofBench::Print(Option_t* option)const{

   printf("fBaseDir=%s\n", fBaseDir.Data()); 

   TString sbenchruntype;
   switch (fRunType) {
   case kRunNotSpecified:
      sbenchruntype="kRunNotSpecified";
      break;
   case kRunNothing:
      sbenchruntype="kRunNothing";
      break;
   case kRunCPUTest:
      sbenchruntype="kRunCPUTest";
      break;
   case kRunGenerateFileBench:
      sbenchruntype="kRunGenerateFileBench";
      break;
   case kRunGenerateFileCleanup:
      sbenchruntype="kRunGenerateFileCleanup";
      break;
   case kRunFullDataRead:
      sbenchruntype="kRunFullDataRead";
      break;
   case kRunOptDataRead:
      sbenchruntype="kRunOptDataRead";
      break;
   case kRunNoDataRead:
      sbenchruntype="kRunNoDataRead";
      break;
   default:
      break;
   }

   TString sbenchhisttype;
   switch (fHistType) {
   case kHistNotSpecified:
      sbenchhisttype="kHistNotSpecified";
      break;
   case kHist1D:
      sbenchhisttype="kHist1D";
      break;
   case kHist2D:
      sbenchhisttype="kHist2D";
      break;
   case kHist3D:
      sbenchhisttype="kHist3D";
      break;
   default:
      break;
   }

   printf("fRunType=%s\n", sbenchruntype.Data());
   printf("fNHists=%ld\n", fNHists);
   printf("fHistType=%s\n", sbenchhisttype.Data());
   printf("fMaxNWorkers=%d\n", fMaxNWorkers);
   printf("fNNodes=%d\n", fNNodes);
   printf("fNFiles=%d\n", fNFiles);
   printf("fNEvents=%lld\n", fNEvents);
   //printf("fNIterations=%d\n", fNIterations);
   printf("fStepSize=%d\n", fStepSize);
   printf("fStart=%d\n", fStart);
   printf("fNFilesGenerated=%d\n", fNFilesGenerated);
   printf("fNEventsGenerated=%lld\n", fNEventsGenerated);
   printf("fDraw=%d\n", fDraw);

   printf("fDirProofBench=%s\n", fDirProofBench->GetPath());

   if (fFile){
       fFile->Print(option);
       fFile->ls(option);
   }
   else{
      printf("no file open\n"); 
   }
   if (fDataSet) fDataSet->Print(option);
   if (fCPerfProfiles){
      printf("Performance Profiles Canvas: Name=%s Title=%s\n", 
              fCPerfProfiles->GetName(), fCPerfProfiles->GetTitle());
   }
   if (fProof) fProof->Print(option);
   printf("pwd=%s\n", gDirectory->GetPath());
}

TFile* TProofBench::FileOpen(const char* filename, 
                             Option_t* option, 
                             Bool_t force, 
                             const char* ftitle, 
                             Int_t compress)
{
//open file
//Current open file, if any, will be written and closed when force is true, 
//option is option to TFile::Open(...) function

   TString sfilename(filename);
   sfilename.Remove(TString::kBoth, ' '); //remove leading and trailing white space(s)
   sfilename.Remove(TString::kBoth, '\t');//remove leading and trailing tab character(s)

   //if (sfilename.Length()<1){
   //   return fFile;
   //}
   
   TFile* ftmp=0;

   TString soption(option);
   soption.ToLower();

   if (fFile && force){
      //see if the specified file is already open
      FileStat_t a, b;
      if (fFile && !gSystem->GetPathInfo(fFile->GetName(), a) && !gSystem->GetPathInfo(sfilename, b) && a.fDev==b.fDev && a.fIno==b.fIno){
         //file already open, check open mode
         if (soption.Contains("new") || (soption.Contains("create") && !soption.Contains("recreate"))){
            //trying to open new file which already exists
            Error("FileOpen", "File exists: %s, doing nothing", fFile->GetName());
         }
         else{//reopen a file with same name 
            fFile->Close();
            fFile=new TFile(sfilename, option, ftitle, compress);
            if (fFile->IsZombie()){//open failed
               Error("FileOpen", "Cannot open file: %s", sfilename.Data());
               fFile->Close();
               fFile=0;
            }
            if (!soption.Contains("read")){
               fFile->mkdir("ProofBench");
               fFile->cd("ProofBench");
               fDirProofBench=gDirectory;
            }
            else{
               gDirectory->cd("Rint:/");
               gDirectory->mkdir("ProofBench");
               gDirectory->cd("ProofBench");
               fDirProofBench=gDirectory;
            }
         }
         return fFile;
      }

      ftmp=new TFile(sfilename, option, ftitle, compress);
      if (ftmp->IsZombie()){//open failed
         Error("FileOpen", "Cannot open file: %s", sfilename.Data());
         ftmp->Close();
               gDirectory->cd("Rint:/");
               gDirectory->mkdir("ProofBench");
               gDirectory->cd("ProofBench");
               fDirProofBench=gDirectory;
      }
      else{//open succeeded
         fFile->Close();        
         fFile=ftmp;

            if (!soption.Contains("read")){
               fFile->mkdir("ProofBench");
               fFile->cd("ProofBench");
               fDirProofBench=gDirectory;
            }
            else{
               gDirectory->cd("Rint:/");
               gDirectory->mkdir("ProofBench");
               gDirectory->cd("ProofBench");
               fDirProofBench=gDirectory;
            }
      }
      return fFile;
   }
   else if (fFile && !force){
        Info("FileOpen", "File (%s) already open; doing nothing. To force use kTRUE", fFile->GetName());
        return fFile;
   }
   else{ //!fFile && (force || !force)
      if (sfilename.Length()<1){
         gDirectory->cd("Rint:/");
         gDirectory->mkdir("ProofBench");
         gDirectory->cd("ProofBench");
         fDirProofBench=gDirectory;
         return 0;
      } 
      ftmp=new TFile(sfilename, option, ftitle, compress);
      if (ftmp->IsZombie()){
         Error("FileOpen", "Cannot open file: %s", sfilename.Data());
         ftmp->Close();
               gDirectory->cd("Rint:/");
               fDirProofBench=gDirectory;
         return fFile;
      }
      else{
         fFile=ftmp;
            if (!soption.Contains("read")){
               fFile->mkdir("ProofBench");
               fFile->cd("ProofBench");
               fDirProofBench=gDirectory;
            }
            else{
               gDirectory->cd("Rint:/");
               gDirectory->mkdir("ProofBench");
               gDirectory->cd("ProofBench");
               fDirProofBench=gDirectory;
            }
         return fFile;
      }
   }
}

TFile* TProofBench::SetFile(TFile* ftmp){
//set file to ftmp
//if file is already open in writable mode, all objects are written before file is set to ftmp 

   if (fFile){
      TString soption=fFile->GetOption();
      soption.ToLower();
      if (soption.Contains("create") || soption.Contains("update")){
         fFile->Write();
      }
      fFile->Close();
      delete fFile;
   }
   fFile=ftmp;
   return fFile;
}

Int_t TProofBench::SetParameters(TString &basedir, 
                                      Long_t &nhists, 
                                      Int_t& maxnworkers, 
                                      Int_t& nnodes, 
                                      Int_t &nfiles, 
                                      Long64_t &nevents, 
                                      //Int_t &niter, 
                                      Int_t &stepsize, 
                                      Int_t& start)
{

   Int_t returnval=0;

   basedir.Remove(TString::kBoth, ' '); //remove leading and trailing white space(s)
   basedir.Remove(TString::kBoth, '\t');//remove leading and trailing tab character(s)
   basedir.Remove(TString::kTrailing, '/'); //remove trailing /

   //see if files were already generated
   if ((basedir.Length()<=0 || basedir==fBaseDir) //no base directory change
    //number of files requested is smaller than files already generated
    && ((nfiles<=0 && fNFilesGenerated>0) || (nfiles>0 && nfiles<=fNFilesGenerated)) 
    //number of events/file is smaller than that in files already generated
    && ((nevents<=0 && fNEventsGenerated>0) || (nevents>0 && nevents<=fNEventsGenerated))){    
   }
   else{ //file generation is needed)
      returnval=1;
   }

   fBaseDir=basedir.Length()>0?basedir:fBaseDir;
   fNHists=nhists>-1?nhists: fNHists;
   fNNodes=nnodes>-1?nnodes: fNNodes;

   if (fProof){
      TList* lslave=fProof->GetListOfSlaveInfos();
      Int_t nslaves=lslave->GetSize();  //number of slave workers regardless of its status, active or inactive
      fMaxNWorkers=maxnworkers>-1?maxnworkers:2*nslaves; //this number to be parameterized in the future
   }
   else{
      fMaxNWorkers=maxnworkers>-1?maxnworkers:fMaxNWorkers;
   }

   fNFiles=nfiles>-1?nfiles: fNFiles;
   fNEvents=nevents>-1?nevents: fNEvents;

   fStepSize=stepsize>-1?stepsize:fStepSize;

   if (start>-1 && start>fMaxNWorkers){
      Info("SetParameters", "starting # workers cannot be greater than max workers;"
           "Setting start # workers to %d", fMaxNWorkers);
      fStepSize=fMaxNWorkers;
   }
   else{
      fStart=start>-1?start:fStart;
   }

   return returnval;
}
