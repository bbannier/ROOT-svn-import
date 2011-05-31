// @(#)root/roostats:$Id$
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_HypoTestInverterNew
#define ROOSTATS_HypoTestInverterNew



#ifndef ROOSTATS_IntervalCalculator
#include "RooStats/IntervalCalculator.h"
#endif

#ifndef ROOSTATS_HypoTestCalculatorGeneric
#include "RooStats/HypoTestCalculatorGeneric.h"
#endif

#ifndef  ROOSTATS_HypoTestInverterResult
#include "RooStats/HypoTestInverterResult.h"
#endif

class RooRealVar;
class TGraphErrors;

#include <memory>

namespace RooStats {

class HypoTestInverterNew : public IntervalCalculator {

public:

   enum ECalculatorType { kUndefined = 0, kHybrid = 1, kFrequentist = 2};

   // default constructor (used only for I/O)
   HypoTestInverterNew();


   // constructor
   HypoTestInverterNew( HypoTestCalculatorGeneric & myhc0,
                        RooRealVar& scannedVariable, 
                        double size = 0.05) ;


   virtual HypoTestInverterResult* GetInterval() const; 


   //bool RunAutoScan( double xMin, double xMax, double target, double epsilon=0.005, unsigned int numAlgorithm=0 );

   void Clear();

   // set for a fixed scan in nbins
   void SetFixedScan(int nBins, double xMin = 1, double xMax = -1) {
      fNBins = nBins; 
      fXmin = xMin; fXmax = xMax; 
   }
     
   // set auto scan (default) 
   void SetAutoScan() { SetFixedScan(0); }

   bool RunFixedScan( int nBins, double xMin, double xMax ) const;

   bool RunOnePoint( double thisX, bool afaptive = false, double clTarget = -1 ) const;

   bool RunLimit(double &limit, double &limitErr, double absTol = 0, double relTol = 0, const double *hint=0) const; 

   void UseCLs( bool on = true) { fUseCLs = on; if (fResults) fResults->UseCLs(on);   }

   virtual void  SetData(RooAbsData &) { } // not needed
     
   virtual void SetModel(const ModelConfig &) { } // not needed 

   // set the size of the test (rate of Type I error) ( Eg. 0.05 for a 95% Confidence Interval)
   virtual void SetTestSize(Double_t size) {fSize = size; if (fResults) fResults->SetTestSize(size); }
   // set the confidence level for the interval (eg. 0.95 for a 95% Confidence Interval)
   virtual void SetConfidenceLevel(Double_t cl) {fSize = 1.-cl;  if (fResults) fResults->SetConfidenceLevel(cl); }
   // Get the size of the test (eg. rate of Type I error)
   virtual Double_t Size() const {return fSize;}
   // Get the Confidence level for the test
   virtual Double_t ConfidenceLevel()  const {return 1.-fSize;}
 
   // destructor
   virtual ~HypoTestInverterNew() ;

   // set verbose level (0,1,2)
   void SetVerbose(int level=1) { fVerbose = level; }
    
protected:
    
   void CreateResults() const; 

   // run the hybrid at a single point
   HypoTestResult * Eval( HypoTestCalculatorGeneric &hc, bool adaptive , double clsTarget) const;

   // template<class HypoTestCalcType> 
   // void AddMoreToys(HypoTestCalcType & hc, HypoTestResult & hcResult, 
   //                  double clsTarget, double & clsMid, double & clsMidErr);

    

#ifdef LATER



   struct Setup {
      RooStats::ModelConfig modelConfig, modelConfig_bonly;
      std::auto_ptr<RooAbsPdf> nuisancePdf;
      std::auto_ptr<RooStats::TestStatistic> qvar;
      std::auto_ptr<RooStats::ToyMCSampler>  toymcsampler;
      std::auto_ptr<RooStats::ProofConfig> pc;
   };


   std::auto_ptr<HybridCalculator> Create(RooWorkspace *w, ModelConfig *mc_s, ModelConfig *mc_b, RooAbsData &data,
                                          double rVal, HypoTestInverterNew::Setup &setup);


#endif


private:


   static unsigned int fgNToys;
   static double fgCLAccuracy;
   static double fgAbsAccuracy;
   static double fgRelAccuracy;
   static std::string fgAlgo;

   // graph, used to compute the limit, not just for plotting!
   mutable std::auto_ptr<TGraphErrors> fLimitPlot;  //! plot of limits
    
    
   // performance counter: remember how many toys have been thrown
   mutable unsigned int fPerf_totalToysRun;

    

   HypoTestCalculatorGeneric* fCalculator0;   // pointer to the calculator passed in the constructor
   RooRealVar* fScannedVariable;     // pointer to the constrained variable
   mutable HypoTestInverterResult* fResults;
     
   HypoTestResult *readToysFromFile(double rValue=0);

   bool fUseCLs;
   double fSize;
   int fVerbose;
   bool fSystematics;
   bool fReadToysFromHere;
   ECalculatorType fCalcType; 
   int fNBins;
   double fXmin; 
   double fXmax; 

protected:

   ClassDef(HypoTestInverterNew,1)  // HypoTestInverterNew class

};
}

#endif
