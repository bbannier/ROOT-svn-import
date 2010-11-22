// @(#)root/roostats:$Id:  cranmer $
// Author: Kyle Cranmer, Akira Shibata
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/


#ifndef ROOSTATS_ESTIMATESUMMARY_h
#define ROOSTATS_ESTIMATESUMMARY_h

#include "TH1F.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "TObject.h"

using namespace std;

namespace RooStats{
namespace TemplateFactory{

  class EstimateSummary : public TObject {

  public:

    struct NormFactor{
      string name;
      double val, high, low;
      bool constant;
    };

    typedef std::vector<string> vecstring;
    typedef std::vector<TH1F*> vechist;
    typedef std::pair<double, double> pairdouble;
    typedef std::map<string, std::pair<double, double> > mappair;

    // simple structure to hold necessary information about each channel
    EstimateSummary();
    virtual ~EstimateSummary();
    void print() const ;
    void AddSyst( string name, TH1F* low, TH1F* high);
    bool operator==(const EstimateSummary &other) const ;
    bool compareHisto( const TH1 * one, const TH1 * two) const ;


    //data members. Left public because original code for EstiamteSummary was a struc not a class
    string name; 
    string channel; 
    string normName;
    TH1F* nominal;  // x pb per jet bin.  all histograms need index of binning to be consistent
    vector<string> systSourceForHist;
    vector<TH1F*> lowHists; // x pb per jet bin for - variations over list of systematics
    vector<TH1F*> highHists; // x pb per jet bin for + variations over list of systematics
    map<string, pair<double, double> > overallSyst; // "acceptance"->(0.8,1.2)
    pair<double, double> dummyForRoot;
    vector<NormFactor> normFactor;

    ClassDef(RooStats::TemplateFactory::EstimateSummary,1)
  };

}
}

#endif
