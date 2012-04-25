
#ifndef MAKEMODELANDMEASUREMENTSFAST_H
#define MAKEMODELANDMEASUREMENTSFAST_H


namespace RooStats{
  namespace HistFactory{


    RooWorkspace* MakeModelAndMeasurementFast( RooStats::HistFactory::Measurement& measurement );
    //RooWorkspace* MakeModelFast( RooStats::HistFactory::Measurement& measurement );

    std::vector<EstimateSummary> GetChannelEstimateSummaries(RooStats::HistFactory::Measurement& measurement, RooStats::HistFactory::Channel& channel);
    // void ConfigureWorkspaceForMeasurement( const std::string&, RooWorkspace*, RooStats::HistFactory::Measurement&);

  }
}


#endif
