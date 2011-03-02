# Check if cmake has the required version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
if(NOT WIN32)
  enable_language(Fortran)
endif()

# Set name of our project to "CBMROOT". Has to be done after check of cmake version
project(ROOT)

# Set pathes where to put the libraries, executables and headers
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(ROOTSYS ${CMAKE_SOURCE_DIR})
set(HEADER_OUTPUT_PATH ${CMAKE_BINARY_DIR}/include)
set(ROOT_INCLUDE_DIR ${HEADER_OUTPUT_PATH})

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

# Load some basic macros which are needed later on
#include(RootMacros)
include(RootNewMacros)
include(CheckCompiler)
#include(FindROOTInternal)
include(MacroEnsureVersion)

include(CTest)

#Check the compiler and set the compile and link flags
Check_Compiler()

# Check if the user wants to build the project in the source directory
ROOT_CHECK_OUT_OF_SOURCE_BUILD()

#---Here we declare the required build options--------------------------------------------------
include(RootBuildOptions)
#---Here we look for installed software and switch on and of the different build options--------
include(SearchInstalledSoftware)
ROOT_SHOW_OPTIONS()

#---Set the library version in the main CMakeLists.txt-------------------------------------------
set(ROOT_MAJOR_VERSION 5)
set(ROOT_MINOR_VERSION 28)
set(ROOT_PATCH_VERSION 00)
set(ROOT_VERSION "${ROOT_MAJOR_VERSION}.${ROOT_MINOR_VERSION}.${ROOT_PATCH_VERSION}")

#set dictionary type to cint
#TODO: Make this more flexible
set(ROOT_DICTTYPE cint)
set(ROOT_CONFIGARGS "")

configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/RConfigure.in include/RConfigure.h)
install(FILES ${CMAKE_BINARY_DIR}/include/RConfigure.h DESTINATION include)

#get_enabled_root_install_options()
add_custom_target(check)

# Recurse into the given subdirectories.  This does not actually
# cause another cmake executable to run.  The same process will walk through
# the project's entire directory structure.
Add_Subdirectory (cint)
Add_Subdirectory (core)
Add_Subdirectory (build)
Add_Subdirectory (math)
Add_Subdirectory (hist)
Add_Subdirectory (tree)
Add_Subdirectory (io)
Add_Subdirectory (net)
Add_Subdirectory (graf2d)
Add_Subdirectory (graf3d)
Add_Subdirectory (gui)
Add_Subdirectory (proof)
Add_Subdirectory (html)
Add_Subdirectory (montecarlo)
Add_Subdirectory (geom)
if(NOT WIN32)
  Add_Subdirectory (rootx)
endif()
Add_Subdirectory (misc)
Add_Subdirectory (main)
Add_Subdirectory (bindings)
if(tmva)
  add_subdirectory(tmva)
endif()

#---Configure and install varrious files----------------------------------------------------------------------
execute_Process(COMMAND uname -a OUTPUT_VARIABLE BuildNodeInfo OUTPUT_STRIP_TRAILING_WHITESPACE )

configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/root-config.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/root-config @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/rootrc.in ${CMAKE_BINARY_DIR}/etc/system.rootrc @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/RConfigOptions.in include/RConfigOptions.h)
configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/compiledata.in include/compiledata.h)
configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/Makefile-comp.in config/Makefile.comp)
configure_file(${PROJECT_SOURCE_DIR}/config/mimes.unix.in ${CMAKE_BINARY_DIR}/etc/root.mimes)

set(bindir ${CMAKE_INSTALL_PREFIX}/bin)
set(libdir ${CMAKE_INSTALL_PREFIX}/lib)
set(mandir ${CMAKE_INSTALL_PREFIX}/man)

configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/genreflex.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/genreflex @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/genreflex-rootcint.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/genreflex-rootcint @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/memprobe.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/memprobe @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/config/thisroot.sh ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/thisroot.sh @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/config/thisroot.csh ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/thisroot.csh @ONLY)

install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/genreflex
              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/genreflex-rootcint
              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/memprobe
              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/thisroot.sh
              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/thisroot.csh
              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/root-config
              PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ 
              DESTINATION bin)

install(FILES ${CMAKE_BINARY_DIR}/include/RConfigOptions.h
              ${CMAKE_BINARY_DIR}/include/compiledata.h 
              DESTINATION include)

install(FILES ${CMAKE_BINARY_DIR}/etc/root.mimes 
              ${CMAKE_BINARY_DIR}/etc/system.rootrc
              DESTINATION etc)

install(FILES ${CMAKE_BINARY_DIR}/config/Makefile.comp
              DESTINATION config)
