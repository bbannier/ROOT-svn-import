#include <algorithm>
#include <numeric>
#include <limits>
#include "TKDE2D.h"
#include "Math/Error.h"
#include "Math/Functor.h"
#include "Math/Integrator.h"
#include "Math/QuantFuncMathCore.h"
#include "Math/RichardsonDerivator.h"

ClassImp(TKDE2D)

TKDE2D::TKDE2D(UInt_t events, const Double_t* x, const Double_t* y, Double_t xMin, Double_t xMax, Double_t yMin, Double_t yMax, EKernelType kern, EIteration iter, EMirror mir, EBinning bin, Double_t rho) :
   fDataTree(new TKDTreeID(events, 2, 50)),
   fXData(events, 0.0),
   fYData(events, 0.0),
   fPDF(0),
   fUpperPDF(0), // ??
   fLowerPDF(0), // ??
   fApproximateBias(0), // ??
   fHistogram(0), // ??
   fNEvents(events),
   fMaxBucketsPoints(1000),
   fNBuckets(3),
   fUseBinsNEvents(3000),
   fXMean(0.0),
   fXSigma(0.0),
   fYMean(0.0),
   fYSigma(0.0),
   fCanonicalBandwidths(std::vector<Double_t>(kTotalKernels, 0.0)),
   fKernelSigmas2(std::vector<Double_t>(kTotalKernels, -1.0))
{
   //Class constructor
   SetOptions(xMin, xMax, yMin, yMax, kern, iter, mir, bin, rho);
   SetKernelFunction();
   SetData(x, y);
   SetCanonicalBandwidths();
   SetKernelSigmas2();
   SetKernel();
}
   
TKDE2D::~TKDE2D() {
   //Class destructor
   if (fHistogram)        delete fHistogram;
   if (fPDF)              delete fPDF;
   if (fUpperPDF)         delete fUpperPDF;
   if (fLowerPDF)         delete fLowerPDF;
   if (fApproximateBias)  delete fApproximateBias;
   delete fKernelFunction;
   delete fKernel;
}
  
void TKDE2D::SetOptions(Double_t xMin, Double_t xMax, Double_t yMin, Double_t yMax, EKernelType kern, EIteration iter, EMirror mir, EBinning bin, Double_t rho, Bool_t IsUserDefinedKernel) {
   // Sets User global options
   fXMin = xMin;
   fXMax = xMax;
   fYMin = yMin;
   fYMax = yMax; 

   if (!(IsUserDefinedKernel) && !(kern >= kGaussian && kern < kUserDefined)) {
      this->Error("TKDE2D::SetOptions", "Illegal user kernel type input - use template constructor for user defined kernel!");
      exit(EXIT_FAILURE);
   }
   fKernelType = kern;
   if (iter != kAdaptive && iter != kFixed) {
      this->Error("TKDE2D::SetOptions", "Illegal user iteration type input!");
      exit(EXIT_FAILURE);
   }
   fIteration = iter;
   if (!(mir >= kNoMirror && mir <= kMirrorAsymBoth)) {
      this->Error("TKDE2D::SetOptions", "Illegal user mirroring type input!");
      exit(EXIT_FAILURE);
   }
   fBinning = bin;
   SetUseBins();
   if (rho <= 0.0) {
      MATH_ERROR_MSG("TKDE2D::SetOptions", "rho cannot be non-positive!!" << std::endl);
      exit(EXIT_FAILURE);
   }
   fRho = rho;
}

void TKDE2D::SetKernelType(EKernelType kern) {
   // Sets User option for the choice of kernel estimator    
   fKernelType = kern;
   SetKernel();
}
   
void TKDE2D::SetIteration(EIteration iter) {
   // Sets User option for fixed or adaptive iteration      
   fIteration = iter;
   SetKernel();
}
      
void TKDE2D::Instantiate(KernelFunction_Ptr kernfunc, UInt_t events, const Double_t* x, const Double_t* y, Double_t xMin, Double_t xMax, Double_t yMin, Double_t yMax, EIteration iter, EMirror mir, EBinning bin, Double_t rho) {
   // Template's constructor surrogate
   fDataTree = new TKDTreeID(events, 2, 50);
   fXData = std::vector<Double_t>(events, 0.0);
   fYData = std::vector<Double_t>(events, 0.0);
   fPDF = 0;
   fUpperPDF = 0; // ??
   fLowerPDF = 0; // ??
   fApproximateBias = 0; // ??
   fHistogram = 0; // ??
   fMaxBucketsPoints = 1000;
   fNBuckets = 3;
   fUseBinsNEvents = 3000;
   fXData = std::vector<Double_t>(events, 0.0);
   fYData = std::vector<Double_t>(events, 0.0);
   fXMean = 0.0;
   fXSigma = 0.0;
   fYMean = 0.0;
   fYSigma = 0.0;
   fCanonicalBandwidths = std::vector<Double_t>(kTotalKernels, 0.0);
   fKernelSigmas2 = std::vector<Double_t>(kTotalKernels, -1.0);
   SetOptions(xMin, xMax, yMin, yMax, kUserDefined, iter, mir, bin, rho, kTRUE);
   SetKernelFunction(kernfunc);
   SetData(x, y);
   SetCanonicalBandwidths();
   SetKernelSigmas2();
}

void TKDE2D::SetMirror(EMirror mir) {
   // Sets User option for mirroring the data
   fMirror = mir;
   fMirrorLeft  = fMirror == kMirrorLeft  || fMirror == kMirrorBoth || fMirror == kMirrorLeftAsymRight;
   fMirrorRight = fMirror == kMirrorRight || fMirror == kMirrorBoth || fMirror == kMirrorAsymLeftRight;
   fAsymLeft    = fMirror == kMirrorAsymLeft  || fMirror == kMirrorAsymLeftRight || fMirror == kMirrorAsymBoth;
   fAsymRight   = fMirror == kMirrorAsymRight || fMirror == kMirrorLeftAsymRight || fMirror == kMirrorAsymBoth;
   SetKernel();
}

void TKDE2D::SetUseBins() {
   // Sets User option for using binned weights  
   switch (fBinning) {
      default:
      case kRelaxedBinning:
         if (fNEvents >= fUseBinsNEvents) {
            fUseBins = kTRUE;
         } else {
            fUseBins = kFALSE;
         }
         break;
      case kForcedBinning:
         fUseBins = kTRUE;
         break;
      case kUnbinned:
         fUseBins = kFALSE;
   }
}

void TKDE2D::SetDataTree(const Double_t* x, const Double_t* y) {
   // Sets 2D-Tree data and builds it
   fDataTree->SetData(0, const_cast<Double_t*>(x));
   fDataTree->SetData(1, const_cast<Double_t*>(y));
   fDataTree->Build();
}

void TKDE2D::SetData(const Double_t* x, const Double_t* y) {
   // Sets the data events input sample
   for (UInt_t i = 0; i < fNEvents; ++i) {
      SetData(x[i], y[i], i);
   }
   if (fXMin >= fXMax) {
      fXMin = *std::min_element(fXData.begin(), fXData.end());
      fXMax = *std::max_element(fXData.begin(), fXData.end());
   }
   if (fYMin >= fYMax) {
      fYMin = *std::min_element(fYData.begin(), fYData.end());
      fYMax = *std::max_element(fYData.begin(), fYData.end());
   }
   SetMean();
   SetSigma();
   if (fMirror != kNoMirror) {
      SetMirroredData();
   }
   if (fUseBins) {
      SetDataTree(x, y);
   }
}

void TKDE2D::SetMirroredData() {
   // Mirrors the data
   if (fMirrorLeft) {
      fXData.resize(2 * fNEvents, 0.0);
      fYData.resize(2 * fNEvents, 0.0);
      for (UInt_t i = 0; i < fNEvents; ++i) {
         SetData(2 * fXMin - fXData[i], 2 * fYMin - fYData[i], i + fNEvents);
      }
   }
   if (fMirrorRight) {
      fXData.resize((fMirrorLeft + 2) * fNEvents, 0.0);
      fYData.resize((fMirrorLeft + 2) * fNEvents, 0.0);
      for (UInt_t i = 0; i < fNEvents; ++i) {
         SetData(2 * fXMax - fXData[i], 2 * fYMax - fYData[i], i + (fMirrorLeft + 1) * fNEvents);
      }   
   }
   fNEvents *= (fMirrorLeft + fMirrorRight + 1);
}
// void TKDE2D::BinData(const Double_t* x, const Double_t* y) {
//    fNBuckets = fNEvents / fMaxBucketsPoints; // For now stick to the case where fNEvents % fMaxBucketsPoints == 0
//    SetDataTree(x, y);
// }

void TKDE2D::SetMean() {
   // Computes input data's mean
   fXMean = accumulate(fXData.begin(), fXData.end(), 0.0) / fXData.size();
   fYMean = accumulate(fYData.begin(), fYData.end(), 0.0) / fYData.size();
}
   
void TKDE2D::SetSigma() { 
   // Computes input data's sigma
   fXSigma = TMath::Sqrt(1. / (fXData.size() - 1) * (inner_product(fXData.begin(), fXData.end(), fXData.begin(), 0.0) - fXData.size() * TMath::Power(fXMean, 2)));
   fYSigma = TMath::Sqrt(1. / (fYData.size() - 1) * (inner_product(fYData.begin(), fYData.end(), fYData.begin(), 0.0) - fYData.size() * TMath::Power(fYMean, 2)));
}

void TKDE2D::SetKernel() {
   // Sets the kernel density estimator
   Double_t weight(fCanonicalBandwidths[kGaussian] * fXSigma * TMath::Power( 3. / (8. * TMath::Sqrt(TMath::Pi())) * fNEvents, -0.2));// Optimal bandwidth (Silverman's rule of thumb with assumed Gaussian density)
   weight *= fCanonicalBandwidths[fKernelType] / fCanonicalBandwidths[kGaussian];
   UInt_t n = fNEvents; 
   fKernel = new TKernel(n, weight, this);
   if (fIteration == kAdaptive) {
      fKernel->ComputeAdaptiveWeights();
   }
}
   
void TKDE2D::SetKernelFunction(KernelFunction_Ptr kernfunc) {
   // Sets kernel estimator
   switch (fKernelType) {
      case kGaussian :
         fKernelFunction = new ROOT::Math::WrappedMemFunction<TKDE2D, Double_t (TKDE2D::*)(Double_t) const>(*this, &TKDE2D::GaussianKernel);
         break;
      case kEpanechnikov :
         fKernelFunction = new ROOT::Math::WrappedMemFunction<TKDE2D, Double_t (TKDE2D::*)(Double_t) const>(*this, &TKDE2D::EpanechnikovKernel);
         break;
      case kBiweight :
         fKernelFunction = new ROOT::Math::WrappedMemFunction<TKDE2D, Double_t (TKDE2D::*)(Double_t) const>(*this, &TKDE2D::BiweightKernel);
         break;
      case kCosineArch :
         fKernelFunction = new ROOT::Math::WrappedMemFunction<TKDE2D, Double_t (TKDE2D::*)(Double_t) const>(*this, &TKDE2D::CosineArchKernel);
         break;
      case kUserDefined :
      case kTotalKernels :
      default:
         fKernelFunction = kernfunc;
         if (fKernelFunction) {
            CheckKernelValidity();
            SetCanonicalBandwidth();
            SetKernelSigma2();
            SetKernel();
         } else {
            MATH_ERROR_MSG("TKDE2D::SetKernelFunction", "Undefined user kernel function input!" << std::endl);
            exit(EXIT_FAILURE);
         }
   }
}

void TKDE2D::SetCanonicalBandwidths() {
   // Sets the canonical bandwidths according to the kernel type
   fCanonicalBandwidths[kGaussian] = 0.7764;     // Checked in Mathematica
   fCanonicalBandwidths[kEpanechnikov] = 1.7188; // Checked in Mathematica
   fCanonicalBandwidths[kBiweight] = 2.03617;    // Checked in Mathematica
   fCanonicalBandwidths[kCosineArch] = 1.7663;   // Checked in Mathematica
}

void TKDE2D::SetKernelSigmas2() {
   // Sets the kernel sigmas2 according to the kernel type
   fKernelSigmas2[kGaussian] = 1.0;
   fKernelSigmas2[kEpanechnikov] = 1.0 / 5.0;
   fKernelSigmas2[kBiweight] = 1.0 / 7.0;
   fKernelSigmas2[kCosineArch] = 1.0 - 8.0 / TMath::Power(TMath::Pi(), 2);
}

// TH2D* TKDE2D::GetHistogram(UInt_t nbins, Double_t xMin, Double_t xMax) {
//    // Returns the histogram
//    return fHistogram ? fHistogram : fHistogram = GetKDEHistogram(nbins, xMin, xMax);
// }

void TKDE2D::SetRange(Double_t xMin, Double_t xMax, Double_t yMin, Double_t yMax) {
   // Sets minimum range value and maximum range value
   if (xMin >= xMax) {
      MATH_ERROR_MSG("TKDE2D::SetRange", "X minimum range cannot be bigger or equal than the maximum range!" << std::endl);
      return;
   }
   if (yMin >= yMax) {
      MATH_ERROR_MSG("TKDE2D::SetRange", "Y minimum range cannot be bigger or equal than the maximum range!" << std::endl);
      return;
   }
   fXMin = xMin;
   fXMax = xMax;
   fYMin = yMin;
   fYMax = yMax;
   SetKernel();
}

TF2* TKDE2D::GetFunction() {
   // Returns the PDF estimate
   return fPDF ? fPDF : GetKDEFunction();
}

// TF2* TKDE2D::GetUpperFunction(Double_t confidenceLevel) {
//    // Returns the PDF upper estimate (upper confidence interval limit)
//    return fUpperPDF ? fUpperPDF : GetPDFUpperConfidenceInterval(confidenceLevel);
// }
// 
// TF2* TKDE2D::GetLowerFunction(Double_t confidenceLevel) {
//    // Returns the PDF lower estimate (lower confidence interval limit)
//    return fLowerPDF ? fLowerPDF : GetPDFLowerConfidenceInterval(confidenceLevel);
// }
// 
// TF2* TKDE2D::GetApproximateBias() {
//    // Returns the PDF estimate bias
//    return fApproximateBias ? fApproximateBias : GetKDEApproximateBias();
// }

void TKDE2D::Fill(Double_t x, Double_t y) {
   // Fills data member with User input data event
   fXData.push_back(x);
   fYData.push_back(y);
   fNEvents++;
   fXMin = std::min(x, fXMin);
   fXMin = std::min(x, fXMin);
   fYMax = std::max(y, fYMin);
   fYMax = std::max(y, fYMin);
   SetMean();
   SetSigma();
   SetKernel();
}

Double_t TKDE2D::operator()(const Double_t* x, const Double_t*) const {
   // The class's unary function: returns the kernel density estimate
   return (*fKernel)(x);
}

TKDE2D::TKernel::TKernel(UInt_t n, Double_t weight, TKDE2D* kde) :
   // Internal class constructor
   fNWeights(n),
   fWeights(n, weight),
   fKDE(kde)
{}
   
// void TKDE2D::TKernel::ComputeAdaptiveWeights() {
//    // Gets the adaptive weights (bandwidths) for TKernel internal computation
//    std::vector<Double_t> weights = fWeights;
//    std::vector<Double_t>::iterator weight = weights.begin();
//    std::vector<Double_t> dataset(fKDE->fData); // ???
//    std::vector<Double_t>::iterator data = dataset.begin();
// //    Double_t minWeight(*weight * TMath::Power(50.0, -0.5)); // = 0.1414 ~ 0.5 - TMath::Erf(1/sqrt(2.))/2.
//    Double_t norm(2.0 * TMath::Sqrt(3.0)); // Adaptive weight normalization TODO: find source of justification: makes a difference in TProfile
//    for (; weight != weights.end(); ++weight, ++data) {
//       Double_t f = (*fKDE->fKernel)(*data);
//       if (f > 0.) {
//          *weight *= fKDE->fRho / fKDE->fSigma * TMath::Power(fKDE->fSigma / f, 0.5) / norm;
//       }
//    }
//    fWeights = weights;
// }

// Double_t TKDE2D::TKernel::GetWeight(Double_t x) const {
//    // Returns the bandwidth
//    return fWeights[Index(x)];
// }

Double_t TKDE2D::TKernel::operator()(const Double_t* x) const {
   // The internal class's unary function: returns the kernel density estimate
   Double_t result(0.0);
   Int_t inode = fKDE->FindTreeNode(x);
   Int_t* index = fKDE->fDataTree->GetPointsIndexes(inode);
   for (Int_t ipoint = 0; ipoint < fKDE->fDataTree->GetNPointsNode(inode); ++ipoint) {
      Double_t prod_result = 1.;  
      for (Int_t idim = 0; idim < 1; ++idim) {
         prod_result *= 1. / fWeights[idim] * (*fKDE->fKernelFunction)((x[idim] - fKDE->fXData[index[ipoint]]/*, idim*/) / fWeights[idim]);
   //TODO       if (fKDE->fAsymLeft) {
   //          result -= 1. / fWeights[j] * (*fKDE->fKernelFunction)((x - (2 * fKDE->fXMin - fKDE->fData[index[ipoint])) / fWeights[j]);
   //       }
   //TODO       if (fKDE->fAsymRight) {
   //          result -= 1. / fWeights[j] * (*fKDE->fKernelFunction)((x - (2 * fKDE->fXMax - fKDE->fData[index[ipoint])) / fWeights[j]);
   //       }
      }
      result += prod_result;
   }
   return result / fKDE->fNEvents;
}

Int_t TKDE2D::FindTreeNode(const Double_t* x) {
//    UInt_t terminalNodeStart = fKDE->fDataTree->GetNNodes(), terminalNodeEnd = fKDE->fDataTree->GetTotalNodes();
   return fDataTree->FindNode(x); //TODO:  find distance d to nearest bucket; find NN with dist < d; return this bucket
}

/*Double_t TKDE2D::TKernel::Eval(Double_t x) {
   return 1. / fWeights[j] * (*fKDE->fKernelFunction)((x - fKDE->fData[index[ipoint]) / fWeights[j]);
}*/
      
// Double_t TKDE2D::UpperConfidenceInterval(const Double_t* x, const Double_t* p) const {
//    // Returns the pointwise upper estimated density
//    Double_t f = this->operator()(x);
//    Double_t fsigma = GetError(*x);
//    Double_t z = ROOT::Math::normal_quantile(*p, 1.0);
//    return f + z * fsigma;
// }
// 
// Double_t TKDE2D::LowerConfidenceInterval(const Double_t* x, const Double_t* p) const {
//    // Returns the pointwise lower estimated density
//    Double_t f = this->operator()(x);
//    Double_t fsigma = GetError(*x);
//    Double_t z = ROOT::Math::normal_quantile(*p, 1.0);
//    return f - z * fsigma;
// }
// 
// Double_t TKDE2D::ApproximateBias(const Double_t* x, const Double_t*) const {
//    // Returns the pointwise approximate estimated density bias
//    ROOT::Math::Functor1D kern(this->fKernel, &TKDE2D::TKernel::operator());
//    ROOT::Math::RichardsonDerivator rd;
//    rd.SetFunction(kern);
//    Double_t df2 = rd.Derivative2(*x);
//    Double_t weight = fKernel->GetWeight(*x); // Bandwidth
//    return  0.5 * fKernelSigmas2[fKernelType] * TMath::Power(weight, 2) * df2;
// }
// Double_t TKDE2D::GetError(Double_t x) const {
//    // Returns the pointwise variance of estimated density
//    Double_t kernelL2Norm = ComputeKernelL2Norm();
//    Double_t f = this->operator()(&x);
//    Double_t weight = fKernel->GetWeight(x); // Bandwidth
//    Double_t result = f * kernelL2Norm / (fNEvents * weight); 
//    return TMath::Sqrt(result);
// }

void TKDE2D::CheckKernelValidity() {
   // Checks if kernel has unit integral, mu = 0 and positive finite sigma conditions
   Double_t mu = ComputeKernelMu();
   Double_t sigma2 = ComputeKernelSigma2();
   Double_t unity = ComputeKernelUnitIntegration();
   Double_t valid = unity == 1.0 && mu == 0.0  && sigma2 > 0 && sigma2 != std::numeric_limits<Double_t>::infinity();//with which tolerance??
   if (!valid) {
      MATH_ERROR_MSG("TKDE2D::CheckKernelValidity", "No valid conditions: either the kernel's mu is not zero or the kernel's sigma2 is not finite positive or the kernel's integration is not 1! Unsuitable kernel." << std::endl);
      exit(EXIT_FAILURE);
   }
}

Double_t TKDE2D::ComputeKernelL2Norm() const {
   // Computes the kernel's L2 norm
   ROOT::Math::IntegratorOneDim ig;
   KernelIntegrand kernel(this, TKDE2D::KernelIntegrand::kNorm);
   ig.SetFunction(kernel);
   Double_t result = ig.Integral();
   return result;
}

Double_t TKDE2D::ComputeKernelSigma2() const {
   // Computes the kernel's sigma squared
   ROOT::Math::IntegratorOneDim ig;
   KernelIntegrand kernel(this, TKDE2D::KernelIntegrand::kSigma2);
   ig.SetFunction(kernel);
   Double_t result = ig.Integral();
   return result;
}
   
Double_t TKDE2D::ComputeKernelMu() const {
   // Computes the kernel's mu
   ROOT::Math::IntegratorOneDim ig;
   KernelIntegrand kernel(this, TKDE2D::KernelIntegrand::kMu);
   ig.SetFunction(kernel);
   Double_t result = ig.Integral();
   return result;
}

Double_t TKDE2D::ComputeKernelUnitIntegration() const {
   // Computes the kernel's integral which ought to be unity
   ROOT::Math::IntegratorOneDim ig;
   KernelIntegrand kernel(this, TKDE2D::KernelIntegrand::kUnitIntegration);
   ig.SetFunction(kernel);
   Double_t result = ig.Integral();
   return result;
}

void TKDE2D::SetCanonicalBandwidth() {
   // Computes the user's input kernel function canonical bandwidth
   fCanonicalBandwidths[kUserDefined] = TMath::Power(ComputeKernelL2Norm() / TMath::Power(ComputeKernelSigma2(), 2), 1. / 5.);
}

void TKDE2D::SetKernelSigma2() {
   // Computes the user's input kernel function sigma2
   fKernelSigmas2[kUserDefined] = ComputeKernelSigma2();
}

TKDE2D::KernelIntegrand::KernelIntegrand(const TKDE2D* kde, EIntegralResult intRes) : fKDE(kde), fIntegralResult(intRes) {
   // Internal class constructor
}
   
Double_t TKDE2D::KernelIntegrand::operator()(Double_t x) const {
   // Internal class unary function
   if (fIntegralResult == kNorm) {
     return TMath::Power((*fKDE->fKernelFunction)(x), 2);
   } else if (fIntegralResult == kMu) {
      return x * (*fKDE->fKernelFunction)(x);
   } else if (fIntegralResult == kSigma2) {
      return TMath::Power(x, 2) * (*fKDE->fKernelFunction)(x);
   } else if (fIntegralResult == kUnitIntegration) {
      return (*fKDE->fKernelFunction)(x);
   } else {
      return -1;
   }
}

// TH2D* TKDE2D::GetKDEHistogram(UInt_t nbins, Double_t xMin, Double_t xMax) {
//    // Returns the histogram of the estimated density at data points
//    if (xMin < xMax) {
//       fHistogram = new TH2D("KDE Histogram", "KDE Histogram", /*nbins > 0 ? nbins : fNBins,*/ xMin, xMax);
//    } else {
//       fHistogram = new TH2D("KDE Histogram", "KDE Histogram", /*nbins > 0 ? nbins : fNBins, fXMin,*/ fXMax);  
//    }
//    for (std::vector<Double_t>::iterator data = fData.begin(); data != fData.end(); ++data) {
//       fHistogram->Fill(*data, (*fKernel)(*data));
//    }
//    fHistogram->Scale(1. / fHistogram->Integral(), "width");
//    return fHistogram;
// }
   
TF2* TKDE2D::GetKDEFunction() {
   //Returns the estimated density 
   return fPDF = new TF2("KDE2D_Func", this, &TKDE2D::operator(), fXMin, fXMax, fYMin, fYMax, 0, "TKDE2D", "operator()");
}

// TF2* TKDE2D::GetPDFUpperConfidenceInterval(Double_t confidenceLevel) {
//    // Returns the upper estimated density 
//    fUpperPDF = new TF2("KDE_UpperFunc", this, &TKDE2D::UpperConfidenceInterval, fXMin, fXMax, 1, "TKDE2D", "UpperConfidenceInterval");
//    fUpperPDF->SetParameter(0, confidenceLevel);
//    return fUpperPDF;
// }
// 
// TF2* TKDE2D::GetPDFLowerConfidenceInterval(Double_t confidenceLevel) {
//    // Returns the upper estimated density 
//    fLowerPDF = new TF2("KDE_LowerFunc", this, &TKDE2D::LowerConfidenceInterval, fXMin, fXMax, 1, "TKDE2D", "LowerConfidenceInterval");
//    fLowerPDF->SetParameter(0, confidenceLevel);
//    return fLowerPDF;
// }
// 
// TF2* TKDE2D::GetKDEApproximateBias(){
//    // Returns the approximate bias
//    return fApproximateBias = new TF2("KDE_ApproxBias", this, &TKDE2D::ApproximateBias, fXMin, fXMax, 0, "TKDE2D", "ApproximateBias");;
// }
