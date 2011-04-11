############################################################################
# CMakeLists.txt file for building ROOT rootfit package
# @author Pere Mato, CERN
############################################################################
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

if(NOT IntegratedBuild)
  project(RooFit)
  find_package(ROOT)
  include(${ROOTSYS}/cmake/modules/StandaloneBuild.cmake)
endif()

ROOT_USE_PACKAGE(roofit/roofitcore)
ROOT_GENERATE_DICTIONARY(G__RooFit *.h LINKDEF LinkDef1.h)
ROOT_GENERATE_ROOTMAP(RooFit LINKDEF LinkDef1.h 
                             DEPENDENCIES RooFitCore Tree RIO Matrix MathCore )
ROOT_LINKER_LIBRARY(RooFit *.cxx G__RooFit.cxx CMAKENOEXPORT LIBRARIES Core Cint 
                           DEPENDENCIES RooFitCore Tree RIO Matrix MathCore)
ROOT_INSTALL_HEADERS()
