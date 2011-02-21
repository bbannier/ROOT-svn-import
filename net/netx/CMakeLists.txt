############################################################################
# CMakeLists.txt file for building ROOT net/netx package
# @author Pere Mato, CERN
############################################################################

ROOT_USE_PACKAGE(net/net)

include_directories(${CMAKE_SOURCE_DIR}/net/xrootd/src/xrootd/src)

ROOT_GENERATE_DICTIONARY(G__Netx *.h LINKDEF LinkDef.h)
ROOT_GENERATE_ROOTMAP(Netx LINKDEF LinkDef.h DEPENDENCIES Net RIO Thread)
ROOT_LINKER_LIBRARY(Netx *.cxx G__Netx.cxx LIBRARIES)

ROOT_INSTALL_HEADERS()

