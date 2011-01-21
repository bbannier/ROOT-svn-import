# Create the library "libCint" and the executable "cint"

set(INCLUDE_DIRECTORIES
  ${ROOT_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/dict
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
include_directories( ${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
)
link_directories( ${LINK_DIRECTORIES})

if(CMAKE_THREAD_LIBS_INIT)
  set(LINK_FLAGS
  ${LINK_FLAGS}
  ${CMAKE_THREAD_LIBS_INIT}
  )
endif(CMAKE_THREAD_LIBS_INIT)

# Define header files which should be installed. 
#file(GLOB CINT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h")
#list(REMOVE_ITEM CINT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB CINT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/inc" "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h")
list(REMOVE_ITEM CINT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/inc")
#List(APPEND CINT_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/iosenum.h)

#Message("CINT_HEADERS: ${CINT_HEADERS}")

#INSTALL_HEADERS("CINT_HEADERS")



#foreach(_value ${CINT_HEADERS})
#  get_filename_component(_name ${_value} NAME) 
#  list(APPEND CINT_HEADERS_INSTALL ${HEADER_OUTPUT_PATH}/${_name})
#endforeach(_value ${CINT_HEADERS})
#list(APPEND CINT_HEADERS_INSTALL ${HEADER_OUTPUT_PATH}/configcint.h)

#add_custom_command(
#  OUTPUT CINT_HEADERS_INSTALL
#  OUTPUT ${CINT_HEADERS_INSTALL}
#  COMMAND cmake -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/inc/ 
#    ${HEADER_OUTPUT_PATH}
#  COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/iosenum.h 
#    ${HEADER_OUTPUT_PATH}
#  COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/iosenum.h 
#    ${CMAKE_BINARY_DIR}/cint/cint/include
#  DEPENDS ${CINT_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/iosenum.h
#  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/iosenum.h
#)

#add_custom_command(
#  OUTPUT ${CINT_HEADERS_INSTALL}
#  COMMAND cmake -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/inc/ 
#    ${HEADER_OUTPUT_PATH}
#  COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/iosenum.h 
#    ${HEADER_OUTPUT_PATH}
#  COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/configcint.h
#    ${HEADER_OUTPUT_PATH}
#  DEPENDS ${CINT_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/iosenum.h
#    ${CMAKE_CURRENT_BINARY_DIR}/configcint.h
#)

#add_custom_target(CINT_HEADERS_INSTALL
#  COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/iosenum.h 
#    ${HEADER_OUTPUT_PATH}
#  COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/iosenum.h 
#    ${CMAKE_BINARY_DIR}/cint/cint/include
#  COMMAND touch ${CMAKE_BINARY_DIR}/CINT_HEADERS_INSTALL
#  DEPENDS ${CINT_HEADERS_INSTALL} 
#  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/iosenum.h
#)

#set(BLA $ENV{ALL_HEADERS_INSTALL})
#set(BLA ${BLA} CINT_HEADERS_INSTALL)
#set(ENV{ALL_HEADERS_INSTALL} "${BLA}")


# Defining the source files to be compiled
file(GLOB CINT_C_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM CINT_C_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(CINT_C_SRCS 
  ${CINT_C_SRCS} 
  ${CMAKE_CURRENT_SOURCE_DIR}/main/G__setup.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config/strlcpy.c 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config/strlcpy.c 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config/strlcat.c 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config/snprintf.c 

)

file(GLOB CINT_CXX_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx")
list(REMOVE_ITEM CINT_CXX_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx")
list(REMOVE_ITEM CINT_CXX_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(CINT_CXX_SRCS 
  ${CINT_CXX_SRCS} 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/dict/Apiif.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/src/dict/stdstrct.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/src/dict/libstrm.cxx
)

SET(FILTER_OUT 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sunos.cxx 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/macos.cxx 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/winnt.cxx 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/newsos.cxx 
)
LIST(REMOVE_ITEM CINT_CXX_SRCS ${FILTER_OUT})

# Add/remove sourcefiles depending on the platform
# Change compile settings depending on the platform
include(${CMAKE_CURRENT_SOURCE_DIR}/PlatformDependentSettings.cmake)
set(CINT_CXX_DEFINITIONS 
  "${CINT_CXX_DEFINITIONS} -DG__HAVE_CONFIG -DG__NOMAKEINFO -DG__CINTBODY"
)
set(CINT_C_DEFINITIONS 
  "${CINT_C_DEFINITIONS} -DG__HAVE_CONFIG -DG__NOMAKEINFO -DG__CINTBODY"
)
ADD_DEFINITIONS(${CINT_CXX_DEFINITIONS})                
                
SET(CINT_SRCS ${CINT_C_SRCS} ${CINT_CXX_SRCS})
LIST(REMOVE_ITEM CINT_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/loadfile.cxx)

# Version of loadfile.cxx with different compiler settings
# needed for the cint_tmp executable
#add_custom_command(
#  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/loadfile_tmp.cxx
Execute_process(
  COMMAND cmake -E copy_if_different 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/loadfile.cxx 
    ${CMAKE_CURRENT_BINARY_DIR}/loadfile_tmp.cxx
)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/loadfile_tmp.cxx 
  COMPILE_FLAGS "-UR__HAVE_CONFIG -DROOTBUILD ${CINT_TMP_INC}"
)

# Generate and/or copy the iosenum headerfile 
# to the include directory
IF(${CLANG_MAJOR} EQUAL 2)
  set(IOSENUM_H 
    ${PROJECT_SOURCE_DIR}/cint/iosenum/iosenum.${ROOT_ARCHITECTURE}3
  )
ENDIF(${CLANG_MAJOR} EQUAL 2)
 
IF(${GCC_MAJOR} EQUAL 4)
  set(IOSENUM_H 
    ${PROJECT_SOURCE_DIR}/cint/iosenum/iosenum.${ROOT_ARCHITECTURE}3
  )
ELSE(${GCC_MAJOR} EQUAL 4)
  IF(${GCC_MAJOR} EQUAL 3)
    set(IOSENUM_H 
      ${PROJECT_SOURCE_DIR}/cint/iosenum/iosenum.${ROOT_ARCHITECTURE}3
    )
  ELSE(${GCC_MAJOR} EQUAL 3)
    set(IOSENUM_H ${PROJECT_SOURCE_DIR}/cint/iosenum/iosenum.${ROOT_ARCHITECTURE}
    )
  ENDIF(${GCC_MAJOR} EQUAL 3)
ENDIF(${GCC_MAJOR} EQUAL 4)

#set(CINT_TMP_INC 
#  "-I${HEADER_OUTPUT_PATH} -I${CMAKE_CURRENT_SOURCE_DIR}/include -I${CMAKE_CURRENT_SOURCE_DIR}/stl -I${CMAKE_CURRENT_SOURCE_DIR}/lib "
#)

if(EXISTS ${IOSENUM_H})
  # copy headerfile from source directory to build directory
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/iosenum.h
    COMMAND cmake -E copy ${IOSENUM_H} 
       ${CMAKE_CURRENT_BINARY_DIR}/iosenum.h
  )
else(EXISTS ${IOSENUM_H})
  #create header file using cint
  add_custom_command(
     OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/iosenum.h
     COMMAND echo 
     COMMAND cint_tmp -I${CMAKE_CURRENT_SOURCE_DIR}/include 
       -I${CMAKE_CURRENT_SOURCE_DIR}/stl -I${CMAKE_CURRENT_SOURCE_DIR}/lib 
       -I${HEADER_OUTPUT_PATH} ${PROJECT_SOURCE_DIR}/cint/iosenum/iosenum.cxx
     DEPENDS  cint_tmp
     #DEPENDS  ${CMAKE_CURRENT_BINARY_DIR}/iosenum.cxx
  )

endif(EXISTS ${IOSENUM_H})

  add_custom_target(IOSENUM_H
    COMMAND cmake -E copy ${CMAKE_CURRENT_BINARY_DIR}/iosenum.h 
      ${HEADER_OUTPUT_PATH}/iosenum.h
    COMMAND cmake -E copy ${CMAKE_CURRENT_BINARY_DIR}/iosenum.h 
      ${CMAKE_BINARY_DIR}/cint/cint/include/iosenum.h
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/iosenum.h
  )

#special compile settings
foreach(_value ${CINT_SRCS})
  get_filename_component(_name ${_value} NAME_WE) 
  string(REGEX MATCH "^.*strm" _bla ${_name})
  set(comp_flags_tmp)
  if(_bla)
    set(comp_flags_tmp "-I${CMAKE_CURRENT_SOURCE_DIR}/lib/${_bla}")
    set_source_files_properties(${_value} COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/lib/${_bla}")
  endif(_bla)
  if(${_name} MATCHES libstrm)
    set_source_files_properties(${_value} COMPILE_FLAGS "${comp_flags_tmp} -I${CMAKE_CURRENT_SOURCE_DIR}/lib/stream")    
  endif(${_name} MATCHES libstrm)
  if(${_name} MATCHES sun5strm)
    set_source_files_properties(${_value} COMPILE_FLAGS "${comp_flags_tmp} -I${CMAKE_CURRENT_SOURCE_DIR}/lib/sunstrm")    
  endif(${_name} MATCHES sun5strm)
  if(${_name} MATCHES vcstrm)
    set_source_files_properties(${_value} COMPILE_FLAGS "${comp_flags_tmp} -I${CMAKE_CURRENT_SOURCE_DIR}/lib/vcstream")    
  endif(${_name} MATCHES vcstrm)
  if(${_name} MATCHES gcc4strm)
    set_source_files_properties(${_value} COMPILE_FLAGS "${comp_flags_tmp} -Wno-strict-aliasing")
  endif(${_name} MATCHES gcc4strm)

  if(${_name} MATCHES stdstrct)
    set_source_files_properties(${_value} COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/lib/stdstrct")
  endif(${_name} MATCHES stdstrct)
endforeach(_value ${CINT_SRCS})

##############################################
#
# Define libraries and executables to be build
#
##############################################

# This static library is needed due to some cmake internal feature/bug
# If a source file is added to several programs the file is compiled
# more than once. In case of cint, cint_tmp, rootcint and rootcint_tmp
# this would need 4 compilations of many files.
# Creating a static library with all the files which are equal in all
# executables/libraries overcomes the problem.
# TODO: Check if this works on all platforms 
add_library(Cint_static STATIC ${CINT_SRCS})
add_dependencies(Cint_static INSTALL_HEADERS CONFIGCINT_INSTALL IOSENUM_H)

# Create the shared Cint library
add_library(Cint SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/loadfile.cxx)
target_link_libraries(Cint Cint_static ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(Cint PROPERTIES ${ROOT_LIBRARY_PROPERTIES})
#add_dependencies(Cint HEADER_FILES)

# Create the cint cint executable
add_executable(cintexe ${CMAKE_CURRENT_SOURCE_DIR}/main/cppmain.cxx)
target_link_libraries(cintexe Cint ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(cintexe PROPERTIES OUTPUT_NAME cint)

# Create the temporary cint_tmp executable
add_executable(cint_tmp ${CMAKE_CURRENT_SOURCE_DIR}/main/cppmain.cxx ${CMAKE_CURRENT_BINARY_DIR}/loadfile_tmp.cxx)
target_link_libraries(cint_tmp Cint_static ${CMAKE_THREAD_LIBS_INIT})
#add_dependencies(cint_tmp HEADER_FILES)

# Create the makecint executable
add_executable(makecint ${CMAKE_SOURCE_DIR}/cint/tool/makecint.cxx)
target_link_libraries(makecint ${CMAKE_THREAD_LIBS_INIT})
#add_dependencies(makecint HEADER_FILES)

################# install ###################
install(TARGETS Cint DESTINATION ${LIB_INSTALL_DIR})
install(TARGETS makecint DESTINATION ${BIN_INSTALL_DIR})
install(FILES ${CINT_HEADERS} ${HEADER_OUTPUT_PATH}/configcint.h ${CMAKE_BINARY_DIR}/cint/cint/include/iosenum.h ${HEADER_OUTPUT_PATH}/RConfigure.h DESTINATION ${INCLUDE_INSTALL_DIR})

# Create the cint-config script needed to run the tests
# Probably this file has not to be installed
# TODO: Check this
configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/cint-config.in
               ${EXECUTABLE_OUTPUT_PATH}/cint-config @ONLY)

# Create the configcint.h header file and copy the file 
# first to the cint/cint directory. Copy it to the 
# temporary include directory only if there is a change.
# Otherwise you have to recompile unneccessarily some files 
# depending on configcint.h all the time. 
Include(WriteConfigCint)

Set(CONFIGCINT_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/include1/configcint.h)
write_config_cint()
#add_custom_command(
#    OUTPUT ${HEADER_OUTPUT_PATH}/configcint.h
#    COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/configcint.h
#      ${HEADER_OUTPUT_PATH}
#    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/configcint.h
#)
add_custom_target(CONFIGCINT_INSTALL
  COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/include1/configcint.h
    ${HEADER_OUTPUT_PATH}
  COMMAND cmake -E copy_if_different ${CMAKE_SOURCE_DIR}/cint/cint/inc/cintdictversion.h
    ${HEADER_OUTPUT_PATH}
#  COMMAND touch ${CMAKE_BINARY_DIR}/CONFIGCINT_INSTALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/include1/configcint.h
)

#set(BLA $ENV{ALL_HEADERS_INSTALL})
#set(BLA ${BLA} CONFIGCINT_INSTALL)
#set(ENV{ALL_HEADERS_INSTALL} "${BLA}")
