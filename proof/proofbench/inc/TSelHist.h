//////////////////////////////////////////////////////////
//
// Example of TSelector implementation to do generic
// processing (filling a set of histograms in this case)
//
//////////////////////////////////////////////////////////

#ifndef ROOT_TSelHist
#define ROOT_TSelHist

#ifndef ROOT_TSelector
#include <TSelector.h>
#endif

class TH1F;
class TH2F;
class TH3F;
class TRandom3;
class TCanvas;

class TSelHist : public TSelector {
public :

   // Specific members
   Int_t            fNhist;
   TH1F           **fHist1D;//[fNhist]
   TH2F           **fHist2D;//[fNhist]
   TH3F           **fHist3D;//[fNhist]
   TRandom3        *fRandom;
   Bool_t 	fDraw;
   Int_t	fHistType;
   TCanvas	*fCHist1D;
   TCanvas	*fCHist2D;
   TCanvas	*fCHist3D;

   TSelHist();
   virtual ~TSelHist();
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
   virtual void    Print(Option_t* option = "") const;

   ClassDef(TSelHist,0);
};

#endif
