cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

if(NOT CMAKE_PROJECT_NAME STREQUAL ROOT)
  project(RooFit)
  include(${ROOTSYS}/cmake/modules/StandaloneBuild.cmake)
endif()

add_subdirectory(roofitcore)
add_subdirectory(roofit)
add_subdirectory(roostats)
add_subdirectory(histfactory)


