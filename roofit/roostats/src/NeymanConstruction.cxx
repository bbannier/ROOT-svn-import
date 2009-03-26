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
#include "RooStats/ToyMCSampler.h"

#include "RooDataSet.h"
#include "RooGlobalFunc.h"
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"

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
  fConfBelt = 0; // constructed with tree data
}

//_______________________________________________________
void NeymanConstruction::SetAdaptiveSampling(Double_t testStat, Int_t ndof)  const{
  // this algorithm makes sense if the test statistic is a likelihood ratio and p-values can
  // be approximated with Wilks' theorem.
  ToyMCSampler* toyMCSampler = dynamic_cast<ToyMCSampler*>(fTestStatSampler);
  if(toyMCSampler){
    // p-value based on Wilks' theorem
    double pWilks = TMath::Prob(testStat, ndof );
    double quantile = TMath::ChisquareQuantile(pWilks, ndof );
    // some parameters for nToys per experiment
    int nominalToys = (int)1./fSize; // nominal situation 
    int maxToys = (int) 10./fSize;  // ntoys will be maximized for this

    // based on nominal number of toys and pWilks, the expected number of toys in tail is
    // pWilks*nominalToys.  Prob to see 1 or more in tail is 1-Pois(0|expectation) = 1.-exp(-expectation)
    double probToyInTail = 1.-exp(-nominalToys*pWilks);
    // convert this fluctuation probability into an information measure \sum -p log(p) of two possibilities
    double informationInToy = (-probToyInTail) * log(probToyInTail) 
      - (1.-probToyInTail)*log(1.-probToyInTail);
    int nToysForThisPoint =  TMath::Max( 10, (int) (maxToys*informationInToy));
    if(pWilks > fSize) // mainly need to protect against testStat < lower bound in this case
      nToysForThisPoint = TMath::Max( (int)(3./(1.-pWilks)), nToysForThisPoint ); // insure some samples in good regions
    cout << "test stat = " << testStat << endl
	 << "ndof = "<< ndof  << endl
	 << "pWilks = " << pWilks  << endl
	 << "alpha = " << fSize << endl
	 << "prob for at least one toy in tail = " << probToyInTail << endl
	 << "info in toy = " << informationInToy << endl
	 << "toys for this point = " << nToysForThisPoint << endl;
    
    toyMCSampler->SetNEventsPerToy( nToysForThisPoint  );
  } else {
    cout << "Adaptive sampling can only be used with a ToyMCSampler" << endl;
  }

}

//_______________________________________________________
ConfInterval* NeymanConstruction::GetInterval() const {
  // Main interface to get a RooStats::ConfInterval.  
  // It constructs a RooStats::SetInterval.

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


     // get the value of the test statistic for this data set
    Double_t thisTestStatistic = fTestStatSampler->EvaluateTestStatistic(*data, *point );

    // find the lower & upper thresholds on the test statistic that 
    // define the acceptance region in the data

    SamplingDistribution* samplingDist=0;
    Double_t sigma;
    Double_t upperEdgeOfAcceptance, upperEdgeMinusSigma, upperEdgePlusSigma;
    Double_t lowerEdgeOfAcceptance, lowerEdgeMinusSigma, lowerEdgePlusSigma;
    Int_t totalMC = 10, additionalMC=0;
    ToyMCSampler* toyMCSampler = dynamic_cast<ToyMCSampler*>(fTestStatSampler);
    if(fAdaptiveSampling && toyMCSampler) {
      do{
	// this will be executed first, then while conditioned checked
	// as an exit condition for the loop.

	// the next line is where most of the time will be spent 
	// generating the sampling dist of the test statistic.
	additionalMC = 2*totalMC; // grow by a factor of two
	samplingDist = 
	  toyMCSampler->AppendSamplingDistribution(*point, 
						   samplingDist, 
						   additionalMC); 
	totalMC=samplingDist->GetSize();

	sigma = 1;
	upperEdgeOfAcceptance = 
	  samplingDist->InverseCDF( 1. - ((1.-fLeftSideFraction) * fSize) , 
				    sigma, upperEdgePlusSigma);
	sigma = -1;
	samplingDist->InverseCDF( 1. - ((1.-fLeftSideFraction) * fSize) , 
				  sigma, upperEdgeMinusSigma);
	
	sigma = 1;
	lowerEdgeOfAcceptance = 
	  samplingDist->InverseCDF( fLeftSideFraction * fSize , 
				    sigma, lowerEdgePlusSigma);
	sigma = -1;
	samplingDist->InverseCDF( fLeftSideFraction * fSize , 
				  sigma, lowerEdgeMinusSigma);
	
	cout << "dbg:"
	     << "total MC = " << totalMC 
	     << " this test stat = " << thisTestStatistic << endl
	     << " upper edge -1sigma = " << upperEdgeMinusSigma
	     << " upperEdge = "<<upperEdgeOfAcceptance
	     << " upper edge +1sigma = " << upperEdgePlusSigma << endl
	     << " lower edge -1sigma = " << lowerEdgeMinusSigma
	     << " lowerEdge = "<<lowerEdgeOfAcceptance
	     << " lower edge +1sigma = " << lowerEdgePlusSigma << endl;
      } while(( 
	      (thisTestStatistic <= upperEdgeOfAcceptance &&
	       thisTestStatistic > upperEdgeMinusSigma)
	      || (thisTestStatistic >= upperEdgeOfAcceptance &&
		  thisTestStatistic < upperEdgePlusSigma)
	      || (thisTestStatistic <= lowerEdgeOfAcceptance &&
		  thisTestStatistic > lowerEdgeMinusSigma)
	      || (thisTestStatistic >= lowerEdgeOfAcceptance &&
		  thisTestStatistic < lowerEdgePlusSigma) 
		) && (totalMC < 100./fSize)
	      ) ; // need ; here
    } else {
      // the next line is where most of the time will be spent 
      // generating the sampling dist of the test statistic.
      samplingDist = fTestStatSampler->GetSamplingDistribution(*point); 
      
      lowerEdgeOfAcceptance = 
	samplingDist->InverseCDF( fLeftSideFraction * fSize );
      upperEdgeOfAcceptance = 
	samplingDist->InverseCDF( 1. - ((1.-fLeftSideFraction) * fSize) );
    }
    
    // add acceptance region to ConfidenceBelt
    //    fConfBelt->AddAcceptanceRegion(*point, 
    //			   lowerEdgeOfAcceptance, 
    //			   upperEdgeOfAcceptance);


    // printout some debug info
    TIter      itr = point->createIterator();
    RooRealVar* myarg;
    while ((myarg = (RooRealVar *)itr.Next())) { 
      cout << myarg->GetName() << "=" << myarg->getVal() << " ";
    }
    std::cout << "\tdbg: "<< i<<"/"<<fPointsToTest->numEntries()<<" : " << lowerEdgeOfAcceptance << ", " 
    	      << upperEdgeOfAcceptance << ", " << thisTestStatistic <<  " " <<
      (thisTestStatistic >= lowerEdgeOfAcceptance && thisTestStatistic <= upperEdgeOfAcceptance) << std::endl << std::endl;

    // Check if this data is in the acceptance region
    if(thisTestStatistic >= lowerEdgeOfAcceptance && thisTestStatistic <= upperEdgeOfAcceptance) {
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

    cout << "dbg: generating point number " << i << " of " << fPointsToTest->numEntries() << " in the interest interval" << endl;

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

//_______________________________________________________
ConfInterval* NeymanConstruction::GetIntervalUsingList() const {
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

  return Run(SamplingList);
}

//_______________________________________________________
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

  return Run(SamplingList);
}

//_______________________________________________________
ConfInterval* NeymanConstruction::Run(TList *SamplingList) const {
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

