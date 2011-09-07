/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 *    File: $Id$
 * Authors:                                                                  *
 *   Alfio Lazzaro, CERN openlab, alfio.lazzaro@cern.ch                      *
 *   Yngve Sneen Lindal, CERN openlab, yngve.sneen.lindal@cern.ch            *
 *                                                                           *
 * Copyright (c) 2011, CERN openlab. All rights reserved.                    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/
#ifndef ROO_PARALLEL_EVALUATOR
#define ROO_PARALLEL_EVALUATOR

#include "Rtypes.h"

#include "Riostream.h"
#include "RooMsgService.h"
#include "RooAbsReal.h"
#include "RooAbsData.h"
#include "RooArgSet.h"
#include "RooArgList.h"
#include "RooValues.h"

template<class Pdf> void ParallelEvaluator(const Pdf& pdf, RooValues& results)
{

  /*
  RooValues value1 = arg1.getVal(data);
  Double_t value2 = arg2.getVal();
  Double_t value3 = arg3.getVal();

  for (Int_t i=0; i<data.numEntries(); i++) {
    results[i] = pdf.eval(value1[i],value2,value3);
  }

  //  RooValues values2 = arg1.getVal(data);
  //  RooValues values3 = arg1.getVal(data);

  */

}

/*

template<class Pdf, class Mother> void ParallelEvaluator(const Pdf& pdf, const RooAbsData& data, RooValues<Double_t> &results, 
					   const RooAbsReal& arg1,
					   const RooAbsReal& arg2,
					   const RooAbsReal& arg3)
*/


class RooParallelEvaluator {};

#endif
