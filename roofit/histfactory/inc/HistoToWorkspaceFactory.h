// @(#)root/roostats:$Id:  cranmer $
// Author: Kyle Cranmer, Akira Shibata
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_HISTOTOWORKSPACEFACTORY
#define ROOSTATS_HISTOTOWORKSPACEFACTORY

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <sstream>

#include <RooPlot.h>
#include <RooArgSet.h>
#include <RooFitResult.h>
#include <RooAbsReal.h>
#include <RooRealVar.h>
#include <RooWorkspace.h>
#include <TObject.h>
#include <TH1F.h>
#include <TDirectory.h>

#include "RooStats/HistFactory/EstimateSummary.h"


namespace RooStats{
namespace HistFactory{
  class HistoToWorkspaceFactory: public TObject {

    public:

      HistoToWorkspaceFactory(  string , vector<string> , double =200, double =20, int =0, int =6, TFile * =0);
      HistoToWorkspaceFactory();
      virtual ~HistoToWorkspaceFactory();

      void addEfficiencyTerms(RooWorkspace* proto, string prefix, string interpName,
            map<string,pair<double,double> > systMap,
            vector<string>& likelihoodTermNames, vector<string>& totSystTermNames);

      string addNormFactor(RooWorkspace *, string & , string & , EstimateSummary & , bool );

      void addMultiVarGaussConstraint(RooWorkspace* proto, string prefix,int lowBin, int highBin, vector<string>& likelihoodTermNames);

      void addPoissonTerms(RooWorkspace* proto, string prefix, string obsPrefix, string expPrefix, int lowBin, int highBin,
               vector<string>& likelihoodTermNames);

      //void combine_old();

      RooWorkspace *  makeCombinedModel(vector<string>, vector<RooWorkspace*>);

      //void combine_ratio(vector<string> , vector<RooWorkspace*>);

      void customize(RooWorkspace* proto, const char* pdfNameChar, map<string,string> renameMap);

      void formatFrameForLikelihood(RooPlot* frame, string XTitle=string("#sigma / #sigma_{SM}"), string YTitle=string("-log likelihood"));


      void linInterpWithConstraint(RooWorkspace* proto, TH1F* nominal, vector<TH1F*> lowHist, vector<TH1F*> highHist,
                 vector<string> sourceName, string prefix, string productPrefix, string systTerm,
                 int lowBin, int highBin, vector<string>& likelihoodTermNames);

      TDirectory* makedirs( TDirectory* file, vector<string> names );

      RooWorkspace* makeSingleChannelModel(vector<RooStats::HistFactory::EstimateSummary> summary, vector<string> systToFix, bool doRatio=false);

      void  makeTotalExpected(RooWorkspace* proto, string totName, string /**/, string /**/,
            int lowBin, int highBin, vector<string>& syst_x_expectedPrefixNames,
            vector<string>& normByNames);

      TDirectory* mkdir( TDirectory * file, string name );

      void printCovarianceMatrix(RooFitResult* result, RooArgSet* params, string filename);
      void processExpectedHisto(TH1F* hist,RooWorkspace* proto, string prefix, string productPrefix, string systTerm, double low, double high, int lowBin, int highBin);
      void setObsToExpected(RooWorkspace* proto, string obsPrefix, string expPrefix, int lowBin, int highBin);
      void fitModel(RooWorkspace *, string, string, string, bool=false  );
      std::string filePrefixStr(std::string);

      string rowTitle;
      vector<string> systToFix;
      double nomLumi, lumiError, lowBin, highBin;    
      std::stringstream resultsPrefixStr;
      TFile * out_f;
      FILE * pFile;

      ClassDef(RooStats::HistFactory::HistoToWorkspaceFactory,1)
  };

}
}

#endif
