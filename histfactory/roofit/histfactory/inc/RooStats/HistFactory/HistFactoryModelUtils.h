
#ifndef HIST_FACTORY_MODEL_UTILS_H
#define HIST_FACTORY_MODEL_UTILS_H

#include "RooAbsPdf.h"
#include "RooArgSet.h"
#include "RooDataSet.h"
#include "RooStats/HistFactory/ParamHistFunc.h"

namespace RooStats {

  void getChannelsFromModel( RooAbsPdf* model, RooArgSet* channels, 
			     RooArgSet* channelsWithConstraints );

  bool getStatUncertaintyFromChannel( RooAbsPdf* channel, ParamHistFunc*& paramfunc, 
				      RooArgList* gammaList );

  RooAbsPdf* getSumPdfFromChannel( RooAbsPdf* channel );

  void getDataValuesForObservables( std::map< std::string, std::vector<double> >& ChannelBinDataMap, 
				    RooAbsData* data, RooAbsPdf* simPdf );

}



#endif
