find_package(ROOT)

#---Checking for the existing installation of ROOT------------------------------------------------
message(STATUS "----Configuring ${name} as build standalone build package using an installation of ROOT")
if(ROOT_FOUND)
  message(STATUS "Found ROOT installation at ${ROOTSYS}")
else()
  message(ERROR "ROOT installation not found")
endif()  

set(CMAKE_MODULE_PATH ${ROOTSYS}/cmake/modules)

include(ROOTExports)
include(RootNewMacros)
include(CheckCompiler)

#---Initialize project----------------------------------------------------------------------------
#project(${name})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${ROOT_INCLUDE_DIRS})
link_directories(${ROOT_LIBRARY_DIRS})

