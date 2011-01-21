# Build core/editline
# Define all header and source files which are needed
# to build the Core library
# The Core library is build in the main CMakeLists.txt
# of the core subdirectory.

# Find all headers except the LinkDef file and copy
# them to the include subdirectory of the build directory
File(GLOB EDITLINE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}" 
          "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h"
    )
List(REMOVE_ITEM EDITLINE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}" 
                 "${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef.h"
    )
INSTALL_HEADERS("EDITLINE_HEADERS")

# Find all source files
File(GLOB EDITLINE_CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}" 
          "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
    )
List(REMOVE_ITEM EDITLINE_CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")


# Define the LinkDef and the output file which are needed
# to create the dictionary.
Set(EDITLINE_LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef.h)
Set(EDITLINE_DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/G__Editline.cxx)

Set(INCLUDE_DIRS 
    ${CMAKE_SOURCE_DIR}/cint/cint/include 
    ${CMAKE_SOURCE_DIR}/cint/cint/stl 
    ${CMAKE_SOURCE_DIR}/cint/cint/lib
    ${CMAKE_SOURCE_DIR}/cint/cint/inc
    ${CMAKE_SOURCE_DIR}/core/base/inc
    ${CMAKE_SOURCE_DIR}/core/clib/inc
    ${CMAKE_SOURCE_DIR}/core/cont/inc
    ${CMAKE_SOURCE_DIR}/core/meta/inc
    ${CMAKE_SOURCE_DIR}/core/editline/inc
   )

GENERATE_DICTIONARY("${EDITLINE_HEADERS}" "${EDITLINE_LINKDEF}" 
                    "${EDITLINE_DICTIONARY}" "${INCLUDE_DIRS}" ""
                   )

# To define this target is needed to get the
# dependencies correct
add_custom_target(EDITLINE_DICTIONARY 
                  DEPENDS ${EDITLINE_DICTIONARY} 
                 )
# Define all the header files which should be installed when
# doing a "make install"
Install(FILES ${EDITLINE_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR})

# Add all source files of this subdirectory to the source files
# neede to build the Core library
# Variables are only defined in this directory-. Some of the
# variables are needed in the parent scope so they are exported
# to the parent scope
List(APPEND LibCore_SRCS ${EDITLINE_CXX_SOURCES} ${EDITLINE_DICTIONARY})
List(APPEND LibCore_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/inc)
List(APPEND LibCore_LINKDEF ${EDITLINE_LINKDEF})
Set(LibCore_LINKDEF ${LibCore_LINKDEF} PARENT_SCOPE)

Set(EDITLINE_DICTIONARY ${EDITLINE_DICTIONARY} PARENT_SCOPE)
Set(EDITLINE_CXX_SOURCES ${EDITLINE_CXX_SOURCES} PARENT_SCOPE)
Set(LibCore_SRCS ${LibCore_SRCS} PARENT_SCOPE)
Set(LibCore_INCLUDE_DIRS ${LibCore_INCLUDE} PARENT_SCOPE)




