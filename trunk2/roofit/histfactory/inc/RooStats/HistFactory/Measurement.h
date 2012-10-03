
#ifndef HISTFACTORY_MEASUREMENT_H
#define HISTFACTORY_MEASUREMENT_H

#include <string>
#include <map>
#include <fstream>
#include <iostream>

#include "TObject.h"
#include "TFile.h"


#include "PreprocessFunction.h"
#include "RooStats/HistFactory/Channel.h"


namespace RooStats{
namespace HistFactory {

class Measurement : public TNamed {


public:

  Measurement();
  //  Measurement( const Measurement& other ); // Copy
  Measurement(const char* Name, const char* Title="");

  //  std::string Name;


  void SetOutputFilePrefix( const std::string& prefix ) { fOutputFilePrefix = prefix; }
  std::string GetOutputFilePrefix() { return fOutputFilePrefix; }

  void SetPOI( const std::string& POI ) { fPOI = POI; }
  std::string GetPOI() { return fPOI; }


  // Add a parameter to be set as constant
  // (Similar to ParamSetting method below)
  void AddConstantParam( const std::string& param );
  void ClearConstantParams() { fConstantParams.clear(); }
  std::vector< std::string >& GetConstantParams() { return fConstantParams; }

  // Set a parameter to a specific value
  // (And optionally fix it)
  void SetParamValue( const std::string& param, double value);
  std::map<std::string, double>& GetParamValues() { return fParamValues; }
  void ClearParamValues() { fParamValues.clear(); }

  void AddPreprocessFunction( std::string name, std::string expression, std::string dependencies );
  void AddFunctionObject( const RooStats::HistFactory::PreprocessFunction function) { fFunctionObjects.push_back( function ); }
  void SetFunctionObjects( std::vector< RooStats::HistFactory::PreprocessFunction > objects ) { fFunctionObjects = objects; }
  std::vector< RooStats::HistFactory::PreprocessFunction >& GetFunctionObjects() { return fFunctionObjects; }
  std::vector< std::string > GetPreprocessFunctions();

  // Make these dynamically from the list of preprocess functions
  // instead of storing them
  // void AddPreprocessFunction( const std::string& function ) { fPreprocessFunctions.push_back( function ); }
  // void SetPreprocessFunctions( std::vector< std::string > functions ) { fPreprocessFunctions = functions;  }
  // std::vector< std::string >& GetPreprocessFunctions()  { return fPreprocessFunctions; }
  // void ClearPreprocessFunctions() { fPreprocessFunctions.clear(); }

  /*
    <ParamSetting Const=\"True\" Val=\"%g\">%s</ParamSetting>\n" % (setting[1],param)
    <ConstraintTerm Type=\"%s\" RelativeUncertainty=\"%g\">%s</ConstraintTerm>\n" % (constraint[0],constraint[1],param)
  */


  void SetLumi(double Lumi ) { fLumi = Lumi; }
  void SetLumiRelErr( double RelErr ) { fLumiRelErr = RelErr; }
  double GetLumi() { return fLumi; }
  double GetLumiRelErr() { return fLumiRelErr; }
  
  void SetBinLow( int BinLow ) { fBinLow = BinLow; }
  void SetBinHigh ( int BinHigh ) { fBinHigh = BinHigh; }
  int GetBinLow() { return fBinLow; }
  int GetBinHigh() { return fBinHigh; } 

  void SetExportOnly( bool ExportOnly ) { fExportOnly = ExportOnly; }
  bool GetExportOnly() { return fExportOnly; }


  void PrintTree( std::ostream& = std::cout ); // Print to a stream
  void PrintXML( std::string Directory="", std::string NewOutputPrefix="" );

  std::vector< RooStats::HistFactory::Channel >& GetChannels() { return fChannels; }
  RooStats::HistFactory::Channel& GetChannel( std::string );
  void AddChannel( RooStats::HistFactory::Channel chan ) { fChannels.push_back( chan ); }

  bool HasChannel( std::string );
  void writeToFile( TFile* file );

  void CollectHistograms();


  void AddGammaSyst(std::string syst, double uncert);
  void AddLogNormSyst(std::string syst, double uncert);
  void AddUniformSyst(std::string syst);
  void AddNoSyst(std::string syst);

  std::map< std::string, double >& GetGammaSyst() { return fGammaSyst; }
  std::map< std::string, double >& GetUniformSyst() { return fUniformSyst; }
  std::map< std::string, double >& GetLogNormSyst() { return fLogNormSyst; }
  std::map< std::string, double >& GetNoSyst() { return fNoSyst; }


private:

  // Configurables of this measurement
  std::string fOutputFilePrefix;
  std::string fPOI;
  double fLumi;
  double fLumiRelErr;
  int fBinLow;
  int fBinHigh;
  bool fExportOnly;
  std::string fInterpolationScheme;

  // Channels that make up this measurement
  std::vector< RooStats::HistFactory::Channel > fChannels;

  // List of Parameters to be set constant
  std::vector< std::string > fConstantParams;

  // Map of parameter names to inital values to be set
  std::map< std::string, double > fParamValues;

  // List of Preprocess Function objects
  std::vector< RooStats::HistFactory::PreprocessFunction > fFunctionObjects;
  // std::vector< std::string > fPreprocessFunctions;

  // List of Alternate constraint terms
  std::map< std::string, double > fGammaSyst;
  std::map< std::string, double > fUniformSyst;
  std::map< std::string, double > fLogNormSyst;
  std::map< std::string, double > fNoSyst;
  
  std::string GetDirPath( TDirectory* dir );

  ClassDef(RooStats::HistFactory::Measurement, 1);

};
 
} // namespace HistFactory
} // namespace RooStats

#endif
