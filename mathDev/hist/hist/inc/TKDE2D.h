// @(#)root/hist:$Id: TKDE2D.h 34049 2010-06-22 10:11:32Z moneta $
// Authors: Bartolomeu Rabacal    07/2010 
/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006 , LCG ROOT MathLib Team                         *
 *                                                                    *
 *                                                                    *
 **********************************************************************/
// Header file for TKDE2D

#ifndef ROOT_TKDE2D
#define ROOT_TKDE2D

#ifndef ROOT_Math_WrappedFunction
#include "Math/WrappedFunction.h"
#endif

#include "TF2.h"
#include "TH2D.h"
// #include "TFoam.h"
#include "TKDTree.h"
#include "Math/Math.h"


/*
   Two dimensional Kernel Density Estimation class. The algorithm is described in (1) "Cranmer KS, Kernel Estimation in High-Energy
   Physics. Computer Physics Communications 136:198-207,2001" - e-Print Archive: hep ex/0011057.
*/
class TKDE2D : public TNamed  {
public:

   enum EKernelType { // Kernel function. 1D since the 2D kernel is either a product of 1D kernels or a 1D kernel evaluated at ||(x,y)||
      kUserDefined, // Internal use only for the class's template constructor
      kGaussian,
      kEpanechnikov,
      kBiweight,
      kCosineArch,
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
   TKDE2D(const KernelFunction& kernfunc, UInt_t events, const Double_t* x, const Double_t* y, Double_t xMin = 1.0, Double_t xMax = 0.0, Double_t yMin = 1.0, Double_t yMax = 0.0, EIteration iter = kAdaptive, EMirror mir = kNoMirror, EBinning bin = kRelaxedBinning, Double_t rho = 1.0) {
      Instantiate(new ROOT::Math::WrappedFunction<const KernelFunction&>(kernfunc), events, x, y, xMin, xMax, yMin, yMax, iter, mir, rho);
   }
      
   TKDE2D(UInt_t events, const Double_t* x, const Double_t* y, Double_t xMin = 1.0, Double_t xMax = 0.0, Double_t yMin = 1.0, Double_t yMax = 0.0, EKernelType kern = kGaussian, EIteration iter = kAdaptive, EMirror mir = kNoMirror, EBinning bin = kRelaxedBinning, Double_t rho = 1.0);
   
   virtual ~TKDE2D();
   
   void Fill(Double_t x, Double_t y);
   void SetKernelType(EKernelType kern);
   void SetIteration(EIteration iter);
   void SetMirror(EMirror mir);
   void SetRange(Double_t xMin, Double_t xMax, Double_t yMin, Double_t yMax); // By default computed from the data
   
   Double_t operator()(const Double_t* x, const Double_t* p = 0) const;
   
//    TH2D* GetHistogram(UInt_t nxbins = 100, UInt_t nybins = 100, Double_t xMin = 1.0, Double_t xMax = 0.0, Double_t yMin = 1.0, Double_t yMax = 0.0);
   
   TF2* GetFunction();
   TF2* GetUpperFunction(Double_t confidenceLevel = 0.95);
   TF2* GetLowerFunction(Double_t confidenceLevel = 0.95);
   TF2* GetApproximateBias();
   
private:
   
   static const Double_t _2_PI_ROOT_INV = 0.398942280401432703; // (2*TMath::Pi())**-0.5
   static const Double_t PI             = 3.14159265358979312;  // TMath::Pi()
   static const Double_t PI_OVER2       = 1.57079632679489656;  // TMath::PiOver2()
   static const Double_t PI_OVER4       = 0.785398163397448279; // TMath::PiOver4()
   
   TKDE2D();                      // Disallowed default constructor
   TKDE2D(TKDE2D& kde);           // Disallowed copy constructor
   TKDE2D operator=(TKDE2D& kde); // Disallowed assign operator
   
   typedef ROOT::Math::IBaseFunctionOneDim* KernelFunction_Ptr;
   KernelFunction_Ptr fKernelFunction;
   
//    TFoam fFoam     // Used for simulation either with input distribution TKDE2D's TF2 PDF or user input data 
   TKDTreeID* fDataTree; // Auxilliary 2-D tree for data "binning"
   
   friend class TKernel;
   class TKernel {
      std::vector<Double_t> fNWeights; // Kernel weights (bandwidth)
      std::vector<Double_t> fWeights; // Kernel weights (bandwidth)
      TKDE2D* fKDE;
      const std::vector<Double_t> GetBinCentreData() const;
      UInt_t Index(Double_t x, UInt_t i) const;
      UInt_t Index(Double_t x) const;
   public:
      TKernel(UInt_t n, Double_t weight, TKDE2D* kde);
      void ComputeAdaptiveWeights();
      Double_t operator()(const Double_t* x) const;
      Double_t GetWeight(Double_t x) const;
   };
   
   TKernel* fKernel;
   
   std::vector<Double_t> fXData; // Abscissa data events
   std::vector<Double_t> fYData; // Ordenate data events
   
   TF2* fPDF;             // Output Kernel Density Estimation PDF function
   TF2* fUpperPDF;        // Output Kernel Density Estimation upper confidence interval PDF function
   TF2* fLowerPDF;        // Output Kernel Density Estimation lower confidence interval PDF function
   TF2* fApproximateBias; // Output Kernel Density Estimation approximate bias
   
   TH2D* fHistogram;      // Output data histogram
   
   EKernelType fKernelType;
   EIteration fIteration;
   EMirror fMirror;
   EBinning fBinning;
   
   Bool_t fMirrorLeft, fMirrorRight, fAsymLeft, fAsymRight;
   Bool_t fUseBins;
   
   UInt_t fNEvents;          // Data's number of events
   UInt_t fMaxBucketsPoints; // Maximum number of data points kept in the tree's leafs, default overridable
   UInt_t fNBuckets;         // Number of tree leafs, that is, the data "bins", default overridable
   UInt_t fUseBinsNEvents;   // If the algorithm is allowed to use binning this is the minimum number of events to do so
         
   Double_t fXMean;  // X Data mean
   Double_t fXSigma; // X Data std deviation
   Double_t fYMean;  // Y Data mean
   Double_t fYSigma; // Y Data std deviation
   Double_t fXMin;   // X Data minimum value
   Double_t fXMax;   // X Data maximum value
   Double_t fYMin;   // Y Data minimum value
   Double_t fYMax;   // Y Data maximum value
   Double_t fRho;    // Adjustment factor for sigma
   
   std::vector<Double_t> fCanonicalBandwidths;
   std::vector<Double_t> fKernelSigmas2;
   
   friend struct KernelIntegrand;
   struct KernelIntegrand {
      enum EIntegralResult{kNorm, kMu, kSigma2, kUnitIntegration};
      KernelIntegrand(const TKDE2D* kde, EIntegralResult intRes);
      Double_t operator()(Double_t x) const;
   private:
      const TKDE2D* fKDE;
      EIntegralResult fIntegralResult;
   };
   
   void Instantiate(KernelFunction_Ptr kernfunc, UInt_t events, const Double_t* x, const Double_t* y, Double_t xMin, Double_t xMax, Double_t yMin, Double_t yMax, EIteration iter, EMirror mir, EBinning bin, Double_t rho);
  
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
   Double_t ComputeKernelUnitIntegration() const;
   
   Int_t FindTreeNode(const Double_t* x);
         
   void CheckOptions();
   void CheckKernelValidity();
   void SetCanonicalBandwidth(); 
   void SetKernelSigma2(); 
   void SetCanonicalBandwidths(); 
   void SetKernelSigmas2(); 
   void SetHistogram();
   void SetUseBins();
   void SetMean();
   void SetSigma();
   void SetKernel();
   void SetKernelFunction(KernelFunction_Ptr kernfunc = 0);
   void SetOptions(Double_t xMin, Double_t xMax, Double_t yMin, Double_t yMax, EKernelType kern, EIteration iter, EMirror mir, EBinning bin, Double_t rho, Bool_t IsUserDefinedKernel = kFALSE);
   void SetDataTree(const Double_t* x, const Double_t* y);
   void SetData(const Double_t* x, const Double_t* y);
   void SetMirroredData();
   
   inline void SetData(Double_t x, Double_t y, UInt_t i) {
   // Set data point at the i-th data vector position
      fXData[i] = x;
      fXData[i] = y;
   }
      
   TH2D* GetKDEHistogram(UInt_t nxbins, Double_t xMin, Double_t xMax, UInt_t nybins, Double_t yMin, Double_t yMax);
   
   TF2* GetKDEFunction();
   TF2* GetPDFUpperConfidenceInterval(Double_t confidenceLevel); // The density to estimate should be at least twice differentiable. 
   TF2* GetPDFLowerConfidenceInterval(Double_t confidenceLevel); // The density to estimate should be at least twice differentiable. 
   TF2* GetKDEApproximateBias();
   
   ClassDef(TKDE2D, 1) // One dimensional semi-parametric Kernel Density Estimation 
   
};

#endif
