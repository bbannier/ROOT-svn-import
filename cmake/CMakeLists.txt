# Check if cmake has the required version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
enable_language(Fortran)

# Set name of our project to "CBMROOT". Has to be done after check of cmake version
project(ROOT)

#---Options----------------------------------------------------------------------------------------
option(enable_afs "Set to ON to build the AFS authentication" OFF)  
option(enable_alien "Set to ON to build the Alien interface" OFF)
option(enable_alloc              "" OFF)
option(enable_asimage            "" ON)      
option(enable_astiff             "" ON)
option(enable_bonjour            "" ON)
option(enable_builtin_afterimage "" ON)
option(enable_builtin_ftgl       "" OFF)
option(enable_builtin_freetype   "" OFF)
option(enable_builtin_glew       "" OFF)
option(enable_builtin_pcre       "" OFF)
option(enable_builtin_zlib       "" OFF)
option(enable_castor             "" ON)
option(enable_chirp              "" ON)
option(enable_cintex             "" ON)
option(enable_clarens            "" ON)
option(enable_cling              "" ON)
option(enable_editline           "" ON)
option(enable_dcache             "" ON)
option(enable_exceptions         "" ON)
option(enable_explicitlink       "" OFF)
option(enable_fftw3              "" ON)
option(enable_fitsio             "" ON)
option(enable_gviz               "" ON)
option(enable_gdml               "" OFF)
option(enable_genvector          "" ON)
option(enable_gfal               "" ON)
option(enable_glite              "" ON)
option(enable_globus             "" OFF)
option(enable_gsl_shared         "" OFF)
option(enable_hdfs               "" ON)
option(enable_krb5               "" ON)
option(enable_ldap               "" ON)
option(enable_mathmore           "" ON)
option(enable_memstat            "" ON)
option(enable_minuit2            "" OFF)
option(enable_monalisa           "" ON)
option(enable_mysql              "" ON)
option(enable_odbc               "" ON)
option(enable_opengl             "" ON)
option(enable_oracle             "" ON)
option(enable_peac               "" ON)
option(enable_pgsql              "" ON)
option(enable_pythia6            "" ON)
option(enable_pythia8            "" ON)
option(enable_python             "" ON)
option(enable_qt                 "" OFF)
option(enable_qtgsi              "" OFF)
option(enable_reflex             "" ON)
option(enable_roofit             "" OFF)
option(enable_ruby               "" OFF)
option(enable_rfio               "" ON)
option(enable_rpath              "" OFF)
option(enable_sapdb              "" ON)
option(enable_shadowpw           "" ON)
option(enable_shared             "" ON)
option(enable_soversion          "" OFF)
option(enable_srp                "" ON)
option(enable_ssl                "" ON)
option(enable_table              "" OFF)
option(enable_tmva               "" ON)
option(enable_unuran             "" OFF)
option(enable_winrtdebug         "" ON)
option(enable_x11                "" ON)
option(enable_xft                "" ON)
option(enable_xml                "" ON)
option(enable_xrootd             "" ON)
                        

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

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ 
# is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# Load some basic macros which are needed later on
include(RootMacros)
include(RootNewMacros)
include(CheckCompiler)
include(FindROOTInternal)
include(MacroEnsureVersion)
include(SearchInstalledSoftware)
include(CTest)

MESSAGE("The length of void* is ${CMAKE_SIZEOF_VOID_P}")

#Check the compiler and set the compile and link flags
Check_Compiler()

# Check if the user wants to build the project in the source
# directory
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
install(FILES include/RConfigure.h DESTINATION include)

get_enabled_root_install_options()
configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/root-config.in
               ${EXECUTABLE_OUTPUT_PATH}/root-config @ONLY)

configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/rootrc.in
               ${PROJECT_BINARY_DIR}/etc/system.rootrc @ONLY)

#MESSAGE("LDFLAGS: ${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS}") 
#MESSAGE("LDFLAGS: ${CMAKE_SHARED_LINKER_FLAGS}") 
#MESSAGE("LDFLAGS: ${CMAKE_MODULE_LINKER_FLAGS}") 
#MESSAGE("LDFLAGS: ${CMAKE_EXE_LINKER_FLAGS}") 

#Add_Custom_Target(check COMMAND ${CMAKE_CTEST_COMMAND})
Add_Custom_Target(check)
#Add_Dependencies(test check)

# Recurse into the given subdirectories.  This does not actually
# cause another cmake executable to run.  The same process will walk through
# the project's entire directory structure.
#Include(${CMAKE_SOURCE_DIR}/cint/CMakeLists_include.txt)
Add_Subdirectory (cint)
Add_Subdirectory (core)
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

#MESSAGE("ALL Headers: $ENV{HeadersToInstall}")
#PM---->INSTALL_ALL_HEADERS()
#add_custom_target(HEADER_FILES ALL
#           DEPENDS $ENV{HeadersToInstall} 
#          )

configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/RConfigOptions.in include/RConfigOptions.h)
install(FILES include/RConfigOptions.h DESTINATION include)

execute_Process(COMMAND uname -a OUTPUT_VARIABLE BuildNodeInfo OUTPUT_STRIP_TRAILING_WHITESPACE )

message("BuildInfo ${BuildNodeInfo}")

configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/compiledata.in include/compiledata.h)
install(FILES  include/compiledata.h DESTINATION include)

#Execute_Process(COMMAND cmake -E copy_directory 
#                        ${CMAKE_SOURCE_DIR}/etc ${CMAKE_BINARY_DIR}/etc
#               )

If(UNIX)
  configure_file(${PROJECT_SOURCE_DIR}/config/mimes.unix.in
                 ${CMAKE_BINARY_DIR}/etc/root.mimes)
EndIf(UNIX)

#configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/genreflex.in
#               ${EXECUTABLE_OUTPUT_PATH}/genreflex @ONLY)
#configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/genreflex-rootcint.in
#               ${EXECUTABLE_OUTPUT_PATH}/genreflex-rootcint @ONLY)
#configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/memprobe.in
#               ${EXECUTABLE_OUTPUT_PATH}/memprobe @ONLY)
#configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/thisroot.sh.in
#               ${EXECUTABLE_OUTPUT_PATH}/thisroot.sh @ONLY)
#configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/thisroot.csh.in
#               ${EXECUTABLE_OUTPUT_PATH}/thisroot.csh @ONLY)
#
#configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/genreflex.final.in
#               ${EXECUTABLE_OUTPUT_PATH}/final/genreflex @ONLY)
#configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/genreflex-rootcint.final.in
#               ${EXECUTABLE_OUTPUT_PATH}/final/genreflex-rootcint @ONLY)
#configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/memprobe.final.in
#               ${EXECUTABLE_OUTPUT_PATH}/final/memprobe @ONLY)
#configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/thisroot.sh.final.in
#               ${EXECUTABLE_OUTPUT_PATH}/final/thisroot.sh @ONLY)
#configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/thisroot.csh.final.in
#               ${EXECUTABLE_OUTPUT_PATH}/final/thisroot.csh @ONLY)
#
#Install(FILES ${EXECUTABLE_OUTPUT_PATH}/final/genreflex
#              ${EXECUTABLE_OUTPUT_PATH}/final/genreflex-rootcint
#              ${EXECUTABLE_OUTPUT_PATH}/final/memprobe
#              ${EXECUTABLE_OUTPUT_PATH}/final/thisroot.sh
#              ${EXECUTABLE_OUTPUT_PATH}/final/thisroot.csh
#        DESTINATION ${BIN_INSTALL_DIR}
#)
