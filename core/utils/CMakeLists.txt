# Create rootcint, rootcint_tmp and rlibmap executables

Set(INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/core/metautils/inc 
  ${CMAKE_SOURCE_DIR}/core/base/inc
  ${CMAKE_SOURCE_DIR}/core/meta/inc
  ${CMAKE_SOURCE_DIR}/cint/cint/inc
  ${ROOT_INCLUDE_DIR}
)
Include_Directories( ${INCLUDE_DIRECTORIES})

#--PM--
set(CLIB_EXT_SOURCES ${CMAKE_SOURCE_DIR}/core/clib/src/snprintf.c 
                     ${CMAKE_SOURCE_DIR}/core/clib/src/strlcpy.c
                     ${CMAKE_SOURCE_DIR}/core/clib/src/strlcat.c)


# File list for rootcint 
File(GLOB ROOTCINT_CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/R*.cxx")
List(REMOVE_ITEM ROOTCINT_CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/rootcint.cxx" "${CMAKE_CURRENT_SOURCE_DIR}/src/rlibmap.cxx")
List(APPEND ROOTCINT_CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/rootcint.cxx)

Add_Executable(rootcint ${ROOTCINT_CXX_SOURCES} ${METAUTILS_CXX_SOURCES} ${CLIB_EXT_SOURCES} )
Target_Link_Libraries(rootcint Cint)
Add_Dependencies(rootcint IOSENUM_H)
Install(TARGETS rootcint DESTINATION ${BIN_INSTALL_DIR})

#####################

Execute_process(
  COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/src/rootcint.cxx
          ${CMAKE_CURRENT_BINARY_DIR}/rootcint_tmp.cxx
  COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/src/RStl.cxx
          ${CMAKE_CURRENT_BINARY_DIR}/RStl_tmp.cxx
  COMMAND cmake -E copy_if_different ${CMAKE_SOURCE_DIR}/cint/cint/src/loadfile.cxx ${CMAKE_CURRENT_BINARY_DIR}/loadfile_tmp.cxx
)

Set(ROOTCINT_TMP_CXX_SOURCES 
  ${CMAKE_CURRENT_BINARY_DIR}/RStl_tmp.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/rootcint_tmp.cxx
)

Set_Source_Files_Properties(${ROOTCINT_TMP_CXX_SOURCES}
  COMPILE_FLAGS "-I${CMAKE_SOURCE_DIR}/cint/cint/src -I${CMAKE_SOURCE_DIR}/cint/cint/src/dict -I${CMAKE_SOURCE_DIR}/cint/cint/inc -I${HEADER_OUTPUT_PATH} -UR__HAVE_CONFIG -DROOTBUILD")

Set_Source_Files_Properties(${CMAKE_CURRENT_BINARY_DIR}/loadfile_tmp.cxx 
  COMPILE_FLAGS "-I${CMAKE_SOURCE_DIR}/cint/cint/src -I${CMAKE_SOURCE_DIR}/cint/cint/src/dict -I${CMAKE_SOURCE_DIR}/cint/cint/inc -I${HEADER_OUTPUT_PATH} ${CINT_CXX_DEFINITIONS} -DG__HAVE_CONFIG -DG__NOMAKEINFO -DG__CINTBODY -UR__HAVE_CONFIG -DROOTBUILD "
)


Add_Executable(rootcint_tmp ${ROOTCINT_TMP_CXX_SOURCES} ${METAUTILS_CXX_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/loadfile_tmp.cxx ${CLIB_EXT_SOURCES})
Target_Link_Libraries(rootcint_tmp Cint_static)
Add_Dependencies(rootcint_tmp CINT_HEADERS IOSENUM_H)

######################

Set(RLIBMAP_CXX_SOURCES src/rlibmap.cxx)

Add_Executable(rlibmap ${RLIBMAP_CXX_SOURCES})
Target_Link_Libraries(rlibmap ${CMAKE_THREAD_LIBS_INIT})
Add_Dependencies(rlibmap HEADER_FILES)
Install(TARGETS rlibmap DESTINATION ${BIN_INSTALL_DIR})
