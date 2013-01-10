#
# CMakeLists.txt -- by Dario Berzano <dario.berzano@cern.ch>
#
# This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
#
# CMake build and install instructions for files into src of afdsmgrd.
#

#
# Directories to include when searching for files. For out-of-source builds,
# include the destination directory before the other ones.
#

include_directories (${CMAKE_CURRENT_BINARY_DIR})

#
# Use -fPIC on 64-bit machines
#

if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
  set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fPIC")
  set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fPIC")
endif ()

#
# List of the libraries to build
#

add_library (afDataSetList afDataSetList.cc)
add_library (afOpQueue afOpQueue.cc sqlite3.c)
add_library (afExtCmd afExtCmd.cc)
add_library (afConfig afConfig.cc)
add_library (afRegex afRegex.cc)
add_library (afLog afLog.cc)
add_library (afNotify afNotify.cc)
add_library (afResMon afResMon.cc)

#
# Link-time dependencies for libraries
#

target_link_libraries(afOpQueue afLog)

#
# Plugins (as shared libraries) and where to install them
#

if (ApMon_FOUND)
  add_library (afdsmgrd_notify_apmon SHARED afNotifyApMon.cc)
  target_link_libraries (afdsmgrd_notify_apmon ${ApMon_LIBRARY} afConfig afLog afRegex)
  install (TARGETS afdsmgrd_notify_apmon LIBRARY DESTINATION ${DIR_LIB})
endif ()

#
# List of executables and their link-time dependencies
#

# Executable wrapper for af::extCmd class
add_executable (afdsmgrd-exec-wrapper execwrapper.cc)

# Daemon executable and its libraries
add_executable (afdsmgrd afdsmgrd.cc)
target_link_libraries (afdsmgrd afLog afConfig afDataSetList afRegex afExtCmd afOpQueue afNotify afResMon ${Root_LIBS} -ldl -pthread)

# Verifier executable and its libraries
add_executable (afverifier.real verifier.cc)
target_link_libraries (afverifier.real afLog afConfig afDataSetList afRegex afExtCmd afOpQueue afResMon ${Root_LIBS} -ldl -pthread)

#
# Where to install the stuff
#

# Can't do rpath since relocation is performed on automatic builds...
#set_target_properties(afverifier.real PROPERTIES
#  INSTALL_RPATH "${Root_LIBDIR}"
#  INSTALL_RPATH_USE_LINK_PATH TRUE)

install (TARGETS afdsmgrd RUNTIME DESTINATION ${DIR_BIN})
install (TARGETS afdsmgrd-exec-wrapper RUNTIME DESTINATION ${DIR_LIBEXEC})
install (TARGETS afverifier.real RUNTIME DESTINATION ${DIR_BIN})

#
# Test executable build and install directives (only in Debug build type)
#

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")

  add_executable (afdsmgrd-test-libs test.cc)
  target_link_libraries (afdsmgrd-test-libs afLog afConfig afDataSetList afRegex afExtCmd afOpQueue afNotify ${Root_LIBS} -ldl -pthread)
  install (TARGETS afdsmgrd-test-libs RUNTIME DESTINATION ${DIR_BIN})

endif ()
