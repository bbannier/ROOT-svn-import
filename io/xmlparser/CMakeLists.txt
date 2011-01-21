############################################################################
# CMakeLists.txt file for building ROOT io/xmlparser package
# @author Pere Mato, CERN
############################################################################

#ROOT_USE_PACKAGE(io/io)

ROOT_GENERATE_DICTIONARY(G__XMLparser *.h  LINKDEF LinkDef.h)
ROOT_GENERATE_ROOTMAP(XMLParser LINKDEF LinkDef.h )

ROOT_LINKER_LIBRARY(XMLParser *.cxx G__XMLParser.cxx  )
ROOT_INSTALL_HEADERS()

