# Build core/meta
# Define all header and source files which are needed
# to build the Core library
# The Core library is build in the main CMakeLists.txt
# of the core subdirectory.

# Find all headers except the LinkDef file and copy
# them to the include subdirectory of the build directory
File(GLOB META_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}" 
          "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h"
    )

#Install all Headers
List(REMOVE_ITEM META_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}" 
                 "${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef.h"
                 "${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef_TCint.h"
    )
INSTALL_HEADERS("META_HEADERS")

# Remove TCint.h which is obsolete if cling is build
List(REMOVE_ITEM META_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/inc/TCint.h")

Set(BuildCling False)
If(NOT BuildCling)
  Set(META_CLING_LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef_TCint.h)
  Set(META_CLING_DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/G__TCint.cxx)
  Set(META_CLING_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/inc/TCint.h)
Else(NOT BuildCling)
  Set(META_CLING_LINKDEF)
  Set(META_CLING_DICTIONARY)
EndIf(NOT BuildCling)

# Find all source files
File(GLOB META_CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}" 
          "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
    )

List(REMOVE_ITEM META_CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")

If(BuildCling)
  List(REMOVE_ITEM META_CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/TCint.cxx")
EndIf(BuildCling)

# Define the LinkDef and the output file which are needed
# to create the dictionary.
# Definde the needed include directories and create the Dictionary
Set(META_LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef.h)
Set(META_DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/G__Meta.cxx)

Set(INCLUDE_DIRS 
    ${CMAKE_SOURCE_DIR}/cint/cint/include 
    ${CMAKE_SOURCE_DIR}/cint/cint/stl 
    ${CMAKE_SOURCE_DIR}/cint/cint/lib
    ${CMAKE_SOURCE_DIR}/cint/cint/inc
    ${CMAKE_SOURCE_DIR}/core/base/inc
    ${CMAKE_SOURCE_DIR}/core/clib/inc
    ${CMAKE_SOURCE_DIR}/core/cont/inc
    ${CMAKE_SOURCE_DIR}/core/meta/inc
   )

GENERATE_DICTIONARY("${META_HEADERS}" "${META_LINKDEF}" 
                    "${META_DICTIONARY}" "${INCLUDE_DIRS}" "-DG__API"
                   )

# To define this target is needed to get the
# dependencies correct
#add_custom_target(META_DICTIONARY 
#                  DEPENDS ${META_DICTIONARY} 
#                 )

If(NOT BuildCling)
  MESSAGE("HIER")
  GENERATE_DICTIONARY("${META_CLING_HEADERS}" "${META_CLING_LINKDEF}" 
                      "${META_CLING_DICTIONARY}" "${INCLUDE_DIRS}" "-DG__API"
                     )

  # To define this target is needed to get the
  # dependencies correct
  add_custom_target(META_CLING_DICTIONARY 
                    DEPENDS ${META_CLING_DICTIONARY} 
                   )
EndIf(NOT BuildCling)

# To define this target is needed to get the
# dependencies correct
add_custom_target(META_DICTIONARY 
                  DEPENDS ${META_DICTIONARY} ${META_CLING_DICTIONARY}
                 )

# Define all the header files which should be installed when
# doing a "make install"
Install(FILES ${META_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR})
Install(FILES ${META_CLING_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR})

# Add all source files of this subdirectory to the source files
# neede to build the Core library
# Variables are only defined in this directory-. Some of the
# variables are needed in the parent scope so they are exported
# to the parent scope
List(APPEND LibCore_SRCS ${META_CXX_SOURCES} ${META_DICTIONARY}
                         ${META_CLING_DICTIONARY} )
List(APPEND LibCore_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/inc)
List(APPEND LibCore_LINKDEF ${META_LINKDEF} ${META_CLING_LINKDEF})
Set(LibCore_LINKDEF ${LibCore_LINKDEF} PARENT_SCOPE)

List(APPEND META_DICTIONARY ${META_CLING_DICTIONARY})
Set(META_DICTIONARY ${META_DICTIONARY} PARENT_SCOPE)
Set(LibCore_SRCS ${LibCore_SRCS} PARENT_SCOPE)
Set(LibCore_INCLUDE_DIRS ${LibCore_INCLUDE} PARENT_SCOPE)




