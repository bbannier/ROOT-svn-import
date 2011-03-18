############################################################################
# CMakeLists.txt file for building ROOT roostats package
# @author Pere Mato, CERN
############################################################################

ROOT_USE_PACKAGE(roofit/roofit)

ROOT_GENERATE_DICTIONARY(G__RooStats RooStats/*.h LINKDEF RooStats/LinkDef.h)

ROOT_GENERATE_ROOTMAP(RooStats LINKDEF RooStats/LinkDef.h 
                               DEPENDENCIES RooFit RooFitCore Tree RIO Hist Matrix MathCore Minuit Foam Graf Gpad )
                                           
ROOT_LINKER_LIBRARY(RooStats *.cxx G__RooStats.cxx LIBRARIES Core Cint 
                             DEPENDENCIES RooFit RooFitCore Tree RIO Hist Matrix MathCore Minuit Foam Graf Gpad )
ROOT_INSTALL_HEADERS()
