############################################################################
# CMakeLists.txt file for building ROOT rootfit/MultiNestPlugin package
# @author Pere Mato, CERN; Sven Kreiss, NYU
############################################################################

ROOT_GENERATE_DICTIONARY(G__MultiNestPlugin RooStats/MultiNestPlugin/*.h LINKDEF LinkDef.h)

ROOT_GENERATE_ROOTMAP(MultiNestPlugin LINKDEF LinkDef.h 
                                      DEPENDENCIES RooFit RooFitCore Tree RIO Hist Matrix MathCore Minuit Foam Graf Gpad RooStats )
                                               
ROOT_LINKER_LIBRARY(MultiNestPlugin  *.cxx G__MultiNestPlugin.cxx LIBRARIES Core Cint 
                                           DEPENDENCIES RooFit RooFitCore Tree RIO Hist Matrix MathCore Minuit Foam Graf Gpad RooStats )

#ROOT_INSTALL_HEADERS()
install(DIRECTORY inc/RooStats/MultiNestPlugin/ DESTINATION include/RooStats/MultiNestPlugin 
                       PATTERN ".svn" EXCLUDE
                       REGEX "LinkDef" EXCLUDE )
