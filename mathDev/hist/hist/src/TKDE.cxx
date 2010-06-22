#include <algorithm>
#include <numeric>
#include "TKDE.h"
#include "Math/Integrator.h"
#include "Math/QuantFuncMathCore.h"

ClassImp(TKDE)

template<class F>
TKDE::TKDE(const F& kernelFunction, UInt_t events, const Double_t* data, Double_t xMin, Double_t xMax, /*EFitMethod fit,*/ EIteration iter, EMirror mir, EBinning bin, Double_t rho) :
   fData(events, 0.0),
   fPDF(0),
   fUpperPDF(0),
   fLowerPDF(0),
   fHistogram(0),
//    fFitMethod(fit),
   fIteration(iter),
   fMirror(mir),
   fBinning(bin),
//    fSettedOptions(std::vector<Bool_t>(kTotalOptions, kFALSE)),
   fNBins(1000),
   fNEvents(events),
   fUseBinsNEvents(1000),
   fMean(0.0),
   fSigma(0.0),
   fXMin(xMin),
   fXMax(xMax),
   fRho(rho)
{
   //Class constructor
//    SetOptions(fit, kUserDefined, iter, mir, bin);
   SetData(data);
   SetKernelFunction(kernelFunction);
   SetKernel();
}   
   
TKDE::TKDE(UInt_t events, const Double_t* data, Double_t xMin, Double_t xMax, /*EFitMethod fit,*/ EKernelType kern, EIteration iter, EMirror mir, EBinning bin, Double_t rho) :
   fData(events, 0.0),
   fPDF(0),
   fUpperPDF(0),
   fLowerPDF(0),
   fHistogram(0),
//    fFitMethod(fit),
   fKernelType(kern),
   fIteration(iter),
   fMirror(mir),
   fBinning(bin),
//    fSettedOptions(std::vector<Bool_t>(kTotalOptions, kFALSE)),
   fNBins(1000),
   fNEvents(events),
   fUseBinsNEvents(1000),
   fMean(0.0),
   fSigma(0.0),
   fXMin(xMin),
   fXMax(xMax),
   fRho(rho)
{
   //Class constructor
//    SetOptions(fit, kern, iter, mir, bin);
   SetData(data);
   SetKernelFunction();
   SetKernel();
}
   
TKDE::~TKDE() {
   //Class destructor
   if (fHistogram) delete fHistogram;
   if (fPDF)       delete fPDF;
   if (fUpperPDF)  delete fUpperPDF;
   if (fLowerPDF)  delete fLowerPDF;
   delete fKernelFunction;
   delete fKernel;
}
    
// void TKDE::ResetOptions() {
//    // Sets options to default
//    fSettedOptions.assign(kTotalOptions, kFALSE);  
// }
   
// void TKDE::SetOptions(EFitMethod fit, EKernelType kern, EIteration iter, EMirror mir, EBinning bin) {
//    // Sets User global options
//    SetFittingMethod(fit);
//    SetKernelType(kern) ;
//    SetIteration(iter);
//    SetMirror(mir);
//    SetBinning(bin);
//    SetOptions();
// }
      
// void TKDE::SetOptions() {
//    // Sets global options
//    fSettedOptions[fFitMethod]  = kTRUE;
//    fSettedOptions[fKernelType] = kTRUE;
//    fSettedOptions[fIteration]  = kTRUE;
//    fSettedOptions[fMirror]     = kTRUE;
//    fSettedOptions[fBinning]    = kTRUE;
// }

// void TKDE::SetMethod(EFitMethod fit) {
//    // Sets User option for density estimator 
//    fFitMethod = fit;
// }
   
// Sets User option for the choice of kernel estimator 
void TKDE::SetKernelType(EKernelType kern) {
   fKernelType = kern;
}
      
// Sets User option for fixed or adaptive iteration      
void TKDE::SetIteration(EIteration iter) {
   fIteration = iter;
}
      
void TKDE::SetMirror(EMirror mir) {
   // Sets User option for mirroring the data
   fMirror = mir;
   fMirrorLeft  = fMirror == kMirrorLeft  || fMirror == kMirrorBoth || fMirror == kMirrorLeftAsymRight;
   fMirrorRight = fMirror == kMirrorRight || fMirror == kMirrorBoth || fMirror == kMirrorAsymLeftRight;
   fAsymLeft    = fMirror == kMirrorAsymLeft  || fMirror == kMirrorAsymLeftRight || fMirror == kMirrorAsymBoth;
   fAsymRight   = fMirror == kMirrorAsymRight || fMirror == kMirrorLeftAsymRight || fMirror == kMirrorAsymBoth;
}
      
void TKDE::SetBinning(EBinning bin) {
   // Sets User option for binning
   fBinning = bin;
   SetUseBins();
}
   
void TKDE::SetUseBins() {
   // Sets User option for using binned weights  
   switch (fBinning) {
//       case kTotalOptions:
//          break;
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

void TKDE::SetData(const Double_t* data) {
   // Sets the data events input sample
   for (UInt_t i = 0; i < fNEvents; ++i) {
      SetData(data[i], i);
   }
   if (fXMin >= fXMax) {
      fXMin = *std::min_element(fData.begin(), fData.end());
      fXMax = *std::max_element(fData.begin(), fData.end());
   }
   SetMean();
   SetSigma();
   if (fMirror != kNoMirror) {
      SetMirroredData();
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

void TKDE::SetMean() {
   // Computes input data's mean
   fMean = accumulate(fData.begin(), fData.end(), 0.0) / fData.size();
}
   
void TKDE::SetSigma() { 
   // Computes input data's sigma
   fSigma = TMath::Sqrt(1. / (fData.size() - 1) * (inner_product(fData.begin(), fData.end(), fData.begin(), 0.0) - fData.size() * TMath::Power(fMean, 2)));
}

void TKDE::SetKernel() {
   // Sets the kernel density estimator
   Double_t weight(fSigma * TMath::Power( 3./ 4. * fNEvents, -0.2)); // bandwidth
   UInt_t n = fUseBins ? fNBins : fNEvents; 
   fKernel = new TKernel(n, weight, this);
   if (fIteration == kAdaptive) {
      fKernel->ComputeAdaptiveWeights();
   }
}
 
template<class F>
void TKDE::SetKernelFunction(const F& kernelFunction) {
   // Sets User input kernel estimator  
   fKernelFunction = new ROOT::Math::WrappedFunction<F>(kernelFunction);
}
   
void TKDE::SetKernelFunction() {
   // Sets kernel estimator
   switch (fKernelType) {
//       case kKernels :
//          break; // No case
      case kUserDefined :
         break; // Case already taken care of in templated SetKernelFunction at constructor
      default:
      case kGaussian :
         fKernelFunction = new ROOT::Math::WrappedMemFunction<TKDE, Double_t (TKDE::*)(Double_t) const>(*this, &TKDE::GaussianKernel);
   }
}

inline void TKDE::SetData(Double_t x, UInt_t i) {
   // set data point at the i-th data vector position
   fData[i] = x;
}

TH1D* TKDE::GetHistogram(UInt_t nbins, Double_t xMin, Double_t xMax) {
   // Returns the histogram
   return fHistogram ? fHistogram : fHistogram = GetKDEHistogram(nbins, xMin, xMax);
}

void TKDE::SetRange(Double_t xMin, Double_t xMax) {
   //Sets minimum range value and maximum range value
   if (xMin >= xMax) {
      std::cerr << "Minimum range cannot be bigger or equal than the maximum range!" << std::endl;
      return;
   } 
   fXMin = xMin;
   fXMax = xMax;
   SetKernel();
}

TF1* TKDE::GetFunction() {
   // Returns the PDF estimate
   return fPDF ? fPDF : fPDF = GetKDEFunction();
}

TF1* TKDE::GetUpperFunction(Double_t confidenceLevel) {
   // Returns the PDF upper estimate (upper confidence interval limit)
   return fUpperPDF ? fUpperPDF : fUpperPDF = GetPDFUpperConfidenceInterval(confidenceLevel);
}

TF1* TKDE::GetLowerFunction(Double_t confidenceLevel) {
   // Returns the PDF lower estimate (lower confidence interval limit)
   return fLowerPDF ? fLowerPDF : fLowerPDF = GetPDFLowerConfidenceInterval(confidenceLevel);
}
void TKDE::Fill(Double_t data) {
   // Fills data member with User input data event
   SetData(data, static_cast<UInt_t>(fData.size()));
   fNEvents++;
   fXMin = std::min(data, fXMin);
   fXMax = std::max(data, fXMin);
   SetMean();
   SetSigma();
}

Double_t TKDE::operator()(const Double_t* x, const Double_t*) const {
   // The class's unary function: returns the kernel density estimate
   return (*fKernel)(*x);
}

TKDE::TKernel::TKernel(UInt_t n, Double_t weight, TKDE* kde) :
   // Internal class constructor
   fNWeights(n),
   fWeights(n, weight),
   fKDE(kde)
{}
   
void TKDE::TKernel::ComputeAdaptiveWeights() {
   // Gets the adaptive weights (bandwidths) for TKernel internal computation
   std::vector<Double_t> weights = fWeights;
   std::vector<Double_t>::iterator weight = weights.begin();
   std::vector<Double_t> dataset(fKDE->fUseBins ? GetBinCentreData() : fKDE->fData);
   std::vector<Double_t>::iterator data = dataset.begin();
   Double_t minWeight(*weight * TMath::Power(50.0, -0.5));
   Double_t norm(2.0 * TMath::Sqrt(3.0)); // Adaptive weight normalization
   for (; weight != weights.end(); ++weight, ++data) {
      *weight *= fKDE->fRho / fKDE->fSigma * TMath::Power(fKDE->fSigma / (*fKDE->fKernel)(*data), 0.5) / norm;
      if (*weight < minWeight) {
         *weight = minWeight;
      }
   }
   fWeights = weights;
}

Double_t TKDE::TKernel::GetWeight(Double_t x) const {
   // Returns the bandwidth
   return fWeights[Index(x)];
}

const std::vector<Double_t> TKDE::TKernel::GetBinCentreData() const {
   // Gets the bins' centres from the data for using with the binned weights
   std::vector<Double_t> result(fKDE->fNBins, 0.0);
   Double_t binWidth((fKDE->fXMax - fKDE->fXMin) / fKDE->fNBins);
   for (UInt_t i = 0; i < fKDE->fNBins; ++i) {
      result[i] = fKDE->fXMin + i * binWidth + 0.5 * binWidth;
   }
   return result;
}

Double_t TKDE::TKernel::operator()(Double_t x) const {
   // The internal class's unary function: returns the kernel density estimate
   Double_t result(0.0);
   for (UInt_t i = 0, j = 0; i < fKDE->fNEvents; ++i) {
      j = Index(fKDE->fData[i], i);
      result += 1. / fWeights[j] * (*fKDE->fKernelFunction)((x - fKDE->fData[i]) / fWeights[j]);
      if (fKDE->fAsymLeft) {
         result -= 1. / fWeights[j] * (*fKDE->fKernelFunction)((x - (2 * fKDE->fXMin - fKDE->fData[i])) / fWeights[j]);
      }
      if (fKDE->fAsymRight) {
         result -= 1. / fWeights[j] * (*fKDE->fKernelFunction)((x - (2 * fKDE->fXMax - fKDE->fData[i])) / fWeights[j]);
      }
   }
   return result / fKDE->fNEvents;
}

UInt_t TKDE::TKernel::Index(Double_t x, UInt_t i) const {
   // Returns the indices for the binned weights. Otherwise, the the data orderinf is returned
   if(fKDE->fNEvents > fNWeights) {
      return Index(x);
   } else {
      return i;
   }
}

UInt_t TKDE::TKernel::Index(Double_t x) const {
   // Returns the indices (bins) for the binned weights
   Int_t bin = Int_t(fNWeights * (x - fKDE->fXMin) / (fKDE->fXMax - fKDE->fXMin));
   if (bin < 0) { // Left Mirrored Data
      bin *= -1;
   } else if (bin > (Int_t)fNWeights) { // Right Mirrored Data
      bin -= fNWeights ;
   }
   if (bin == (Int_t)fNWeights) {
      bin = fNWeights - 1;
   }
   return bin;
}

Double_t TKDE::UpperConfidenceInterval(const Double_t* x, const Double_t* p) const {
   // Returns the pointwise upper estimated density
   Double_t f = this->operator()(x);
   Double_t fsigma = GetError(*x);
   Double_t z = ROOT::Math::normal_quantile(*p, 1.0);
   return  f + z * fsigma;
}

Double_t TKDE::LowerConfidenceInterval(const Double_t* x, const Double_t* p) const {
   // Returns the pointwise lower estimated density
   Double_t f = this->operator()(x);
   Double_t fsigma = GetError(*x);
   Double_t z = ROOT::Math::normal_quantile(*p, 1.0);
   return  f - z * fsigma;
}

Double_t TKDE::GetError(Double_t x) const {
   // Returns the pointwise variance of estimated density
   Double_t kernelL2Norm = ComputeKernelL2Norm();
   Double_t f = this->operator()(&x);// (fSigma * TMath::Power( 3./ 4. * fNEvents, -0.2)
   Double_t weight = fKernel->GetWeight(x); // Bandwidth
   Double_t result = f * kernelL2Norm / (fNEvents * weight); 
   return TMath::Sqrt(result);
}

Double_t TKDE::ComputeKernelL2Norm() const {
   // Computes the kernel's L2 norm
   ROOT::Math::IntegratorOneDim ig;
   KernelIntegrand kernel(this);
   ig.SetFunction(kernel);
   Double_t result = ig.Integral();
   return result;
}

Double_t TKDE::GaussianKernel(Double_t x) const {
   // Returns the kernel evaluation at x 
   return TMath::Gaus(x, 0.0, 1.0, kTRUE);
}

TKDE::KernelIntegrand::KernelIntegrand(const TKDE* kde) : fKDE(kde) {
   // Internal class constructor
}
   
Double_t TKDE::KernelIntegrand::operator()(Double_t x) const {
   // Internal class unary function
   return TMath::Power((*fKDE->fKernelFunction)(x), 2);
}

TH1D* TKDE::GetKDEHistogram(UInt_t nbins, Double_t xMin, Double_t xMax) {
   // Returns the histogram of the estimated density at data points
   if (xMin < xMax) {
      fHistogram = new TH1D("KDE Histogram", "KDE Histogram", nbins, xMin, xMax);
   } else {
      fHistogram = new TH1D("KDE Histogram", "KDE Histogram", nbins, fXMin, fXMax);  
   }
   fHistogram = new TH1D("KDE Histogram", "KDE Histogram", nbins, fXMin, fXMax);  
   for (std::vector<Double_t>::iterator data = fData.begin(); data != fData.end(); ++data) {
      fHistogram->Fill(*data, (*fKernel)(*data));
   }
   fHistogram->Scale(1 / fHistogram->Integral(), "width");
   return fHistogram;
}
   
TF1* TKDE::GetKDEFunction() {
   //Returns the estimated density 
   return fPDF = new TF1("KDE_Func", this, &TKDE::operator(), fXMin, fXMax, 0, "TKDE", "operator()");
}

TF1* TKDE::GetPDFUpperConfidenceInterval(Double_t confidenceLevel) {
   // Returns the upper estimated density 
   TF1* fPDFUpper = new TF1("KDE_UpperFunc", this, &TKDE::UpperConfidenceInterval, fXMin, fXMax, 1, "TKDE", "UpperConfidenceInterval");
   fPDFUpper->SetParameter(0, confidenceLevel);
   return fPDFUpper;
}

TF1* TKDE::GetPDFLowerConfidenceInterval(Double_t confidenceLevel) {
   // Returns the upper estimated density 
   TF1* fPDFLower = new TF1("KDE_LowerFunc", this, &TKDE::LowerConfidenceInterval, fXMin, fXMax, 1, "TKDE", "LowerConfidenceInterval");
   fPDFLower->SetParameter(0, confidenceLevel);
   return fPDFLower;
}
