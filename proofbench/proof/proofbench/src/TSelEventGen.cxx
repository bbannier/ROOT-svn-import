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
   fNTries(10),
   fNEvents(10000),
   fNWorkersPerNode(0),
   fWorkerNumber(0),
   fNTracksBench(10),
   fNTracksCleanup(100)
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
   fNTries(10),
   fNEvents(10000),
   fNWorkersPerNode(0),
   fWorkerNumber(0),
   fNTracksBench(10),
   fNTracksCleanup(100)
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
      if (sinput.Contains("fNTries")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fNTries= a->GetVal();
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
      if (sinput.Contains("listofslaveinfos")){
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
   }
   else {
      Error("SlaveBegin", "Slave info empty");
      return;
   }


   TString sfilegenerated="listoffilesgenerated_"+hostname+"_"+thisordinal;

   TList* lfilegenerated=new TList;
   lfilegenerated->SetName(sfilegenerated);
   TDSet* dataset=new TDSet("TTree","EventTree"); 

   Long64_t nentries;
   //generate files
   if (fRunType==TProofBench::kRunGenerateFileBench){
      //for (Int_t i=fWorkerNumber; i<fMaxNWorkers; i+=fNWorkersPerNode){
         //for (Int_t j=0; j<fNTries; j++){
         for (Int_t j=fWorkerNumber; j<fNTries; j+=fNWorkersPerNode){
            
            TString seed = hostname;
            seed += "_FullDataRead_";   
            //seed += i+1; 
            seed += 0;   //0 for all runs
            seed += "_";
            seed += j;
            gRandom->SetSeed(static_cast<UInt_t>(TMath::Hash(seed)));
   
            TString filename = fBaseDir;
            filename += "/event_tree_";
            filename += seed;
            filename += ".root";
   
            nentries=GenerateFiles(fRunType, filename, fNEvents);
   
            dataset->Add(filename);
   
            seed = hostname;
            seed += "_OptDataRead_";  
            //seed += i+1; 
            seed += 0;   //0 for all runs
            seed += "_";
            seed += j;
            gRandom->SetSeed(static_cast<UInt_t>(TMath::Hash(seed)));
   
            filename = fBaseDir;
            filename += "/event_tree_";
            filename += seed;
            filename += ".root";
   
            //printf("filename=%s\n", filename.Data());
   
            nentries=GenerateFiles(fRunType, filename, fNEvents);
   
            dataset->Add(filename);
         }
      //}
      dataset->Lookup();
      dataset->Validate();

      lfilegenerated->Add(dataset);
      lfilegenerated->Print("a");

      fOutput->Add(lfilegenerated);
      //fOutput->Add(dataset);
   }
   else if (fRunType==TProofBench::kRunGenerateFileCleanup){

      MemInfo_t meminfo;
      if (gSystem->GetMemInfo(&meminfo)){
          Error("SlaveBegin", "Cannot get memory information, returning");
          return;
      }
      Info("SlaveBegin", "Total memory on this node: %d MB", meminfo.fMemTotal);

      //Long64_t memorytotal=(Long64_t)(meminfo.fMemTotal)*1024*1024;
      //Long64_t memorytotal=4.5*1024*1024*1024;
      Long64_t memorytotal=(Long64_t)10*1024*1024;
  
      Long64_t memorythisworker=memorytotal/fNWorkersPerNode+1;

      Long64_t bytestowrite=memorythisworker;
      Long64_t byteswritten=0;
      Int_t i=0;
      do {
         TString seed = hostname;
         seed += "_" "Cleanup";
         seed += fWorkerNumber;
         seed += "_";
         seed += i++; //serial number in case file is larger than TTree::fgMaxTreeSize

         gRandom->SetSeed(static_cast<UInt_t>(TMath::Hash(seed)));

         TString filename = fBaseDir;
         filename += "/event_tree_";
         filename += seed;
         filename += ".root";

         byteswritten=GenerateFiles(fRunType, filename, bytestowrite);
         bytestowrite-=byteswritten;
         dataset->Add(filename);
      }
      while(bytestowrite>0 && byteswritten>0);

      dataset->Lookup();
      dataset->Validate();

      lfilegenerated->Add(dataset);
      lfilegenerated->Print("a");

      fOutput->Add(lfilegenerated);
   }

   return;
}

Long64_t TSelEventGen::GenerateFiles(TProofBench::ERunType runtype, TString filename, Long64_t size)
{
//runtype is run type either TProofBench::kRunGenerateFileBench or TProofBench::kRunGenerateFileCleanup
//filename is the name of the file to be generated
//size is number of events to generate when runtype==TProofBench::kRunGenerateFileBench
// and size of the file to generate when runtype==TProofBench::kRunGenerateFileCleanup
//returns number of entries in the file when runtype==TProofBench::kRunGenerateFileBench
//returns bytes rewritten when runtype==TProofBench::kRunGenerateFileCleanup
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

   if (runtype==TProofBench::kRunGenerateFileBench){
      Info("GenerateFiles", "Generating %s with %lld event(s)", filename.Data(), size);   
      for(i=0; i<size; i++) {
         event->Build(i,fNTracksBench,0);
         eventtree->Fill();
      }
   }
   else if (runtype==TProofBench::kRunGenerateFileCleanup){
      Info("GenerateFiles", "Generating %s", filename.Data());   
      f->SetCompressionLevel(0); //no compression

      //const Long64_t maxtreesize=TTree::GetMaxTreeSize();
      const Long64_t maxtreesize=Long64_t(1024+512)*1024*1024; //hard limit 1.5 GB
      //const Long64_t maxtreesize=1024*1024; //hard limit for quick test
      printf("fileend=%lld size=%lld maxtreesize=%lld\n", fileend, size, maxtreesize);
      while (fileend<size && (fileend+buffersize)<maxtreesize){
         event->Build(i++, fNTracksCleanup,0);
         eventtree->Fill();
         fileend=f->GetEND();
      }
      Info("GenerateFiles", "Total bytes of %lld written", fileend);
   }

   savedir = gDirectory;

   f = eventtree->GetCurrentFile();
   f->cd();
   eventtree->Write();
   eventtree->SetDirectory(0);

   nentries=eventtree->GetEntries();
   //f->Write();
   //printf("current dir=%s\n", gDirectory->GetPath());
   f->Close();
   delete f;
   f = 0;
   event->Delete();
   eventtree->Delete();
   savedir->cd();

   if (runtype==TProofBench::kRunGenerateFileBench) return nentries;
   else if (runtype==TProofBench::kRunGenerateFileCleanup) return fileend;
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
