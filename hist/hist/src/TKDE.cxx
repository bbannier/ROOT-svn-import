#include <algorithm>
#include <functional>
#include <numeric>
#include <limits>

#include "Math/Error.h"
#include "Math/Functor.h"
#include "Math/Integrator.h"
#include "Math/QuantFuncMathCore.h"
#include "Math/RichardsonDerivator.h"

#include "TKDE.h"


ClassImp(TKDE)

const Double_t TKDE::_2_PI_ROOT_INV = 0.398942280401432703;
const Double_t TKDE::PI             = 3.14159265358979312;
const Double_t TKDE::PI_OVER2       = 1.57079632679489656;
const Double_t TKDE::PI_OVER4       = 0.785398163397448279;

class TKDE::TKernel {
   TKDE* fKDE;
   UInt_t fNWeights; // Number of kernel weights (bandwidth as vectorized for binning)
   std::vector<Double_t> fWeights; // Kernel weights (bandwidth)
   UInt_t Index(Double_t x) const;
   public:
      TKernel(Double_t weight, TKDE* kde);
      void ComputeAdaptiveWeights();
      Double_t operator()(Double_t x) const;
      Double_t GetWeight(Double_t x) const;
};

struct TKDE::KernelIntegrand {
   enum EIntegralResult{kNorm, kMu, kSigma2, kUnitIntegration};
   KernelIntegrand(const TKDE* kde, EIntegralResult intRes);
   Double_t operator()(Double_t x) const;
   private:
      const TKDE* fKDE;
      EIntegralResult fIntegralResult;
};

TKDE::TKDE(UInt_t events, const Double_t* data, Double_t xMin, Double_t xMax, EKernelType kern, EIteration iter, EMirror mir, EBinning bin, Double_t rho) :
   fData(events, 0.0),
   fPDF(0),
   fUpperPDF(0),
   fLowerPDF(0),
   fApproximateBias(0),
   fHistogram(0),
   fNBins(events < 10000 ? events : events / 10),
   fNEvents(events),
   fUseBinsNEvents(10000),
   fMean(0.0),
   fSigma(0.0),
   fXMin(xMin),
   fXMax(xMax),
   fAdaptiveBandwidthFactor(1.0),
   fCanonicalBandwidths(std::vector<Double_t>(kTotalKernels, 0.0)),
   fKernelSigmas2(std::vector<Double_t>(kTotalKernels, -1.0))
{
   //Class constructor
   SetOptions(kern, iter, mir, bin, rho);
   SetKernelFunction();
   SetData(data);
   SetCanonicalBandwidths();
   SetKernelSigmas2();
   SetKernel();
}
   
TKDE::~TKDE() {
   //Class destructor
   if (fHistogram)        delete fHistogram;
   if (fPDF)              delete fPDF;
   if (fUpperPDF)         delete fUpperPDF;
   if (fLowerPDF)         delete fLowerPDF;
   if (fApproximateBias)  delete fApproximateBias;
   delete fKernelFunction;
   delete fKernel;
}
  
void TKDE::SetOptions(EKernelType kern, EIteration iter, EMirror mir, EBinning bin, Double_t rho, Bool_t isUserDefinedKernel) {
   // Sets User global options
   if (!(isUserDefinedKernel) && !(kern >= kGaussian && kern < kUserDefined)) {
      this->Error("TKDE::SetOptions", "Illegal user kernel type input! Use template constructor for user defined kernel.");
      exit(EXIT_FAILURE);
   }
   fKernelType = kern;
   if (iter != kAdaptive && iter != kFixed) {
      this->Error("TKDE::SetOptions", "Illegal user iteration type input!");
      exit(EXIT_FAILURE);
   }
   fIteration = iter;
   if (!(mir >= kNoMirror && mir <= kMirrorAsymBoth)) {
      this->Error("TKDE::SetOptions", "Illegal user mirroring type input!");
      exit(EXIT_FAILURE);
   }
   fMirror = mir;
   SetMirror();
   if (!(bin >= kUnbinned && bin <= kForcedBinning)) {
      this->Error("TKDE::SetOptions", "Illegal user binning type input!");
      exit(EXIT_FAILURE);
   }
   fBinning = bin;
   SetUseBins();
   if (fUseBins && fUseMirroring) {
      this->Warning("TKDE::SetOptions", "Data mirroring option cannot be used with data binning one! Mirroring ignored.");
      fUseMirroring = kFALSE;
   }
//    if (rho <= 0.0) {
//       MATH_ERROR_MSG("TKDE::SetOptions", "rho cannot be non-positive!" << std::endl);
//       exit(EXIT_FAILURE);
//    }
   fRho = rho;
}

void TKDE::SetKernelType(EKernelType kern) {
   // Sets User option for the choice of kernel estimator    
   fKernelType = kern;
   SetKernel();
}
   
void TKDE::SetIteration(EIteration iter) {
   // Sets User option for fixed or adaptive iteration      
   fIteration = iter;
   SetKernel();
}
      
void TKDE::Instantiate(KernelFunction_Ptr kernfunc, UInt_t events, const Double_t* data, Double_t xMin, Double_t xMax, EIteration iter, EMirror mir, EBinning bin, Double_t rho) {
   // Template's constructor surrogate
   fData = std::vector<Double_t>(events, 0.0);
   fPDF = 0;
   fUpperPDF = 0;
   fLowerPDF = 0;
   fHistogram = 0;
   fApproximateBias = 0;
   fNBins = events < 10000 ? events : events / 10;
   fNEvents = events;
   fUseBinsNEvents = 10000;
   fMean = 0.0;
   fSigma = 0.0;
   fXMin = xMin;
   fXMax = xMax;
   fAdaptiveBandwidthFactor = 1.;
   fCanonicalBandwidths = std::vector<Double_t>(kTotalKernels, 0.0);
   fKernelSigmas2 = std::vector<Double_t>(kTotalKernels, -1.0);
   SetOptions(kUserDefined, iter, mir, bin, rho, kTRUE);
   SetKernelFunction(kernfunc);
   SetData(data);
   SetCanonicalBandwidths();
   SetKernelSigmas2();
}

void TKDE::SetMirror(EMirror mir) {
   // Sets User option for mirroring the data
   fMirror = mir;
   SetMirror();
   SetKernel();
}

void TKDE::SetMirror() {
   // Sets the mirroring
   fMirrorLeft   = fMirror == kMirrorLeft      || fMirror == kMirrorBoth          || fMirror == kMirrorLeftAsymRight;
   fMirrorRight  = fMirror == kMirrorRight     || fMirror == kMirrorBoth          || fMirror == kMirrorAsymLeftRight;
   fAsymLeft     = fMirror == kMirrorAsymLeft  || fMirror == kMirrorAsymLeftRight || fMirror == kMirrorAsymBoth;
   fAsymRight    = fMirror == kMirrorAsymRight || fMirror == kMirrorLeftAsymRight || fMirror == kMirrorAsymBoth;
   fUseMirroring = fMirrorLeft || fMirrorRight || fAsymLeft || fAsymRight || fAsymRight;
}
      
void TKDE::SetBinning(EBinning bin) {
   // Sets User option for binning the weights
   fBinning = bin;
   SetUseBins();
   SetKernel();
}
   
void TKDE::SetUseBins() {
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

void TKDE::SetNBins(UInt_t nBins) {
   // Sets User option for number of bins    
   fNBins = nBins;
   SetKernel();
}

void TKDE::SetUseBinsNEvents(UInt_t nEvents) {
   fUseBinsNEvents = nEvents;
   SetUseBins();
   SetKernel();
}

void TKDE::SetData(const Double_t* data) {
   // Sets the data events input sample or bin centres for binned option and computes basic estimators
   if (fXMin >= fXMax) {
      fXMin = *std::min_element(data, data + fNEvents);
      fXMax = *std::max_element(data, data + fNEvents);
   }
   Double_t midspread = ComputeMidspread(data, fNEvents);
   SetMean(data, fNEvents);
   SetSigma(data, fNEvents, midspread);
   fWeightSize = (fUseBins ? fNBins : fNEvents) / (fXMax - fXMin);
   if (fUseBins) {
      fData = GetBinCentreData();
   } else {
      for (UInt_t i = 0; i < fNEvents; ++i) {
         SetData(data[i], i);
      }
      if (fMirror != kNoMirror) {
         SetMirroredData();
      }
   }
}
   
void TKDE::SetMirroredData() {
   // Mirrors the data
   if (fMirrorLeft) {
      fData.resize(2 * fNEvents, 0.0);
      for (UInt_t i = 0; i < fNEvents; ++i) {
         SetData(2 * fXMin - fData[i], i + fNEvents);
      }
   }
   if (fMirrorRight) {
      fData.resize((fMirrorLeft + 2) * fNEvents, 0.0);
      for (UInt_t i = 0; i < fNEvents; ++i) {
         SetData(2 * fXMax - fData[i], i + (fMirrorLeft + 1) * fNEvents);
      }   
   }
   fNEvents *= (fMirrorLeft + fMirrorRight + 1);
}

void TKDE::SetMean(const Double_t* data, UInt_t dataSize) {
   // Computes input data's mean
   fMean = std::accumulate(data, data + dataSize, 0.0) / dataSize;
}
   
void TKDE::SetSigma(const Double_t* data, UInt_t dataSize, Double_t R) { 
   // Computes input data's sigma
   fSigma = std::sqrt(1. / (dataSize - 1.) * (std::inner_product(data, data + dataSize, data, 0.0) - dataSize * std::pow(fMean, 2.)));
   fSigma = std::min(fSigma,  R / 1.34); // Sigma's robust estimator
}

void TKDE::SetKernel() {
   // Sets the kernel density estimator
   UInt_t n = fUseBins ? fNBins : fNEvents;
   //Optimal bandwidth (Silverman's rule of thumb with assumed Gaussian density) :
   Double_t weight(fCanonicalBandwidths[kGaussian] * fSigma * std::pow(3. / (8. * std::sqrt(PI)) * n, -0.2)); 
   weight *= fCanonicalBandwidths[fKernelType] / fCanonicalBandwidths[kGaussian];
   fKernel = new TKernel(weight, this);
   if (fIteration == kAdaptive) {
      fKernel->ComputeAdaptiveWeights();
   }
}
   
void TKDE::SetKernelFunction(KernelFunction_Ptr kernfunc) {
   // Sets kernel estimator
   switch (fKernelType) {
      case kGaussian :
         fKernelFunction = new ROOT::Math::WrappedMemFunction<TKDE, Double_t (TKDE::*)(Double_t) const>(*this, &TKDE::GaussianKernel);
         break;
      case kEpanechnikov :
         fKernelFunction = new ROOT::Math::WrappedMemFunction<TKDE, Double_t (TKDE::*)(Double_t) const>(*this, &TKDE::EpanechnikovKernel);
         break;
      case kBiweight :
         fKernelFunction = new ROOT::Math::WrappedMemFunction<TKDE, Double_t (TKDE::*)(Double_t) const>(*this, &TKDE::BiweightKernel);
         break;
      case kCosineArch :
         fKernelFunction = new ROOT::Math::WrappedMemFunction<TKDE, Double_t (TKDE::*)(Double_t) const>(*this, &TKDE::CosineArchKernel);
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
            MATH_ERROR_MSG("TKDE::SetKernelFunction", "Undefined user kernel function input!" << std::endl);
            exit(EXIT_FAILURE);
         }
   }
}

void TKDE::SetCanonicalBandwidths() {
   // Sets the canonical bandwidths according to the kernel type
   fCanonicalBandwidths[kGaussian] = 0.7764;     // Checked in Mathematica
   fCanonicalBandwidths[kEpanechnikov] = 1.7188; // Checked in Mathematica
   fCanonicalBandwidths[kBiweight] = 2.03617;    // Checked in Mathematica
   fCanonicalBandwidths[kCosineArch] = 1.7663;   // Checked in Mathematica
}

void TKDE::SetKernelSigmas2() {
   // Sets the kernel sigmas2 according to the kernel type
   fKernelSigmas2[kGaussian] = 1.0;
   fKernelSigmas2[kEpanechnikov] = 1.0 / 5.0;
   fKernelSigmas2[kBiweight] = 1.0 / 7.0;
   fKernelSigmas2[kCosineArch] = 1.0 - 8.0 / std::pow(PI, 2);
}

TH1D* TKDE::GetHistogram(UInt_t nbins, Double_t xMin, Double_t xMax) {
   // Returns the histogram
   return fHistogram ? fHistogram : fHistogram = GetKDEHistogram(nbins, xMin, xMax);
}

void TKDE::SetRange(Double_t xMin, Double_t xMax) {
   // Sets minimum range value and maximum range value
   if (xMin >= xMax) {
      MATH_ERROR_MSG("TKDE::SetRange", "Minimum range cannot be bigger or equal than the maximum range! Present range values remain the same." << std::endl);
      return;
   }
   fXMin = xMin;
   fXMax = xMax;
   SetKernel();
}

TF1* TKDE::GetFunction() {
   // Returns the PDF estimate
   return fPDF ? fPDF : GetKDEFunction();
}

TF1* TKDE::GetUpperFunction(Double_t confidenceLevel) {
   // Returns the PDF upper estimate (upper confidence interval limit)
   return fUpperPDF ? fUpperPDF : GetPDFUpperConfidenceInterval(confidenceLevel);
}

TF1* TKDE::GetLowerFunction(Double_t confidenceLevel) {
   // Returns the PDF lower estimate (lower confidence interval limit)
   return fLowerPDF ? fLowerPDF : GetPDFLowerConfidenceInterval(confidenceLevel);
}

TF1* TKDE::GetApproximateBias() {
   // Returns the PDF estimate bias
   return fApproximateBias ? fApproximateBias : GetKDEApproximateBias();
}

void TKDE::Fill(Double_t data) {
   // Fills data member with User input data event for the unbinned option
   if (fUseBins) {
      this->Warning("TKDE::Fill", "Cannot fill data with data binned option. Data input ignored.");
      return;
   }
   fData.push_back(data);
   ((fSigma *= fSigma) *= (fNEvents - 1.)) += fNEvents * fMean * fMean;
   ((fMean *= fNEvents++) += data) /= fNEvents;
   (fSigma += data * data - fNEvents * fMean * fMean) /= (fNEvents - 1.);
   fXMin = std::min(data, fXMin);
   fXMax = std::max(data, fXMin);
   SetKernel();
}

Double_t TKDE::operator()(const Double_t* x, const Double_t*) const {
   // The class's unary function: returns the kernel density estimate
   return (*fKernel)(*x);
}

TKDE::TKernel::TKernel(Double_t weight, TKDE* kde) :
   // Internal class constructor
   fKDE(kde),
   fNWeights(kde->fData.size()),
   fWeights(fNWeights, weight)  
{}
   
void TKDE::TKernel::ComputeAdaptiveWeights() {
   // Gets the adaptive weights (bandwidths) for TKernel internal computation
   std::vector<Double_t> weights = fWeights;
   std::vector<Double_t>::iterator weight = weights.begin();
   Double_t minWeight = *weight * std::sqrt(0.02);
   std::vector<Double_t>::iterator data = fKDE->fData.begin();
   //Double_t x = fKDE->fRho / fKDE->fSigma * std::sqrt(fKDE->fSigma);
   Double_t f = 0.0;
   for (; weight != weights.end(); ++weight, ++data) {
      f =  (*fKDE->fKernel)(*data);
      *weight = *weight  / std::sqrt(f); 
      fKDE->fAdaptiveBandwidthFactor += std::log(f);
   }
   fKDE->fAdaptiveBandwidthFactor = std::sqrt(std::exp(fKDE->fAdaptiveBandwidthFactor / fKDE->fData.size())); 
   for (; weight != weights.end(); ++weight, ++data) {
      if (fKDE->fRho >  0) 
         *weight = *weight * fKDE->fRho * fKDE->fAdaptiveBandwidthFactor;
      else 
         *weight = *weight * std::abs(fKDE->fRho); 

      *weight = std::max(*weight, minWeight);
   }
   //transform(weights.begin(), weights.end(), fWeights.begin(), std::bind2nd(std::multiplies<Double_t>(), ));
 }

Double_t TKDE::TKernel::GetWeight(Double_t x) const {
   // Returns the bandwidth
   return fWeights[Index(x)];
}

std::vector<Double_t> TKDE::GetBinCentreData() {
   // Gets the bins' centres from the data for using with the binned option
   std::vector<Double_t> result(fNBins, 0.0);
   Double_t binWidth((fXMax - fXMin) / fNBins);
   for (UInt_t i = 0; i < fNBins; ++i) {
      result[i] = fXMin + i * binWidth + 0.5 * binWidth;
   }
   return result;
}

Double_t TKDE::TKernel::operator()(Double_t x) const {
   // The internal class's unary function: returns the kernel density estimate
   Double_t result(0.0);
   for (UInt_t i = 0; i < (fKDE->fUseBins ? fKDE->fNBins : fKDE->fNEvents); ++i) {
      result += 1. / fWeights[i] * (*fKDE->fKernelFunction)((x - fKDE->fData[i]) / fWeights[i]);
      if (fKDE->fAsymLeft) {
         result -= 1. / fWeights[i] * (*fKDE->fKernelFunction)((x - (2 * fKDE->fXMin - fKDE->fData[i])) / fWeights[i]);
      }
      if (fKDE->fAsymRight) {
         result -= 1. / fWeights[i] * (*fKDE->fKernelFunction)((x - (2 * fKDE->fXMax - fKDE->fData[i])) / fWeights[i]);
      }
   }
   return result / fKDE->fNEvents;
}

UInt_t TKDE::TKernel::Index(Double_t x) const {
   // Returns the indices (bins) for the binned weights
   Int_t bin = Int_t((x - fKDE->fXMin) * fKDE->fWeightSize);
   if (bin == (Int_t)fNWeights) {
      return --bin;
   }
   if (fKDE->fUseMirroring && bin < 0) { // Left Mirrored Data
      return bin *= -1;
   } else if (fKDE->fUseMirroring && bin > (Int_t)fNWeights) { // Right Mirrored Data
      return bin -= fNWeights ;
   }
   return bin;
}

Double_t TKDE::UpperConfidenceInterval(const Double_t* x, const Double_t* p) const {
   // Returns the pointwise upper estimated density
   Double_t f = this->operator()(x);
   Double_t fsigma = GetError(*x);
   Double_t z = ROOT::Math::normal_quantile(*p, 1.0);
   return f + z * fsigma;
}

Double_t TKDE::LowerConfidenceInterval(const Double_t* x, const Double_t* p) const {
   // Returns the pointwise lower estimated density
   Double_t f = this->operator()(x);
   Double_t fsigma = GetError(*x);
   Double_t z = ROOT::Math::normal_quantile(*p, 1.0);
   return f - z * fsigma;
}

Double_t TKDE::ApproximateBias(const Double_t* x, const Double_t*) const {
   // Returns the pointwise approximate estimated density bias
   ROOT::Math::Functor1D kern(this->fKernel, &TKDE::TKernel::operator());
   ROOT::Math::RichardsonDerivator rd;
   rd.SetFunction(kern);
   Double_t df2 = rd.Derivative2(*x);
   Double_t weight = fKernel->GetWeight(*x); // Bandwidth
   return  0.5 * fKernelSigmas2[fKernelType] * std::pow(weight, 2) * df2;
}
Double_t TKDE::GetError(Double_t x) const {
   // Returns the pointwise variance of estimated density
   Double_t kernelL2Norm = ComputeKernelL2Norm();
   Double_t f = this->operator()(&x);
   Double_t weight = fKernel->GetWeight(x); // Bandwidth
   Double_t result = f * kernelL2Norm / (fNEvents * weight); 
   return std::sqrt(result);
}

void TKDE::CheckKernelValidity() {
   // Checks if kernel has unit integral, mu = 0 and positive finite sigma conditions
   Double_t valid = kTRUE;
   Double_t unity = ComputeKernelIntegral();
   valid = valid && unity == 1.;
   if (!valid) {
      MATH_ERROR_MSG("TKDE::CheckKernelValidity", "Kernel's integral is " << unity << std::endl);
   }
   Double_t mu = ComputeKernelMu();
   valid = valid && mu == 0.;
   if (!valid) {
      MATH_ERROR_MSG("TKDE::CheckKernelValidity", "Kernel's mu is " << mu << std::endl);
   }
   Double_t sigma2 = ComputeKernelSigma2();
   valid = valid && sigma2 > 0 && sigma2 != std::numeric_limits<Double_t>::infinity();
   if (!valid) {
      MATH_ERROR_MSG("TKDE::CheckKernelValidity", "Kernel's sigma2 is " << sigma2 << std::endl);
   }
   if (!valid) {
      MATH_ERROR_MSG("TKDE::CheckKernelValidity", "Validation conditions: the kernel's integral must be 1, the kernel's mu must be zero and the kernel's sigma2 must be finite positive to be a suitable kernel." << std::endl);
      exit(EXIT_FAILURE);
   }
}

Double_t TKDE::ComputeKernelL2Norm() const {
   // Computes the kernel's L2 norm
   ROOT::Math::IntegratorOneDim ig(ROOT::Math::IntegrationOneDim::kGAUSS);
   KernelIntegrand kernel(this, TKDE::KernelIntegrand::kNorm);
   ig.SetFunction(kernel);
   Double_t result = ig.Integral();
   return result;
}

Double_t TKDE::ComputeKernelSigma2() const {
   // Computes the kernel's sigma squared
   ROOT::Math::IntegratorOneDim ig( ROOT::Math::IntegrationOneDim::kGAUSS);
   KernelIntegrand kernel(this, TKDE::KernelIntegrand::kSigma2);
   ig.SetFunction(kernel);
   Double_t result = ig.Integral();
   return result;
}
   
Double_t TKDE::ComputeKernelMu() const {
   // Computes the kernel's mu
   ROOT::Math::IntegratorOneDim ig(ROOT::Math::IntegrationOneDim::kGAUSS);
   KernelIntegrand kernel(this, TKDE::KernelIntegrand::kMu);
   ig.SetFunction(kernel);
   Double_t result = ig.Integral();
   return result;
}

Double_t TKDE::ComputeKernelIntegral() const {
   // Computes the kernel's integral which ought to be unity
   ROOT::Math::IntegratorOneDim ig(ROOT::Math::IntegrationOneDim::kGAUSS);
   KernelIntegrand kernel(this, TKDE::KernelIntegrand::kUnitIntegration);
   ig.SetFunction(kernel);
   Double_t result = ig.Integral();
   return result;
}

Double_t TKDE::ComputeMidspread (const Double_t* data, UInt_t dataSize) const {
   std::vector<Double_t> sample(data, data + dataSize);
   sort(sample.begin(), sample.end());
   UInt_t midpoint = dataSize / 2;
   UInt_t quarterpoint = midpoint / 2;
   Double_t lowerquartile = (midpoint % 2 != 0 ? sample[quarterpoint + 1] : (sample[quarterpoint] + sample[quarterpoint + 1]) / 2);
   Double_t upperquartile = (midpoint % 2 != 0 ? sample[quarterpoint + midpoint + 1] : (sample[quarterpoint + midpoint] + sample[quarterpoint + midpoint + 1]) / 2);
   return upperquartile - lowerquartile;
}

void TKDE::SetCanonicalBandwidth() {
   // Computes the user's input kernel function canonical bandwidth
   fCanonicalBandwidths[kUserDefined] = std::pow(ComputeKernelL2Norm() / std::pow(ComputeKernelSigma2(), 2), 1. / 5.);
}

void TKDE::SetKernelSigma2() {
   // Computes the user's input kernel function sigma2
   fKernelSigmas2[kUserDefined] = ComputeKernelSigma2();
}

TKDE::KernelIntegrand::KernelIntegrand(const TKDE* kde, EIntegralResult intRes) : fKDE(kde), fIntegralResult(intRes) {
   // Internal class constructor
}
   
Double_t TKDE::KernelIntegrand::operator()(Double_t x) const {
   // Internal class unary function
   if (fIntegralResult == kNorm) {
     return std::pow((*fKDE->fKernelFunction)(x), 2);
   } else if (fIntegralResult == kMu) {
      return x * (*fKDE->fKernelFunction)(x);
   } else if (fIntegralResult == kSigma2) {
      return std::pow(x, 2) * (*fKDE->fKernelFunction)(x);
   } else if (fIntegralResult == kUnitIntegration) {
      return (*fKDE->fKernelFunction)(x);
   } else {
      return -1;
   }
}

TH1D* TKDE::GetKDEHistogram(UInt_t nbins, Double_t xMin, Double_t xMax) {
   // Returns the histogram of the estimated density at data points
   if (xMin < xMax) {
      fHistogram = new TH1D("KDE Histogram", "KDE Histogram", nbins > 0 ? nbins : 100 /*fNBins*/, xMin, xMax);
   } else {
      fHistogram = new TH1D("KDE Histogram", "KDE Histogram", nbins > 0 ? nbins : 100 /*fNBins*/, fXMin, fXMax);  
   }
   for (Int_t bin = 1; bin <= fHistogram->GetNbinsX(); ++bin) {
      Double_t binCenter = fHistogram->GetBinCenter(bin);
      fHistogram->Fill(binCenter, (*fKernel)(binCenter));
   }
   fHistogram->Scale(1. / fHistogram->Integral(), "width");
   return fHistogram;
}
   
TF1* TKDE::GetKDEFunction() {
   //Returns the estimated density 
   fPDF = new TF1("KDE_Func", this, &TKDE::operator(), fXMin, fXMax, 0, "TKDE", "operator()");
   return fPDF;
}

TF1* TKDE::GetPDFUpperConfidenceInterval(Double_t confidenceLevel) {
   // Returns the upper estimated density 
   fUpperPDF = new TF1("KDE_UpperFunc", this, &TKDE::UpperConfidenceInterval, fXMin, fXMax, 1, "TKDE", "UpperConfidenceInterval");
   fUpperPDF->SetParameter(0, confidenceLevel);
   return fUpperPDF;
}

TF1* TKDE::GetPDFLowerConfidenceInterval(Double_t confidenceLevel) {
   // Returns the upper estimated density 
   fLowerPDF = new TF1("KDE_LowerFunc", this, &TKDE::LowerConfidenceInterval, fXMin, fXMax, 1, "TKDE", "LowerConfidenceInterval");
   fLowerPDF->SetParameter(0, confidenceLevel);
   return fLowerPDF;
}

TF1* TKDE::GetKDEApproximateBias(){
   // Returns the approximate bias
   return fApproximateBias = new TF1("KDE_ApproxBias", this, &TKDE::ApproximateBias, fXMin, fXMax, 0, "TKDE", "ApproximateBias");;
}
