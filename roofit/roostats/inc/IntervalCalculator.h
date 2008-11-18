// @(#)root/roostats:$Id: IntervalCalculator.h 24970 2008-10-10  $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_IntervalCalculator
#define ROOSTATS_IntervalCalculator

//_________________________________________________
/*
BEGIN_HTML
<p>
IntervalCalculator is an interface class for a tools which produce RooStats ConfIntervals.  
The interface currently assumes that any interval calculator can be configured by specifying:
<ul>
 <li>a model,</li>
 <li>a data set, </li>
 <li>a set of parameters of interest,</li>
 <li>a set of nuisance parameters (eg. parameters on which the model depends, but are not of interest), and </li>
 <li>a confidence level or size of the test (eg. rate of Type I error).</li>
</ul>
The interface allows one to pass the model, data, and parameters via a workspace and then specify them with names.
The interface will be extended so that one does not need to use a workspace.
</p>
<p>
After configuring the calculator, one only needs to ask GetInterval, which will return a ConfInterval pointer.
</p>
<p>
The concrete implementations of this interface should deal with the details of how the nuisance parameters are
dealt with (eg. integration vs. profiling) and which test-statistic is used (perhaps this should be added to the interface).
</p>
<p>
The motivation for this interface is that we hope to be able to specify the problem in a common way for several concrete calculators.
</p>
END_HTML
*/
//

#include "TNamed.h"
#include "RooStats/ConfInterval.h"
#include "RooAbsPdf.h"
#include "RooArgSet.h"
#include "RooWorkspace.h"

namespace RooStats {
  class IntervalCalculator {//: public TNamed {
  public:
     //IntervalCalculator();
     virtual ~IntervalCalculator() {}
    
    // Main interface to get a ConfInterval, pure virtual
    virtual ConfInterval* GetInterval() const = 0; 

    
    // Get the size of the test (eg. rate of Type I error)
    virtual Double_t Size() const = 0;
    // Get the Confidence level for the test
    virtual Double_t ConfidenceLevel()  const = 0;  
    // set a workspace that owns all the necessary components for the analysis
    virtual void SetWorkspace(RooWorkspace* ws) = 0;
    // specify the name of the PDF in the workspace to be used
    virtual void SetPdf(const char* name) = 0;
    // specify the name of the dataset in the workspace to be used
    virtual void SetData(const char* name) = 0;
    // specify the parameters of interest in the interval
    virtual void SetParameters(RooArgSet*) = 0;
    // specify the nuisance parameters (eg. the rest of the parameters)
    virtual void SetNuisanceParameters(RooArgSet*) = 0;
    // set the size of the test (rate of Type I error) ( Eg. 0.05 for a 95% Confidence Interval)
    virtual void SetSize(Double_t size) = 0;
    // set the confidence level for the interval (eg. 0.95 for a 95% Confidence Interval)
    virtual void SetConfidenceLevel(Double_t cl) = 0;
    

  protected:
    ClassDef(IntervalCalculator,1)        
  };
}

// Without this macro the THtml doc for TMath can not be generated
//#if !defined(R__ALPHA) && !defined(R__SOLARIS) && !defined(R__ACC) && !defined(R__FBSD)
//NamespaceImp(RooStats)
//#endif

ClassImp(RooStats::IntervalCalculator) ;

#endif
