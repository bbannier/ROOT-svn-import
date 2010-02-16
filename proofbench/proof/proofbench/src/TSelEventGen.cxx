#define TSelEventGen_cxx
// The class definition in TSelEventGen.h has been generated automatically
// by the ROOT utility TTree::MakeSelector(). This class is derived
// from the ROOT class TSelector. For more information on the TSelector
// framework see $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.

// The following methods are defined in this file:
//    Begin():        called everytime a loop on the tree starts,
//                    a convenient place to create your histograms.
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//                    called only on the slave servers.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//
// To use this file, try the following session on your Tree T:
//
// Root > T->Process("TSelEventGen.C")
// Root > T->Process("TSelEventGen.C","some options")
// Root > T->Process("TSelEventGen.C+")
//

#include "TSelEventGen.h"
#include "TParameter.h"
#include "TSystem.h"
#include "TProofNodeInfo.h"
#include "TRandom.h"
#include "TProofBench.h"
#include "TROOT.h"
#include "TProof.h"
#include "TMap.h" 
#include "TDSet.h"
#include "TFileInfo.h"
#include "TFile.h"
#include "TSortedList.h"

ClassImp(TSelEventGen)

TSelEventGen::TSelEventGen()
   :fRunType(TProofBench::kRunNotSpecified),
   fMaxNWorkers(0),
   fNFilesAWorker(1),
   fNEvents(10000),
   fNWorkersPerNode(0),
   fWorkerNumber(0),
   fNTracksBench(10),
   fNTracksCleanup(100),
   fRegenerate(false)
{
   if (gProofServ){
      fBaseDir=gProofServ->GetDataDir();
      fBaseDir.Remove(fBaseDir.Last('/'));
   }
   else{
      fBaseDir="";
   }
}

TSelEventGen::TSelEventGen(TTree*)
   :fRunType(TProofBench::kRunNotSpecified),
   fMaxNWorkers(0),
   fNFilesAWorker(1),
   fNEvents(10000),
   fNWorkersPerNode(0),
   fWorkerNumber(0),
   fNTracksBench(10),
   fNTracksCleanup(100),
   fRegenerate(false)
{
   if (gProofServ){
      fBaseDir=gProofServ->GetDataDir();
      fBaseDir.Remove(fBaseDir.Last('/'));
   }
   else{
      fBaseDir="";
   }
}

void TSelEventGen::Begin(TTree *)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();
}

void TSelEventGen::SlaveBegin(TTree *tree)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   Init(tree);

   TString option = GetOption();

   //get parameters

   TIter nxt(fInput);
   TString sinput;
   TObject *obj;

   TSortedList* listofslaveinfos=0;

   while ((obj = nxt())){

      sinput=obj->GetName();
      //Info("SlaveBegin", "Input list: %s", sinput.Data());

      if (sinput.Contains("fRunType")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fRunType=(TProofBench::ERunType)a->GetVal();
            continue;
         }
         continue;
      }
      if (sinput.Contains("fBaseDir")){
         TNamed *a=dynamic_cast<TNamed*>(obj);
         if (a){
            TString basedir=a->GetTitle();
            if (gSystem->AccessPathName(basedir)) {
               Info("SlaveBegin", "No such file or directory: %s; using default directory: %s", 
                     basedir.Data(), fBaseDir.Data());
               continue;
            }
            fBaseDir=basedir;
         }
         continue;
      }
      if (sinput.Contains("fBenchmarkMode")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fBenchmarkMode=(TProofBench::EBenchmarkMode)a->GetVal();
         }
         continue;
      }
      if (sinput.Contains("fNFilesAWorker")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fNFilesAWorker=a->GetVal();
         }
         continue;
      }
      if (sinput.Contains("fNFilesANode")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fNFilesANode=a->GetVal();
         }
         continue;
      }
      if (sinput.Contains("fNEvents")){
         TParameter<Long64_t>* a=dynamic_cast<TParameter<Long64_t>*>(obj);
         if (a){
            fNEvents= a->GetVal();
         }
         continue;
      }
      if (sinput.Contains("fMaxNWorkers")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fMaxNWorkers=a->GetVal();
         }
         continue;
      }
      if (sinput.Contains("fNTracksBench")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fNTracksBench=a->GetVal();
         }
         continue;
      }
      if (sinput.Contains("fNTracksCleanup")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fNTracksCleanup=a->GetVal();
         }
         continue;
      }
      if (sinput.Contains("fRegenerate")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fRegenerate=a->GetVal();
         }
         continue;
      }
      if (sinput.Contains("PROOF_SlaveInfos")){
         obj->Print("a"); 
         listofslaveinfos=dynamic_cast<TSortedList*>(obj);
         continue;
      }

   }
   
   TString hostname=gSystem->HostName();
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

         //See if proof is running on localhost
         if (nodename.Contains("localhost")){
            //hostname="localhost.localdomain";
            nodename=gSystem->HostName();
         }
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


   TString sfilegenerated="PROOF_FilesGenerated" "_"+hostname+"_"+thisordinal;

   TList* lfilegenerated=new TList;
   lfilegenerated->SetName(sfilegenerated);
   TDSet* dataset=new TDSet("TTree","EventTree"); 

   //generate files
   if (fRunType==TProofBench::kRunGenerateFileBench){
      Int_t from, to, inc;
      switch (fBenchmarkMode){
         case TProofBench::kModeStaticNode:
            //from=fWorkerNumber;
            from=0;
            to=fNFilesANode;
            //inc=fNWorkersPerNode;
            inc=1;
         break;
         case TProofBench::kModeStaticWorkersNode:
            from=0;
            to=fNFilesAWorker;
            inc=1;
         break;
         case TProofBench::kModeStaticCluster:
         break;
         case TProofBench::kModeDynamicWorkers:
         break;
         default:
         break;
      } 

         for (Int_t i=from; i*fNWorkersPerNode+fWorkerNumber<to; i+=inc){
      //for (Int_t i=0; i<fNFilesAWorker; i++){
            Long64_t neventstogenerate=fNEvents;
            Int_t k=0; //serial number of file when a file becomes larger 
                    //than maximum tree size (1.9 GB) limit
            while(neventstogenerate>0){
               //TString seed = hostname;
               TString seed = "_" "Benchmark" "_";   
               seed += i; 
               seed += "_";
               seed += k++;  //serial number
               gRandom->SetSeed(static_cast<UInt_t>(TMath::Hash(seed)));
      
               TString filename = fBaseDir;
               filename += "/EventTree";
               filename += seed;
               filename += ".root";
      
               if (!fRegenerate){
                  //see if a file exists
                  FileStat_t filestat;
                  if (!gSystem->GetPathInfo(filename, filestat)){//stat'ed
                     //Check if file is ok
                     TFile f(filename);
                     if (!f.IsZombie()){
                        Long64_t size=f.GetSize();
                        TTree* t=(TTree*)f.Get("EventTree");
                        if (size!=-1 && t){
                           Long64_t entries_file=t->GetEntries();
                           Long64_t sizetree=t->GetTotBytes();
                           //Long64_t maxtreesize=TTree::GetMaxTreeSize();
                           Long64_t maxtreesize=100*1024*1024;//test
                           if (entries_file==neventstogenerate
                          || (entries_file<neventstogenerate && 0.9*maxtreesize<sizetree && sizetree<1.1*maxtreesize)){
                              //file size seems to be correct, skip generation
                              Info("SlaveBegin", "Bench file (%s, entries=%lld) exists."
                                   " Skipping generation", filename.Data(), entries_file);
                              neventstogenerate-=entries_file;
                              dataset->Add(filename);
                              continue;
                           }
                        }
                     }
                  }
               }
               neventstogenerate-=GenerateFiles(fRunType, filename, neventstogenerate);
               dataset->Add(filename);
            }
         }
         
         dataset->Lookup();
         dataset->Validate();
   
         lfilegenerated->Add(dataset);
         lfilegenerated->Print("a");
   
         fOutput->Add(lfilegenerated);
      //fOutput->Add(dataset);
   }
   else if (fRunType==TProofBench::kRunGenerateFileCleanup){

      //(re)generate files
      MemInfo_t meminfo;
      if (gSystem->GetMemInfo(&meminfo)){
          Error("SlaveBegin", "Cannot get memory information, returning");
          return;
      }
      Info("SlaveBegin", "Total memory on this node: %d MB", meminfo.fMemTotal);

      //Long64_t memorytotal=(Long64_t)(meminfo.fMemTotal)*1024*1024;
      Long64_t memorytotal=(Long64_t)200*1024*1024;
  
      Long64_t memorythisworker=memorytotal/fNWorkersPerNode+1;

      Long64_t bytestowrite=memorythisworker;
      Long64_t byteswritten=0;
      Int_t i=0;
      while(bytestowrite>0){
         //TString seed = hostname;
         TString seed = "_" "Cleanup" "_";
//         seed += fWorkerNumber;
         seed += i++; //serial number in case file is larger than TTree::fgMaxTreeSize
//         seed += "_";

         gRandom->SetSeed(static_cast<UInt_t>(TMath::Hash(seed)));

         TString filename = fBaseDir;
         filename += "/EventTree";
         filename += seed;
         filename += ".root";


         if (!fRegenerate){
            //see if a file exists
            FileStat_t filestat;
            if (!gSystem->GetPathInfo(filename, filestat)){//stat'ed
               //Check if file is ok
               //Long64_t size=filestat.fSize;
               TFile f(filename);
               if (!f.IsZombie()){
                  TTree* tree=(TTree*)f.Get("EventTree");
                  if (tree){
                     Long64_t sizetree=tree->GetTotBytes();
                     //Long64_t maxtreesize=TTree::GetMaxTreeSize();
                     Long64_t maxtreesize=100*1024*1024;//test
                     if ( ((0.9*maxtreesize<sizetree && sizetree<1.1*maxtreesize)
                      || (0.9*bytestowrite<sizetree && sizetree<1.1*bytestowrite))){
                        //file size seems to be correct, skip generation
                        byteswritten=sizetree;
                        bytestowrite-=byteswritten;
                        Info("SlaveBegin", "Cleanup file (%s, tree size=%lld) exists."
                             " Skipping generation", filename.Data(), sizetree);
                        dataset->Add(filename);
                        continue;
                     }
                  }
               }
            }
         }


         byteswritten=GenerateFiles(fRunType, filename, bytestowrite);
         bytestowrite-=byteswritten;
         dataset->Add(filename);
      }

      dataset->Lookup();
      dataset->Validate();

      lfilegenerated->Add(dataset);
      lfilegenerated->Print("a");

      fOutput->Add(lfilegenerated);
   }
   else{
      Error("SlaveBegin", "Run type not recognized: fRunType=%d, returning", fRunType);
      return;
   }

   return;
}

Long64_t TSelEventGen::GenerateFiles(TProofBench::ERunType runtype, TString filename, Long64_t sizenevents)
{
//runtype is run type either TProofBench::kRunGenerateFileBench or TProofBench::kRunGenerateFileCleanup
//filename is the name of the file to be generated
//sizenevents is number of events to generate when runtype==TProofBench::kRunGenerateFileBench
// and size of the file to generate when runtype==TProofBench::kRunGenerateFileCleanup
//returns number of entries in the file when runtype==TProofBench::kRunGenerateFileBench
//returns bytes written when runtype==TProofBench::kRunGenerateFileCleanup
//return 0 in case error

   if (!(runtype==TProofBench::kRunGenerateFileCleanup 
      || runtype==TProofBench::kRunGenerateFileBench)){
      Error("GenerateFiles", "Run type not supported");
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
   Long64_t fileend=0;
   Long64_t size_generated=0;

   f->SetCompressionLevel(0); //no compression
   //take control on file change
   //const Long64_t maxtreesize_org=TTree::GetMaxTreeSize();
   const Long64_t maxtreesize_org=100*1024*1024;  //100 MB limit for test
   TTree::SetMaxTreeSize(10*maxtreesize_org);

   if (runtype==TProofBench::kRunGenerateFileBench){
      Info("GenerateFiles", "Generating %s", filename.Data());   
      while (sizenevents-- && size_generated<maxtreesize_org){
         event->Build(i++,fNTracksBench,0);
         size_generated+=eventtree->Fill();
      }
      nentries=eventtree->GetEntries();
      Info("GenerateFiles", "%s generated with %lld entries", filename.Data(), nentries);
   }
   else if (runtype==TProofBench::kRunGenerateFileCleanup){
      Info("GenerateFiles", "Generating %s", filename.Data());   
      //while (fileend<size && (fileend+buffersize)<fMaxTreeSize){
      while (size_generated<sizenevents && size_generated<maxtreesize_org){
         event->Build(i++, fNTracksCleanup,0);
         size_generated+=eventtree->Fill();
         //fileend=f->GetEND();
      }
      Info("GenerateFiles", "%s generated with %lld bytes", filename.Data(), size_generated);
   }
   //set it back
   //TTree::SetMaxTreeSize(maxtreesize_org);
   TTree::SetMaxTreeSize(1900000000);

   savedir = gDirectory;

   f = eventtree->GetCurrentFile();
   f->cd();
   eventtree->Write();
   eventtree->SetDirectory(0);

   //nentries=eventtree->GetEntries();
   //f->Write();
   //printf("current dir=%s\n", gDirectory->GetPath());
   f->Close();
   delete f;
   f = 0;
   eventtree->Delete();
   event->Delete();
   savedir->cd();

   if (runtype==TProofBench::kRunGenerateFileBench) return nentries;
   else if (runtype==TProofBench::kRunGenerateFileCleanup) return size_generated;
   else return 0;
}

Bool_t TSelEventGen::Process(Long64_t)
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

   return kTRUE;
}

void TSelEventGen::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

void TSelEventGen::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.
}

void TSelEventGen::Print(Option_t *) const
{

   printf("fMaxNWorkers=%d\n", fMaxNWorkers);
   Printf("fBenchmarkMode=%d\n", fBenchmarkMode);
   Printf("fNFilesAWorker=%d\n", fNFilesAWorker);
   Printf("fNFilesANode=%d\n", fNFilesANode);
   Printf("fNEvents=%lld\n", fNEvents);
   Printf("fNWorkersPerNode=%d\n", fNWorkersPerNode);
   printf("fWorkerNumber=%d\n", fWorkerNumber);
   printf("fNTracksBench=%d\n", fNTracksBench);
   printf("fNTracksCleanup=%d\n", fNTracksCleanup);
   printf("fRegenerate=%d\n", fRegenerate);
}

