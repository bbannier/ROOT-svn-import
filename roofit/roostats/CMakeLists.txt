############################################################################
# CMakeLists.txt file for building ROOT roostats package
# @author Pere Mato, CERN
############################################################################

#---The following is needed becuase client codes of RooStats assumes to find header files under /RooStats
execute_process(COMMAND cmake -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/inc ${CMAKE_CURRENT_SOURCE_DIR}/inc/RooStats)

ROOT_USE_PACKAGE(roofit/roofit)
ROOT_GENERATE_DICTIONARY(G__RooStats RooStats/*.h LINKDEF RooStats/LinkDef.h)
ROOT_GENERATE_ROOTMAP(RooStats LINKDEF RooStats/LinkDef.h 
                               DEPENDENCIES RooFit RooFitCore Tree RIO Hist Matrix MathCore Minuit Foam Graf Gpad )
ROOT_LINKER_LIBRARY(RooStats *.cxx G__RooStats.cxx LIBRARIES Core Cint 
                               DEPENDENCIES RooFit RooFitCore Tree RIO Hist Matrix MathCore Minuit Foam Graf Gpad )

#ROOT_INSTALL_HEADERS()
install(DIRECTORY inc/ DESTINATION include/RooStats 
                       PATTERN ".svn" EXCLUDE
                       REGEX "LinkDef" EXCLUDE )
