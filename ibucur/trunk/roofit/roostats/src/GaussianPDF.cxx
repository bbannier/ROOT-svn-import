

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
#include "RooStats/GaussianPDF.h"
#include "RooRandom.h"

using namespace std;
using namespace RooStats;

ClassImp(GaussianPDF);

//_____________________________________________________________________________
GaussianPDF::GaussianPDF(
   const char* name, 
   const char* title, 
   RooAbsReal& obs, 
   RooAbsReal& mu,  
   RooAbsReal& sig 
) :
   RooGaussian(name, title, obs, mu, sig),
   fObs("obs", "Observable", this, obs),
   fMean("mean", "Mean", this, mu),
   fSigma("sigma", "Standard Deviation", this, sig)
{
   // Constructor
}


//_____________________________________________________________________________
GaussianPDF::GaussianPDF(const GaussianPDF& other, const char* name) :
   RooGaussian(other, name),
   fObs("obs", this, other.fObs),
   fMean("mean", this, other.fMean),
   fSigma("sigma", this, other.fSigma)
{
   // Copy constructor
}


//_____________________________________________________________________________
GaussianPDF::GaussianPDF(const RooGaussian& gauss, const char* name) :
   RooGaussian(gauss, name),
   fObs(x),
   fMean(mean),
   fSigma(sigma)
{
   // Conversion constructor from RooGaussian
}
 

//_____________________________________________________________________________
Double_t GaussianPDF::evaluate() const
{  // TODO: also make use of 1 / sigma
   Double_t arg = (fObs - fMean) / fSigma; // TODO: verify numerical accuracy
   return exp(-0.5 * arg * arg);
}


//_____________________________________________________________________________
Double_t GaussianPDF::getLogVal(const RooArgSet*) const
{  // TODO: use nset
   Double_t arg = (fObs - fMean) / fSigma;
   return -0.5 * arg * arg;
}


//_____________________________________________________________________________
void GaussianPDF::generateEvent(Int_t)
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
Double_t GaussianPDF::analyticalIntegral(Int_t, const char* rangeName) const
{
   static const Double_t rootPiBy2 = sqrt(M_PI_2); // defined in Math.h
   // XXX rootPiBy2 could be used in evaluate()

   Double_t obsScale = M_SQRT2 * fSigma;
   return rootPiBy2 * fSigma * (ROOT::Math::erf((fObs.max(rangeName) - fMean) / obsScale) -
      ROOT::Math::erf((fObs.min(rangeName) - fMean) / obsScale));
   // TODO this could be optimized to work without string literals

}



