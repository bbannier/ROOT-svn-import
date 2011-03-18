############################################################################
# CMakeLists.txt file for building ROOT rootfit/histfactory package
# @author Pere Mato, CERN
############################################################################

ROOT_USE_PACKAGE(roofit/roostats)
ROOT_USE_PACKAGE(io/xmlparser)

ROOT_GENERATE_DICTIONARY(G__HistFactory RooStats/HistFactory/*.h LINKDEF RooStats/HistFactory/LinkDef.h)

ROOT_GENERATE_ROOTMAP(HistFactory LINKDEF RooStats/HistFactory/LinkDef.h 
                                  DEPENDENCIES RooFit RooFitCore Tree RIO Hist Matrix MathCore Minuit Foam Graf Gpad RooStats XmlParser )
                                               
ROOT_LINKER_LIBRARY(HistFactory *.cxx G__HistFactory.cxx LIBRARIES Core Cint 
                                DEPENDENCIES RooFit RooFitCore Tree RIO Hist Matrix MathCore Minuit Foam Graf Gpad RooStats XmlParser)
ROOT_EXECUTABLE(hist2workspace MakeModelAndMeasurements.cxx 
                               LIBRARIES HistFactory RooFit RooFitCore Tree RIO Matrix Hist MathCore Graf Gpad Minuit Foam RooStats XMLParser)

ROOT_INSTALL_HEADERS()
