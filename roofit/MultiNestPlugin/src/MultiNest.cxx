/*
 * MULTINEST.cxx
 *
 *  Created on: Nov 24, 2009
 *      Author: Sven Kreiss
 */

#include "RooStats/MultiNestPlugin/MultiNest.h"
#include "RooRealVar.h"
#include <iostream>
#include <sstream>

#include "RooGaussian.h"

using namespace std;
using namespace RooFit;
using namespace RooStats;

ClassImp(RooStats::MultiNest)


MultiNest *MultiNest::theFitter = NULL;

void MultiNest::LogLike(RooAbsReal *logLike, RooArgList& argList) {
   LogLike(&LogLikeGlue);

   fLogLike = logLike;
   fArgList = &argList;

   Ndims(fArgList->getSize());
   Npar(fArgList->getSize());
   NClsPar(fArgList->getSize());
}
void MultiNest::NegLogLike(RooAbsReal *logLike, RooArgList& argList) {
   LogLike(&NegLogLikeGlue);

   fLogLike = logLike;
   fArgList = &argList;

   Ndims(fArgList->getSize());
   Npar(fArgList->getSize());
   NClsPar(fArgList->getSize());
}

void MultiNest::LogLikeGlue(double *Cube, int &ndim, int& /*npars*/, double &lnew, void* /*context*/) {
   theFitter->fNoOfLogLikeEv++;

   // rescaling
   for (int i = 0; i < ndim; i++) {
      RooRealVar *var = (RooRealVar*) theFitter->fArgList->at(i);
      double min = var->getMin();
      double max = var->getMax();
      Cube[i] = min + Cube[i] * (max - min);

      // update parameters for getVal() of pdf
      var->setVal(Cube[i]);
   }

   //cout << theFitter->fLogLike->getVal() << endl;
   lnew = theFitter->fLogLike->getVal();
}
void MultiNest::NegLogLikeGlue(double *Cube, int &ndim, int& /*npars*/, double &lnew, void* /*context*/) {
   /*
      Cube(1:nPar) has nonphysical parameters
      scale Cube(1:n_dim) & return the scaled parameters in Cube(1:n_dim) & additional parameters that you want to
      returned by MultiNest along with the actual parameters in Cube(n_dim+1:nPar)
      Return the log-likelihood in lnew
   */


   theFitter->fNoOfLogLikeEv++;

   // rescaling
   for (int i = 0; i < ndim; i++) {
      RooRealVar *var = (RooRealVar*) theFitter->fArgList->at(i);
/*
      if( theFitter->fPrior->dependsOn(*var) ) {
         RooArgList priorComp( *theFitter->fPrior->getComponents() );
         for( int i=1; i < priorComp.getSize(); i++) {
            RooAbsReal* c = (RooAbsReal*) priorComp.at(i);
            if( c->dependsOn(*var) ) {
               cout << "this component: " << c->GetName() << "  depends on: " << var->GetName() << endl;
               if( (RooGaussian*)c ) {
                  cout << "prior is Gaussian" << endl;
                  RooGaussian* g = (RooGaussian*)c;
                  g->getVariables()->Print("v");
                  g->getParameters( *var )->Print("v");
                  g->getComponents()->Print("v");
                  //cout << "mean: " << ((RooRealVar&)(*g->getComponents())["mean"]).getVal();// << "   sigma: " << ((RooRealVar&)(*g->getComponents())["sigma"]).getVal() << endl;
               }
            }
         }
      }//else{
*/
      double min = var->getMin();
      double max = var->getMax();
      Cube[i] = min + Cube[i] * (max - min);

      // update parameters for getVal() of pdf
      var->setVal(Cube[i]);
   }

   //cout << theFitter->fLogLike->getVal() << endl;
   lnew = -theFitter->fLogLike->getVal();
   //if( lnew < -1e7 ) lnew = -1e7;
   //cout << lnew << endl;
}

