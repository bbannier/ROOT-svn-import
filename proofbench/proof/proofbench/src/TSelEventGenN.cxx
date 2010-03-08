#define TSelEventGenN_cxx

//////////////////////////////////////////////////////////
//
// Auxilliary TSelector used to test PROOF functionality
//
//////////////////////////////////////////////////////////

#include "TSelEventGenN.h"
#include <TDirectory.h>
#include <TDSet.h>
#include <TFile.h>
#include <TMap.h>
#include <TMath.h>
#include <TParameter.h>
#include <TProofServ.h>
#include <TString.h>
#include <TSystem.h>
#include <TTree.h>
#include <test/Event.h>

//_____________________________________________________________________________
TSelEventGenN::TSelEventGenN() : fFileType(TProofBench::kFileNotSpecified),
                                 fNTries(10), fNEvents(10000),
                                 fNWorkersPerNode(0), fWorkerNumber(0),
                                 fNTracksBench(50), fNTracksCleanup(100),
                                 fTotalGen(0)
{
   // Constructor
   /* if (gProofServ){
      fBaseDir=gProofServ->GetDataDir();
      fBaseDir.Remove(fBaseDir.Last('/'));
   }
   else{
      fBaseDir="";
   }*/
}

//_____________________________________________________________________________
TSelEventGenN::~TSelEventGenN()
{
   // Destructor

}

//_____________________________________________________________________________
void TSelEventGenN::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The trPROOF_BenchmarkNTracksCleanupee argument is deprecated (on PROOF 0 is passed).

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

//_____________________________________________________________________________
void TSelEventGenN::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

   TIter nxt(fInput);
   TString sinput;
   TObject *obj;

   while ((obj = nxt())){

      sinput=obj->GetName();
      //Info("SlaveBegin", "Input list: %s", sinput.Data());

      if (sinput.Contains("PROOF_BenchmarkFileType")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fFileType = (TProofBench::EFileType) a->GetVal();
            continue;
         }
         continue;
      }
      /* if (sinput.Contains("fBaseDir")){
         TNamed *a=dynamic_cast<TNamed*>(obj);
         if (a){
            TString basedir=a->GetTitle();
            if (gSystem->AccessPathName(basedir)) {
               Info("SlaveBegin", "No such file or directory: %s; using default directory: %s", 
                     basedir.Data(), fBaseDir.Data());
               continue;
            }
            fBaseDir = basedir;
         }
         continue;
      }
      if (sinput.Contains("fNTries")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fNTries = a->GetVal();
         }
         continue;
      }*/
      if (sinput.Contains("PROOF_BenchmarkNEvents")){
         TParameter<Long64_t>* a=dynamic_cast<TParameter<Long64_t>*>(obj);
         if (a){
            fNEvents = a->GetVal();
         }
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkNTracksBench")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fNTracksBench = a->GetVal();
         }
         continue;
      }
      if (sinput.Contains("PROOF_BenchmarkNTracksCleanup")){
         TParameter<Int_t>* a=dynamic_cast<TParameter<Int_t>*>(obj);
         if (a){
            fNTracksCleanup = a->GetVal();
         }
         continue;
      }
   }

   // Output
#if 0
   TParameter<Long64_t> *totalGen = new TParameter<Long64_t>("TotalGen", (Long64_t)0);
   fOutput->Add(totalGen);
   fTotalGen = (TObject *) totalGen;
#endif
}

//_____________________________________________________________________________
Bool_t TSelEventGenN::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either TSelEventGenN::GetEntry() or TBranch::GetEntry()
   // to read either all or the required parts of the data. When processing
   // keyed objects with PROOF, the object is already loaded and is available
   // via the fObject pointer.
   //
   // This function should contain the "body" of the analysis. It can contain
   // simple or elaborate selection criteria, run algorithms on the data
   // of the event and typically fill histograms.
   //
   // The processing can be stopped by calling Abort().
   //
   // Use fStatus to set the return value of TTree::Process().
   //
   // The return value is currently not used.

   // Link to current element, if any
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
#else
   Long64_t tg = 0;
   tg += GenerateFiles(fFileType, fCurrent->GetName(), fNEvents);
#endif

   return kTRUE;
}

//_____________________________________________________________________________
void TSelEventGenN::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

//_____________________________________________________________________________
void TSelEventGenN::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

}

//_____________________________________________________________________________
Long64_t TSelEventGenN::GenerateFiles(TProofBench::EFileType filetype,
                                      const char *filename, Long64_t size)
{
   // 'runtype' is run type either TProofBench::kRunGenerateFileBench or
   // TProofBench::kRunGenerateFileCleanup
   // 'filename' is the name of the file to be generated
   // 'size' is number of events to generate when runtype==TProofBench::kRunGenerateFileBench
   // and size of the file to generate when runtype==TProofBench::kRunGenerateFileCleanup
   // Returns number of entries in the file when runtype==TProofBench::kRunGenerateFileBench
   // Returns bytes rewritten when runtype==TProofBench::kRunGenerateFileCleanup
   // Return 0 in case error

   if (!(filetype==TProofBench::kFileCleanup 
      || filetype==TProofBench::kFileBenchmark)){
      Error("GenerateFiles", "File type '%d' not supported", filetype);
      return 0;
   }
    
   Long64_t nentries=0;
   TDirectory* savedir = gDirectory;
   //printf("current dir=%s\n", gDirectory->GetPath());

   TFile *f = new TFile(filename, "RECREATE");

   savedir->cd();

   if (!f || f->IsZombie()) return 0;

   Event *event = new Event();
   Event *ep = event;
   TTree* eventtree= new TTree("EventTree", "Event Tree");
   eventtree->SetDirectory(f);

   const Int_t buffersize=32000;
   eventtree->Branch("event", "Event", &ep, buffersize, 1);
   eventtree->AutoSave();

   Long64_t i=0;
   Long64_t fileend=0;

   if (filetype==TProofBench::kFileBenchmark){
      Info("GenerateFiles", "Generating %s with %lld event(s)", filename, size);
      for(i=0; i<size; i++) {
         event->Build(i,fNTracksBench,0);
         eventtree->Fill();
      }
   } else if (filetype==TProofBench::kFileCleanup){
      Info("GenerateFiles", "Generating %s", filename);
      f->SetCompressionLevel(0); //no compression

      //const Long64_t maxtreesize=TTree::GetMaxTreeSize();
      const Long64_t maxtreesize=Long64_t(1024+512)*1024*1024; //hard limit 1.5 GB
      //const Long64_t maxtreesize=1024*1024; //hard limit for quick test
      Info("GenerateFiles", "fileend=%lld size=%lld maxtreesize=%lld\n", fileend, size, maxtreesize);
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

   if (filetype==TProofBench::kFileBenchmark) return nentries;
   else if (filetype==TProofBench::kFileCleanup) return fileend;
   else return 0;
}
