
// Author: Ioan Gabriel Bucur         25/04/2012
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
   RooStatsUnitTest is an abstract base class for regression unit tests,
designed specifically for RooStats. It is derived from the more general
class RooUnitTest, used for RooFit.
</p>
<p>
   Implementations of this class must include the abstract method
testCode(), which defines the regression test to be performed. The
regression test can register objects on which the functionality of the
unit is evaluated.
   The classes of objects currently supported are:
      RooPlot      - regPlot()
      RooFitResult - regResult()
      Double_t     - regValue()
      RooTable     - regTable()
      TH1/2/3      - regTH()
      RooWorkspace - regWS()
   The evaluation of the unit functionality is constructed as a two-step
process. In WRITE mode, the objects' registration consists of writing
them to a .root file for reference. In COMPARE (NON-WRITE) mode, the
registered objects go through a series of comparison tests. Each object
is compared to its corresponding reference (which must have the same
name), previously saved in a file in WRITE mode.
</p>
END_HTML
*/

// RooStats headers
#include "RooStats/RooStatsUnitTest.h"


//_________________________________________________
RooStatsUnitTest::RooStatsUnitTest(const char *name, TFile *refFile, Bool_t writeRef, Int_t verbose) :
   RooUnitTest(name, refFile, writeRef, verbose) {}

//_________________________________________________
RooStatsUnitTest::~RooStatsUnitTest()
{
   if(fWS) delete fWS;
}


//_________________________________________________




//_________________________________________________




