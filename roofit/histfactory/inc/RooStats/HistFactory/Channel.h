// @(#)root/roostats:$Id$
// Author: George Lewis, Kyle Cranmer
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
 
#ifndef HISTFACTORY_CHANNEL_H
#define HISTFACTORY_CHANNEL_H

#include <string>
#include <fstream>
#include <iostream>


#include "RooStats/HistFactory/Data.h"
#include "RooStats/HistFactory/Sample.h"
#include "RooStats/HistFactory/Systematics.h"

namespace RooStats{
namespace HistFactory {

class Channel  {


public:
  friend class Measurement;

  Channel();
  Channel(std::string Name, std::string InputFile="");
  

  void SetName( const std::string& Name ) { fName = Name; }
  std::string GetName() { return fName; }

  void SetInputFile( const std::string& file ) { fInputFile = file; }
  std::string GetInputFile() { return fInputFile; }

  void SetHistoPath( const std::string& file ) { fHistoPath = file; }
  std::string GetHistoPath() { return fHistoPath; }

  void SetData( const RooStats::HistFactory::Data& data ) { fData = data; }
  void SetData( std::string HistoName, std::string InputFile, std::string HistoPath="" );
  void SetData( double Val );
  void SetData( TH1* hData );
  RooStats::HistFactory::Data& GetData() { return fData; }

  void AddAdditionalData( const RooStats::HistFactory::Data& data ) { fAdditionalData.push_back(data); } 
  std::vector<RooStats::HistFactory::Data>& GetAdditionalData() { return fAdditionalData; }

  void SetStatErrorConfig( double RelErrorThreshold, Constraint::Type ConstraintType );
  void SetStatErrorConfig( double RelErrorThreshold, std::string ConstraintType );
  void SetStatErrorConfig( RooStats::HistFactory::StatErrorConfig Config ) { fStatErrorConfig = Config; }
  HistFactory::StatErrorConfig& GetStatErrorConfig() { return fStatErrorConfig; }

  void AddSample( RooStats::HistFactory::Sample sample );
  std::vector< RooStats::HistFactory::Sample >& GetSamples() { return fSamples; }

  void Print(std::ostream& = std::cout);  
  void PrintXML( std::string Directory, std::string Prefix="" );

  void CollectHistograms();
  bool CheckHistograms();

protected:
  
  std::string fName;
  std::string fInputFile;
  std::string fHistoPath;

  HistFactory::Data fData;

  // One can add additional datasets
  // These are simply added to the xml under a different name
  std::vector<RooStats::HistFactory::Data> fAdditionalData;

  HistFactory::StatErrorConfig fStatErrorConfig;

  std::vector< RooStats::HistFactory::Sample > fSamples;

  // Open a file and copy a histogram
  TH1* GetHistogram( std::string InputFile, std::string HistoPath, std::string HistoName );


};

  extern Channel BadChannel;
 
} // namespace HistFactory
} // namespace RooStats

#endif
