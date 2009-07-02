// @(#)root/roostats:$Id: PdfProposal.cxx 26805 2009-06-17 14:31:02Z kbelasco $
// Authors: Kevin Belasco        17/06/2009
// Authors: Kyle Cranmer         17/06/2009
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
PdfProposal is a concrete implementation of the ProposalFunction interface.
It proposes points across the parameter space in the distribution of the
given PDF.
</p>
END_HTML
*/
//_________________________________________________

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#include "RooStats/PdfProposal.h"
#include "RooStats/RooStatsUtils.h"
#include "RooArgSet.h"
#include "RooDataSet.h"
#include "RooAbsPdf.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "TIterator.h"

#include <map>

ClassImp(RooStats::PdfProposal);

using namespace RooFit;
using namespace RooStats;

PdfProposal::PdfProposal() : ProposalFunction()
{
   fPdf = NULL;
   fCacheSize = 1;
   fCachePosition = 0;
   fCache = NULL;
}

PdfProposal::PdfProposal(RooAbsPdf& pdf) : ProposalFunction()
{
   fPdf = &pdf;
   fCacheSize = 1;
   fCachePosition = 0;
   fCache = NULL;
}

Bool_t PdfProposal::Equals(RooArgSet& x1, RooArgSet& x2)
{
   if (x1.equals(x2)) {
      TIterator* it = x1.createIterator();
      RooRealVar* r;
      while ((r = (RooRealVar*)it->Next()) != NULL)
         if (r->getVal() != x2.getRealValue(r->GetName()))
            return kFALSE;
      return kTRUE;
   }
   return kFALSE;
}

// Populate xPrime with a new proposed point
void PdfProposal::Propose(RooArgSet& xPrime, RooArgSet& x)
{
   if (fLastX.getSize() == 0) {
      // fLastX not yet initialized
      fLastX.addClone(x);
      // generate initial cache
      fCache = fPdf->generate(xPrime, fCacheSize);
   }
   Bool_t moved = !Equals(fLastX, x);

   // if we've moved, set the values of the variables in the PDF to the
   // corresponding values of the variables in x, according to the
   // mappings (i.e. let the variables in x set the given values for the
   // PDF that will generate xPrime)
   if (moved) {
      // update the pdf parameters
      for (fIt = fMap.begin(); fIt != fMap.end(); fIt++)
         fIt->first->setVal(x.getRealValue(fIt->second->GetName()));
      // save the new x in fLastX
      RooStats::SetParameters(&x, &fLastX);
   }

   // generate new cache if necessary
   if (moved || fCachePosition >= fCacheSize) {
      delete fCache;
      fCache = fPdf->generate(xPrime, fCacheSize);
      fCachePosition = 0;
   }

   const RooArgSet* proposal = fCache->get(fCachePosition);
   fCachePosition++;
   RooStats::SetParameters(proposal, &xPrime);
}

// Determine whether or not the proposal density is symmetric for
// points x1 and x2 - that is, whether the probabilty of reaching x2
// from x1 is equal to the probability of reaching x1 from x2
Bool_t PdfProposal::IsSymmetric(RooArgSet& /* x1 */, RooArgSet& /* x2 */)
{
   // kbelasco: is there a better way to do this?
   return false;
}

// Return the probability of proposing the point x1 given the starting
// point x2
Double_t PdfProposal::GetProposalDensity(RooArgSet& x1, RooArgSet& x2)
{
   for (fIt = fMap.begin(); fIt != fMap.end(); fIt++)
      fIt->first->setVal(x2.getRealValue(fIt->second->GetName()));
   RooArgSet* temp = fPdf->getObservables(x1);
   RooStats::SetParameters(&x1, temp);
   delete temp;
   return fPdf->getVal(&x1); // could just as well use x2
}

void PdfProposal::AddMapping(RooRealVar& proposalParam, RooRealVar& poi)
{
   fMap.insert(pair<RooRealVar*, RooRealVar*>(&proposalParam, &poi));
}
