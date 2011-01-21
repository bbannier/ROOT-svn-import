# Create libNew
# Define all header and source files which are needed
# to build the New library

# Find all headers except the LinkDef file and copy
# them to the include subdirectory of the build directory
File(GLOB NEW_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}"
          "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h"
    )
List(REMOVE_ITEM NEW_HEADERS
                 "${CMAKE_CURRENT_SOURCE_DIR}"
    )
INSTALL_HEADERS("NEW_HEADERS")

# Find all source files
File(GLOB NEW_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}"
          "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
    )
List(REMOVE_ITEM NEW_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")

Set(INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_SOURCE_DIR}/io/io/inc 
  ${CMAKE_SOURCE_DIR}/core/clib/inc 
  ${CMAKE_SOURCE_DIR}/core/cont/inc 
  ${CMAKE_SOURCE_DIR}/core/base/inc
  ${CMAKE_SOURCE_DIR}/core/meta/inc
)
Include_Directories( ${INCLUDE_DIRECTORIES})

Add_Library(New SHARED ${NEW_SOURCES})
Set_Target_properties(New PROPERTIES ${ROOT_LIBRARY_PROPERTIES})
Install(TARGETS New DESTINATION ${LIB_INSTALL_DIR})
