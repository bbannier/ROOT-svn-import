# Create libRint
# Define all header and source files which are needed
# to build the Rint library

# Find all headers except the LinkDef file and copy
# them to the include subdirectory of the build directory
File(GLOB RINT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}"
          "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h"
    )
List(REMOVE_ITEM RINT_HEADERS
                 "${CMAKE_CURRENT_SOURCE_DIR}"
                 "${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef.h"
    )
INSTALL_HEADERS("RINT_HEADERS")

# Find all source files
File(GLOB RINT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}"
          "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
    )
List(REMOVE_ITEM RINT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")

# Define the LinkDef and the output file which are needed
# to create the dictionary.
# Definde the needed include directories and create the Dictionary
Set(RINT_LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef.h)
Set(RINT_DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/G__Rint.cxx)

Set(INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/core/rint/inc
    ${CMAKE_SOURCE_DIR}/cint/cint/include
    ${CMAKE_SOURCE_DIR}/cint/cint/stl
    ${CMAKE_SOURCE_DIR}/cint/cint/lib
    ${CMAKE_SOURCE_DIR}/cint/cint/inc
    ${CMAKE_SOURCE_DIR}/core/base/inc
    ${CMAKE_SOURCE_DIR}/core/clib/inc
    ${CMAKE_SOURCE_DIR}/core/meta/inc
    ${CMAKE_SOURCE_DIR}/core/unix/inc
)

GENERATE_DICTIONARY("${RINT_HEADERS}"
                    "${RINT_LINKDEF}" "${RINT_DICTIONARY}"
                    "${INCLUDE_DIRS}" ""
                   )
If(BuildEditLine)
  Set_Source_Files_Properties(${RINT_SOURCES}
  COMPILE_FLAGS "-DR__BUILDEDITLINE")
EndIf(BuildEditLine)

GENERATE_ROOTMAP("${RINT_LINKDEF}" Rint)

Set(INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_SOURCE_DIR}/core/editline/inc 
  ${CMAKE_SOURCE_DIR}/core/clib/inc 
  ${CMAKE_SOURCE_DIR}/core/cont/inc 
  ${CMAKE_SOURCE_DIR}/cint/cint/inc
  ${CMAKE_SOURCE_DIR}/core/base/inc
  ${CMAKE_SOURCE_DIR}/core/meta/inc
  ${ROOT_INCLUDE_DIR}
)
Include_Directories( ${INCLUDE_DIRECTORIES})

Add_Library(Rint SHARED ${RINT_SOURCES} ${RINT_DICTIONARY})
Set_Target_properties(Rint PROPERTIES ${ROOT_LIBRARY_PROPERTIES})
#Target_Link_Libraries(rootcint_tmp Cint_static)
#Add_Dependencies(rootcint_tmp CINT_HEADERS IOSENUM_H)
Install(TARGETS Rint DESTINATION ${LIB_INSTALL_DIR})
