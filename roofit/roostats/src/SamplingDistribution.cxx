// @(#)root/roostats:$Id: SamplingDistribution.h 26427 2009-01-13 15:45:36Z cranmer $

/*************************************************************************
 * Project: RooStats                                                     *
 * Package: RooFit/RooStats                                              *
 * Authors:                                                              *
 *   Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke       *
 *************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//____________________________________________________________________
/*
SamplingDistribution : 

This class simply holds a sampling distribution of some test statistic.  
The distribution can either be an emperical distribution (eg. the samples themselves) or
a weighted set of points (eg. for the FFT method).
The class supports merging.
*/


#include "RooStats/SamplingDistribution.h"
#include <algorithm>

/// ClassImp for building the THtml documentation of the class 
ClassImp(RooStats::SamplingDistribution)

using namespace RooStats;

//_______________________________________________________
SamplingDistribution::SamplingDistribution( const char *name, const char *title,
					    std::vector<Double_t>& samplingDist) :
  TNamed(name,title)
{
  // SamplingDistribution constructor
  fSamplingDist = samplingDist;
  // need to check STL stuff here.  Will this = operator work as wanted, or do we need:
  //  std::copy(samplingDist.begin(), samplingDist.end(), fSamplingDist.begin());
}

//_______________________________________________________
SamplingDistribution::SamplingDistribution( const char *name, const char *title,
					    std::vector<Double_t>& samplingDist, std::vector<Double_t>& sampleWeights) :
  TNamed(name,title)
{
  // SamplingDistribution constructor
  fSamplingDist = samplingDist;
  fSampleWeights = sampleWeights;
  // need to check STL stuff here.  Will this = operator work as wanted, or do we need:
  //  std::copy(samplingDist.begin(), samplingDist.end(), fSamplingDist.begin());
}

//_______________________________________________________
SamplingDistribution::SamplingDistribution( const char *name, const char *title) :
  TNamed(name,title)
{
   // SamplingDistribution constructor (with name and title)
}

//_______________________________________________________
SamplingDistribution::SamplingDistribution( ) :
  TNamed("SamplingDistribution_DefaultName","SamplingDistribution")
{
   // SamplingDistribution default constructor
}

//_______________________________________________________
SamplingDistribution::~SamplingDistribution()
{
   // SamplingDistribution destructor

   fSamplingDist.clear();
   fSampleWeights.clear();
}


//_______________________________________________________
void SamplingDistribution::Add(SamplingDistribution* other)
{
   // merge SamplingDistributions

  fSamplingDist = other->fSamplingDist;
  fSampleWeights = other->fSampleWeights;
  // need to check STL stuff here.  Will this = operator work as wanted, or do we need:
  //  std::copy(samplingDist.begin(), samplingDist.end(), fSamplingDist.begin());

  return;
}

//_______________________________________________________
Double_t SamplingDistribution::InverseCDF(Double_t pvalue)
{
   // merge SamplingDistributions

  if(fSamplingDist.size() == 0) return 0;

  // will need to deal with weights, but for now:
  sort(fSamplingDist.begin(), fSamplingDist.end());
  if(pvalue<0. ) return fSamplingDist[0]; // should return min of the test statistic instead
  if( pvalue > 1.) return fSamplingDist[fSamplingDist.size() - 1]; // should return the max of the test statistic instead
  
  return fSamplingDist[(unsigned int) (pvalue*fSamplingDist.size())];
}






