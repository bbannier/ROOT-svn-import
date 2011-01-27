# Check if cmake has the required version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
enable_language(Fortran)

# Set name of our project to "CBMROOT". Has to be done after check of cmake version
project(ROOT)

#---Options----------------------------------------------------------------------------------------

# Set pathes where to put the libraries, executables and headers
Set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
Set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
Set(HEADER_OUTPUT_PATH ${PROJECT_BINARY_DIR}/include)
Set(ROOTSYS ${CMAKE_SOURCE_DIR})

Set(ROOT_INCLUDE_DIR ${HEADER_OUTPUT_PATH})
Set(ENV{ALL_HEADERS_INSTALL} "")
Set(RootConfigOptions)
Set(RootConfigFeatures)
Set(ENV{HeadersToInstall})

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# Load some basic macros which are needed later on
include(RootMacros)
include(RootNewMacros)
include(CheckCompiler)
include(FindROOTInternal)
include(MacroEnsureVersion)
include(SearchInstalledSoftware)
include(CTest)

#Check the compiler and set the compile and link flags
Check_Compiler()

# Check if the user wants to build the project in the source directory
CHECK_OUT_OF_SOURCE_BUILD()

# Check if we are on an UNIX system. If not stop with an error
# message
IF(NOT UNIX)
  MESSAGE(FATAL_ERROR "You're not on an UNIX system. The project was up to now only tested on UNIX systems, so we break here. If you want to go on please edit the CMakeLists.txt in the source directory.")
ENDIF(NOT UNIX)  

# Set the library version in the main CMakeLists.txt
Set(ROOT_MAJOR_VERSION 5)
Set(ROOT_MINOR_VERSION 26)
Set(ROOT_PATCH_VERSION 00)
Set(ROOT_VERSION "${ROOT_MAJOR_VERSION}.${ROOT_MINOR_VERSION}.${ROOT_PATCH_VERSION}")
Set(LibSuffix "so")
SET(ROOT_LIBRARY_PROPERTIES ${ROOT_LIBRARY_PROPERTIES}
    VERSION "${ROOT_VERSION}"
    SOVERSION "${ROOT_MAJOR_VERSION}"
    SUFFIX ".${LibSuffix}"
)

# Here we look for installed software and switch on and of the
# different build options
SearchInstalledSoftware()

#set dictionary type to cint
#TODO: Make this more flexible
set(ROOT_DICTTYPE cint)
set(ROOT_CONFIGARGS "")

configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/RConfigure.in include/RConfigure.h)
install(FILES ${CMAKE_BINARY_DIR}/include/RConfigure.h DESTINATION include)

get_enabled_root_install_options()

configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/root-config.in ${EXECUTABLE_OUTPUT_PATH}/root-config @ONLY)

configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/rootrc.in ${CMAKE_BINARY_DIR}/etc/system.rootrc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/etc/system.rootrc DESTINATION etc)

#Add_Custom_Target(check COMMAND ${CMAKE_CTEST_COMMAND})
Add_Custom_Target(check)
#Add_Dependencies(test check)

# Recurse into the given subdirectories.  This does not actually
# cause another cmake executable to run.  The same process will walk through
# the project's entire directory structure.
#Include(${CMAKE_SOURCE_DIR}/cint/CMakeLists_include.txt)
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
Add_Subdirectory (rootx)
Add_Subdirectory (misc)
Add_Subdirectory (main)

configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/RConfigOptions.in include/RConfigOptions.h)
install(FILES ${CMAKE_BINARY_DIR}/include/RConfigOptions.h DESTINATION include)

execute_Process(COMMAND uname -a OUTPUT_VARIABLE BuildNodeInfo OUTPUT_STRIP_TRAILING_WHITESPACE )

message("BuildInfo ${BuildNodeInfo}")

configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/compiledata.in include/compiledata.h)
install(FILES  ${CMAKE_BINARY_DIR}/include/compiledata.h DESTINATION include)

#Execute_Process(COMMAND cmake -E copy_directory 
#                        ${CMAKE_SOURCE_DIR}/etc ${CMAKE_BINARY_DIR}/etc
#               )

If(UNIX)
  configure_file(${PROJECT_SOURCE_DIR}/config/mimes.unix.in ${CMAKE_BINARY_DIR}/etc/root.mimes)
  install(FILES ${CMAKE_BINARY_DIR}/etc/root.mimes DESTINATION etc)
EndIf(UNIX)

set(bindir ${CMAKE_INSTALL_PREFIX}/bin)
set(libdir ${CMAKE_INSTALL_PREFIX}/lib)
set(mandir ${CMAKE_INSTALL_PREFIX}/man)

configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/genreflex.in ${EXECUTABLE_OUTPUT_PATH}/genreflex @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/genreflex-rootcint.in ${EXECUTABLE_OUTPUT_PATH}/genreflex-rootcint @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/memprobe.in ${EXECUTABLE_OUTPUT_PATH}/memprobe @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/config/thisroot.sh ${EXECUTABLE_OUTPUT_PATH}/thisroot.sh @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/config/thisroot.csh ${EXECUTABLE_OUTPUT_PATH}/thisroot.csh @ONLY)

install(FILES ${EXECUTABLE_OUTPUT_PATH}/genreflex
              ${EXECUTABLE_OUTPUT_PATH}/genreflex-rootcint
              ${EXECUTABLE_OUTPUT_PATH}/memprobe
              ${EXECUTABLE_OUTPUT_PATH}/thisroot.sh
              ${EXECUTABLE_OUTPUT_PATH}/thisroot.csh
              ${EXECUTABLE_OUTPUT_PATH}/root-config
              PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ 
              DESTINATION bin)

