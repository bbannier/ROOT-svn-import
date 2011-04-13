############################################################################
# CMakeLists.txt file for building ROOT rootfit package
# @author Pere Mato, CERN
############################################################################

ROOT_USE_PACKAGE(roofit/roofitcore)
if (mathmore) 
ROOT_USE_PACKAGE(math/mathmore)
set(ROOFITLIBEXTRA MathMore)
endif()

ROOT_GENERATE_DICTIONARY(G__RooFit *.h LINKDEF LinkDef1.h)
ROOT_GENERATE_ROOTMAP(RooFit LINKDEF LinkDef1.h 
                             DEPENDENCIES RooFitCore Tree RIO Matrix MathCore ${ROOFITLIBEXTRA} )
ROOT_LINKER_LIBRARY(RooFit *.cxx G__RooFit.cxx CMAKENOEXPORT LIBRARIES Core Cint 
                           DEPENDENCIES RooFitCore Tree RIO Matrix MathCore ${ROOFITLIBEXTRA} )
ROOT_INSTALL_HEADERS()
