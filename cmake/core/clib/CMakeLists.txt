# Build core/clib
# Define all header and source files which are needed
# to build the Core library
# The Core library is build in the main CMakeLists.txt
# of the core subdirectory. 

# Find all headers except the LinkDef file and copy
# them to the include subdirectory of the build directory
File(GLOB CLIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}" 
          "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h"
    )
List(REMOVE_ITEM CLIB_HEADERS 
                 "${CMAKE_CURRENT_SOURCE_DIR}" 
                 "${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef.h"
    )
INSTALL_HEADERS("CLIB_HEADERS")

# Find all source files 
# If editline support is build the file Getline.c is
# included from the core/editline directory and has to
# be excluded here. 
File(GLOB CLIB_C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}" 
          "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    )
List(REMOVE_ITEM CLIB_C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
If(BuildEditLine)
  List(REMOVE_ITEM CLIB_C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/Getline.c")
EndIf(BuildEditLine)

# Define the LinkDef and the output file which are needed
# to create the dictionary.
# Definde the needed include directories and create the Dictionary
Set(CLIB_LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef.h)
Set(CLIB_DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/G__Clib.cxx)

Set(INCLUDE_DIRS 
    ${CMAKE_SOURCE_DIR}/cint/cint/include 
    ${CMAKE_SOURCE_DIR}/cint/cint/stl 
    ${CMAKE_SOURCE_DIR}/cint/cint/lib 
    ${CMAKE_SOURCE_DIR}/core/meta/inc
    ${CMAKE_SOURCE_DIR}/core/base/inc
    ${CMAKE_SOURCE_DIR}/core/clib/inc
)

GENERATE_DICTIONARY("${CMAKE_CURRENT_SOURCE_DIR}/inc/Getline.h" 
                    "${CLIB_LINKDEF}" "${CLIB_DICTIONARY}" 
                    "${INCLUDE_DIRS}" ""
                   )

# To define this target is needed to get the
# dependencies correct
add_custom_target(CLIB_DICTIONARY 
                  DEPENDS ${CLIB_DICTIONARY} 
                 )

# Define all the header files which should be installed when
# doing a "make install"
Install(FILES ${CLIB_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR})

# Add all source files of this subdirectory to the source files
# neede to build the Core library
# Variables are only defined in this directory-. Some of the
# variables are needed in the parent scope so they are exported
# to the parent scope
List(APPEND LibCore_SRCS ${CLIB_C_SOURCES})
List(APPEND LibCore_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/inc)
List(APPEND LibCore_LINKDEF ${CLIB_LINKDEF})
Set(LibCore_LINKDEF ${LibCore_LINKDEF} PARENT_SCOPE)

Set(CLIB_DICTIONARY ${CLIB_DICTIONARY} PARENT_SCOPE)
Set(LibCore_SRCS ${LibCore_SRCS} PARENT_SCOPE)
Set(LibCore_INCLUDE_DIRS ${LibCore_INCLUDE} PARENT_SCOPE)






