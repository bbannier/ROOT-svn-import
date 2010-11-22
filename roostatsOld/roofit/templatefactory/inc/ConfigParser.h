// @(#)root/roostats:$Id:  cranmer $
// Author: Kyle Cranmer, Akira Shibata
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include <cstdlib>
#include <string>
#include <TXMLNode.h>

#include "TList.h"
#include "TFile.h"
#include "TXMLAttr.h"

#include "RooStats/TemplateFactory/EstimateSummary.h"

// KC: Should make this a class and have it do some of what is done in MakeModelAndMeasurements

namespace RooStats{
   namespace TemplateFactory {

     typedef pair<double,double> UncertPair;
     void ReadXmlConfig( string, vector<RooStats::TemplateFactory::EstimateSummary>& , Double_t );
     void AddSystematic( RooStats::TemplateFactory::EstimateSummary &, TXMLNode*, string, string,string);
   }
}
