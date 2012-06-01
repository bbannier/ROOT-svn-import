// @(#)root/roostats:$Id$
// Authors: Sven Kreiss, Kyle Cranmer       October 2011
// based on MCMCCalculator
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
MultiNestCalculator is a concrete implementation of IntervalCalculator.  It uses a
MetropolisHastings object to construct a Markov Chain of data points in the
parameter space.  From this Markov Chain, this class can generate a
MultiNestInterval as per user specification.
</p>

<p>
The interface allows one to pass the model, data, and parameters via a
workspace and then specify them with names.
</p>

<p>
After configuring the calculator, one only needs to ask GetInterval(), which
will return an ConfInterval (MultiNestInterval in this case).
</p>
END_HTML
*/
//_________________________________________________

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif
#ifndef ROO_GLOBAL_FUNC
#include "RooGlobalFunc.h"
#endif
#ifndef ROO_ABS_REAL
#include "RooAbsReal.h"
#endif
#ifndef ROO_ARG_SET
#include "RooArgSet.h"
#endif
#ifndef ROO_ARG_LIST
#include "RooArgList.h"
#endif
#ifndef ROOSTATS_ModelConfig
#include "RooStats/ModelConfig.h"
#endif
#ifndef RooStats_RooStatsUtils
#include "RooStats/RooStatsUtils.h"
#endif
#ifndef ROOSTATS_MultiNestCalculator
#include "RooStats/MultiNestCalculator.h"
#endif
#ifndef ROOT_TIterator
#include "TIterator.h"
#endif
#ifndef ROO_PROD_PDF
#include "RooProdPdf.h"
#endif

#include "TROOT.h"
#include "TPluginManager.h"
#include "RooStats/MultiNestInterface.h"




ClassImp(RooStats::MultiNestCalculator);

using namespace std;
using namespace RooFit;
using namespace RooStats;

// default constructor
MultiNestCalculator::MultiNestCalculator() :
   fCeff( kTRUE ),
   fMModal( kTRUE ),
   fTol( 0.5 ),
   fEfr( 0.8 ),
   fMaxModes( 100 ),
   fNlive( 1000 ),

   fPdf(0), 
   fPriorPdf(0),
   fData(0),
   fAxes(0)
{
   fNumBins = 0;
   fUseKeys = kFALSE;
   fUseSparseHist = kFALSE;
   fSize = -1;
   fIntervalType = MultiNestInterval::kShortest;
   fLeftSideTF = -1;
   fEpsilon = -1;
   fDelta = -1;

   SetupBasicUsage();
}

// constructor from a Model Config with a basic settings package configured
// by SetupBasicUsage()
MultiNestCalculator::MultiNestCalculator(RooAbsData& data, const ModelConfig & model) :
   fCeff( kTRUE ),
   fMModal( kTRUE ),
   fTol( 0.5 ),
   fEfr( 0.8 ),
   fMaxModes( 100 ),
   fNlive( 1000 ),

   fData(&data),
   fAxes(0)
{
   SetModel(model);
   SetupBasicUsage();
}

void MultiNestCalculator::SetModel(const ModelConfig & model) {
   // set the model
   fPdf = model.GetPdf();  
   fPriorPdf = model.GetPriorPdf();
   fPOI.removeAll();
   fNuisParams.removeAll();
   if (model.GetParametersOfInterest())
      fPOI.add(*model.GetParametersOfInterest());
   if (model.GetNuisanceParameters())
      fNuisParams.add(*model.GetNuisanceParameters());
}

// Constructor for automatic configuration with basic settings.
// Finds a 95% confidence interval.
void MultiNestCalculator::SetupBasicUsage()
{
   fCeff = kTRUE;
   fMModal = kTRUE;
   fTol = 0.5;
   fEfr = 0.8;
   fMaxModes = 100;
   fNlive = 1000;

   fNumBins = 50;
   fUseKeys = kFALSE;
   fUseSparseHist = kFALSE;
   SetTestSize(0.05);
   fIntervalType = MultiNestInterval::kShortest;
   fLeftSideTF = -1;
   fEpsilon = -1;
   fDelta = -1;
}

void MultiNestCalculator::SetLeftSideTailFraction(Double_t a)
{
   if (a < 0 || a > 1) {
      coutE(InputArguments) << "MultiNestCalculator::SetLeftSideTailFraction: "
         << "Fraction must be in the range [0, 1].  "
         << a << "is not allowed." << endl;
      return;
   }

   fLeftSideTF = a;
   fIntervalType = MultiNestInterval::kTailFraction;
}

MultiNestInterval* MultiNestCalculator::GetInterval() const
{
   // Main interface to get a RooStats::ConfInterval.  

   if (fData == NULL) { cout << "Data not found." << endl; return 0; }
   if (fPdf == NULL) { cout << "Pdf not found." << endl; return 0; }
   if (fPOI.getSize() == 0) { cout << "No POI specified." << endl; return 0; }

   if (fSize < 0) {
      coutE(InputArguments) << "MultiNestCalculator::GetInterval: "
         << "Test size/Confidence level not set.  Returning NULL." << endl;
      return NULL;
   }

   // if a proposal funciton has not been specified create a default one
   bool usePriorPdf = false;//(fPriorPdf != 0);

   // if prior is given create product 
   RooAbsPdf * prodPdf = fPdf;
   if (usePriorPdf) { 
      TString prodName = TString("product_") + TString(fPdf->GetName()) + TString("_") + TString(fPriorPdf->GetName() );   
      prodPdf = new RooProdPdf(prodName,prodName,RooArgList(*fPdf,*fPriorPdf) );
   }

   RooArgSet* constrainedParams = prodPdf->getParameters(*fData);
   RemoveConstantParameters( constrainedParams );
   RooAbsReal* nll = prodPdf->createNLL(*fData, Constrain(*constrainedParams), CloneData(kFALSE));
   delete constrainedParams;

   nll->setEvalErrorLoggingMode( RooAbsReal::CountErrors );

   RooArgSet* params = nll->getParameters(*fData);
   RemoveConstantParameters(params);
   RooArgList paramsList( *params );
   // RooArgList paramsList( fPOI );
   // RooArgSet nuisPars( fNuisParams );
   // RemoveConstantParameters( &nuisPars );
   // paramsList.add( nuisPars );
   // RooArgSet *params = new RooArgSet( paramsList ); // for convenience

   // This is the map so that when MultiNest says
   // "Parameter 62 converges to edge of prior" you know
   // which parameter it is.
   cout << endl << "--- MultiNest: list of parameters ---" << endl;
   for(int i=0; i < paramsList.getSize(); i++) {
      cout << "  " << i+1 << "\t:  " << paramsList.at(i)->GetName() << endl;
   }
   cout << endl;


   TPluginHandler *h = NULL;
   MultiNestInterface *m = NULL;
   if ((h = gROOT->GetPluginManager()->FindHandler("RooStats::MultiNestInterface", "MultiNest"))) {
      if (h->LoadPlugin() == -1) {
         cout << "Error loading MultiNest" << endl;
         return NULL;
      }
      m = reinterpret_cast<RooStats::MultiNestInterface*>( h->ExecPlugin(0) );
   }
   if( !m ) {
      cout << "ERROR: MultiNest plugin could not be loaded. Library missing?" << endl;
      return NULL;
   }


   // apply configuration ...
   m->NegLogLike( nll, paramsList );
   m->Tol( fTol );
   m->Ceff( fCeff );
   m->MModal( fMModal );
   m->Efr( fEfr );
   m->MaxModes( fMaxModes );
   m->Nlive( fNlive );
   m->SetPrior( fPriorPdf );
   //m->InitMPI( true );

   // ... and run
   m->Run();




   RooArgSet postParams;
   if( fPosteriorParams.getSize() > 0 ) postParams.add( fPosteriorParams );
   else postParams.add( *params );

   TString chainName = TString("posterior_") + TString(GetName());
   PosteriorDistribution* chain = new PosteriorDistribution(chainName, chainName, postParams);

   // fill chain
   vector< vector<double> >& pd = m->GetPosteriorDistribution();
   vector< vector<double> >::iterator it;
   for(it = pd.begin(); it != pd.end(); it++) {
      if( (int)(*it).size()-2 != paramsList.getSize() ) {
         cout << "!!!!!!!!!! mismatching params" << endl;
         continue;
      }

      for( int i=0; i < paramsList.getSize(); i++ )
         dynamic_cast<RooRealVar&>(paramsList[i]).setVal( (*it)[i] );

      double nllVal = -(*it)[it->size()-2]; // second last value
      double weight = (*it).back();
      chain->Add( postParams, nllVal, weight );
   }





   TString intervalName = TString("MultiNestInterval_") + TString(GetName() );
   MultiNestInterval* interval = new MultiNestInterval(intervalName, fPOI, *chain);
   if (fAxes != NULL)
      interval->SetAxes(*fAxes);
   interval->SetUseKeys(fUseKeys);
   interval->SetUseSparseHist(fUseSparseHist);
   interval->SetIntervalType(fIntervalType);
   if (fIntervalType == MultiNestInterval::kTailFraction)
      interval->SetLeftSideTailFraction(fLeftSideTF);
   if (fEpsilon >= 0)
      interval->SetEpsilon(fEpsilon);
   if (fDelta >= 0)
      interval->SetDelta(fDelta);
   interval->SetConfidenceLevel(1.0 - fSize);

   //if (useDefaultPropFunc) delete fPropFunc;
   if (usePriorPdf) delete prodPdf;
   delete params;
   delete nll;

   return interval;
}
