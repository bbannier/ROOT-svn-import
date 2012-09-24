

/*****************************************************************************
 * Copyright (C) 2012, ROOT Development Team.                                *
 * All rights reserved.                                                      *
 *****************************************************************************/

/*
 * BEGIN_HTML
 * Gaussian probability density function
 * END_HTML
 */



//_____________________________________________________________________________
#include <cmath>
#include "Math/Math.h"
#include "Math/SpecFunc.h"
#include "RooStats/GaussianPdf.h"
#include "RooAbsPdf.h"
#include "RooRandom.h"

using namespace std;
using namespace RooStats;

ClassImp(GaussianPdf);

//_____________________________________________________________________________
GaussianPdf::GaussianPdf(
   const char* name, 
   const char* title, 
   RooAbsReal& obs, 
   RooAbsReal& mu,  
   RooAbsReal& sig 
) :
   RooAbsPdf(name, title),
   fObs("obs", "Observable", this, obs),
   fMean("mean", "Mean", this, mu),
   fSigma("sigma", "Standard Deviation", this, sig)
{
}


 
//_____________________________________________________________________________
GaussianPdf::GaussianPdf(const GaussianPdf& other, const char *name) :
   RooAbsPdf(other, name),
   fObs("obs", this, other.fObs),
   fMean("mean", this, other.fMean),
   fSigma("sigma", this, other.fSigma)
{
}


//_____________________________________________________________________________
Double_t GaussianPdf::evaluate() const
{
   Double_t arg = (fObs - fMean) / fSigma; // TODO: verify numerical accuracy
   return exp(-0.5 * arg * arg);
}


//_____________________________________________________________________________
Double_t GaussianPdf::getLogVal(const RooArgSet*) const
{  // TODO: use nset
   Double_t arg = (fObs - fMean) / fSigma;
   return -0.5 * arg * arg;
}


//_____________________________________________________________________________
void GaussianPdf::generateEvent(Int_t)
{
   Double_t obsGenerated;
   Double_t obsMin = fObs.min(); // TODO: maybe cache at class level
   Double_t obsMax = fObs.max(); 
   do {
      obsGenerated = RooRandom::randomGenerator()->Gaus(fMean, fSigma);
      // TODO: (maybe) use a new RooStats random generator
   }
   while(obsGenerated > obsMax && obsGenerated < obsMin);
   // FIXME: in RooFit it would be >= obsMax && <= obsMin - see if particular cases cause any trouble
}


//_____________________________________________________________________________
Double_t GaussianPdf::analyticalIntegral(Int_t, const char* rangeName) const
{
   static const Double_t rootPiBy2 = sqrt(M_PI_2); // defined in Math.h
   // XXX rootPiBy2 could be used in evaluate()

   Double_t obsScale = M_SQRT2 * fSigma;
   return rootPiBy2 * fSigma * (ROOT::Math::erf((fObs.max(rangeName) - fMean) / obsScale) -
      ROOT::Math::erf((fObs.min(rangeName) - fMean) / obsScale));
   // TODO this could be optimized to work without string literals

}



