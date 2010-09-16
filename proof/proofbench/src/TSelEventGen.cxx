// @(#)root/proofx:$Id$
// Author: Sangsu Ryu 22/06/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TSelEventGen                                                         //
//                                                                      //
// PROOF selector for event file generation.                            //
// List of files to be generated for each node is provided by client.   //
// And list of files generated is sent back.                            //
// Existing files are reused if not forced to be regenerated.           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define TSelEventGen_cxx

#include "TSelEventGen.h"
#include "TParameter.h"
#include "TProofNodeInfo.h"
#include "TProofBenchMode.h"
#include "TProof.h"
#include "TMap.h" 
#include "TDSet.h"
#include "TFileInfo.h"
#include "TFile.h"
#include "TSortedList.h"
#include "TRandom.h"
#include "test/Event.h"

ClassImp(TSelEventGen)

//______________________________________________________________________________
TSelEventGen::TSelEventGen():
   fFileType(TProofBenchMode::kFileNotSpecified),
   fBaseDir(""),
   fNEvents(10000),
   fNTracks(100),
   fRegenerate(kFALSE),
   fNWorkersPerNode(0),
   fWorkerNumber(0),
   fTotalGen(0),
   fFilesGenerated(0),
   fChain(0)
{
   if (gProofServ){
      fBaseDir=gProofServ->GetDataDir();
      //2 directories up
      fBaseDir.Remove(fBaseDir.Last('/'));
//#if 0
      fBaseDir.Remove(fBaseDir.Last('/'));
//#endif
   }
   else{
      fBaseDir="";
   }
}

//______________________________________________________________________________
void TSelEventGen::Begin(TTree *)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();
   // Determine the test type
   TMap *filemap = dynamic_cast<TMap *>(fInput->FindObject("PROOF_FilesToProcess"));
   if (filemap) {
      Info("Begin", "dumping the file map:");
      filemap->Print();
   } else {
      if (fInput->FindObject("PROOF_FilesToProcess")) {
         Error("Begin", "object 'PROOF_FilesToProcess' found but not a map (%s)",
                        fInput->FindObject("PROOF_FilesToProcess")->ClassName());
      } else {
         Error("Begin", "object 'PROOF_FilesToProcess' not found");
      }
   }
}

//______________________________________________________________________________
void TSelEventGen::SlaveBegin(TTree *tree)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   Init(tree);

   TString option = GetOption();

   //get parameters

   Bool_t found_filetype=kFALSE;
   Bool_t found_basedir=kFALSE;
   Bool_t found_nevents=kFALSE;
   Bool_t found_ntracks=kFALSE;
   Bool_t found_regenerate=kFALSE;
   Bool_t found_slaveinfos=kFALSE;

   TIter nxt(fInput);
   TString sinput;
   TObject *obj;

   TSortedList* listofslaveinfos=0;

   while ((obj = nxt())){

      sinput=obj->GetName();
      //Info("SlaveBegin", "Input list: %s", sinput.Data());

      if (sinput.Contains("PROOF_BenchmarkFileType")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fFileType=(TProofBenchMode::EFileType)a->GetVal();
            found_filetype=kTRUE; 
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkFileType not type TParameter<Int_t>*");
         }
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkBaseDir")){
         TNamed* a=dynamic_cast<TNamed*>(obj);
         if (a){
            TString proof_benchmarkbasedir=a->GetTitle();
            if (!proof_benchmarkbasedir.IsNull()){
               if (!gSystem->AccessPathName(proof_benchmarkbasedir, kWritePermission)){
                  //directory is writable
                  fBaseDir=proof_benchmarkbasedir;
                  Info("BeginSlave", "Using directory \"%s\"", fBaseDir.Data());
               }
               else{
                  //directory is not writable, use default directory
                  Warning("BeginSlave", "\"%s\" directory is not writable, using default directory: %s",
                          proof_benchmarkbasedir.Data(), fBaseDir.Data());
               }
            } 
            else{
               Info("BeginSlave", "Using default directory: %s", fBaseDir.Data());
            }
            found_basedir=kTRUE; 
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkBaseDir not type TNamed");
         }
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkNEvents")){
         TParameter<Long64_t>* a=dynamic_cast<TParameter<Long64_t>*>(obj);
         if (a){
            fNEvents= a->GetVal();
            found_nevents=kTRUE; 
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkEvents not type TParameter<Long64_t>*");
         }
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkNTracks")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fNTracks=a->GetVal();
            found_ntracks=kTRUE; 
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkNTracks not type TParameter<Int_t>*");
         }
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkRegenerate")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fRegenerate=a->GetVal();
            found_regenerate=kTRUE; 
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkRegenerate not type TParameter<Int_t>*");
         }
         continue;
      }
      if (sinput.Contains("PROOF_SlaveInfos")){
         obj->Print("a"); 
         TSortedList* a=dynamic_cast<TSortedList*>(obj);
         if (a){
            listofslaveinfos=a;
            found_slaveinfos=kTRUE; 
         }
         else{
            Error("SlaveBegin", "PROOF_SlaveInfos not type TSortedList*");
         }
         continue;
      }
   }
   
   if (!found_filetype){
      Warning("SlaveBegin", "PROOF_BenchmarkFileType not found; using default: %d", fFileType);
   }
   if (!found_basedir){
      Warning("SlaveBegin", "PROOF_BenchmarkBaseDir not found; using default: %s", fBaseDir.Data());
   }
   if (!found_nevents){
      Warning("SlaveBegin", "PROOF_BenchmarkNEvents not found; using default: %lld", fNEvents);
   }
   if (!found_ntracks){
      Warning("SlaveBegin", "PROOF_BenchmarkNTracks not found; using default: %d", fNTracks);
   }
   if (!found_regenerate){
      Warning("SlaveBegin", "PROOF_BenchmarkRegenerate not found; using default: %d", fRegenerate);
   }
   if (!found_slaveinfos){
      Error("SlaveBegin", "PROOF_SlaveInfos not found");
      return;
   }

   // Output
#if 0
   TParameter<Long64_t> *totalGen = new TParameter<Long64_t>("TotalGen", (Long64_t)0);
   fOutput->Add(totalGen);
   fTotalGen = (TObject *) totalGen;
#endif

   TString hostname=TUrl(gSystem->HostName()).GetHostFQDN();
   TString thisordinal=gProofServ->GetOrdinal();

   //chop starting part. Temporary solution
   Ssiz_t from=thisordinal.Index(".")+1;
   Ssiz_t to=thisordinal.Length();
   TString thisordinalchopped(thisordinal(from,to));
   //printf("thisordinalchopped=%s\n", thisordinalchopped.Data());

   //Int_t ntotalworkers=0;   //total number of worker on the cluster

   if (listofslaveinfos){
      TIter iter(listofslaveinfos);

      TSlaveInfo *si = 0;

      TString nodename, ordinal;

      while ((si=(TSlaveInfo*)iter())){
         //obj->Print(); 
         nodename=si->GetName();
         ordinal=si->GetOrdinal();
         //printf("nodename=%s\n", nodename.Data());
         //printf("ordinal=%s\n", ordinal.Data());

         /*//See if proof is running on localhost
         if (nodename.Contains("localhost")){
            //hostname="localhost.localdomain";
            nodename=gSystem->HostName();
         }*/
         if (hostname==nodename){
            if (ordinal==thisordinal){
                fWorkerNumber=fNWorkersPerNode;
            }
            fNWorkersPerNode++;
         }
      }

      Info("SlaveBegin", "Number of workers on this node (%s) is %d", hostname.Data(), fNWorkersPerNode); 
      Info("SlaveBegin", "Worker number on this node (%s) is %d", hostname.Data(), fWorkerNumber); 

      Print();
   }
   else {
      Error("SlaveBegin", "Slave info empty");
      return;
   }

   fFilesGenerated=new TList();

   TString sfilegenerated="PROOF_FilesGenerated" "_"+hostname+"_"+thisordinal;
   fFilesGenerated->SetName(sfilegenerated);
   fFilesGenerated->SetOwner(kTRUE);
}

//______________________________________________________________________________
Long64_t TSelEventGen::GenerateFiles(TProofBenchMode::EFileType filetype, TString filename, Long64_t sizenevents)
{
//runtype is run type either TProofBench::kRunGenerateFileBench or TProofBench::kRunGenerateFileCleanup
//filename is the name of the file to be generated
//sizenevents is number of events to generate when runtype==TProofBench::kRunGenerateFileBench
// and size of the file to generate when runtype==TProofBench::kRunGenerateFileCleanup
//returns number of entries in the file when runtype==TProofBench::kRunGenerateFileBench
//returns bytes written when runtype==TProofBench::kRunGenerateFileCleanup
//return 0 in case error

   if (!(filetype==TProofBenchMode::kFileBenchmark || filetype==TProofBenchMode::kFileCleanup)){
      Error("GenerateFiles", "File type not supported; %d", filetype);
      return 0;
   }
    
   Long64_t nentries=0;
   TDirectory* savedir = gDirectory;
   //printf("current dir=%s\n", gDirectory->GetPath());

   TFile* f=new TFile(filename, "RECREATE");

   savedir->cd();

   if (!f || f->IsZombie()) return 0;

   Event *event=new Event();
   Event *ep = event;
   TTree* eventtree= new TTree("EventTree", "Event Tree");
   eventtree->SetDirectory(f);

   const Int_t buffersize=32000;
   eventtree->Branch("event", "Event", &ep, buffersize, 1);
   eventtree->AutoSave();

   Long64_t i=0;
   Long64_t size_generated=0;

   f->SetCompressionLevel(0); //no compression

   if (filetype==TProofBenchMode::kFileBenchmark){
      Info("GenerateFiles", "Generating %s", filename.Data());   
      while (sizenevents--){
         //event->Build(i++,fNTracksBench,0);
         event->Build(i++, fNTracks, 0);
         size_generated+=eventtree->Fill();
      }
      nentries=eventtree->GetEntries();
      Info("GenerateFiles", "%s generated with %lld entries", filename.Data(), nentries);
   }
   else if (filetype==TProofBenchMode::kFileCleanup){
      Info("GenerateFiles", "Generating %s", filename.Data());   
      while (size_generated<sizenevents){
         event->Build(i++, fNTracks, 0);
         size_generated+=eventtree->Fill();
      }
      Info("GenerateFiles", "%s generated with %lld bytes", filename.Data(), size_generated);
   }
   savedir = gDirectory;

   f = eventtree->GetCurrentFile();
   f->cd();
   eventtree->Write();
   eventtree->SetDirectory(0);

   f->Close();
   delete f;
   f = 0;
   eventtree->Delete();
   event->Delete();
   savedir->cd();

   if (filetype==TProofBenchMode::kFileBenchmark) return nentries;
   else if (filetype==TProofBenchMode::kFileCleanup) return size_generated;
   else return 0;
}

//______________________________________________________________________________
Bool_t TSelEventGen::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either TTree::GetEntry() or TBranch::GetEntry()
   // to read either all or the required parts of the data. When processing
   // keyed objects with PROOF, the object is already loaded and is available
   // via the fObject pointer.
   //
   // This function should contain the "body" of the analysis. It can contain
   // simple or elaborate selection criteria, run algorithms on the data
   // of the event and typically fill histograms.

   // WARNING when a selector is used with a TChain, you must use
   //  the pointer to the current TTree to call GetEntry(entry).
   //  The entry is always the local entry number in the current tree.
   //  Assuming that fChain is the pointer to the TChain being processed,
   //  use fChain->GetTree()->GetEntry(entry).

   TDSetElement *fCurrent = 0;
   TPair *elemPair = 0;
   if (fInput && (elemPair = dynamic_cast<TPair *>(fInput->FindObject("PROOF_CurrentElement")))) {
      if ((fCurrent = dynamic_cast<TDSetElement *>(elemPair->Value()))) {
         Info("Process", "entry %lld: file: '%s'", entry, fCurrent->GetName());
      } else {
         Error("Process", "entry %lld: no file specified!", entry);
         return kFALSE;
      }
   }

   // Generate
#if 0
   TParameter<Long64_t> *totalGen = (TParameter<Long64_t> *)fTotalGen;
   Long64_t tg = totalGen->GetVal();
   tg += GenerateFiles(fRunType, fCurrent->GetName(), fNEvents);
   totalGen->SetVal(tg);
#endif

   TString filename=fCurrent->GetName();
   filename=fBaseDir+"/"+filename;

   TString hostfqdn=TUrl(gSystem->HostName()).GetHostFQDN();
   TFileInfo* fi=new TFileInfo("root://"+hostfqdn+"/"+filename);
   TString seed = hostfqdn+"/"+filename;

   //generate files
   if (fFileType==TProofBenchMode::kFileBenchmark){
      Long64_t neventstogenerate=fNEvents;

      Bool_t filefound=kFALSE;
      FileStat_t filestat;
      if (!fRegenerate && !gSystem->GetPathInfo(filename, filestat)){//stat'ed
         TFile f(filename);
         if (!f.IsZombie()){
            TTree* t=(TTree*)f.Get("EventTree");
            if (t){
               Long64_t entries_file=t->GetEntries();
               if ( entries_file==neventstogenerate ){
                  //file size seems to be correct, skip generation
                  Info("Process", "Bench file (%s, entries=%lld) exists."
                       " Skipping generation", fi->GetFirstUrl()->GetFile(), entries_file);
                  neventstogenerate-=entries_file;
                  fFilesGenerated->Add(fi);
                  filefound=kTRUE;
               }
            }
         }
         f.Close();
      }

      if (!filefound){
         gRandom->SetSeed(static_cast<UInt_t>(TMath::Hash(seed)));
         neventstogenerate-=GenerateFiles(fFileType, filename, neventstogenerate);
         fFilesGenerated->Add(fi);
      }
   }
   else if (fFileType==TProofBenchMode::kFileCleanup){

      //(re)generate files
      MemInfo_t meminfo;
      Bool_t filefound=kFALSE;

      if (gSystem->GetMemInfo(&meminfo)){
          Error("SlaveBegin", "Cannot get memory information, returning");
          return kFALSE;
      }
      Info("SlaveBegin", "Total memory on this node: %d MB", meminfo.fMemTotal);

      Long64_t memorytotal=(Long64_t)(meminfo.fMemTotal)*1024*1024;
  
      Long64_t memorythisworker=memorytotal/fNWorkersPerNode+1;
      Long64_t bytestowrite=memorythisworker;
      Long64_t byteswritten=0;
      
      FileStat_t filestat;
      if (!fRegenerate && !gSystem->GetPathInfo(filename, filestat)){//stat'ed
         TFile f(filename);
         if (!f.IsZombie()){
            TTree* t=(TTree*)f.Get("EventTree");
            if (t){
               Long64_t sizetree=t->GetTotBytes();
               Info("Process", "sizetree=%lld bytestowrite=%lld", sizetree, bytestowrite);
               if (0.9*bytestowrite<sizetree && sizetree<1.1*bytestowrite){
                  //file size seems to be correct, skip generation
                  byteswritten=sizetree;
                  bytestowrite-=byteswritten;
                  Info("Process", "Cleanup file (%s, tree size=%lld) exists."
                       " Skipping generation", fi->GetFirstUrl()->GetFile(), sizetree);
                  fFilesGenerated->Add(fi);
                  filefound=kTRUE;
               }
            }
         }
         f.Close();
      }

      if (!filefound){
         gRandom->SetSeed(static_cast<UInt_t>(TMath::Hash(seed)));
         byteswritten=GenerateFiles(fFileType, filename, bytestowrite);
         bytestowrite-=byteswritten;
         fFilesGenerated->Add(fi);
      }
   }
   else{
      Error("Process", "Run type not recognized: fFileType=%d, returning", fFileType);
      return kTRUE;
   }

   return kTRUE;
}

//______________________________________________________________________________
void TSelEventGen::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.
   fOutput->Add(fFilesGenerated);

   //set it back
   //TTree::SetMaxTreeSize(MaxTreeSizeOrg);

}

//______________________________________________________________________________
void TSelEventGen::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.
}

//______________________________________________________________________________
void TSelEventGen::Print(Option_t *) const
{

   Printf("fNEvents=%lld", fNEvents);
   Printf("fBaseDir=%s", fBaseDir.Data());
   Printf("fWorkerNumber=%d", fWorkerNumber);
   Printf("fNTracks=%d", fNTracks);
   Printf("fRegenerate=%d", fRegenerate);
   Printf("fNWorkersPerNode=%d", fNWorkersPerNode);
}

