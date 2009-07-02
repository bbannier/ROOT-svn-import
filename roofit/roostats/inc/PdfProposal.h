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

#ifndef ROOSTATS_PdfProposal
#define ROOSTATS_PdfProposal

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef RooStats_ProposalFunction
#include "RooStats/ProposalFunction.h"
#endif

#include "RooArgSet.h"
#include "RooMsgService.h"
#include "RooRealVar.h"

#include <map>
#include <string>

using namespace std;

namespace RooStats {

   class PdfProposal : public ProposalFunction {

   public:
      PdfProposal();
      PdfProposal(RooAbsPdf& pdf);

      // Populate xPrime with a new proposed point
      virtual void Propose(RooArgSet& xPrime, RooArgSet& x);

      // Determine whether or not the proposal density is symmetric for
      // points x1 and x2 - that is, whether the probabilty of reaching x2
      // from x1 is equal to the probability of reaching x1 from x2
      virtual Bool_t IsSymmetric(RooArgSet& x1, RooArgSet& x2);

      // Return the probability of proposing the point x1 given the starting
      // point x2
      virtual Double_t GetProposalDensity(RooArgSet& x1, RooArgSet& x2);

      // Set the PDF to be the proposal density function
      virtual void SetPdf(RooAbsPdf& pdf) { fPdf = &pdf; }

      // specify a mapping between a parameter of the proposal function and
      // a parameter of interest.  this mapping is used to set the value of
      // proposalParam equal to the value of poi to determine the
      // proposal function.
      // proposalParam is a parameter of the proposal function that must
      // be set to the value of poi (from the current point) in order to
      // propose a new point.
      virtual void AddMapping(RooRealVar& proposalParam, RooRealVar& poi);

      virtual void printMappings()
      {
         map<RooRealVar*, RooRealVar*>::iterator it;
         for (it = fMap.begin(); it != fMap.end(); it++)
         cout << it->first->GetName() << " => " << it->second->GetName() << endl;
      }

      virtual void SetCacheSize(Int_t size)
      {
         if (size > 0)
            fCacheSize = size;
         else
            coutE(Eval) << "Warning: Requested non-positive cache size: " <<
                           size << ". Cache size unchanged." << endl;
      }

      virtual ~PdfProposal() {}

   protected:
      RooAbsPdf* fPdf; // the proposal density function
      map<RooRealVar*, RooRealVar*> fMap; // map of values in pdf to update
      map<RooRealVar*, RooRealVar*>::iterator fIt; // pdf iterator
      RooArgSet fLastX; // the last point we were at
      Int_t fCacheSize; // how many points to generate each time
      Int_t fCachePosition; // our position in the cached proposal data set
      RooDataSet* fCache; // the cached proposal data set

      // determine whether these two RooArgSets represent the same point
      virtual Bool_t Equals(RooArgSet& x1, RooArgSet& x2);

      // Interface for tools setting limits (producing confidence intervals)
      ClassDef(PdfProposal,1)
   };
}

#endif
