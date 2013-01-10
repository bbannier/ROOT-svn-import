# Install script for directory: /home/ibucur/root/proof/afdsmgrd/afdsmgrd/src

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
  IF(EXISTS "$ENV{DESTDIR}/usr/local/bin/afdsmgrd" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/afdsmgrd")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}/usr/local/bin/afdsmgrd"
         RPATH "")
  ENDIF()
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/bin/afdsmgrd")
FILE(INSTALL DESTINATION "/usr/local/bin" TYPE EXECUTABLE FILES "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/build/src/afdsmgrd")
  IF(EXISTS "$ENV{DESTDIR}/usr/local/bin/afdsmgrd" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/afdsmgrd")
    FILE(RPATH_REMOVE
         FILE "$ENV{DESTDIR}/usr/local/bin/afdsmgrd")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/bin/strip" "$ENV{DESTDIR}/usr/local/bin/afdsmgrd")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  IF(EXISTS "$ENV{DESTDIR}/usr/local/libexec/afdsmgrd-exec-wrapper" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/libexec/afdsmgrd-exec-wrapper")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}/usr/local/libexec/afdsmgrd-exec-wrapper"
         RPATH "")
  ENDIF()
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/libexec/afdsmgrd-exec-wrapper")
FILE(INSTALL DESTINATION "/usr/local/libexec" TYPE EXECUTABLE FILES "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/build/src/afdsmgrd-exec-wrapper")
  IF(EXISTS "$ENV{DESTDIR}/usr/local/libexec/afdsmgrd-exec-wrapper" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/libexec/afdsmgrd-exec-wrapper")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/bin/strip" "$ENV{DESTDIR}/usr/local/libexec/afdsmgrd-exec-wrapper")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  IF(EXISTS "$ENV{DESTDIR}/usr/local/bin/afverifier.real" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/afverifier.real")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}/usr/local/bin/afverifier.real"
         RPATH "")
  ENDIF()
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/bin/afverifier.real")
FILE(INSTALL DESTINATION "/usr/local/bin" TYPE EXECUTABLE FILES "/home/ibucur/root/proof/afdsmgrd/afdsmgrd/build/src/afverifier.real")
  IF(EXISTS "$ENV{DESTDIR}/usr/local/bin/afverifier.real" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/afverifier.real")
    FILE(RPATH_REMOVE
         FILE "$ENV{DESTDIR}/usr/local/bin/afverifier.real")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/bin/strip" "$ENV{DESTDIR}/usr/local/bin/afverifier.real")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

