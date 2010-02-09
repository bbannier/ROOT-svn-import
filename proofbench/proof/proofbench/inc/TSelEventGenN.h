//////////////////////////////////////////////////////////
//
// Auxilliary TSelector used to test PROOF functionality
//
//////////////////////////////////////////////////////////

#ifndef TSelEventGenN_h
#define TSelEventGenN_h

#ifndef ROOT_TProofBench
#include <TProofBench.h>
#endif
#ifndef ROOT_TSelector
#include <TSelector.h>
#endif

class TSelEventGenN : public TSelector {

private:
   TProofBench::ERunType fRunType;
   TString      fBaseDir;
   Int_t        fNTries;
   Long64_t     fNEvents;
   Int_t        fNWorkersPerNode; //total number of workers on this node
   Int_t        fWorkerNumber;    //worker number on this node
   Int_t        fNTracksBench;    //number of tracks in an event for benchmark files
   Int_t        fNTracksCleanup;  //number of tracks in an event for cleanup files

   TObject     *fTotalGen; // Events generated on this worker

   Long64_t GenerateFiles(TProofBench::ERunType, const char *filename, Long64_t nevents);

public :

   // Specific members

   TSelEventGenN();
   virtual ~TSelEventGenN();
   virtual Int_t   Version() const { return 2; }
   virtual void    Begin(TTree *tree);
   virtual void    SlaveBegin(TTree *tree);
   virtual Bool_t  Process(Long64_t entry);
   virtual void    SetOption(const char *option) { fOption = option; }
   virtual void    SetObject(TObject *obj) { fObject = obj; }
   virtual void    SetInputList(TList *input) { fInput = input; }
   virtual TList  *GetOutputList() const { return fOutput; }
   virtual void    SlaveTerminate();
   virtual void    Terminate();

   ClassDef(TSelEventGenN,0);
};

#endif
