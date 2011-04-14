############################################################################
# CMakeLists.txt file for building ROOT rootfit/histfactory package
# @author Pere Mato, CERN
############################################################################

#---The following is needed becuase client codes of HistFactory assumes to find header files under RooStats/HistFactory
execute_process(COMMAND cmake -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/inc/RooStats)
execute_process(COMMAND cmake -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/inc ${CMAKE_CURRENT_SOURCE_DIR}/inc/RooStats/HistFactory)

ROOT_USE_PACKAGE(roofit/roostats)
ROOT_USE_PACKAGE(io/xmlparser)

ROOT_GENERATE_DICTIONARY(G__HistFactory RooStats/HistFactory/*.h LINKDEF RooStats/HistFactory/LinkDef.h)

ROOT_GENERATE_ROOTMAP(HistFactory LINKDEF RooStats/HistFactory/LinkDef.h 
                                  DEPENDENCIES RooFit RooFitCore Tree RIO Hist Matrix MathCore Minuit Foam Graf Gpad RooStats XmlParser )
                                               
ROOT_LINKER_LIBRARY(HistFactory *.cxx G__HistFactory.cxx CMAKENOEXPORT LIBRARIES Core Cint 
                                DEPENDENCIES RooFit RooFitCore Tree RIO Hist Matrix MathCore Minuit Foam Graf Gpad RooStats XmlParser)
ROOT_EXECUTABLE(hist2workspace MakeModelAndMeasurements.cxx CMAKENOEXPORT
			       LIBRARIES HistFactory RooFit RooFitCore Tree RIO Matrix Hist MathCore Graf Gpad Minuit Foam RooStats XMLParser)

#ROOT_INSTALL_HEADERS()
install(DIRECTORY inc/ DESTINATION include/RooStats/HistFactory 
                       PATTERN ".svn" EXCLUDE
                       REGEX "LinkDef" EXCLUDE )
