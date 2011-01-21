# Build core/metautils 
# Define all header and source files which are needed
# The Core library is build in the main CMakeLists.txt
# of the core subdirectory. The source files are also
# used in the build of rootcint and rootcint_tmp which
# are defined in the core/utils directory

# Find all headers except the LinkDef file and copy
# them to the include subdirectory of the build directory
File(GLOB METAUTILS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}" 
          "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h"
    )
List(REMOVE_ITEM METAUTILS_HEADERS 
                 "${CMAKE_CURRENT_SOURCE_DIR}" 
                  "${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef.h"
    )
INSTALL_HEADERS("METAUTILS_HEADERS")

# Find all source files
File(GLOB METAUTILS_CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}" 
          "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
    )
List(REMOVE_ITEM METAUTILS_CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")

# Define the LinkDef and the output file which are needed
# to create the dictionary.
# Definde the needed include directories and create the Dictionary
Set(METAUTILS_LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef.h)
Set(METAUTILS_DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/G__MetaUtils.cxx)

Set(INCLUDE_DIRS 
    ${CMAKE_SOURCE_DIR}/core/base/inc 
    ${CMAKE_SOURCE_DIR}/core/meta/inc 
    ${CMAKE_SOURCE_DIR}/cint/cint/stl 
    ${CMAKE_SOURCE_DIR}/cint/cint/lib  
    ${CMAKE_SOURCE_DIR}/cint/cint/include 
    ${CMAKE_SOURCE_DIR}/core/clib/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
   )

GENERATE_DICTIONARY("${METAUTILS_HEADERS}" "${METAUTILS_LINKDEF}" 
                    "${METAUTILS_DICTIONARY}" "${INCLUDE_DIRS}" 
                    "-DG__API"
                   )
# Define a target that the dictionary is created before the
# object files is created. This is needed to get the 
# dependencies correct
Add_Custom_Target(METAUTILS_DICTIONARY 
                  DEPENDS ${METAUTILS_DICTIONARY} 
                 )

# Define all the header files which should be installed when
# doing a "make install"
Install(FILES ${METAUTILS_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR})


# Add all source files of this subdirectory to the source files
# neede to build the Core library
# Variables are only defined in this directory-. Some of the
# variables are needed in the parent scope so they are exported
# to the parent scope
List(APPEND LibCore_SRCS ${METAUTILS_CXX_SOURCES})
List(APPEND LibCore_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/inc)
List(APPEND LibCore_LINKDEF ${METAUTILS_LINKDEF})
Set(LibCore_LINKDEF ${LibCore_LINKDEF} PARENT_SCOPE)

Set(METAUTILS_DICTIONARY ${METAUTILS_DICTIONARY} PARENT_SCOPE)
Set(METAUTILS_CXX_SOURCES ${METAUTILS_CXX_SOURCES} PARENT_SCOPE)
Set(LibCore_SRCS ${LibCore_SRCS} PARENT_SCOPE)
Set(LibCore_INCLUDE_DIRS ${LibCore_INCLUDE} PARENT_SCOPE)




