

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
#include "RooStats/GaussianPdf.h"
#include "RooAbsPdf.h"
#include "Riostream.h"

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
   return exp(-0.5 * arg * arg); // TODO: normalize
}


//_____________________________________________________________________________
Double_t GaussianPdf::getLogVal(const RooArgSet* nset) const
{  // TODO: use nset, normalize
   Double_t arg = (fObs - fMean) / fSigma;
   return -0.5 * arg * arg;
}


