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
   , fRadiusFraction(0.3)
   , fRadius(0.0)
   , fBucketSize(300)
   , fCompress(kTRUE)
{
   // init KDTree objects
}

//_______________________________________________________________________
TMVA::MethodKDTree::MethodKDTree(DataSetInfo& dsi,
                                 const TString& theWeightFile,
                                 TDirectory* theTargetDir)
   : MethodBase(Types::kKDTree, dsi, theWeightFile, theTargetDir)
   , fKDTree()
   , fRadiusFraction(0.3)
   , fRadius(0.0)
   , fBucketSize(300)
   , fCompress(kTRUE)
{
   // constructor from weight file
}

//_______________________________________________________________________
TMVA::MethodKDTree::~MethodKDTree(void)
{
   for (std::vector<TKDTreeIF*>::iterator it = fKDTree.begin(); it != fKDTree.end(); ++it)
      delete *it;
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
   fRadiusFraction = 0.3;
   fRadius         = 0.0;
   fBucketSize     = 300;
   fCompress       = kTRUE;  // compress ROOT output file

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
   DeclareOptionRef(fRadiusFraction = 0.3, "RadiusFraction", "Fraction of event sample size to use for the range searching radius radius");
   DeclareOptionRef(fBucketSize = 300, "BucketSize", "Number of events in terminal nodes");
   DeclareOptionRef(fCompress = kTRUE, "Compress", "Compress ROOT output file");
}

//_______________________________________________________________________
void TMVA::MethodKDTree::ProcessOptions()
{
   // process user options
   if (!(fRadiusFraction >= 0. && fRadiusFraction <= 1.)) {
      Log() << kWARNING << "Radius fraction not in [0,1] ==> using 0.3 instead" << Endl;
      fRadiusFraction = 0.3;
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
   CalcRadius();
}

//_______________________________________________________________________
void TMVA::MethodKDTree::CalcRadius()
{
   Float_t xmin = std::numeric_limits<Float_t>::max();
   Float_t xmax = std::numeric_limits<Float_t>::min();

   // loop over all testing singnal and BG events and clac minimal and
   // maximal value of every variable
   for (Long64_t i = 0; i< GetNEvents() ; ++i) {
      const Event* ev = GetEvent(i);
      for (UInt_t ivar = 0; ivar < GetNvar(); ++ivar) {
         const Float_t val = ev->GetValue(ivar);
         if (val < xmin)
            xmin = val;
         if (val > xmax)
            xmax = val;
      }
   }
   fRadius = (xmax - xmin) * fRadiusFraction;
}

//_______________________________________________________________________
Double_t TMVA::MethodKDTree::GetMvaValue(Double_t* err, Double_t* errUpper)
{
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
   gTools().ReadAttr(wghtnode, "RadiusFraction",    fRadiusFraction);
   gTools().ReadAttr(wghtnode, "Radius",            fRadius);
   gTools().ReadAttr(wghtnode, "BucketSize",        fBucketSize);
   gTools().ReadAttr(wghtnode, "Compress",          fCompress);

   ReadKDTreesFromFile();
}

//_______________________________________________________________________
void TMVA::MethodKDTree::ReadKDTreesFromFile()
{
   // read foams from file

   TString rootFileName(GetWeightFileName());

   // replace in case of txt weight file
   rootFileName.ReplaceAll(TString(".") + gConfig().GetIONames().fWeightFileExtension + ".txt", ".xml");

   // add foam indicator to distinguish from main weight file
   rootFileName.ReplaceAll(".xml", "_kdtree.root");

   Log() << kINFO << "Read KDTrees from file: " << gTools().Color("lightblue")
         << rootFileName << gTools().Color("reset") << Endl;

   TFile *rootFile = new TFile(rootFileName, "READ");
   if (rootFile->IsZombie())
      Log() << kFATAL << "Cannot open file \"" << rootFileName << "\"" << Endl;

   // read trees from file
   fKDTree.clear();
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
   gTools().AddAttr(wght, "RadiusFraction",    fRadiusFraction);
   gTools().AddAttr(wght, "Radius",            fRadius);
   gTools().AddAttr(wght, "BucketSize",        fBucketSize);
   gTools().AddAttr(wght, "Compress",          fCompress);

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
