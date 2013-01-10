#
# CMakeLists.txt -- by Dario Berzano <dario.berzano@cern.ch>
#
# This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
#
# Main CMake build and install file for afdsmgrd. Other CMakeLists.txt may be
# included in subdirectories.
#

#
# Common parameters
#

# Name of the project and minimum version required
project (afdsmgrd)
cmake_minimum_required (VERSION 2.6)

# Where to find additional modules
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# The build type
#set (CMAKE_BUILD_TYPE "Debug")

# Tells the user the installation path
message (STATUS "Installation prefix: ${CMAKE_INSTALL_PREFIX}")

#
# Supported build types
#

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")

  message (STATUS "Building in release mode")
  message (STATUS "C++ common build flags: ${CMAKE_CXX_FLAGS_RELEASE}")

elseif (${CMAKE_BUILD_TYPE} STREQUAL "Debug")

  message (STATUS "Building in debug mode")
  message (STATUS "C++ common build flags: ${CMAKE_CXX_FLAGS_DEBUG}")

else ()

  message (FATAL_ERROR "Unsupported build type: ${CMAKE_BUILD_TYPE}")

endif ()

#
# A handy macro to copy a file in install phase without overwriting it. As
# destination directory, both absolute and relative paths are supported.
# Absolute paths are left intact, while relative paths get the CMAKE install
# prefix prepended
#

#macro (install_dont_overwrite SRCFILE DESTDIR)
#
#  get_filename_component (_FILENAME ${SRCFILE} NAME)
#  get_filename_component (_FULLSRC ${SRCFILE} ABSOLUTE)
#
#  if (IS_ABSOLUTE ${DESTDIR})
#    get_filename_component (_FULLDEST ${DESTDIR}/${_FILENAME} ABSOLUTE)
#  else ()
#    get_filename_component (_FULLDEST ${CMAKE_INSTALL_PREFIX}/${DESTDIR}/${_FILENAME} ABSOLUTE)
#  endif ()
#
#  install (CODE "
#    if (NOT EXISTS ${_FULLDEST})
#      message (STATUS \"Installing: ${_FULLDEST}\")
#      configure_file ( \"${_FULLSRC}\" \"${_FULLDEST}\" COPY_ONLY )
#    else ()
#      message (STATUS \"Keeping:    ${_FULLDEST}\")
#    endif ()
#  ")
#
#endmacro ()

#
# Macro to simplify the repetitive use of configure_file () CMake macros.
#

macro (hardconf RELSRC)

  get_filename_component (_ABSSRC ${PROJECT_SOURCE_DIR}/${RELSRC} ABSOLUTE)
  string (REGEX REPLACE "\\.in$" "" _RELDST ${RELSRC})
  get_filename_component (_ABSDST ${PROJECT_BINARY_DIR}/${_RELDST} ABSOLUTE)

  message (STATUS "Configuring ${_ABSSRC}" )

  if (NOT EXISTS ${_ABSSRC})
    message (FATAL_ERROR "Can't find file to configure: ${RELSRC}")
  endif ()

  if (${RELSRC} STREQUAL ${_RELDST})
    message (FATAL_ERROR "File to configure must end with \".in\"")
  endif ()

  configure_file ("${_ABSSRC}" "${_ABSDST}" @ONLY)

endmacro ()

#
# Test for packages
#

# ROOT
find_package (Root REQUIRED)
include_directories (${Root_INCDIR})
link_directories (${Root_LIBDIR})

# ApMon
if (NOT ApMon_DISABLED)

  find_package (ApMon)

  if (ApMon_FOUND)
    include_directories (${ApMon_INCDIR})
    link_directories (${ApMon_LIBDIR})
    set (WITH_APMON TRUE)
  endif ()

endif ()

# Extra include paths to add: useful when building from within ROOT with CMake,
# in case `root-config --incdir` is not yet created
if (DEFINED EXTRA_INCDIR)
  string(REPLACE ":" ";" EXTRA_INCDIR_List ${EXTRA_INCDIR})
  message (STATUS "Extra include paths appended:")
  foreach (An_EXTRA_INCDIR ${EXTRA_INCDIR_List})
    message (STATUS " * ${An_EXTRA_INCDIR}")
    include_directories (${An_EXTRA_INCDIR})
  endforeach ()
endif ()

#
# Variables for the main destinations of "install" target, and for substitutions
# in configuration files. Typical directories are: etc, bin, libexec and var.
#

if (ROOT_MODE)
  message (STATUS "Compiling from within ROOT")
  set (DIR_ETC     "${ROOTSYS}/etc/proof")
  set (DIR_BIN     "${ROOTSYS}/bin")
  set (DIR_LIB     "${ROOTSYS}/etc/proof/lib")
  set (DIR_LIBEXEC "${ROOTSYS}/etc/proof")
  set (DIR_VAR     "${ROOTSYS}/etc/proof/var")
  set (DIR_SHARE   "${ROOTSYS}/etc/proof/utils/afdsmgrd")
else ()
  set (DIR_ETC     "${CMAKE_INSTALL_PREFIX}/etc")
  set (DIR_BIN     "${CMAKE_INSTALL_PREFIX}/bin")
  set (DIR_LIB     "${CMAKE_INSTALL_PREFIX}/lib")
  set (DIR_LIBEXEC "${CMAKE_INSTALL_PREFIX}/libexec")
  set (DIR_VAR     "${CMAKE_INSTALL_PREFIX}/var")
  set (DIR_SHARE   "${CMAKE_INSTALL_PREFIX}/share")
endif ()

message (STATUS "Directory \"etc\" is ${DIR_ETC}")
message (STATUS "Directory \"bin\" is ${DIR_BIN}")
message (STATUS "Directory \"lib\" is ${DIR_LIB}")
message (STATUS "Directory \"libexec\" is ${DIR_LIBEXEC}")
message (STATUS "Directory \"var\" is ${DIR_VAR}")
message (STATUS "Directory \"share\" is ${DIR_SHARE}")

#
# Imports subdirectories
#

add_subdirectory (src)

#
# Auto-generate configuration files with proper variables hardcoded: see the
# hardconf macro for more information
#

hardconf ( src/afOptions.h.in )
hardconf ( etc/sysconfig/afdsmgrd.example.in )
hardconf ( etc/init.d/afdsmgrd.in )
hardconf ( etc/afdsmgrd.conf.example.in )
hardconf ( scripts/afdsmgrd-root.sh.in )
hardconf ( etc/afverifier.conf.example.in )
hardconf ( scripts/afverifier-xrd-locate.sh.in )
hardconf ( scripts/afverifier-xrd-rm.sh.in )
hardconf ( scripts/afdsmgrd-xrd-stage-verify.sh.in )
hardconf ( scripts/afverifier.in )

#
# Install directives for non-src files
#

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/etc/afdsmgrd.conf.example DESTINATION ${DIR_ETC})
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/etc/sysconfig/afdsmgrd.example DESTINATION ${DIR_ETC}/sysconfig)
install (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/etc/init.d/afdsmgrd DESTINATION ${DIR_ETC}/init.d)
install (FILES macros/Verify.C DESTINATION ${DIR_LIBEXEC}/afdsmgrd-macros)
install (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/scripts/afdsmgrd-root.sh DESTINATION ${DIR_LIBEXEC})
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/etc/afverifier.conf.example DESTINATION ${DIR_ETC})
install (FILES macros/LocateVerifyXrd.C DESTINATION ${DIR_LIBEXEC}/afdsmgrd-macros)
install (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/scripts/afverifier-xrd-locate.sh DESTINATION ${DIR_LIBEXEC})
install (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/scripts/afverifier-xrd-rm.sh DESTINATION ${DIR_LIBEXEC})
install (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/scripts/afdsmgrd-xrd-stage-verify.sh DESTINATION ${DIR_LIBEXEC})
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/macros/par/afdsutil/afdsutil.C DESTINATION ${DIR_SHARE})
install (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/scripts/afverifier DESTINATION ${DIR_BIN})
install (PROGRAMS scripts/afdsiperm.sh DESTINATION ${DIR_LIBEXEC})
install (PROGRAMS scripts/afdsmgr DESTINATION ${DIR_BIN})

#
# Renew authentication daemon (optional, only in debug mode)
#

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")

  # Renew authentication daemon: startup script
  configure_file (
    "${PROJECT_SOURCE_DIR}/etc/init.d/afrenewauthd.in"
    "${PROJECT_BINARY_DIR}/etc/init.d/afrenewauthd"
    @ONLY
  )

  # Renew authentication daemon: startup configuration
  configure_file (
    "${PROJECT_SOURCE_DIR}/etc/sysconfig/afrenewauthd.example.in"
    "${PROJECT_BINARY_DIR}/etc/sysconfig/afrenewauthd.example"
    @ONLY
  )

  install (PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/scripts/afrenewauth DESTINATION ${DIR_LIBEXEC})
  install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/scripts/aflib DESTINATION ${DIR_LIBEXEC})
  install (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/etc/init.d/afrenewauthd DESTINATION ${DIR_ETC}/init.d)
  install (FILES ${CMAKE_CURRENT_BINARY_DIR}/etc/sysconfig/afrenewauthd.example DESTINATION ${DIR_ETC}/sysconfig)

endif ()

#
# Custom uninstall target (external module)
#

add_custom_target (uninstall
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_MODULE_PATH}/Uninstall.cmake)
