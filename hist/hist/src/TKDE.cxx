// // @(#)root/hist:$Id$
// Authors: Bartolomeu Rabacal    07/2010
/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006 , ROOT MathLib Team                             *
 *                                                                    *
 * For the licensing terms see $ROOTSYS/LICENSE.                      *
 * For the list of contributors see $ROOTSYS/README/CREDITS.          *
 *                                                                    *
 **********************************************************************/
//////////////////////////////////////////////////////////////////////////////
/*
    Kernel Density Estimation class.
    The three main references are (1) "Scott DW, Multivariate Density Estimation. Theory, Practice and Visualization. New York: Wiley",
    (2) "Jann Ben - ETH Zurich, Switzerland -, Univariate kernel density estimation document for KDENS:
         Stata module for univariate kernel density estimation."
    (3) "Hardle W, Muller M, Sperlich S, Werwatz A, Nonparametric and Semiparametric Models. Springer."
   The algorithm is briefly described in (4) "Cranmer KS, Kernel Estimation in High-Energy
   Physics. Computer Physics Communications 136:198-207,2001" - e-Print Archive: hep ex/0011057.
   A binned version is also implemented to address the performance issue due to its data size dependance.
*/


#include <functional>
#include <algorithm>
#include <numeric>
#include <limits>

#include "Math/Error.h"
#include "TMath.h"
#include "Math/Functor.h"
#include "Math/Integrator.h"
#include "Math/QuantFuncMathCore.h"
#include "Math/RichardsonDerivator.h"
#include "TGraphErrors.h"
#include "TCanvas.h"

#include "TKDE.h"


ClassImp(TKDE)

const Double_t TKDE::_2_PI_ROOT_INV = 0.398942280401432703;
const Double_t TKDE::PI             = 3.14159265358979312;
const Double_t TKDE::PI_OVER2       = 1.57079632679489656;
const Double_t TKDE::PI_OVER4       = 0.785398163397448279;
const Double_t APPRX_GEO_MEAN       = 0.241970724519143365; // 1 / TMath::Power(2 * TMath::Pi(), .5) * TMath::Exp(-.5)

class TKDE::TKernel {
   TKDE* fKDE;
   UInt_t fNWeights; // Number of kernel weights (bandwidth as vectorized for binning)
   std::vector<Double_t> fWeights; // Kernel weights (bandwidth)
public:
   TKernel(Double_t weight, TKDE* kde);
   void ComputeAdaptiveWeights();
   Double_t operator()(Double_t x) const;
   Double_t GetWeight(Double_t x) const;
   Double_t GetFixedWeight() const;
   UInt_t GetNumberOfWeights() const;
   const std::vector<Double_t> & GetAdaptiveWeights() const;
};

struct TKDE::KernelIntegrand {
   enum EIntegralResult{kNorm, kMu, kSigma2, kUnitIntegration};
   KernelIntegrand(const TKDE* kde, EIntegralResult intRes);
   Double_t operator()(Double_t x) const;
private:
   const TKDE* fKDE;
   EIntegralResult fIntegralResult;
};

TKDE::TKDE(UInt_t events, const Double_t* data, Double_t xMin, Double_t xMax, const Option_t* option, Double_t rho) :
   fData(events, 0.0),
   fEvents(events, 0.0),
   fPDF(0),
   fUpperPDF(0),
   fLowerPDF(0),
   fApproximateBias(0),
   fNewData(false),
   fUseMinMaxFromData((xMin >= xMax)),
   fNBins(events < 10000 ? 100: events / 10),
   fNEvents(events),
   fUseBinsNEvents(10000),
   fMean(0.0),
   fSigma(0.0),
   fXMin(xMin),
   fXMax(xMax),
   fAdaptiveBandwidthFactor(1.0),
   fCanonicalBandwidths(std::vector<Double_t>(kTotalKernels, 0.0)),
   fKernelSigmas2(std::vector<Double_t>(kTotalKernels, -1.0)),
   fSettedOptions(std::vector<Bool_t>(4, kFALSE))
{
   //Class constructor
   SetOptions(option, rho);
   CheckOptions();
   SetMirror();
   SetUseBins();
   SetKernelFunction();
   SetData(data);
   SetCanonicalBandwidths();
   SetKernelSigmas2();
   SetKernel();
}

TKDE::~TKDE() {
   //Class destructor
   if (fPDF)              delete fPDF;
   if (fUpperPDF)         delete fUpperPDF;
   if (fLowerPDF)         delete fLowerPDF;
   if (fApproximateBias)  delete fApproximateBias;
   delete fKernelFunction;
   delete fKernel;
}

void TKDE::Instantiate(KernelFunction_Ptr kernfunc, UInt_t events, const Double_t* data, Double_t xMin, Double_t xMax, const Option_t* option, Double_t rho) {
   // Template's constructor surrogate
   fData = std::vector<Double_t>(events, 0.0);
   fEvents = std::vector<Double_t>(events, 0.0);
   fPDF = 0;
   fUpperPDF = 0;
   fLowerPDF = 0;
   fApproximateBias = 0;
   fNBins = events < 10000 ? 100 : events / 10;
   fNEvents = events;
   fUseBinsNEvents = 10000;
   fMean = 0.0;
   fSigma = 0.0;
   fXMin = xMin;
   fXMax = xMax;
   fUseMinMaxFromData = (fXMin >= fXMax);
   fAdaptiveBandwidthFactor = 1.;
   fCanonicalBandwidths = std::vector<Double_t>(kTotalKernels, 0.0);
   fKernelSigmas2 = std::vector<Double_t>(kTotalKernels, -1.0);
   fSettedOptions = std::vector<Bool_t>(4, kFALSE);
   SetOptions(option, rho);
   CheckOptions(kTRUE);
   SetMirror();
   SetUseBins();
   SetKernelFunction(kernfunc);
   SetData(data);
   SetCanonicalBandwidths();
   SetKernelSigmas2();
   SetKernel();
}

void TKDE::SetOptions(const Option_t* option, Double_t rho) {
   TString opt = option;
   opt.ToLower();
   std::string options = opt.Data();
   size_t numOpt = 4;
   std::vector<std::string> voption(numOpt, "");
   for (std::vector<std::string>::iterator it = voption.begin(); it != voption.end() && !options.empty(); ++it) {
      size_t pos = options.find_last_of(';');
      *it = options.substr(pos + 1);
      options = options.substr(0, pos);
      if (pos == std::string::npos) break;
   }
   for (std::vector<std::string>::iterator it = voption.begin(); it != voption.end(); ++it) {
      size_t pos = (*it).find(':');
      if (pos != std::string::npos) {
         GetOptions((*it).substr(0, pos) , (*it).substr(pos + 1));
      }
   }
   AssureOptions();
   fRho = rho;
}

void TKDE::SetDrawOptions(const Option_t* option, TString& plotOpt, std::map<TString, TString>& canvasNameOpt, std::map<TString, TString>& canvasTitleOpt, std::map<TString, TString>& drawOpt) {
   canvasNameOpt["estimate"] = "KDE_Estimate";
   canvasTitleOpt["estimate"] = "KDE Estimate";
   drawOpt["estimate"] = "";
   canvasNameOpt["error"] = "KDE_Error";
   canvasTitleOpt["error"] = "KDE Error";
   drawOpt["error"] = "APL";
   canvasNameOpt["bothsinglecanvas"] = "KDE_Estimate_Error";
   canvasTitleOpt["bothsinglecanvas"] = "KDE Estimate and Error";
   drawOpt["bothsinglecanvas"] = "APL";
   size_t numOpt = 4;
   std::string options = TString(option).Data();
   std::vector<std::string> voption(numOpt, "");
   for (std::vector<std::string>::iterator it = voption.begin(); it != voption.end() && !options.empty(); ++it) {
      size_t pos = options.find_last_of(';');
      *it = options.substr(pos + 1);
      options = options.substr(0, pos);
      if (pos == std::string::npos) break;
   }
   Bool_t foundPlotOPt = kFALSE;
   std::vector<std::string>::iterator plotpos = voption.begin();
   TString plotType;
   TString plotInstance;
   while(plotpos != voption.end() && !foundPlotOPt) {
      size_t pos = (*plotpos).find(':');
      if (pos != std::string::npos) {
         plotType = (*plotpos).substr(0, pos);
         plotInstance = (*plotpos).substr(pos + 1);
         plotType.ToLower();
         if (std::string(plotType.Data()).compare("plot") == 0) {
            foundPlotOPt = kTRUE;
            break;
         }
      }
      ++plotpos;
   }
   plotInstance.ToLower();
   Bool_t hasPlotOPt = plotInstance.Contains("estimate") || plotInstance.Contains("error") ||
   plotInstance.Contains("bothsinglecanvas") || plotInstance.Contains("bothdoublecanvas");
   if (!foundPlotOPt || !hasPlotOPt) {
      this->Warning("SetDrawOptions", "Unknown plotting option or no plotting option: setting to KDE estimate plot.");
      plotOpt = "estimate";
      return;
   }
   if (plotInstance.Contains("+confidenceinterval")) {
      if (plotInstance.Contains("error")) {
         this->Warning("SetDrawOptions", "Only Estimate plotting can have a Confidence Interval option: ignored option.");
      } else {
         plotInstance.Remove(plotInstance.Index("+confidenceinterval"), (Ssiz_t)(TString("+confidenceinterval").Length()));
         drawOpt["estimate"] += "confidenceinterval";
      }
   }
   plotOpt = plotInstance;
   if (plotpos !=  voption.end())
      voption.erase(plotpos);
   for (std::vector<std::string>::iterator it = voption.begin(); it != voption.end(); ++it) {
      size_t pos = (*it).find(':');
      if (pos == std::string::npos || (*it).empty()) {
         continue;
      } else {
         TString optionType = (*it).substr(0, pos);
         std::string optionInstance = (*it).substr(pos + 1);
         optionType.ToLower();
         if (std::string(optionType.Data()).compare("canvasname") == 0) {
            if (optionInstance.empty()) {
               canvasNameOpt[plotInstance] = "KDE_Canvas";
            } else {
               canvasNameOpt[plotInstance] = optionInstance;
            }
         } else if (std::string(optionType.Data()).compare("canvastitle") == 0) {
            canvasTitleOpt[plotInstance] = optionInstance;
         } else if (std::string(optionType.Data()).compare("draw") == 0) {
            drawOpt[plotInstance] += optionInstance;
         }
      }
   }
}

void TKDE::GetOptions(std::string optionType, std::string option) {
   if (optionType.compare("kerneltype") == 0) {
      fSettedOptions[0] = kTRUE;
      if (option.compare("gaussian") == 0) {
         fKernelType = kGaussian;
      } else if (option.compare("epanechnikov") == 0) {
         fKernelType = kEpanechnikov;
      } else if (option.compare("biweight") == 0) {
         fKernelType = kBiweight;
      } else if (option.compare("cosinearch") == 0) {
         fKernelType = kCosineArch;
      } else if (option.compare("userdefined") == 0) {
         fKernelType = kUserDefined;
      } else {
         this->Warning("GetOptions", "Unknown kernel type option: setting to Gaussian");
         fKernelType = kGaussian;
      }
   } else if (optionType.compare("iteration") == 0) {
      fSettedOptions[1] = kTRUE;
      if (option.compare("adaptive") == 0) {
         fIteration = kAdaptive;
      } else if (option.compare("fixed") == 0) {
         fIteration = kFixed;
      } else {
         this->Warning("GetOptions", "Unknown iteration option: setting to Adaptive");
         fIteration = kAdaptive;
      }
   } else if (optionType.compare("mirror") == 0) {
      fSettedOptions[2] = kTRUE;
      if (option.compare("nomirror") == 0) {
         fMirror = kNoMirror;
      } else if (option.compare("mirrorleft") == 0) {
         fMirror = kMirrorLeft;
      } else if (option.compare("mirrorright") == 0) {
         fMirror = kMirrorRight;
      } else if (option.compare("mirrorboth") == 0) {
         fMirror = kMirrorBoth;
      } else if (option.compare("mirrorasymleft") == 0) {
         fMirror = kMirrorAsymLeft;
      } else if (option.compare("mirrorasymleftright") == 0) {
         fMirror = kMirrorAsymLeftRight;
      } else if (option.compare("mirrorasymright") == 0) {
         fMirror = kMirrorAsymRight;
      } else if (option.compare("mirrorleftasymright") == 0) {
         fMirror = kMirrorLeftAsymRight;
      } else if (option.compare("mirrorasymboth") == 0) {
         fMirror = kMirrorAsymBoth;
      } else {
         this->Warning("GetOptions", "Unknown mirror option: setting to NoMirror");
         fMirror = kNoMirror;
      }
   } else if (optionType.compare("binning") == 0) {
      fSettedOptions[3] = kTRUE;
      if (option.compare("unbinned") == 0) {
         fBinning = kUnbinned;
      } else if (option.compare("relaxedbinning") == 0) {
         fBinning = kRelaxedBinning;
      } else if (option.compare("forcedbinning") == 0) {
         fBinning = kForcedBinning;
      } else {
         this->Warning("GetOptions", "Unknown binning option: setting to RelaxedBinning");
         fBinning = kRelaxedBinning;
      }
   }
}

void TKDE::AssureOptions() {
   if (!fSettedOptions[0]) {
      fKernelType = kGaussian;
   }
   if (!fSettedOptions[1]) {
      fIteration = kAdaptive;
   }
   if (!fSettedOptions[2]) {
      fMirror = kNoMirror;
   }
   if (!fSettedOptions[3]) {
      fBinning = kRelaxedBinning;
   }
}

void TKDE::CheckOptions(Bool_t isUserDefinedKernel) {
   // Sets User global options
   if (!(isUserDefinedKernel) && !(fKernelType >= kGaussian && fKernelType < kUserDefined)) {
      Error("CheckOptions", "Illegal user kernel type input! Use template constructor for user defined kernel.");
   }
   if (fIteration != kAdaptive && fIteration != kFixed) {
      Warning("CheckOptions", "Illegal user iteration type input - use default value !");
      fIteration = kAdaptive;
   }
   if (!(fMirror >= kNoMirror && fMirror <= kMirrorAsymBoth)) {
      Warning("CheckOptions", "Illegal user mirroring type input - use default value !");
      fMirror = kNoMirror;
   }
   if (!(fBinning >= kUnbinned && fBinning <= kForcedBinning)) {
      Warning("CheckOptions", "Illegal user binning type input - use default value !");
      fBinning = kRelaxedBinning;
   }
   if (fRho <= 0.0) {
      Warning("CheckOptions", "Tuning factor rho cannot be non-positive - use default value !");
      fRho = 1.0;
   }
}

void TKDE::SetKernelType(EKernelType kern) {
   // Sets User option for the choice of kernel estimator
   fKernelType = kern;
   CheckOptions();
   SetKernel();
}

void TKDE::SetIteration(EIteration iter) {
   // Sets User option for fixed or adaptive iteration
   fIteration = iter;
   CheckOptions();
   SetKernel();
}


void TKDE::SetMirror(EMirror mir) {
   // Sets User option for mirroring the data
   fMirror = mir;
   CheckOptions();
   SetMirror();
   if (fUseMirroring) {
      SetMirroredEvents();
   }
   SetKernel();
}


void TKDE::SetBinning(EBinning bin) {
   // Sets User option for binning the weights
   fBinning = bin;
   CheckOptions();
   SetUseBins();
   SetKernel();
}

void TKDE::SetNBins(UInt_t nbins) {
   // Sets User option for number of bins
   if (!nbins) {
      Error("SetNBins", "Number of bins must be greater than zero.");
      return;
   }
   fNBins = nbins;
   fWeightSize = fNBins / (fXMax - fXMin);
   SetBinCentreData(fXMin, fXMax);
   SetBinCountData();

   if (fBinning == kUnbinned) {
      Warning("SetNBins", "Bin type using SetBinning must set for using a binned evaluation");
      return;
   }

   SetKernel();
}

void TKDE::SetUseBinsNEvents(UInt_t nEvents) {
   // Sets User option for the minimum number of events for allowing automatic binning
   fUseBinsNEvents = nEvents;
   SetUseBins();
   SetKernel();
}

void TKDE::SetTuneFactor(Double_t rho) {
   // Factor which can be used to tune the smoothing.
   // It is used as multiplicative factor for the fixed and adaptive bandwidth.
   // A value < 1 will reproduce better the tails but oversmooth the peak
   // while a factor > 1 will overestimate the tail
   fRho = rho;
   CheckOptions();
   SetKernel();
}

void TKDE::SetRange(Double_t xMin, Double_t xMax) {
   // Sets minimum range value and maximum range value
   if (xMin >= xMax) {
      Error("SetRange", "Minimum range cannot be bigger or equal than the maximum range! Present range values remain the same.");
      return;
   }
   fXMin = xMin;
   fXMax = xMax;
   fUseMinMaxFromData = false;
   SetKernel();
}

// private methods

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

void TKDE::SetMirror() {
   // Sets the mirroring
   fMirrorLeft   = fMirror == kMirrorLeft      || fMirror == kMirrorBoth          || fMirror == kMirrorLeftAsymRight;
   fMirrorRight  = fMirror == kMirrorRight     || fMirror == kMirrorBoth          || fMirror == kMirrorAsymLeftRight;
   fAsymLeft     = fMirror == kMirrorAsymLeft  || fMirror == kMirrorAsymLeftRight || fMirror == kMirrorAsymBoth;
   fAsymRight    = fMirror == kMirrorAsymRight || fMirror == kMirrorLeftAsymRight || fMirror == kMirrorAsymBoth;
   fUseMirroring = fMirrorLeft                 || fMirrorRight ;
}


void TKDE::SetData(const Double_t* data) {
   // Sets the data events input sample or bin centres for binned option and computes basic estimators
   if (!data) {
      if (fNEvents) fData.reserve(fNEvents);
      return;
   }
   fEvents.assign(data, data + fNEvents);
   if (fUseMinMaxFromData) {
      fXMin = *std::min_element(fEvents.begin(), fEvents.end());
      fXMax = *std::max_element(fEvents.begin(), fEvents.end());
   }
   Double_t midspread = ComputeMidspread();
   SetMean();
   SetSigma(midspread);
   if (fUseBins) {
      if (fNBins >= fNEvents) {
         this->Warning("SetData", "Default number of bins is greater or equal to number of events. Use SetNBins(UInt_t) to set the appropriate number of bins");
      }
      fWeightSize = fNBins / (fXMax - fXMin);
      SetBinCentreData(fXMin, fXMax);
      SetBinCountData();
   } else {
      fWeightSize = fNEvents / (fXMax - fXMin);
      fData = fEvents;
   }
   if (fUseMirroring) {
      SetMirroredEvents();
   }
}

void TKDE::InitFromNewData() {
   // re-initialize when new data have been filled in TKDE
   // re-compute kernel quantities and mean and sigma
   fNewData = false;
   fEvents = fData;
   if (fUseMinMaxFromData) {
      fXMin = *std::min_element(fEvents.begin(), fEvents.end());
      fXMax = *std::max_element(fEvents.begin(), fEvents.end());
   }
   Double_t midspread = ComputeMidspread();
   SetMean();
   SetSigma(midspread);
//    if (fUseBins) {
// } // bin usage is not supported in this case
//
   fWeightSize = fNEvents / (fXMax - fXMin);
   if (fUseMirroring) {
      SetMirroredEvents();
   }
   SetKernel();
}


void TKDE::SetMirroredEvents() {
   // Mirrors the data
   std::vector<Double_t> originalEvents = fEvents;
   if (fMirrorLeft) {
      fEvents.resize(2 * fNEvents, 0.0);
      transform(fEvents.begin(), fEvents.begin() + fNEvents, fEvents.begin() + fNEvents, std::bind1st(std::minus<Double_t>(), 2 * fXMin));
   }
   if (fMirrorRight) {
      fEvents.resize((fMirrorLeft + 2) * fNEvents, 0.0);
      transform(fEvents.begin(), fEvents.begin() + fNEvents, fEvents.begin() + (fMirrorLeft + 1) * fNEvents, std::bind1st(std::minus<Double_t>(), 2 * fXMax));
   }
   if(fUseBins) {
      fNBins *= (fMirrorLeft + fMirrorRight + 1);
      Double_t xmin = fMirrorLeft  ? 2 * fXMin - fXMax : fXMin;
      Double_t xmax = fMirrorRight ? 2 * fXMax - fXMin : fXMax;
      SetBinCentreData(xmin, xmax);
      SetBinCountData();
   } else {
      fData = fEvents;
   }
   fEvents = originalEvents;
}

void TKDE::SetMean() {
   // Computes input data's mean
   fMean = std::accumulate(fEvents.begin(), fEvents.end(), 0.0) / fEvents.size();
}

void TKDE::SetSigma(Double_t R) {
   // Computes input data's sigma
   fSigma = std::sqrt(1. / (fEvents.size() - 1.) * (std::inner_product(fEvents.begin(), fEvents.end(), fEvents.begin(), 0.0) - fEvents.size() * std::pow(fMean, 2.)));
   fSigmaRob = std::min(fSigma, R / 1.349); // Sigma's robust estimator
}

void TKDE::SetKernel() {
   // Sets the kernel density estimator
   UInt_t n = fData.size();
   if (n == 0) return;
   // Optimal bandwidth (Silverman's rule of thumb with assumed Gaussian density)
   Double_t weight(fCanonicalBandwidths[kGaussian] * fSigmaRob * std::pow(3. / (8. * std::sqrt(PI)) * n, -0.2));
   weight *= fRho * fCanonicalBandwidths[fKernelType] / fCanonicalBandwidths[kGaussian];
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
            Error("SetKernelFunction", "Undefined user kernel function input!");
            //exit(EXIT_FAILURE);
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

TF1* TKDE::GetFunction(UInt_t npx, Double_t xMin, Double_t xMax) {
   // Returns the PDF estimate as a function sampled in npx between xMin and xMax
   // the KDE is not re-normalized to the xMin/xMax range.
   // The user manages the returned function
   // For getting a non-sampled TF1, one can create a TF1 directly from the TKDE by doing
   // TF1 * f1  = new TF1("f1",kde,xMin,xMax,0);
   return GetKDEFunction(npx,xMin,xMax);
}

TF1* TKDE::GetUpperFunction(Double_t confidenceLevel, UInt_t npx, Double_t xMin, Double_t xMax) {
   // Returns the PDF upper estimate (upper confidence interval limit)
   return GetPDFUpperConfidenceInterval(confidenceLevel,npx,xMin,xMax);
}

TF1* TKDE::GetLowerFunction(Double_t confidenceLevel, UInt_t npx, Double_t xMin, Double_t xMax) {
   // Returns the PDF lower estimate (lower confidence interval limit)
   return GetPDFLowerConfidenceInterval(confidenceLevel,npx,xMin,xMax);
}

TF1* TKDE::GetApproximateBias(UInt_t npx, Double_t xMin, Double_t xMax) {
   // Returns the PDF estimate bias
   return GetKDEApproximateBias(npx,xMin,xMax);
}

void TKDE::Fill(Double_t data) {
   // Fills data member with User input data event for the unbinned option
   if (fUseBins) {
      this->Warning("Fill", "Cannot fill data with data binned option. Data input ignored.");
      return;
   }
   fData.push_back(data);
   fNEvents++;
   fNewData = kTRUE;
}

Double_t TKDE::operator()(const Double_t* x, const Double_t*) const {
   // The class's unary function: returns the kernel density estimate
   return (*this)(*x);
}

Double_t TKDE::operator()(Double_t x) const {
   // The class's unary function: returns the kernel density estimate
   if (fNewData) (const_cast<TKDE*>(this))->InitFromNewData();
   return (*fKernel)(x);
}

Double_t TKDE::GetMean() const {
   // return the mean of the data
   if (fNewData) (const_cast<TKDE*>(this))->InitFromNewData();
   return fMean;
}

Double_t TKDE::GetSigma() const {
   // return the standard deviation  of the data
   if (fNewData) (const_cast<TKDE*>(this))->InitFromNewData();
   return fSigma;
}

Double_t TKDE::GetRAMISE() const {
   // Returns the Root Asymptotic Mean Integrated Squared Error according to Silverman's rule of thumb with assumed Gaussian density
   Double_t result = 5. / 4. * fKernelSigmas2[fKernelType] * std::pow(fCanonicalBandwidths[fKernelType], 4) * std::pow(3. / (8. * std::sqrt(PI)) , -0.2) * fSigmaRob * std::pow(fNEvents, -0.8);
   return std::sqrt(result);
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
   Double_t minWeight = *weight * 0.05;
   std::vector<Double_t>::iterator data = fKDE->fData.begin();
   Double_t f = 0.0;
   for (; weight != weights.end(); ++weight, ++data) {
      f = (*fKDE->fKernel)(*data);
      *weight = std::max(*weight /= std::sqrt(f), minWeight);
      fKDE->fAdaptiveBandwidthFactor += std::log(f);
   }
   fKDE->fAdaptiveBandwidthFactor = fKDE->fUseMirroring ? APPRX_GEO_MEAN / fKDE->fSigmaRob : std::sqrt(std::exp(fKDE->fAdaptiveBandwidthFactor / fKDE->fData.size()));
   transform(weights.begin(), weights.end(), fWeights.begin(), std::bind2nd(std::multiplies<Double_t>(), fKDE->fAdaptiveBandwidthFactor));
 }

Double_t TKDE::TKernel::GetWeight(Double_t x) const {
   // Returns the bandwidth
   return fWeights[fKDE->Index(x)];
}

UInt_t TKDE::TKernel::GetNumberOfWeights() const {
   return fNWeights;
}

void TKDE::SetBinCentreData(Double_t xmin, Double_t xmax) {
   // Returns the bins' centres from the data for using with the binned option
   fData.assign(fNBins, 0.0);
   Double_t binWidth((xmax - xmin) / fNBins);
   for (UInt_t i = 0; i < fNBins; ++i) {
      fData[i] = xmin + (i + 0.5) * binWidth;
   }
}

void TKDE::SetBinCountData() {
   // Returns the bins' count from the data for using with the binned option
   fBinCount.resize(fNBins);
   for (UInt_t i = 0; i < fNEvents; ++i) {
      if (fEvents[i] >= fXMin && fEvents[i] < fXMax)
         fBinCount[Index(fEvents[i])]++;
   }
}

void TKDE::Draw(const Option_t* option) {
   TString plotOpt;
   std::map<TString, TString> canvasNameOpt;
   std::map<TString, TString> canvasTitleOpt;
   std::map<TString, TString> drawOpt;
   SetDrawOptions(option, plotOpt, canvasNameOpt, canvasTitleOpt, drawOpt);
   if (plotOpt.Contains("bothdoublecanvas")) {
      TCanvas* c1 = new TCanvas(canvasNameOpt["bothdoublecanvas"] + "_Estimate", canvasTitleOpt["bothdoublecanvas"] + " Estimate");
      c1->cd(1);
      DrawEstimate(drawOpt["estimate"]);
      TCanvas* c2 = new TCanvas(canvasNameOpt["bothdoublecanvas"] + "_Error", canvasTitleOpt["bothdoublecanvas"] + " Error");
      c2->cd(1);
      DrawError(drawOpt["error"]);
   } else if (plotOpt.Contains("bothsinglecanvas")) {
      TCanvas* c1 = new TCanvas(canvasNameOpt["bothsinglecanvas"], canvasTitleOpt["bothsinglecanvas"]);
      c1->Divide(2, 1);
      c1->cd(1);
      DrawEstimate(drawOpt["estimate"]);
      c1->cd(2);
      DrawError(drawOpt["error"]);
   } else if (plotOpt.Contains("estimate")) {
      TCanvas* c1 = new TCanvas(canvasNameOpt["estimate"], canvasTitleOpt["estimate"]);
      c1->cd(1);
      DrawEstimate(drawOpt["estimate"]);
   } else if (plotOpt.Contains("error")){
      TCanvas* c1 = new TCanvas(canvasNameOpt["error"], canvasTitleOpt["error"]);
      c1->cd(1);
      DrawError(drawOpt["error"]);
   }
}

void TKDE::DrawError(TString drawOpt) { // Prototype to improve
   UInt_t n = 100;
   Double_t* x = new Double_t[n + 1];
   Double_t* ex = new Double_t[n + 1];
   Double_t* y = new Double_t[n + 1];
   Double_t* ey = new Double_t[n + 1];
   for (UInt_t i = 0; i <= n; ++i) {
      x[i] = this->fXMin + i * (this->fXMax - this->fXMin) / n;
      y[i] = (*this)(x[i]);
      ey[i] = this->GetError(x[i]);
   }
   TGraphErrors* ge = new TGraphErrors(n,&x[0],&y[0],&ex[0],&ey[0]);
   ge->SetTitle("Error");
   ge->Draw(drawOpt.Data());
}

void TKDE::DrawEstimate(TString drawOpt) {
   Bool_t drawConfidenceInterval = kFALSE;
   if (drawOpt.Contains("confidenceinterval")) {
      drawConfidenceInterval = kTRUE;
      drawOpt.Remove(drawOpt.Index("confidenceinterval"), (Ssiz_t)(TString("confidenceinterval").Length()));
      drawOpt.Insert(0, "same");
   }
   GetKDEFunction()->Draw();
   if (drawConfidenceInterval) {
      TF1* upper = GetUpperFunction();
      upper->SetLineColor(kBlue);
      upper->Draw(drawOpt.Data());
      TF1* lower = GetLowerFunction();
      lower->SetLineColor(kRed);
      lower->Draw(drawOpt.Data());
   }
}

Double_t TKDE::GetFixedWeight() const {
   Double_t result = -1.;
   if (fIteration == TKDE::kAdaptive) {
      this->Warning("GetFixedWeight()", "Fixed iteration option not enabled. Returning %f.", result);
   } else {
      result = fKernel->GetFixedWeight();
   }
   return result;
}

const Double_t *  TKDE::GetAdaptiveWeights() const {
   if (fIteration != TKDE::kAdaptive) {
      this->Warning("GetFixedWeight()", "Adaptive iteration option not enabled. Returning a NULL pointer");
      return 0;
   }
   if (fNewData) (const_cast<TKDE*>(this))->InitFromNewData();
   return &(fKernel->GetAdaptiveWeights()).front();
}

Double_t TKDE::TKernel::GetFixedWeight() const {
   return fWeights[0];
}

const std::vector<Double_t> & TKDE::TKernel::GetAdaptiveWeights() const {
   return fWeights;
}

Double_t TKDE::TKernel::operator()(Double_t x) const {
   // The internal class's unary function: returns the kernel density estimate
   Double_t result(0.0);
   UInt_t n = fKDE->fData.size();
   Bool_t useBins = (fKDE->fBinCount.size() == n);
   for (UInt_t i = 0; i < n; ++i) {
      Double_t binCount = (useBins) ? fKDE->fBinCount[i] : 1.0;
      result += binCount / fWeights[i] * (*fKDE->fKernelFunction)((x - fKDE->fData[i]) / fWeights[i]);
      if (fKDE->fAsymLeft) {
         result -= binCount / fWeights[i] * (*fKDE->fKernelFunction)((x - (2. * fKDE->fXMin - fKDE->fData[i])) / fWeights[i]);
      }
      if (fKDE->fAsymRight) {
         result -= binCount / fWeights[i] * (*fKDE->fKernelFunction)((x - (2. * fKDE->fXMax - fKDE->fData[i])) / fWeights[i]);
      }
   }
   return result / fKDE->fNEvents;
}

UInt_t TKDE::Index(Double_t x) const {
   // Returns the indices (bins) for the binned weights
   Int_t bin = Int_t((x - fXMin) * fWeightSize);
   if (bin == (Int_t)fData.size()) return --bin;
   if (fUseMirroring && (fMirrorLeft || !fMirrorRight)) {
      bin += fData.size() / (fMirrorLeft + fMirrorRight + 1);
   }
   if (bin > (Int_t)fData.size()) {
      return (Int_t)(fData.size()) - 1;
   } else if (bin <= 0) {
      return 0;
   }
   return bin;
}

Double_t TKDE::UpperConfidenceInterval(const Double_t* x, const Double_t* p) const {
   // Returns the pointwise upper estimated density
   Double_t f = (*this)(x);
   Double_t sigma = GetError(*x);
   Double_t prob = 1. - (1.-*p)/2;   // this is 1.-alpha/2
   Double_t z = ROOT::Math::normal_quantile(prob, 1.0);
   return f + z * sigma;
}

Double_t TKDE::LowerConfidenceInterval(const Double_t* x, const Double_t* p) const {
   // Returns the pointwise lower estimated density
   Double_t f = (*this)(x);
   Double_t sigma = GetError(*x);
   Double_t prob = (1.-*p)/2;    // this is alpha/2
   Double_t z = ROOT::Math::normal_quantile(prob, 1.0);
   return f + z * sigma;
}


Double_t TKDE::GetBias(Double_t x) const {
   // Returns the pointwise approximate estimated density bias
   ROOT::Math::WrappedFunction<const TKDE&> kern(*this);
   ROOT::Math::RichardsonDerivator rd;
   rd.SetFunction(kern);
   Double_t df2 = rd.Derivative2(x);
   Double_t weight = fKernel->GetWeight(x); // Bandwidth
   return  0.5 * fKernelSigmas2[fKernelType] * std::pow(weight, 2) * df2;
}
Double_t TKDE::GetError(Double_t x) const {
   // Returns the pointwise sigma of estimated density
   Double_t kernelL2Norm = ComputeKernelL2Norm();
   Double_t f = (*this)(x);
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
      Error("CheckKernelValidity", "Kernel's integral is %f",unity);
   }
   Double_t mu = ComputeKernelMu();
   valid = valid && mu == 0.;
   if (!valid) {
      Error("CheckKernelValidity", "Kernel's mu is %f" ,mu);
   }
   Double_t sigma2 = ComputeKernelSigma2();
   valid = valid && sigma2 > 0 && sigma2 != std::numeric_limits<Double_t>::infinity();
   if (!valid) {
      Error("CheckKernelValidity", "Kernel's sigma2 is %f",sigma2);
   }
   if (!valid) {
      Error("CheckKernelValidity", "Validation conditions: the kernel's integral must be 1, the kernel's mu must be zero and the kernel's sigma2 must be finite positive to be a suitable kernel.");
      //exit(EXIT_FAILURE);
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

Double_t TKDE::ComputeMidspread () {
   // Computes the inter-quartile range from the data
   std::sort(fEvents.begin(), fEvents.end());
   Double_t quantiles[2] = {0.0, 0.0};
   Double_t prob[2] = {0.25, 0.75};
   TMath::Quantiles(fEvents.size(), 2, &fEvents[0], quantiles, prob);
   Double_t lowerquartile = quantiles[0];
   Double_t upperquartile = quantiles[1];
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

TF1* TKDE::GetKDEFunction(UInt_t npx, Double_t xMin , Double_t xMax) {
   //Returns the estimated density
   TString name = "KDEFunc_"; name+= GetName();
   TString title = "KDE "; title+= GetTitle();
   if (xMin >= xMax) { xMin = fXMin; xMax = fXMax; }
   fPDF = new TF1(name.Data(), this, xMin, xMax, 0);
   if (npx > 0) fPDF->SetNpx(npx);
   fPDF->SetTitle(title);
   return (TF1*)fPDF->Clone();
}

TF1* TKDE::GetPDFUpperConfidenceInterval(Double_t confidenceLevel, UInt_t npx, Double_t xMin , Double_t xMax) {
   // Returns the upper estimated density
   TString name;
   name.Form("KDE_UpperCL%f5.3_%s",confidenceLevel,GetName());
   if (xMin >= xMax) { xMin = fXMin; xMax = fXMax; }
   fUpperPDF = new TF1(name, this, &TKDE::UpperConfidenceInterval, xMin, xMax, 1, "TKDE", "UpperConfidenceInterval");
   fUpperPDF->SetParameter(0, confidenceLevel);
   if (npx > 0) fUpperPDF->SetNpx(npx);
   return (TF1*)fUpperPDF->Clone();
   }

TF1* TKDE::GetPDFLowerConfidenceInterval(Double_t confidenceLevel, UInt_t npx, Double_t xMin , Double_t xMax) {
   // Returns the upper estimated density
   TString name;
   name.Form("KDE_LowerCL%f5.3_%s",confidenceLevel,GetName());
   if (xMin >= xMax) { xMin = fXMin; xMax = fXMax; }
   fLowerPDF = new TF1(name, this, &TKDE::LowerConfidenceInterval, xMin, xMax, 1);
   fLowerPDF->SetParameter(0, confidenceLevel);
   if (npx > 0) fLowerPDF->SetNpx(npx);
   return (TF1*)fLowerPDF->Clone();
}

TF1* TKDE::GetKDEApproximateBias(UInt_t npx, Double_t xMin , Double_t xMax) {
   // Returns the approximate bias
   TString name = "KDE_Bias_"; name += GetName();
   if (xMin >= xMax) { xMin = fXMin; xMax = fXMax; }
   fApproximateBias = new TF1(name, this, &TKDE::ApproximateBias, xMin, xMax, 0);
   if (npx > 0) fApproximateBias->SetNpx(npx);
   return (TF1*)fApproximateBias->Clone();
}
