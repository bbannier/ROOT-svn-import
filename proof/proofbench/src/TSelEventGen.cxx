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

static const Long64_t MaxTreeSizeOrg=TTree::GetMaxTreeSize();
//static const Long64_t MaxTreeSizeOrg=100*1000*1000; //100 MB max tree size for test

ClassImp(TSelEventGen)

TSelEventGen::TSelEventGen():
   fFileType(TProofBench::kFileNotSpecified),
   fBenchmarkMode(TProofBench::kModeNotSpecified),
   fBaseDir(""),
   fMaxNWorkers(0),
   fNEvents(10000),
   fNTracksBench(10),
   fNTracksCleanup(100),
   fRegenerate(kFALSE),
   fNWorkersPerNode(0),
   fWorkerNumber(0),
   fTotalGen(0),
   fDataSet(0)
{
   if (gProofServ){
      fBaseDir=gProofServ->GetDataDir();
      //2 directories up
      fBaseDir.Remove(fBaseDir.Last('/'));
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

void TSelEventGen::SlaveBegin(TTree *tree)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   printf("MaxTreeSizeOrg=%lld\n", MaxTreeSizeOrg);
   Init(tree);

   TString option = GetOption();

   //get parameters

   Bool_t found_filetype=kFALSE;
   Bool_t found_mode=kFALSE;
   Bool_t found_maxnworkers=kFALSE;
   Bool_t found_nevents=kFALSE;
   Bool_t found_ntracksbench=kFALSE;
   Bool_t found_ntrackscleanup=kFALSE;
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
            fFileType=(TProofBench::EFileType)a->GetVal();
            found_filetype=kTRUE; 
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkFileType not type TParameter<Int_t>*");
         }
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkMode")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fBenchmarkMode=a->GetVal();
            found_mode=kTRUE; 
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkMode not type TParameter<Int_t>*");
         }
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkMaxNWorkers")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fMaxNWorkers=a->GetVal();
            found_maxnworkers=kTRUE; 
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkMaxNWorkers not type TParameter<Int_t>*");
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
      if (sinput.Contains("PROOF_BenchmarkNTracksBench")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fNTracksBench=a->GetVal();
            found_ntracksbench=kTRUE; 
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkNTracksBench not type TParameter<Int_t>*");
         }
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkNTracksCleanup")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fNTracksCleanup=a->GetVal();
            found_ntrackscleanup=kTRUE; 
         }
         else{
            Error("SlaveBegin", "PROOF_BenchmarkNTracksCleanup not type TParameter<Int_t>*");
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
      Warning("Begin", "PROOF_BenchmarkFileType not found; using default: %d", fFileType);
   }
   if (!found_mode){
      Warning("Begin", "PROOF_BenchmarkMode not found; using default: %d", fBenchmarkMode);
   }
   if (!found_maxnworkers){
      Warning("Begin", "PROOF_BenchmarkMaxNWorkers not found; using default: %d", fMaxNWorkers);
   }
   if (!found_nevents){
      Warning("Begin", "PROOF_BenchmarkNEvents not found; using default: %lld", fNEvents);
   }
   if (!found_ntracksbench){
      Warning("Begin", "PROOF_BenchmarkNTracksBench not found; using default: %d", fNTracksBench);
   }
   if (!found_ntrackscleanup){
      Warning("Begin", "PROOF_BenchmarkNTracksCleanup not found; using default: %d", fNTracksCleanup);
   }
   if (!found_regenerate){
      Warning("Begin", "PROOF_BenchmarkRegenerate not found; using default: %d", fRegenerate);
   }
   if (!found_slaveinfos){
      Error("Begin", "PROOF_SlaveInfos not found");
      return;
   }

   // Output
#if 0
   TParameter<Long64_t> *totalGen = new TParameter<Long64_t>("TotalGen", (Long64_t)0);
   fOutput->Add(totalGen);
   fTotalGen = (TObject *) totalGen;
#endif

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

   fDataSet=new TDSet("TTree","EventTree");

   TTree::SetMaxTreeSize(10*MaxTreeSizeOrg);
}

Long64_t TSelEventGen::GenerateFiles(TProofBench::EFileType filetype, TString filename, Long64_t sizenevents)
{
//runtype is run type either TProofBench::kRunGenerateFileBench or TProofBench::kRunGenerateFileCleanup
//filename is the name of the file to be generated
//sizenevents is number of events to generate when runtype==TProofBench::kRunGenerateFileBench
// and size of the file to generate when runtype==TProofBench::kRunGenerateFileCleanup
//returns number of entries in the file when runtype==TProofBench::kRunGenerateFileBench
//returns bytes written when runtype==TProofBench::kRunGenerateFileCleanup
//return 0 in case error

   if (!(filetype==TProofBench::kFileBenchmark || filetype==TProofBench::kFileCleanup)){
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
   Long64_t fileend=0;
   Long64_t size_generated=0;

   f->SetCompressionLevel(0); //no compression
   //take control on file change
   //const Long64_t maxtreesize_org=TTree::GetMaxTreeSize();
   //const Long64_t maxtreesize_org=100*1024*1024;  //100 MB limit for test

   if (filetype==TProofBench::kFileBenchmark){
      Info("GenerateFiles", "Generating %s", filename.Data());   
      while (sizenevents-- && size_generated<MaxTreeSizeOrg){
         event->Build(i++,fNTracksBench,0);
         size_generated+=eventtree->Fill();
      }
      nentries=eventtree->GetEntries();
      Info("GenerateFiles", "%s generated with %lld entries", filename.Data(), nentries);
   }
   else if (filetype==TProofBench::kFileCleanup){
      Info("GenerateFiles", "Generating %s", filename.Data());   
      //while (fileend<size && (fileend+buffersize)<fMaxTreeSize){
      while (size_generated<sizenevents && size_generated<MaxTreeSizeOrg){
         event->Build(i++, fNTracksCleanup,0);
         size_generated+=eventtree->Fill();
         //fileend=f->GetEND();
      }
      Info("GenerateFiles", "%s generated with %lld bytes", filename.Data(), size_generated);
   }
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

   if (filetype==TProofBench::kFileBenchmark) return nentries;
   else if (filetype==TProofBench::kFileCleanup) return size_generated;
   else return 0;
}

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

   //check if the directory is writable
   const char* basename=gSystem->BaseName(filename.Data());
   const char* dirname=gSystem->DirName(filename.Data());
   if (gSystem->AccessPathName(dirname, kWritePermission)){
      //directory is not writable, use default directory
      Info("Process", "%s directory is not writable, using default directory: %s", 
                      dirname, fBaseDir.Data());
      filename=fBaseDir+"/"+basename;
   } 

   //generate files
   if (fFileType==TProofBench::kFileBenchmark){
      Long64_t neventstogenerate=fNEvents;

      Int_t serial=0;//serial number of file when a file becomes larger 
                     //than maximum tree size limit
      while(neventstogenerate>0){
         //increase serial number
         TString newfilename=filename;
         newfilename.Replace(newfilename.Last('_')+1, newfilename.Length(), "", newfilename.Length());
         newfilename=TString::Format("%s%d.root", newfilename.Data(), serial);

         serial++;
         if (!fRegenerate){
            //see if a file exists
            FileStat_t filestat;
            if (!gSystem->GetPathInfo(newfilename, filestat)){//stat'ed
               //Check if file is ok
               TFile f(newfilename);
               if (!f.IsZombie()){
                  Long64_t size=f.GetSize();
                  TTree* t=(TTree*)f.Get("EventTree");
                  if (size!=-1 && t){
                     Long64_t entries_file=t->GetEntries();
                     Long64_t sizetree=t->GetTotBytes();
                     //Long64_t maxtreesize=TTree::GetMaxTreeSize();
                     //Long64_t maxtreesize=100*1024*1024;//test
                     if (entries_file==neventstogenerate
                    || (entries_file<neventstogenerate && 0.9*MaxTreeSizeOrg<sizetree && sizetree<1.1*MaxTreeSizeOrg)){
                        //file size seems to be correct, skip generation
                        Info("Process", "Bench file (%s, entries=%lld) exists."
                             " Skipping generation", newfilename.Data(), entries_file);
                        neventstogenerate-=entries_file;
                        fDataSet->Add(newfilename);
                        continue;
                     }
                  }
               }
            }
         }
         neventstogenerate-=GenerateFiles(fFileType, newfilename, neventstogenerate);
         fDataSet->Add(newfilename);
      }
      //fListOfFilesGenerated->Add(fDataSet);
      //fListOfFilesGenerated->Print("a");

      //fOutput->Add(fListOfFilesGenerated);
   }
   else if (fFileType==TProofBench::kFileCleanup){

      //(re)generate files
      MemInfo_t meminfo;
      if (gSystem->GetMemInfo(&meminfo)){
          Error("SlaveBegin", "Cannot get memory information, returning");
          return kFALSE;
      }
      Info("SlaveBegin", "Total memory on this node: %d MB", meminfo.fMemTotal);

      //Long64_t memorytotal=(Long64_t)(meminfo.fMemTotal)*1024*1024;
      Long64_t memorytotal=(Long64_t)200*1024*1024;
  
      Long64_t memorythisworker=memorytotal/fNWorkersPerNode+1;
      Long64_t bytestowrite=memorythisworker;
      Long64_t byteswritten=0;

      Int_t serial=0;
      while(bytestowrite>0){

         //increase serial number
         TString newfilename=filename;
         newfilename.Replace(newfilename.Last('_')+1, newfilename.Length(), "", newfilename.Length());
         newfilename=TString::Format("%s%d.root", newfilename.Data(), serial);

         serial++;

         if (!fRegenerate){
            //see if a file exists
            FileStat_t filestat;
            if (!gSystem->GetPathInfo(newfilename, filestat)){//stat'ed
               //Check if file is ok
               //Long64_t size=filestat.fSize;
               TFile f(newfilename);
               if (!f.IsZombie()){
                  TTree* tree=(TTree*)f.Get("EventTree");
                  if (tree){
                     Long64_t sizetree=tree->GetTotBytes();
                     //Long64_t maxtreesize=TTree::GetMaxTreeSize();
                     //Long64_t maxtreesize=100*1024*1024;//test
                     if ( ((0.9*MaxTreeSizeOrg<sizetree && sizetree<1.1*MaxTreeSizeOrg)
                      || (0.9*bytestowrite<sizetree && sizetree<1.1*bytestowrite))){
                        //file size seems to be correct, skip generation
                        byteswritten=sizetree;
                        bytestowrite-=byteswritten;
                        Info("Process", "Cleanup file (%s, tree size=%lld) exists."
                             " Skipping generation", newfilename.Data(), sizetree);
                        fDataSet->Add(newfilename);
                        continue;
                     }
                  }
               }
            }
         }

         byteswritten=GenerateFiles(fFileType, newfilename, bytestowrite);
         bytestowrite-=byteswritten;
         fDataSet->Add(newfilename);
      }
   }
   else{
      Error("Process", "Run type not recognized: fFileType=%d, returning", fFileType);
      return kTRUE;
   }

   return kTRUE;
}

void TSelEventGen::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.
   TString hostname=gSystem->HostName();
   TString thisordinal=gProofServ->GetOrdinal();

   TString sfilegenerated="PROOF_FilesGenerated" "_"+hostname+"_"+thisordinal;

   TList* listoffilesgenerated=new TList;
   listoffilesgenerated->SetName(sfilegenerated);
   listoffilesgenerated->Add(fDataSet);
   fOutput->Add(listoffilesgenerated);

   //set it back
   TTree::SetMaxTreeSize(MaxTreeSizeOrg);
   //TTree::SetMaxTreeSize(1900000000);

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
   Printf("fNEvents=%lld\n", fNEvents);
   printf("fWorkerNumber=%d\n", fWorkerNumber);
   printf("fNTracksBench=%d\n", fNTracksBench);
   printf("fNTracksCleanup=%d\n", fNTracksCleanup);
   printf("fRegenerate=%d\n", fRegenerate);
   Printf("fNWorkersPerNode=%d\n", fNWorkersPerNode);
}

