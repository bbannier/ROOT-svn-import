MACRO ( Check_Compiler )

SET(GCC_MAJOR 0)
SET(GCC_MINOR 0)

#TODO: test if clang setup works
If (${CC} MATCHES clang)
  exec_program(${CMAKE_C_COMPILER} ARGS "-v" OUTPUT_VARIABLE _clang_version_info)
  STRING(REGEX REPLACE "^.*[ ]([0-9]+)\\.[0-9].*$" "\\1" CLANG_MAJOR "${_clang_version_info}")
  STRING(REGEX REPLACE "^.*[ ][0-9]+\\.([0-9]).*$" "\\1" CLANG_MINOR "${_clang_version_info}")
Else (${CC} MATCHES clang)
  SET(CLANG_MAJOR 0)
  SET(CLANG_MINOR 0)
EndIf (${CC} MATCHES clang)

if (CMAKE_COMPILER_IS_GNUCXX)
  exec_program(${CMAKE_C_COMPILER} ARGS "-dumpversion" OUTPUT_VARIABLE _gcc_version_info)
  STRING(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]" "\\1" GCC_MAJOR "${_gcc_version_info}")
  STRING(REGEX REPLACE "^[0-9]+\\.([0-9]+)\\.[0-9]" "\\1" GCC_MINOR "${_gcc_version_info}")
  STRING(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9])" "\\1" GCC_PATCH "${_gcc_version_info}")
  MESSAGE(STATUS "Found GCC major version ${GCC_MAJOR}")
  MESSAGE(STATUS "Found GCC minor version ${GCC_MINOR}")
  Set(COMPILER_VERSION gcc${GCC_MAJOR}${GCC_MINOR}${GCC_PATCH})
endif (CMAKE_COMPILER_IS_GNUCXX)

# Set a default build type for single-configuration
# CMake generators if no build type is set.
if (NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif (NOT CMAKE_BUILD_TYPE)

Find_Package(Threads)

if(CMAKE_SYSTEM_NAME MATCHES Linux)
  include(SetUpLinux)
elseif(APPLE)
  include(SetUpMacOS)
elseif(WIN32)
  include(SetupWindows)
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

STRING(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UPPER)

if(CMAKE_BUILD_TYPE_UPPER MATCHES "RELWITHDEBINFO")
  SET (ALL_CXX_FLAGS ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
endif(CMAKE_BUILD_TYPE_UPPER MATCHES "RELWITHDEBINFO")
if(CMAKE_BUILD_TYPE_UPPER MATCHES "DEBUG")
  SET (ALL_CXX_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
endif(CMAKE_BUILD_TYPE_UPPER MATCHES "DEBUG")

message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler Flags: ${CMAKE_CXX_FLAGS} ${ALL_CXX_FLAGS}")

endmacro()