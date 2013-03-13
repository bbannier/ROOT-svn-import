// @(#)root/roostats:$Id$
// Authors: Giovanni Petrucciani 4/21/2011
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/


#include "RooStats/SequentialProposal.h"
#include <RooArgSet.h>
#include <iostream>
#include <memory>
#include <TIterator.h>
#include <RooRandom.h>
#include <RooStats/RooStatsUtils.h>

using namespace std;

ClassImp(RooStats::SequentialProposal)

namespace RooStats { 

SequentialProposal::SequentialProposal(double divisor) : 
    ProposalFunction(),
    fDivisor(1./divisor),
    fImportantVariables(NULL),
    fImportanceFactor(0)
{
}

SequentialProposal::SequentialProposal(double divisor, const RooArgSet& importantVariables, int importanceFactor) : 
    ProposalFunction(),
    fDivisor(1./divisor),
    fImportantVariables(&importantVariables),
    fImportanceFactor(importanceFactor)
{
}
     

// Populate xPrime with a new proposed point
void SequentialProposal::Propose(RooArgSet& xPrime, RooArgSet& x )
{
   RooStats::SetParameters(&x, &xPrime);

   int n = xPrime.getSize();
   if( fImportanceFactor > 0  &&  fImportantVariables )
      n += fImportanceFactor * fImportantVariables->getSize();

   int j = int( floor(RooRandom::uniform()*n) );
   
   RooRealVar* var = NULL;
   if( j < xPrime.getSize() ) {
      //std::cout << "Normal sampling" << std::endl;
      std::auto_ptr<TIterator> it(xPrime.createIterator());
      for (int i = 0; (var = (RooRealVar*)it->Next()) != NULL; ++i) {
         if (i == j) break;
      }
   }else{
      //std::cout << "Oversampling" << std::endl;
      int osj = (j-xPrime.getSize()) % fImportantVariables->getSize();
      std::auto_ptr<TIterator> it(fImportantVariables->createIterator());
      for (int i = 0; (var = (RooRealVar*)it->Next()) != NULL; ++i) {
         if (i == osj) {
            // need to get the corresponding var in xPrime before exiting the loop
            var = (RooRealVar*)( &xPrime[var->GetName()] );
            break;
         }
      }
   }
   if( var == NULL ) std::cout << "ERROR: This should not happen." << std::endl;

   //std::cout << "Proposing a step along " << var->GetName() << std::endl;
   double val = var->getVal(), max = var->getMax(), min = var->getMin(), len = max - min;
   val += RooRandom::gaussian() * len * fDivisor;
   while (val > max) val -= len;
   while (val < min) val += len;
   var->setVal(val);
}

Bool_t SequentialProposal::IsSymmetric(RooArgSet& , RooArgSet& ) {
   return true;
}

// Return the probability of proposing the point x1 given the starting
// point x2
Double_t SequentialProposal::GetProposalDensity(RooArgSet& ,
                                                RooArgSet& )
{
   return 1.0; // should not be needed
}

}

