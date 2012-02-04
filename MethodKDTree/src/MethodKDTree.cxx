// @(#)root/tmva $Id$
// Author: Alexander Voigt

#include "TMVA/MethodKDTree.h"
#include "TMVA/Tools.h"
#include "TMVA/ClassifierFactory.h"
#include "TMVA/Config.h"

#include "TFile.h"

#include <vector>
using namespace std;

REGISTER_METHOD(KDTree)

ClassImp(TMVA::MethodKDTree)

//_______________________________________________________________________
TMVA::MethodKDTree::MethodKDTree(const TString& jobName,
                                 const TString& methodTitle,
                                 DataSetInfo& dsi,
                                 const TString& theOption,
                                 TDirectory* theTargetDir)
   : MethodBase(jobName, Types::kKDTree, methodTitle, dsi, theOption, theTargetDir)
   , fKDTree()
   , fTailCut(0.001)
   , fVolFrac(0.1)
   , fBucketSize(300)
   , fCompress(kTRUE)
   , fRadius(0.0)
{
   // init KDTree objects
}

//_______________________________________________________________________
TMVA::MethodKDTree::MethodKDTree(DataSetInfo& dsi,
                                 const TString& theWeightFile,
                                 TDirectory* theTargetDir)
   : MethodBase(Types::kKDTree, dsi, theWeightFile, theTargetDir)
   , fKDTree()
   , fTailCut(0.001)
   , fVolFrac(0.1)
   , fBucketSize(300)
   , fCompress(kTRUE)
   , fRadius(0.0)
{
   // constructor from weight file
}

//_______________________________________________________________________
TMVA::MethodKDTree::~MethodKDTree(void)
{
   DeleteKDTrees();
}

//_______________________________________________________________________
void TMVA::MethodKDTree::DeleteKDTrees()
{
   // Delete all KDTrees
   for (std::vector<TKDTreeIF*>::iterator it = fKDTree.begin();
        it != fKDTree.end(); ++it) {
      delete *it;
   }
   fKDTree.clear();
}

//_______________________________________________________________________
void TMVA::MethodKDTree::Reset()
{
   // reset MethodKDTree:  delete all KDTrees
   DeleteKDTrees();
}

//_______________________________________________________________________
Bool_t TMVA::MethodKDTree::HasAnalysisType(Types::EAnalysisType type, UInt_t numberClasses, UInt_t /*numberTargets*/)
{
   // KDTree can handle classification with two classes
   if (type == Types::kClassification && numberClasses == 2) return kTRUE;
   if (type == Types::kMulticlass) return kFALSE;
   if (type == Types::kRegression) return kFALSE;
   return kFALSE;
}

//_______________________________________________________________________
void TMVA::MethodKDTree::Init(void)
{
   // default initialization called by all constructors
   fTailCut        = 0.001;    // fraction of outlier events
   fVolFrac        = 0.1;      // range searching volume
   fBucketSize     = 300;      // number of events in terminal nodes
   fCompress       = kTRUE;    // compress ROOT output file
   fRadius         = 0.0;      // radius of range searching volume

   fKDTreeName.clear();
   fKDTreeName.push_back("SignalKDTree");
   fKDTreeName.push_back("BgKDTree");
}

//_______________________________________________________________________
void TMVA::MethodKDTree::DeclareOptions()
{
   //
   // Declare MethodKDTree options
   //
   DeclareOptionRef(fTailCut = 0.001,  "TailCut",  "Fraction of outlier events that are excluded");
   DeclareOptionRef(fVolFrac = 0.1,    "VolFrac",  "Relative size of range searching volume");
   DeclareOptionRef(fBucketSize = 300, "BucketSize", "Number of events in terminal nodes");
   DeclareOptionRef(fCompress = kTRUE, "Compress", "Compress ROOT output file");
}

//_______________________________________________________________________
void TMVA::MethodKDTree::ProcessOptions()
{
   // process user options
   if (!(fTailCut >= 0. && fTailCut <= 1.)) {
      Log() << kWARNING << "TailCut not in [0,1] ==> using 0.001 instead" << Endl;
      fTailCut = 0.001;
   }
   if (!(fVolFrac >= 0. && fVolFrac <= 1.)) {
      Log() << kWARNING << "VolFrac not in [0,1] ==> using 0.1 instead" << Endl;
      fTailCut = 0.1;
   }

   if (fBucketSize > GetNEvents()) {
      Log() << kWARNING << "BucketSize < number of events ==> adjusting BucketSize to "
            << GetNEvents() << Endl;
      fBucketSize = GetNEvents();
   }
}

//_______________________________________________________________________
void TMVA::MethodKDTree::Train()
{
   // Train the method
   //
   // Create two KDTrees, one filled with signal, the other filled
   // with background events.

   static const UInt_t nKDTrees = 2;

   for (UInt_t i = 0; i < nKDTrees; ++i) {
      // i == 0 : signal
      // i == 1 : background

      // create vectors of data
      vector<vector<Float_t>*> data;
      for (UInt_t ivar = 0; ivar < GetNvar(); ++ivar) {
         data.push_back(new vector<Float_t>());
      }

      Log() << kVERBOSE << "Filling event arrays" << Endl;

      // fill event arrays
      for (Long64_t k = 0; k < GetNEvents(); ++k) {
         const Event* ev = GetEvent(k);
         if ((i == 0 && DataInfo().IsSignal(ev)) || (i == 1 && !DataInfo().IsSignal(ev))) {
            if (!(IgnoreEventsWithNegWeightsInTraining() && ev->GetWeight() <= 0)) {
               for (UInt_t ivar = 0; ivar < GetNvar(); ++ivar) {
                  data[ivar]->push_back(ev->GetValue(ivar));
               }
            }
         }
      }

      Log() << kINFO << "Creating KDTree \"" << fKDTreeName.at(i) << "\"" << Endl;

      TKDTreeIF *kdtree = new TKDTreeIF(data[0]->size(), GetNvar(), 1);
      for (UInt_t ivar = 0; ivar < GetNvar(); ++ivar) {
         kdtree->SetData(ivar, &(data[ivar]->front()));
      }

      Log() << kVERBOSE << "Building KDTree \"" << fKDTreeName.at(i) << "\" ... ";
      kdtree->Build();
      Log() << kVERBOSE << "done (" << kdtree->GetNNodes() << " nodes created)" << Endl;
      fKDTree.push_back(kdtree);
   }

   // calculate radius
   CalculateRadius();
}

//_______________________________________________________________________
void TMVA::MethodKDTree::CalculateRadius()
{
   // Calculate the radius of the range searching sphere

   const UInt_t kDim = GetNvar(); // == Data()->GetNVariables();
   Float_t *xmin = new Float_t[kDim];
   Float_t *xmax = new Float_t[kDim];

   // set default values
   for (UInt_t dim = 0; dim < kDim; ++dim) {
      xmin[dim] = FLT_MAX;
      xmax[dim] = FLT_MIN;
   }

   Log() << kDEBUG << "Number of training events: " << Data()->GetNTrainingEvents() << Endl;
   // number of events that are outside the range
   const Int_t nEventsOutside = (Int_t)(Data()->GetNTrainingEvents() * fTailCut);
   // number of bins in histograms
   const Int_t nRangeHistBins = 10000;

   // loop over all testing singnal and background events and
   // calculate minimal and maximal value of every variable
   for (Long64_t i = 0; i < GetNEvents() ; ++i) {
      const Event* ev = GetEvent(i);
      for (UInt_t dim = 0; dim < kDim; ++dim) {
         const Float_t val = ev->GetValue(dim);
         if (val < xmin[dim])
            xmin[dim] = val;
         if (val > xmax[dim])
            xmax[dim] = val;
      }
   }

   // Create and fill histograms for each dimension (with same events
   // as before), to determine range based on number of events outside
   // the range
   TH1F **range_h = new TH1F*[kDim];
   for (UInt_t dim = 0; dim < kDim; ++dim) {
      range_h[dim]  = new TH1F(Form("range%i", dim), "range", nRangeHistBins, xmin[dim], xmax[dim]);
   }

   // fill all testing events into histos
   for (Long64_t i = 0; i < GetNEvents(); ++i) {
      const Event* ev = GetEvent(i);
      for (UInt_t dim = 0; dim < kDim; ++dim) {
         range_h[dim]->Fill(ev->GetValue(dim));
      }
   }

   // calc Xmin, Xmax from Histos
   for (UInt_t dim = 0; dim < kDim; ++dim) {
      for (Int_t i = 1; i < nRangeHistBins + 1; i++) { // loop over bins
         if (range_h[dim]->Integral(0, i) > nEventsOutside) {
            // calc left limit (integral over bins 0..i = nEventsOutside)
            xmin[dim] = range_h[dim]->GetBinLowEdge(i);
            break;
         }
      }
      for (Int_t i = nRangeHistBins; i > 0; i--) {
         // calc right limit (integral over bins i..max = nEventsOutside)
         if (range_h[dim]->Integral(i, (nRangeHistBins + 1)) > nEventsOutside) {
            xmax[dim] = range_h[dim]->GetBinLowEdge(i + 1);
            break;
         }
      }
   }
   // now xmin[] and xmax[] contain upper/lower limits for every dimension

   // calculate the range searching volume
   Float_t volume = 1.0;
   for (UInt_t dim = 0; dim < kDim; ++dim) {
      volume *= xmax[dim] - xmin[dim];
   }
   volume *= fVolFrac;

   // calculate radius of n-dimensional sphere with that volume
   fRadius = TMath::Power(volume * TMath::Gamma(1 + kDim / 2.0), 1. / kDim) / TMath::Sqrt(TMath::Pi());

   // clean up
   delete[] xmin;
   delete[] xmax;
   for (UInt_t dim = 0; dim < kDim; ++dim)
      delete range_h[dim];
   delete[] range_h;
}

//_______________________________________________________________________
Double_t TMVA::MethodKDTree::GetMvaValue(Double_t* err, Double_t* errUpper)
{
   // Calculate the discriminant
   //
   //    D = N_sig / (N_bkg + N_sig)
   //
   // where
   //
   //    N_sig = # signal events in range searching sphere
   //    N_bkg = # background events in range searching sphere
   //
   // If N_bkg + N_sig == 0, then 0.5 is returned.

   const Event* ev = GetEvent();
   std::vector<Float_t> xvec(ev->GetValues());

   vector<Int_t> sigNodes;
   fKDTree.at(0)->FindInRange(&(xvec.front()), fRadius, sigNodes);
   vector<Int_t> bkgNodes;
   fKDTree.at(1)->FindInRange(&(xvec.front()), fRadius, bkgNodes);

   const size_t nSig = sigNodes.size();
   const size_t nBkg = bkgNodes.size();

   // calculate the mva value
   Float_t mvaValue = 0.;
   if (nSig + nBkg > 0)
      mvaValue = 1.0 * nSig / (nSig + nBkg);
   else
      mvaValue = 0.5; // assume 50% signal probability, if no events found (bad assumption, but can be overruled by cut on error)

   return mvaValue;
}

//_______________________________________________________________________
void TMVA::MethodKDTree::ReadWeightsFromXML(void* wghtnode)
{
   // read KDTree variables from xml weight file
   gTools().ReadAttr(wghtnode, "TailCut",           fTailCut);
   gTools().ReadAttr(wghtnode, "VolFrac",           fVolFrac);
   gTools().ReadAttr(wghtnode, "BucketSize",        fBucketSize);
   gTools().ReadAttr(wghtnode, "Compress",          fCompress);
   gTools().ReadAttr(wghtnode, "Radius",            fRadius);

   DeleteKDTrees();
   ReadKDTreesFromFile();
}

//_______________________________________________________________________
void TMVA::MethodKDTree::ReadKDTreesFromFile()
{
   // read KDTrees from file

   TString rootFileName(GetWeightFileName());

   // replace in case of txt weight file
   rootFileName.ReplaceAll(TString(".") + gConfig().GetIONames().fWeightFileExtension + ".txt", ".xml");

   // add kdtree indicator to distinguish from main weight file
   rootFileName.ReplaceAll(".xml", "_kdtree.root");

   Log() << kINFO << "Read KDTrees from file: " << gTools().Color("lightblue")
         << rootFileName << gTools().Color("reset") << Endl;

   TFile *rootFile = new TFile(rootFileName, "READ");
   if (rootFile->IsZombie())
      Log() << kFATAL << "Cannot open file \"" << rootFileName << "\"" << Endl;

   // read trees from file
   fKDTree.push_back(ReadClonedKDTreeFromFile(rootFile, fKDTreeName.at(0)));
   fKDTree.push_back(ReadClonedKDTreeFromFile(rootFile, fKDTreeName.at(1)));

   rootFile->Close();
   delete rootFile;

   for (std::vector<TKDTreeIF*>::iterator it = fKDTree.begin(); it != fKDTree.end(); ++it) {
      if (!(*it))
         Log() << kFATAL << "Could not load KDTree!" << Endl;
   }
}

//_______________________________________________________________________
TKDTreeIF* TMVA::MethodKDTree::ReadClonedKDTreeFromFile(TFile* file, const TString& treeName) const
{
   if (file == NULL) {
      Log() << kWARNING << "<ReadClonedKDTreeFromFile>: NULL pointer given" << Endl;
      return NULL;
   }

   // try to load the kdtree from the file
   TKDTreeIF *kdtree = (TKDTreeIF*) file->Get(treeName);
   if (kdtree == NULL) {
      return NULL;
   }
   // try to clone the kdtree
   kdtree = (TKDTreeIF*) kdtree->Clone();
   if (kdtree == NULL) {
      Log() << kWARNING << "<ReadClonedKDTreeFromFile>: " << treeName
            << " could not be cloned!" << Endl;
      return NULL;
   }

   return kdtree;
}

//_______________________________________________________________________
void TMVA::MethodKDTree::AddWeightsXMLTo(void* parent) const
{
   // create XML output of KDTree method variables
   void* wght = gTools().AddChild(parent, "Weights");
   gTools().AddAttr(wght, "TailCut",           fTailCut);
   gTools().AddAttr(wght, "VolFrac",           fVolFrac);
   gTools().AddAttr(wght, "BucketSize",        fBucketSize);
   gTools().AddAttr(wght, "Compress",          fCompress);
   gTools().AddAttr(wght, "Radius",            fRadius);

   WriteKDTreesToFile();
}

//_______________________________________________________________________
void TMVA::MethodKDTree::WriteKDTreesToFile() const
{
   // Write KDTrees to file

   TString rootFileName(GetWeightFileName());

   // replace in case of txt weight file
   rootFileName.ReplaceAll(TString(".") + gConfig().GetIONames().fWeightFileExtension + ".txt", ".xml");

   // add kdtree indicator to distinguish from main weight file
   rootFileName.ReplaceAll(".xml", "_kdtree.root");

   TFile *rootFile = NULL;
   if (fCompress) rootFile = new TFile(rootFileName, "RECREATE", "KDTreeFile", 9);
   else           rootFile = new TFile(rootFileName, "RECREATE");

   // write the trees
   for (UInt_t i = 0; i < fKDTree.size(); ++i) {
      Log() << kVERBOSE << "writing tree \"" << fKDTreeName.at(i) << "\" to file" << Endl;
      fKDTree.at(i)->Write(fKDTreeName.at(i).Data());
   }

   rootFile->Close();
   Log() << kINFO << "KDTrees written to file: "
         << gTools().Color("lightblue") << rootFileName
         << gTools().Color("reset") << Endl;
}

//_______________________________________________________________________
void TMVA::MethodKDTree::GetHelpMessage() const
{
}
