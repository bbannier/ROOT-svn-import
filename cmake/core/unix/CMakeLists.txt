# Build core/unix
# Define all header and source files which are needed
# to build the Core library
# The Core library is build in the main CMakeLists.txt
# of the core subdirectory.

# Find all headers except the LinkDef file and copy
# them to the include subdirectory of the build directory
File(GLOB UNIX_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}" 
          "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h"
    )
List(REMOVE_ITEM UNIX_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}" 
                 "${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef.h"
    )
INSTALL_HEADERS("UNIX_HEADERS")

# Find all source files
File(GLOB UNIX_CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}" 
          "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
    )
List(REMOVE_ITEM UNIX_CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")

# Define the LinkDef and the output file which are needed
# to create the dictionary.
# Definde the needed include directories and create the Dictionary
Set(UNIX_LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef.h)
Set(UNIX_DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/G__Unix.cxx)

Set(INCLUDE_DIRS 
    ${CMAKE_SOURCE_DIR}/cint/cint/include 
    ${CMAKE_SOURCE_DIR}/cint/cint/stl 
    ${CMAKE_SOURCE_DIR}/cint/cint/lib
    ${CMAKE_SOURCE_DIR}/cint/cint/inc
    ${CMAKE_SOURCE_DIR}/core/base/inc
    ${CMAKE_SOURCE_DIR}/core/clib/inc
    ${CMAKE_SOURCE_DIR}/core/meta/inc
    ${CMAKE_SOURCE_DIR}/core/unix/inc
   )

GENERATE_DICTIONARY("${UNIX_HEADERS}" "${UNIX_LINKDEF}" 
                    "${UNIX_DICTIONARY}" "${INCLUDE_DIRS}" ""
                   )
# To define this target is needed to get the
# dependencies correct
add_custom_target(UNIX_DICTIONARY 
                  DEPENDS ${UNIX_DICTIONARY} 
                 )

# Define all the header files which should be installed when
# doing a "make install"
install(FILES ${UNIX_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR})

# Add all source files of this subdirectory to the source files
# neede to build the Core library
# Variables are only defined in this directory-. Some of the
# variables are needed in the parent scope so they are exported
# to the parent scope
List(APPEND LibCore_SRCS ${UNIX_CXX_SOURCES} ${UNIX_DICTIONARY})
List(APPEND LibCore_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/inc)

Set(UNIX_DICTIONARY ${UNIX_DICTIONARY} PARENT_SCOPE)
Set(LibCore_SRCS ${LibCore_SRCS} PARENT_SCOPE)
Set(LibCore_INCLUDE_DIRS ${LibCore_INCLUDE} PARENT_SCOPE)




