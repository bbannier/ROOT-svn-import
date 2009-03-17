// @(#)root/roostats:$Id: NeymanConstruction.h 26805 2009-01-13 17:45:57Z cranmer $
// Author: Kyle Cranmer   January 2009

/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//_________________________________________________
/*
BEGIN_HTML
<p>
NeymanConstruction is a concrete implementation of teh NeymanConstruction interface that, as the name suggests,
performs a NeymanConstruction.  
It produces a RooStats::PointSetInterval, which is a concrete implementation of the ConfInterval interface.  
</p>
<p>
The Neyman Construction is not a uniquely defined statistical technique, it requires that one specify an ordering rule 
or ordering principle, which is usually incoded by choosing a specific test statistic and limits of integration 
(corresponding to upper/lower/central limits).  As a result, this class must be configured with the corresponding
information before it can produce an interval.  Common configurations, such as the Feldman-Cousins approach, can be 
enforced by other light weight classes.
</p>
<p>
The Neyman Construction considers every point in the parameter space independently, no assumptions are 
made that the interval is connected or of a particular shape.  As a result, the PointSetInterval class is used to 
represent the result.  The user indicate which points in the parameter space to perform the constrution by providing
a PointSetInterval instance with the desired points.
</p>
<p>
This class is fairly light weight, because the choice of parameter points to be considered is factorized and so is the 
creation of the sampling distribution of the test statistic (which is done by a concrete class implementing the DistributionCreator interface).  As a result, this class basically just drives the construction by:
<ul>
<li> using a DistributionCreator to create the SamplingDistribution of a user-defined test statistic for each parameter point of interest,</li>
<li>defining the acceptance region in the data by finding the thresholds on the test statistic such that the integral of the sampling distribution is of the appropriate size and consistent with the limits of integration (eg. upper/lower/central limits), </li>
<li> and finally updating the PointSetInterval based on whether the value of the test statistic evaluated on the data are in the acceptance region.</li>
</p>
END_HTML
*/
//

#ifndef RooStats_NeymanConstruction
#include "RooStats/NeymanConstruction.h"
#endif

#ifndef RooStats_RooStatsUtils
#include "RooStats/RooStatsUtils.h"
#endif

#ifndef RooStats_PointSetInterval
#include "RooStats/PointSetInterval.h"
#endif

#include "RooStats/SamplingDistribution.h"

#include "RooDataSet.h"
#include "RooGlobalFunc.h"
#include "TFile.h"
#include "TTree.h"

ClassImp(RooStats::NeymanConstruction) ;

using namespace RooFit;
using namespace RooStats;


//_______________________________________________________
NeymanConstruction::NeymanConstruction() {
   // default constructor
  fWS = new RooWorkspace();
  fOwnsWorkspace = true;
  fDataName = "";
  fPdfName = "";
}


//_______________________________________________________
TList* NeymanConstruction::GenSamplingDistribution(const char* asciiFilePat) const {
//This method generates the sampling distribution for each point of the study. If a file path
//is provided, the distribution is saved in a root file. Returns the list of the distributions
//for each point.

  RooArgSet* point; 
  TList* SamplingList = new TList();
  TTree *savedTree = new TTree("SamplingDistributions","List of sampling distributions");
  vector<Double_t> *distrVector = 0;

  if(asciiFilePat && *asciiFilePat){
    savedTree->Branch("distrVector",&distrVector);
  }

  // loop over points to test
  for(Int_t i=0; i<fPointsToTest->numEntries(); ++i){
    // get a parameter point from the list of points to test.
    point = (RooArgSet*) fPointsToTest->get(i)->clone("temp");

    // the next line is where most of the time will be spent generating the sampling dist of the test statistic.
    SamplingDistribution* samplingDist = fTestStatSampler->GetSamplingDistribution(*point); 

    cout << "dbg: generating point number " << i << " of the interest interval" << endl;

    SamplingList->Add(samplingDist);

    if(asciiFilePat && *asciiFilePat){ 
      vector<Double_t> dummyVector = samplingDist->GetSamplingDistribution();
      distrVector = &dummyVector;
      savedTree->Fill();
    }

  }

  if(asciiFilePat && *asciiFilePat){
    TFile fSaveList(asciiFilePat,"RECREATE");
    fSaveList.cd();
    savedTree->Write();
    fSaveList.Close();
  }

  return SamplingList;
}

ConfInterval* NeymanConstruction::GetInterval() const {
  // Main interface to get a RooStats::ConfInterval.  
  // It constructs a RooStats::PointSetInterval.

  // local variables
  RooAbsData* data = fWS->data(fDataName);
  if(!data) {
    cout << "Data is not set, NeymanConstruction not initialized" << endl;
    return 0;
  }

  TList *SamplingList = (TList*)GenSamplingDistribution();

  vector<Double_t> dum = ((SamplingDistribution*)SamplingList->At(1))->GetSamplingDistribution();

  assert(SamplingList->GetSize() > 0);

  return run(SamplingList);
}

ConfInterval* NeymanConstruction::GetInterval(const char* asciiFilePat) const {
//This method returns a confidence interval exactly like GetInterval(), but
//instead of generating the sampling disribution (long computation) it takes
//the distribution from the file provided

  // local variables
  RooAbsData* data = fWS->data(fDataName);
  if(!data) {
    cout << "Data is not set, NeymanConstruction not initialized" << endl;
    return 0;
  }

  TList* SamplingList = new TList();
  TFile _fileSampling(asciiFilePat);
  _fileSampling.cd();
  TTree *savedTree = (TTree*)_fileSampling.Get("SamplingDistributions");
  vector<Double_t> *distrVector = 0;
  savedTree->SetBranchAddress("distrVector", &distrVector);


  for(Int_t i=0; i<fPointsToTest->numEntries(); ++i){
    savedTree->GetEntry(i);
    SamplingDistribution *dummyDist = new SamplingDistribution("TemplatedDistribution","",*distrVector);
      SamplingList->Add(dummyDist);
  }

  assert(SamplingList->GetSize() > 0);

  return run(SamplingList);
}

ConfInterval* NeymanConstruction::run(TList *SamplingList) const {
  //Main method to perform the interval calculation

  // local variables
  RooAbsData* data = fWS->data(fDataName);
  if(!data) {
    cout << "Data is not set, NeymanConstruction not initialized" << endl;
    return 0;
  }
  Int_t npass = 0;
  RooArgSet* point; 

  // loop over points to test
  for(Int_t i=0; i<fPointsToTest->numEntries(); ++i){
    // get a parameter point from the list of points to test.
    point = (RooArgSet*) fPointsToTest->get(i)->clone("temp");

    // the next line is where most of the time will be spent generating the sampling dist of the test statistic.
    SamplingDistribution* samplingDist = (SamplingDistribution*)SamplingList->At(i);

    // find the lower & upper thresholds on the test statistic that define the acceptance region in the data
    Double_t lowerEdgeOfAcceptance = samplingDist->InverseCDF( fLeftSideFraction * fSize );
    Double_t upperEdgeOfAcceptance = samplingDist->InverseCDF( 1. - ((1.-fLeftSideFraction) * fSize) );

    // get the value of the test statistic for this data set
    Double_t thisTestStatistic = fTestStatSampler->EvaluateTestStatistic(*data, *point );

    TIter      itr = point->createIterator();
    RooRealVar* myarg;
    while ((myarg = (RooRealVar *)itr.Next())) { 
      cout << myarg->GetName() << "=" << myarg->getVal() << " ";
    }
    std::cout << "\tdbg= " << lowerEdgeOfAcceptance << ", " 
	      << upperEdgeOfAcceptance << ", " << thisTestStatistic <<  " " <<
      (thisTestStatistic > lowerEdgeOfAcceptance && thisTestStatistic < upperEdgeOfAcceptance) << std::endl;

    // Check if this data is in the acceptance region
    if(thisTestStatistic > lowerEdgeOfAcceptance && thisTestStatistic < upperEdgeOfAcceptance) {
      // if so, set this point to true
      fPointsToTest->add(*point, 1.); 
      ++npass;
    }
    delete samplingDist;
    delete point;
  }
  std::cout << npass << " points in interval" << std::endl;

  // create an interval based fPointsToTest
  PointSetInterval* interval 
    = new PointSetInterval("ClassicalConfidenceInterval", "ClassicalConfidenceInterval", *fPointsToTest);
  
  delete data;
  return interval;
}

