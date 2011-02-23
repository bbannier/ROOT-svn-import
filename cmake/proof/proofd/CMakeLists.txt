############################################################################
# CMakeLists.txt file for building ROOT proof/proofd package
# @author Pere Mato, CERN
############################################################################

ROOT_USE_PACKAGE(proof/proof)
ROOT_USE_PACKAGE(net/rpdutils)
ROOT_USE_PACKAGE(net/auth)

#---Using or nor an external installation of XROOTD-------------------------
if(NOT XROOTD_FOUND)
  set(XROOTD_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/net/xrootd/src/xrootd/lib)
  set(XROOTD_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/net/xrootd/src/xrootd/src)
endif()
include_directories(${XROOTD_INCLUDE_DIR})
link_directories(${XROOTD_LIBRARY_DIR})

if(WIN32)
  ROOT_LINKER_LIBRARY(XrdProofd XrdProofConn.cxx XrdProofPhyConn.cxx XProofProtUtils.cxx LIBRARIES XrdClient)
else()
  set(libs XrdOuc XrdNet XrdSys XrdClient XrdSut ${BONJOUR_LIBRARIES})
  set(exelibs libXrd.a libXrdClient.a libXrdNet.a libXrdOuc.a libXrdSys.a libXrdSut.a)

  ROOT_EXECUTABLE(proofd proofd.cxx LIBRARIES ${exelibs} rpdutil rsa ${OPENSSL_LIBRARIES})
  ROOT_EXECUTABLE(xproofd X*.cxx LIBRARIES ${exelibs})
  ROOT_LINKER_LIBRARY(XrdProofd X*.cxx LIBRARIES ${libs})
endif()

ROOT_INSTALL_HEADERS()
