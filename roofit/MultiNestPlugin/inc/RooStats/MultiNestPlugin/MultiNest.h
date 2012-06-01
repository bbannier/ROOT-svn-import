/*
 * MULTINEST.h
 *
 *  Created on: Nov 24, 2009
 *      Author: Sven Kreiss
 */

#ifndef ROOSTATS_MULTINEST_H_
#define ROOSTATS_MULTINEST_H_

#include "TMultiNest.h"
#include "RooAbsPdf.h"
#include "RooArgList.h"
#include "RooArgSet.h"
#include "RooDataSet.h"

using namespace RooFit;

namespace RooStats {

class MultiNest: public TMultiNest {
   public:
      MultiNest() : TMultiNest() {
         theFitter = this;
      }

      using TMultiNest::LogLike;  // explicitly "use" LogLike from TMultiNest so that it can be overloaded instead of overwritten in the next line
      void LogLike(RooAbsReal *logLike, RooArgList& argList);
      void NegLogLike(RooAbsReal *logLike, RooArgList& argList);

      /// this is very restrictive right now: it can only
      /// be a product of a certain set of PDFs
      void SetPrior(RooAbsPdf *p) { fPrior = p; }

      static MultiNest *theFitter;

   private:
      static void LogLikeGlue(double *Cube, int &ndim, int &npars, double &lnew, void* context);
      static void NegLogLikeGlue(double *Cube, int &ndim, int &npars, double &lnew, void* context);

      RooAbsReal *fLogLike;
      RooAbsPdf *fLogLikePdf;
      RooArgList *fArgList;
      RooArgSet *fNormSet;

      RooAbsPdf *fPrior;

   protected:
   ClassDef(MultiNest,0)
};

} // namespace RooStats

#endif /* ROOSTATS_MULTINEST_H_ */

