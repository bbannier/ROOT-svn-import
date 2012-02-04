// @(#)root/tmva $Id$
// Author: Alexander Voigt

#ifndef ROOT_TMVA_MethodKDTree
#define ROOT_TMVA_MethodKDTree

#ifndef ROOT_TMVA_MethodBase
#include "TMVA/MethodBase.h"
#endif

#include "TKDTree.h"

namespace TMVA {

   class MethodKDTree : public MethodBase {

   public:

      MethodKDTree(const TString& jobName,
                   const TString& methodTitle,
                   DataSetInfo& dsi,
                   const TString& theOption = "KDTree",
                   TDirectory* theTargetDir = 0);

      MethodKDTree(DataSetInfo& dsi,
                   const TString& theWeightFile,
                   TDirectory* theTargetDir = NULL);

      virtual ~MethodKDTree();

      virtual Bool_t HasAnalysisType(Types::EAnalysisType type, UInt_t numberClasses, UInt_t numberTargets);

      // training methods
      void Train(void);

      // write weights to stream
      void AddWeightsXMLTo(void* parent) const;

      using MethodBase::ReadWeightsFromStream;

      // read weights from stream
      void ReadWeightsFromStream(std::istream&) {};
      void ReadWeightsFromXML(void* wghtnode);

      // calculate the MVA value
      Double_t GetMvaValue(Double_t* err = 0, Double_t* errUpper = 0);

      // ranking of input variables
      const Ranking* CreateRanking() { return NULL; }

      // reset the method
      virtual void Reset();

   protected:

      // get help message text
      void GetHelpMessage() const;

      // calculate range searching radius
      void CalculateRadius();

      // read KDTrees from file
      void ReadKDTreesFromFile();
      TKDTreeIF* ReadClonedKDTreeFromFile(TFile*, const TString&) const;

      // write KDTrees to file
      void WriteKDTreesToFile() const;

      // Delete KDTrees
      void DeleteKDTrees();

   private:

      // the option handling methods
      void DeclareOptions();
      void DeclareCompatibilityOptions() {}
      void ProcessOptions();

      // default initialisation called by all constructors
      void Init();

      std::vector<TKDTreeIF*> fKDTree;    // grown KDTrees
      std::vector<TString> fKDTreeName;   // KDTree names
      Float_t fVolFrac;                   // range searching volume
      Float_t fBucketSize;                // number of events in terminal nodes
      Bool_t fCompress;                   // compress ROOT output file
      Float_t fRadius;                    // radius of range searching volume

      ClassDef(MethodKDTree, 0) // Multi-dimensional probability density estimator using TKDTree
   };
}

#endif
