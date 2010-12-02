// @(#)root/mathcore:$Id$
// Authors: B. Rabacal   11/2010

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2010 , LCG ROOT MathLib Team                         *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// implementation file for class TKDTreeBinning
//

#include <iostream>
#include <iterator>
#include <cassert>
#include <algorithm>
#include <limits>
#include <cmath>

#include "TKDTreeBinning.h"

ClassImp(TKDTreeBinning)


TKDTreeBinning::TKDTreeBinning(UInt_t dataSize, UInt_t dataDim, Double_t* data, UInt_t nBins)
// Class's constructor
: fData(0), fBinMinEdges(std::vector<Double_t>()), fBinMaxEdges(std::vector<Double_t>()), fDataBins((TKDTreeID*)0), fDim(dataDim),
fDataSize(dataSize), fDataThresholds(std::vector<std::pair<Double_t, Double_t> >(fDim, std::make_pair(0., 0.))) {
   if (data) {
      SetData(data);
      SetNBins(nBins);
   } else {
      if (!fData)
         this->Warning("TKDTreeBinning", "Data is nil. Nothing is built.");
   }
}

TKDTreeBinning::~TKDTreeBinning() {
   // Class's destructor
   if (fData)     delete[] fData;
   if (fDataBins) delete   fDataBins;
}

void TKDTreeBinning::SetNBins(UInt_t bins) {
   fNBins = bins;
   if (fDim && fNBins && fDataSize) {
      if (fDataSize / fNBins) {
         Bool_t remainingData = fDataSize % fNBins;
         if (remainingData) {
            fNBins += 1;
            this->Info("SetNBins", "Number of bins is not enough to hold the data. Extra bin added.");
         }
         fDataBins = new TKDTreeID(fDataSize, fDim, fDataSize / (fNBins - remainingData));
         SetTreeData();
         fDataBins->Build();
         SetBinsEdges();
      } else {
         fDataBins = (TKDTreeID*)0;
         this->Warning("SetNBins", "Number of bins is bigger than data size. Nothing is built.");
      }
   } else {
      fDataBins = (TKDTreeID*)0;
      if (!fDim)
         this->Warning("SetNBins", "Data dimension is nil. Nothing is built.");
      if (!fNBins)
         this->Warning("SetNBins", "Number of bins is nil. Nothing is built.");
      if (!fDataSize)
         this->Warning("SetNBins", "Data size is nil. Nothing is built.");
   }
}


void TKDTreeBinning::SetData(Double_t* data) {
   // Sets the data and finds minimum and maximum by dimensional coordinate
   fData = new Double_t*[fDim];
   for (UInt_t i = 0; i < fDim; ++i) {
      fData[i] = &data[i * fDataSize];
      fDataThresholds[i] = std::make_pair(*std::min_element(fData[i], fData[i] + fDataSize), *std::max_element(fData[i], fData[i] + fDataSize));
   }
}

void TKDTreeBinning::SetTreeData() {
   // Sets the data for constructing the kD-tree
   for (UInt_t i = 0; i < fDim; ++i)
      fDataBins->SetData(i, fData[i]);
}

void TKDTreeBinning::SetBinsEdges() {
   // Sets the bins' edges
   Double_t* rawBinEdges = fDataBins->GetBoundary(fDataBins->GetNNodes());
   fCheckedBinEdges = std::vector<std::vector<std::pair<Bool_t, Bool_t> > >(fDim, std::vector<std::pair<Bool_t, Bool_t> >(fNBins, std::make_pair(kFALSE, kFALSE)));
   fCommonBinEdges = std::vector<std::map<Double_t, std::vector<UInt_t> > >(fDim, std::map<Double_t, std::vector<UInt_t> >());
   SetCommonBinEdges(rawBinEdges);
   ReadjustMinBinEdges(rawBinEdges);
   ReadjustMaxBinEdges(rawBinEdges);
   SetBinMinMaxEdges(rawBinEdges);
}

void TKDTreeBinning::SetBinMinMaxEdges(Double_t* binEdges) {
   // Sets the bins' minimum and maximum edges
   fBinMinEdges.reserve(fNBins * fDim);
   fBinMaxEdges.reserve(fNBins * fDim);
   for (UInt_t i = 0; i < fNBins; ++i)
      for (UInt_t j = 0; j < fDim; ++j) {
         fBinMinEdges.push_back(binEdges[(i * fDim + j) * 2]);
         fBinMaxEdges.push_back(binEdges[(i * fDim + j) * 2 + 1]);
      }
}

void TKDTreeBinning::SetCommonBinEdges(Double_t* binEdges) {
   // Sets indexing on the bin edges which have common boundaries
   for (UInt_t i = 0; i < fDim; ++i) {
      for (UInt_t j = 0; j < fNBins; ++j) {
         Double_t binEdge = binEdges[(j * fDim + i) * 2];
         if(fCommonBinEdges[i].find(binEdge) == fCommonBinEdges[i].end()) {
            std::vector<UInt_t> commonBinEdges;
            for (UInt_t k = 0; k < fNBins; ++k) {
               UInt_t minBinEdgePos = (k * fDim + i) * 2;
               if (std::fabs(binEdge - binEdges[minBinEdgePos]) < std::numeric_limits<Double_t>::epsilon())
                  commonBinEdges.push_back(minBinEdgePos);
               UInt_t maxBinEdgePos = ++minBinEdgePos;
               if (std::fabs(binEdge - binEdges[maxBinEdgePos]) < std::numeric_limits<Double_t>::epsilon())
                  commonBinEdges.push_back(maxBinEdgePos);
            }
            fCommonBinEdges[i][binEdge] = commonBinEdges;
         }
      }
   }
}

void TKDTreeBinning::ReadjustMinBinEdges(Double_t* binEdges) {
   // Readjusts the bins' minimum edge
   for (UInt_t i = 0; i < fDim; ++i) {
      for (UInt_t j = 0; j < fNBins; ++j) {
         if (!fCheckedBinEdges[i][j].first) {
            Double_t binEdge = binEdges[(j * fDim + i) * 2];
            Double_t adjustedBinEdge = binEdge;
            std::vector<Double_t> data(fData[i], fData[i] + fDataSize);
            std::vector<Double_t>::iterator dataOnBinEdges = std::find(data.begin(), data.end(), binEdge);
            Bool_t foundDataOnBinEdges = dataOnBinEdges != data.end();
            do {
               adjustedBinEdge -= 1.5 * std::numeric_limits<Double_t>::epsilon();
               data.erase(dataOnBinEdges);
               foundDataOnBinEdges = std::find(data.begin(), data.end(), adjustedBinEdge) != data.end();
            } while(foundDataOnBinEdges);
            for (UInt_t k = 0; k < fCommonBinEdges[i][binEdge].size(); ++k) {
               UInt_t binEdgePos = fCommonBinEdges[i][binEdge][k];
               Bool_t isMinBinEdge = binEdgePos % 2 == 0;
               UInt_t bin = isMinBinEdge ? (binEdgePos / 2 - i) / fDim : ((binEdgePos - 1) / 2 - i) / fDim;
               binEdges[binEdgePos] = adjustedBinEdge;
               if (isMinBinEdge)
                  fCheckedBinEdges[i][bin].first = kTRUE;
               else
                  fCheckedBinEdges[i][bin].second = kTRUE;
            }
         }
      }
   }
}

void TKDTreeBinning::ReadjustMaxBinEdges(Double_t* binEdges) {
   // Readjusts the bins' maximum edge
   for (UInt_t i = 0; i < fDim; ++i) {
      for (UInt_t j = 0; j < fNBins; ++j) {
         if (!fCheckedBinEdges[i][j].second) {
            Double_t& binEdge = binEdges[(j * fDim + i) * 2 + 1];
            binEdge += 1.5 * std::numeric_limits<Double_t>::epsilon();
         }
      }
   }
}

const Double_t* TKDTreeBinning::GetBinsMinEdges() const {
   // Returns the bins' minimum edges
   if (fDataBins)
      return &fBinMinEdges[0];
   this->Warning("GetBinsMinEdges", "Binning kd-tree is nil. No bin edges retrieved.");
   this->Info("GetBinsMinEdges", "Returning null pointer.");
   return (Double_t*)0;
}

const Double_t* TKDTreeBinning::GetBinsMaxEdges() const {
   // Returns the bins' maximum edges
   if (fDataBins)
      return &fBinMaxEdges[0];
   this->Warning("GetBinsMaxEdges", "Binning kd-tree is nil. No bin edges retrieved.");
   this->Info("GetBinsMaxEdges", "Returning null pointer.");
   return (Double_t*)0;
}

std::pair<const Double_t*, const Double_t*> TKDTreeBinning::GetBinsEdges() const {
   // Returns the bins' edges
   if (fDataBins)
      return std::make_pair(GetBinsMinEdges(), GetBinsMaxEdges());
   this->Warning("GetBinsEdges", "Binning kd-tree is nil. No bin edges retrieved.");
   this->Info("GetBinsEdges", "Returning null pointer pair.");
   return std::make_pair((Double_t*)0, (Double_t*)0);
}

const Double_t* TKDTreeBinning::GetBinMinEdges(UInt_t bin) const {
   // Returns the bin's minimum edges. 'bin' is between 0 and fNBins - 1
   if (fDataBins)
      if (bin < fNBins)
         return &fBinMinEdges[bin * fDim];
      else
         this->Warning("GetBinMinEdges", "No such bin. 'bin' is between 0 and %d", fNBins - 1);
   else
      this->Warning("GetBinMinEdges", "Binning kd-tree is nil. No bin edges retrieved.");
   this->Info("GetBinMinEdges", "Returning null pointer.");
   return (Double_t*)0;
}

const Double_t* TKDTreeBinning::GetBinMaxEdges(UInt_t bin) const {
   // Returns the bin's maximum edges. 'bin' is between 0 and fNBins - 1
   if (fDataBins)
      if (bin < fNBins)
         return &fBinMaxEdges[bin * fDim];
      else
         this->Warning("GetBinMaxEdges", "No such bin. 'bin' is between 0 and %d", fNBins - 1);
   else
      this->Warning("GetBinMaxEdges", "Binning kd-tree is nil. No bin edges retrieved.");
   this->Info("GetBinMaxEdges", "Returning null pointer.");
   return (Double_t*)0;
}

std::pair<const Double_t*, const Double_t*> TKDTreeBinning::GetBinEdges(UInt_t bin) const {
   // Returns the bin's edges. 'bin' is between 0 and fNBins - 1
   if (fDataBins)
      if (bin < fNBins)
         return std::make_pair(GetBinMinEdges(bin), GetBinMaxEdges(bin));
      else
         this->Warning("GetBinEdges", "No such bin. 'bin' is between 0 and %d", fNBins - 1);
   else
      this->Warning("GetBinEdges", "Binning kd-tree is nil. No bin edges retrieved.");
   this->Info("GetBinEdges", "Returning null pointer pair.");
   return std::make_pair((Double_t*)0, (Double_t*)0);
}

UInt_t TKDTreeBinning::GetNBins() const {
   // Returns the number of bins
   return fNBins;
}

UInt_t TKDTreeBinning::GetDim() const {
   // Returns the number of dimensions
   return fDim;
}

UInt_t TKDTreeBinning::GetBinContent(UInt_t bin) const {
   // Returns the number of points in bin. 'bin' is between 0 and fNBins - 1
   if(bin < fNBins - 1)
      return fDataBins->GetBucketSize();
   if (bin == fNBins - 1)
      return !(fDataSize % fNBins) ? fDataBins->GetBucketSize() : fDataSize % fNBins;
   this->Warning("GetBinContent", "No such bin. Returning 0.");
   this->Info("GetBinContent", "'bin' is between 0 and %d.", fNBins - 1);
   return 0;
}

TKDTreeID* TKDTreeBinning::GetTree() const {
   // Returns the kD-Tree structure of the binning
   if (fDataBins)
      return fDataBins;
   this->Warning("GetTree", "Binning kd-tree is nil. No embedded kd-tree retrieved. Returning null pointer.");
   return (TKDTreeID*)0;
}

const Double_t* TKDTreeBinning::GetDimData(UInt_t dim) const {
   // Returns the data in the dim coordinate. 'dim' is between 0 and fDim - 1
   if(dim < fDim)
      return fData[dim];
   this->Warning("GetDimData", "No such dimensional coordinate. No coordinate data retrieved. Returning null pointer.");
   this->Info("GetDimData", "'dim' is between 0 and %d.", fDim - 1);
   return 0;
}

Double_t TKDTreeBinning::GetDataMin(UInt_t dim) const {
   // Returns the data minimum in the dim coordinate. 'dim' is between 0 and fDim - 1
   if(dim < fDim)
      return fDataThresholds[dim].first;
   this->Warning("GetDataMin", "No such dimensional coordinate. No coordinate data minimum retrieved. Returning +inf.");
   this->Info("GetDataMin", "'dim' is between 0 and %d.", fDim - 1);
   return std::numeric_limits<Double_t>::infinity();
}

Double_t TKDTreeBinning::GetDataMax(UInt_t dim) const {
   // Returns the data maximum in the dim coordinate. 'dim' is between 0 and fDim - 1
   if(dim < fDim)
      return fDataThresholds[dim].second;
   this->Warning("GetDataMax", "No such dimensional coordinate. No coordinate data maximum retrieved. Returning -inf.");
   this->Info("GetDataMax", "'dim' is between 0 and %d.", fDim - 1);
   return -1 * std::numeric_limits<Double_t>::infinity();
}

Double_t TKDTreeBinning::GetBinDensity(UInt_t bin) const {
   // Returns the density in bin. 'bin' is between 0 and fNBins - 1
   if(bin < fNBins) {
      Double_t area = GetBinArea(bin);
      if (!area)
         this->Warning("GetBinDensity", "Area is null. Returning -1.");
      return GetBinContent(bin) / area;
   }
   this->Warning("GetBinDensity", "No such bin. Returning -1.");
   this->Info("GetBinDensity", "'bin' is between 0 and %d.", fNBins - 1);
   return -1.;
}

Double_t TKDTreeBinning::GetBinArea(UInt_t bin) const {
   // Returns the (hyper)area of bin. 'bin' is between 0 and fNBins - 1
   if(bin < fNBins) {
      std::pair<const Double_t*, const Double_t*> binEdges = GetBinEdges(bin);
      Double_t area = 1.;
      for (UInt_t i = 0; i < fDim; ++i) {
         area *= binEdges.second[i] - binEdges.first[i];
      }
      return area;
   }
   this->Warning("GetBinArea", "No such bin. Returning 0.");
   this->Info("GetBinArea", "'bin' is between 0 and %d.", fNBins - 1);
   return 0.;
}

const Double_t* TKDTreeBinning::GetSortedOneDimensionalBinning() const {
   // Returns the sorted minimum edges for one dimensional binning only.
   if (fDim == 1) {
      const Double_t* binsMinEdges = GetBinsMinEdges();
      UInt_t indices[fNBins];
      TMath::Sort(fNBins, binsMinEdges, indices, kFALSE);
      Double_t binsEdges[fNBins + 1];
      for(UInt_t i = 0; i < fNBins; ++i)
         binsEdges[i] = binsMinEdges[indices[i]];
      binsEdges[fNBins] = binsMinEdges[indices[fNBins - 1]]; // Upper edge
   }
   this->Warning("GetSortedOneDimensionalBinning", "Data is multidimensional. No sorted bin edges retrieved. Returning null pointer.");
   this->Info("GetSortedOneDimensionalBinning", "This method can only be invoked if the data is a one dimensional set");
   return 0;
}
