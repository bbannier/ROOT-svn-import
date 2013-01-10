# Install script for directory: /home/ibucur/root/proof/afdsmgrd/afdsmgrd

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "0")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/etc/afdsmgrd.conf.example")
FILE(INSTALL DESTINATION "/usr/local/etc" TYPE FILE FILES "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/build/etc/afdsmgrd.conf.example")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/etc/sysconfig/afdsmgrd.example")
FILE(INSTALL DESTINATION "/usr/local/etc/sysconfig" TYPE FILE FILES "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/build/etc/sysconfig/afdsmgrd.example")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/etc/init.d/afdsmgrd")
FILE(INSTALL DESTINATION "/usr/local/etc/init.d" TYPE PROGRAM FILES "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/build/etc/init.d/afdsmgrd")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/libexec/afdsmgrd-macros/Verify.C")
FILE(INSTALL DESTINATION "/usr/local/libexec/afdsmgrd-macros" TYPE FILE FILES "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/macros/Verify.C")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/libexec/afdsmgrd-root.sh")
FILE(INSTALL DESTINATION "/usr/local/libexec" TYPE PROGRAM FILES "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/build/scripts/afdsmgrd-root.sh")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/etc/afverifier.conf.example")
FILE(INSTALL DESTINATION "/usr/local/etc" TYPE FILE FILES "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/build/etc/afverifier.conf.example")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/libexec/afdsmgrd-macros/LocateVerifyXrd.C")
FILE(INSTALL DESTINATION "/usr/local/libexec/afdsmgrd-macros" TYPE FILE FILES "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/macros/LocateVerifyXrd.C")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/libexec/afverifier-xrd-locate.sh")
FILE(INSTALL DESTINATION "/usr/local/libexec" TYPE PROGRAM FILES "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/build/scripts/afverifier-xrd-locate.sh")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/libexec/afverifier-xrd-rm.sh")
FILE(INSTALL DESTINATION "/usr/local/libexec" TYPE PROGRAM FILES "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/build/scripts/afverifier-xrd-rm.sh")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/libexec/afdsmgrd-xrd-stage-verify.sh")
FILE(INSTALL DESTINATION "/usr/local/libexec" TYPE PROGRAM FILES "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/build/scripts/afdsmgrd-xrd-stage-verify.sh")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/afdsutil.C")
FILE(INSTALL DESTINATION "/usr/local/share" TYPE FILE FILES "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/macros/par/afdsutil/afdsutil.C")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/bin/afverifier")
FILE(INSTALL DESTINATION "/usr/local/bin" TYPE PROGRAM FILES "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/build/scripts/afverifier")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/libexec/afdsiperm.sh")
FILE(INSTALL DESTINATION "/usr/local/libexec" TYPE PROGRAM FILES "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/scripts/afdsiperm.sh")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/bin/afdsmgr")
FILE(INSTALL DESTINATION "/usr/local/bin" TYPE PROGRAM FILES "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/scripts/afdsmgr")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/home/ibucur/root/proof/afdsmgrd/afdsmgrd/build/src/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

IF(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
ELSE(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
ENDIF(CMAKE_INSTALL_COMPONENT)

FILE(WRITE "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/build/${CMAKE_INSTALL_MANIFEST}" "")
FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})
  FILE(APPEND "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/build/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
ENDFOREACH(file)
