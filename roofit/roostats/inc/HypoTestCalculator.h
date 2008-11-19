// @(#)root/roostats:$Id: HypoTestCalculator.h 24970 2008-10-10  $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_HypoTestCalculator
#define ROOSTATS_HypoTestCalculator

#include "TNamed.h"
#include "RooStats/HypoTestResult.h"
#include "RooAbsPdf.h"
#include "RooArgSet.h"
#include "RooWorkspace.h"

//_________________________________________________
/*
BEGIN_HTML
<p>
HypoTestCalculator is an interface class for a tools which produce RooStats HypoTestResults.  
The interface currently assumes that any hypothesis test calculator can be configured by specifying:
<ul>
 <li>a model for the null,</li>
 <li>a model for the alternate,</li>
 <li>a data set, </li>
 <li>a set of parameters of which specify the null (including values and const/non-const status), and </li>
 <li>a set of parameters of which specify the alternate (including values and const/non-const status).</li>
</ul>
The interface allows one to pass the model, data, and parameters via a workspace and then specify them with names.
The interface will be extended so that one does not need to use a workspace.
</p>
<p>
After configuring the calculator, one only needs to ask GetHypoTest, which will return a HypoTestResult pointer.
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

namespace RooStats {
  class HypoTestCalculator {
  public:

     virtual ~HypoTestCalculator() {}
    
    // main interface to get a HypoTestResult, pure virtual
    virtual HypoTestResult* GetHypoTest() const = 0;   

    // set a workspace that owns all the necessary components for the analysis
    virtual void SetWorkspace(RooWorkspace* ws) = 0;
    // set the PDF for the null hypothesis
    virtual void SetNullPdf(const char* name) = 0;
    // set the PDF for the alternate hypothesis
    virtual void SetAlternatePdf(const char* name) = 0;
    // set a common PDF for both the null and alternate hypotheses
    virtual void SetCommonPdf(const char* name) = 0;
    // set parameter values for the null if using a common PDF
    virtual void SetNullParameters(RooArgSet*) = 0;
    // set parameter values for the alternate if using a common PDF
    virtual void SetAlternateParameters(RooArgSet*) = 0;
    

  protected:
    ClassDef(HypoTestCalculator,1)        
  };
}

// Without this macro the THtml doc for TMath can not be generated
//#if !defined(R__ALPHA) && !defined(R__SOLARIS) && !defined(R__ACC) && !defined(R__FBSD)
//NamespaceImp(RooStats)
//#endif

ClassImp(RooStats::HypoTestCalculator) ;


#endif
