// @(#)root/hist:$Id$
// Authors: Bartolomeu Rabacal    07/2010 
/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006 , LCG ROOT MathLib Team                         *
 *                                                                    *
 *                                                                    *
 **********************************************************************/
// Header file for TKDE

#ifndef ROOT_TKDE
#define ROOT_TKDE

#ifndef ROOT_Math_WrappedFunction
#include "Math/WrappedFunction.h"
#endif

#include "TF1.h"
#include "TH1D.h"
#include "Math/Math.h"

/*
   Kernel Density Estimation class. The algorithm is described in (1) "Cranmer KS, Kernel Estimation in High-Energy
   Physics. Computer Physics Communications 136:198-207,2001" - e-Print Archive: hep ex/0011057.
   A binned version of this paper's algorithm is also implemented to address the performance issue due to its data
   size dependance.
*/
class TKDE : public TNamed  {
public:
   
   enum EKernelType { // Kernel function
      kGaussian,
      kEpanechnikov,
      kBiweight,
      kCosineArch,
      kUserDefined, // Internal use only for the class's template constructor
      kTotalKernels // Internal use only for member initialization
   };
   
   enum EIteration { // KDE fitting option
      kAdaptive,
      kFixed
   };
   
   enum EMirror { // Data "mirroring" to address the probability "spill out" boundary effect
      kNoMirror,
      kMirrorLeft,
      kMirrorRight,
      kMirrorBoth,
      kMirrorAsymLeft,
      kMirrorAsymLeftRight,
      kMirrorAsymRight,
      kMirrorLeftAsymRight,
      kMirrorAsymBoth
   };
   
   enum EBinning{ 
      kUnbinned,
      kRelaxedBinning, // The algorithm is allowed to use binning if the data is large enough
      kForcedBinning
   };
   
   template<class KernelFunction>
   TKDE(const KernelFunction& kernfunc, UInt_t events, const Double_t* data, Double_t xMin = 1.0, Double_t xMax = 0.0, EIteration iter = kAdaptive, EMirror mir = kNoMirror, EBinning bin = kRelaxedBinning, Double_t rho = 1.0) {
      Instantiate(new ROOT::Math::WrappedFunction<const KernelFunction&>(kernfunc), events, data, xMin, xMax, iter, mir, bin, rho);
   }
      
   TKDE(UInt_t events, const Double_t* data, Double_t xMin = 1.0, Double_t xMax = 0.0, EKernelType kern = kGaussian, EIteration iter = kAdaptive, EMirror mir = kNoMirror, EBinning bin = kRelaxedBinning, Double_t rho = 1.0);
   
   virtual ~TKDE();
   
   void Fill(Double_t data);
   void SetKernelType(EKernelType kern);
   void SetIteration(EIteration iter);
   void SetMirror(EMirror mir);
   void SetBinning(EBinning);
   void SetNBins(UInt_t nBins);
   void SetUseBinsNEvents(UInt_t nEvents);
   void SetRange(Double_t xMin, Double_t xMax); // By default computed from the data
   
   Double_t operator()(const Double_t* x, const Double_t* p = 0) const;
   
   TH1D* GetHistogram(UInt_t nbins = 100, Double_t xMin = 1.0, Double_t xMax = 0.0);
   
   TF1* GetFunction();
   TF1* GetUpperFunction(Double_t confidenceLevel = 0.95);
   TF1* GetLowerFunction(Double_t confidenceLevel = 0.95);
   TF1* GetApproximateBias();
   
private:
   
   static const Double_t _2_PI_ROOT_INV = 0.398942280401432703; // (2*TMath::Pi())**-0.5
   static const Double_t PI             = 3.14159265358979312;  // TMath::Pi()
   static const Double_t PI_OVER2       = 1.57079632679489656;  // TMath::PiOver2()
   static const Double_t PI_OVER4       = 0.785398163397448279; // TMath::PiOver4()
   
   TKDE();                    // Disallowed default constructor
   TKDE(TKDE& kde);           // Disallowed copy constructor
   TKDE operator=(TKDE& kde); // Disallowed assign operator
   
   typedef ROOT::Math::IBaseFunctionOneDim* KernelFunction_Ptr;
   KernelFunction_Ptr fKernelFunction;
   
   friend class TKernel;
   class TKernel {
      UInt_t fNWeights; // Number of kernel weights (bandwidth as vectorized for binning)
      std::vector<Double_t> fWeights; // Kernel weights (bandwidth)
      TKDE* fKDE;
      const std::vector<Double_t> GetBinCentreData() const;
      UInt_t Index(Double_t x, UInt_t i) const;
      UInt_t Index(Double_t x) const;
   public:
      TKernel(UInt_t n, Double_t weight, TKDE* kde);
      void ComputeAdaptiveWeights();
      Double_t operator()(Double_t x) const;
      Double_t GetWeight(Double_t x) const;
   };
   
   TKernel* fKernel;
   
   std::vector<Double_t> fData; // Data events
   
   TF1* fPDF;             // Output Kernel Density Estimation PDF function
   TF1* fUpperPDF;        // Output Kernel Density Estimation upper confidence interval PDF function
   TF1* fLowerPDF;        // Output Kernel Density Estimation lower confidence interval PDF function
   TF1* fApproximateBias; // Output Kernel Density Estimation approximate bias
   
   TH1D* fHistogram; // Output data histogram
   
   EKernelType fKernelType;
   EIteration fIteration;
   EMirror fMirror;
   EBinning fBinning;
   
   Bool_t fMirrorLeft, fMirrorRight, fAsymLeft, fAsymRight;
   Bool_t fUseBins;
   
   UInt_t fNBins;          // Histograms' settable number of bins
   UInt_t fNEvents;        // Data's number of events
   UInt_t fUseBinsNEvents; // If the algorithm is allowed to use binning this is the minimum number of events to do so
   
   Double_t fMean;  // Data mean
   Double_t fSigma; // Data std deviation
   Double_t fXMin;  // Data minimum value
   Double_t fXMax;  // Data maximum value
   Double_t fRho;   // Adjustment factor for sigma
  
   Double_t fWeightSize; // Caches the weight size
   
   std::vector<Double_t> fCanonicalBandwidths;
   std::vector<Double_t> fKernelSigmas2;
   
   friend struct KernelIntegrand;
   struct KernelIntegrand {
      enum EIntegralResult{kNorm, kMu, kSigma2, kUnitIntegration};
      KernelIntegrand(const TKDE* kde, EIntegralResult intRes);
      Double_t operator()(Double_t x) const;
   private:
      const TKDE* fKDE;
      EIntegralResult fIntegralResult;
   };
   
   void Instantiate(KernelFunction_Ptr kernfunc, UInt_t events, const Double_t* data, Double_t xMin, Double_t xMax, EIteration iter, EMirror mir, EBinning bin, Double_t rho);
   
   inline Double_t GaussianKernel(Double_t x) const {
      // Returns the kernel evaluation at x 
      return _2_PI_ROOT_INV * std::exp(-.5 * x * x);
   }
   inline Double_t EpanechnikovKernel(Double_t x) const {
      return (x >= -1. &&  x <= 1.) ? 3. / 4. * (1. - x * x) : 0.0;
   }
   inline Double_t BiweightKernel(Double_t x) const {
      // Returns the kernel evaluation at x 
      return (x >= -1. &&  x <= 1.) ? 15. / 16. * (1. - x * x) * (1. - x * x) : 0.0;
   }
   inline Double_t CosineArchKernel(Double_t x) const {
      // Returns the kernel evaluation at x 
      return (x >= -1. &&  x <= 1.) ? PI_OVER4 * std::cos(PI_OVER2 * x) : 0.0;
   }
   Double_t UpperConfidenceInterval(const Double_t* x, const Double_t* p) const; // Valid if the bandwidth is small compared to nEvents**1/5
   Double_t LowerConfidenceInterval(const Double_t* x, const Double_t* p) const; // Valid if the bandwidth is small compared to nEvents**1/5
   Double_t ApproximateBias(const Double_t* x, const Double_t* p) const;
   Double_t GetError(Double_t x) const;
   Double_t ComputeKernelL2Norm() const;
   Double_t ComputeKernelSigma2() const;
   Double_t ComputeKernelMu() const;
   Double_t ComputeKernelIntegral() const;

   void CheckKernelValidity();
   void SetCanonicalBandwidth(); 
   void SetKernelSigma2(); 
   void SetCanonicalBandwidths(); 
   void SetKernelSigmas2(); 
   void SetHistogram();
   void SetUseBins();
   void SetMirror();
   void SetMean();
   void SetSigma();
   void SetKernel();
   void SetKernelFunction(KernelFunction_Ptr kernfunc = 0);
   void SetOptions(Double_t xMin, Double_t xMax, EKernelType kern, EIteration iter, EMirror mir, EBinning bin, Double_t rho, Bool_t IsUserDefinedKernel = kFALSE);
   void SetData(const Double_t* data);
   void SetMirroredData();
   
   inline void SetData(Double_t x, UInt_t i) {
      // Set data point at the i-th data vector position
      fData[i] = x;
   }
      
   TH1D* GetKDEHistogram(UInt_t nbins, Double_t xMin, Double_t xMax);
   
   TF1* GetKDEFunction();
   TF1* GetPDFUpperConfidenceInterval(Double_t confidenceLevel); // The density to estimate should be at least twice differentiable. 
   TF1* GetPDFLowerConfidenceInterval(Double_t confidenceLevel); // The density to estimate should be at least twice differentiable. 
   TF1* GetKDEApproximateBias();
   
   ClassDef(TKDE, 1) // One dimensional semi-parametric Kernel Density Estimation 
   
};

#endif
