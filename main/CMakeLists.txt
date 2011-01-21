############################################################################
# CMakeLists.txt file for building ROOT main package
# @author Pere Mato, CERN
############################################################################

ROOT_USE_PACKAGE(io/io)
ROOT_USE_PACKAGE(tree/tree)
ROOT_USE_PACKAGE(core/rint)


ROOT_EXECUTABLE(root.exe rmain.cxx LIBRARIES Core Cint MathCore Rint)
ROOT_EXECUTABLE(rootn.exe rmain.cxx LIBRARIES New Core Cint MathCore Rint)
ROOT_EXECUTABLE(roots.exe roots.cxx LIBRARIES Core Cint MathCore)
ROOT_EXECUTABLE(proofserv.exe pmain.cxx LIBRARIES Core Cint MathCore)

configure_file(${CMAKE_SOURCE_DIR}/config/proofserv.in ${EXECUTABLE_OUTPUT_PATH}/proofserv.sh @ONLY)
install(FILES ${EXECUTABLE_OUTPUT_PATH}/proofserv.sh RUNTIME DESTINATION bin)
configure_file(${PROJECT_SOURCE_DIR}/config/roots.in ${EXECUTABLE_OUTPUT_PATH}/roots.sh @ONLY)
install(FILES ${EXECUTABLE_OUTPUT_PATH}/roots.sh RUNTIME DESTINATION bin)

ROOT_EXECUTABLE(hadd hadd.cxx LIBRARIES Core Cint RIO Net Hist Graf Graf3d Gpad Tree Matrix MathCore Thread)
ROOT_EXECUTABLE(ssh2rpd ssh2rpd.cxx ${CMAKE_SOURCE_DIR}/core/clib/src/strlcpy.c )

if(CMAKE_Fortran_COMPILER)
  ROOT_EXECUTABLE(g2root g2root.f LIBRARIES minicern)
  #ROOT_EXECUTABLE(h2root h2root.cxx LIBRARIES Core Cint RIO Net Hist Graf Graf3d Gpad Tree Matrix MathCore Thread minicern)
endif()

