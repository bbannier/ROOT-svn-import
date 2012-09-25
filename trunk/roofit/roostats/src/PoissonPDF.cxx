

/*****************************************************************************
 * Copyright (C) 2012, ROOT Development Team.                                *
 * All rights reserved.                                                      *
 *****************************************************************************/

/*
 * BEGIN_HTML
 * Poisson probability density function
 * END_HTML
 */



//_____________________________________________________________________________
#include "TMath.h"
#include "RooRandom.h"
#include "RooStats/PoissonPDF.h"


using namespace std;
using namespace RooStats;

ClassImp(PoissonPDF);

//_____________________________________________________________________________
PoissonPDF::PoissonPDF(
   const char* name, 
   const char* title, 
   RooAbsReal& obs,
   RooAbsReal& mu
) :
   RooAbsPdf(name, title),
   fObs("obs", "Observable", this, obs),
   fMean("mean", "Mean", this, mu)
{
   // Constructor
}


//_____________________________________________________________________________
PoissonPDF::PoissonPDF(const PoissonPDF& other, const char* newName) :
   RooAbsPdf(other, newName),
   fObs("obs", this, other.fObs),
   fMean("mean", this, other.fMean)
{
   // Copy constructor
}


//_____________________________________________________________________________
Double_t PoissonPDF::evaluate() const
{
   return TMath::Poisson(fObs, fMean);
}


//_____________________________________________________________________________
Double_t PoissonPDF::getLogVal(const RooArgSet*) const
{
   return fLogMean * fObs - fMean;
}


//_____________________________________________________________________________
Double_t PoissonPDF::analyticalIntegral(Int_t, const char*) const
{
   // TODO: implement
   return 0.0;
}


//_____________________________________________________________________________
void PoissonPDF::generateEvent(Int_t)
{
   Double_t obsGenerated;
   Double_t obsMin = fObs.min();
   Double_t obsMax = fObs.max();

   do {
      // FIXME: replace random generator, make more efficient
      obsGenerated = RooRandom::randomGenerator()->Poisson(fMean);
   }  while(obsGenerated > obsMax && obsGenerated < obsMin);
   // FIXME: difference vs RooFit (>= obsMax)&&(<= obsMin)
}

