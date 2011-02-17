Set(LibCore_SRCS)
Set(LibCore_INCLUDE_DIRS)
Set(LibCore_LINKDEF)

ROOT_USE_PACKAGE(core/base)
ROOT_USE_PACKAGE(core/clib)
ROOT_USE_PACKAGE(core/cont)
ROOT_USE_PACKAGE(core/meta)
ROOT_USE_PACKAGE(core/editline)
ROOT_USE_PACKAGE(core/unix)
ROOT_USE_PACKAGE(core/zip)
ROOT_USE_PACKAGE(cint/cint)

#---Now build the builtin version of pcre and zlib ----------------------------------------
if(builtin_pcre)
  add_subdirectory(pcre)
  set(PCRE_INCLUDE_DIR ${CMAKE_BINARY_DIR}/core/pcre/pcre-7.8)
  set(PCRE_LIBRARIES "-L${LIBRARY_OUTPUT_PATH} -lpcre") 
endif()

Set(BuildZlib TRUE)
Set(ZLIB_LIBRARY "")

add_subdirectory(rint)
add_subdirectory(thread)
add_subdirectory(newdelete)
add_subdirectory(clib)
add_subdirectory(cont)
if(editline)
  add_subdirectory(editline)
endif()
add_subdirectory(meta)
if(UNIX)
  add_subdirectory(unix)
endif()
add_subdirectory(zip)
add_subdirectory(base)
add_subdirectory(metautils)
add_subdirectory(utils)

Set(CORE_DICTIONARIES 
    ${METAUTILS_DICTIONARY}
    ${CLIB_DICTIONARY} 
    ${CONT_DICTIONARY} 
    ${EDITLINE_DICTIONARY} 
    ${META_DICTIONARY} 
    ${UNIX_DICTIONARY} 
    ${BASE1_DICTIONARY} 
    ${BASE2_DICTIONARY} 
    ${BASE3_DICTIONARY} 
    ${BASE4_DICTIONARY} 
   ) 

Set_Source_Files_Properties(${METAUTILS_DICTIONARY} GENERATED)
Set_Source_Files_Properties(${CLIB_DICTIONARY} GENERATED)
Set_Source_Files_Properties(${CONT_DICTIONARY} GENERATED)
Set_Source_Files_Properties(${EDITLINE_DICTIONARY} GENERATED)
Set_Source_Files_Properties(${META_DICTIONARY} GENERATED)
Set_Source_Files_Properties(${UNIX_DICTIONARY} GENERATED)
Set_Source_Files_Properties(${BASE1_DICTIONARY} GENERATED)
Set_Source_Files_Properties(${BASE2_DICTIONARY} GENERATED)
Set_Source_Files_Properties(${BASE3_DICTIONARY} GENERATED)
Set_Source_Files_Properties(${BASE4_DICTIONARY} GENERATED)

Set_Source_Files_Properties(${EDITLINE_CXX_SOURCES} 
                            COMPILE_FLAGS -DR__CURSESHDR=\\"${CURSES_HEADER_FILE}\\"
                           )


Set_Source_Files_Properties(${CMAKE_SOURCE_DIR}/core/base/src/TPRegexp.cxx
                            ${BASE1_DICTIONARY} ${BASE2_DICTIONARY}
                            COMPILE_FLAGS -I${PCRE_INCLUDE_DIR}
                           )

If(${GCC_MAJOR} EQUAL 4 AND ${GCC_MINOR} EQUAL 1)
  Set_Source_Files_Properties(${CMAKE_SOURCE_DIR}/core/base/src/TString.cxx
                              ${CMAKE_SOURCE_DIR}/core/base/src/TContextMenu.cxx
                              COMPILE_FLAGS -Wno-strict-aliasing
                             )
EndIf(${GCC_MAJOR} EQUAL 4 AND ${GCC_MINOR} EQUAL 1)

If(${ROOT_ARCHITECTURE} MATCHES linuxicc)
  Set_Source_Files_Properties(${BASE3_DICTIONARY} 
                              COMPILE_FLAGS -wd191
                             )
EndIf(${ROOT_ARCHITECTURE} MATCHES linuxicc)


Set(INCLUDE_DIRECTORIES 
    ${CMAKE_SOURCE_DIR}/core/metautils/inc 
    ${CMAKE_SOURCE_DIR}/core/meta/inc 
    ${CMAKE_SOURCE_DIR}/core/cont/inc 
    ${CMAKE_SOURCE_DIR}/core/unix/inc 
    ${CMAKE_SOURCE_DIR}/graf3d/g3d/inc
    ${CMAKE_SOURCE_DIR}/net/net/inc
    ${CMAKE_SOURCE_DIR}/core/base/src 
    ${ROOT_INCLUDE_DIR} 
   ) 

#GENERATE_ROOTMAP("${LibCore_LINKDEF}" Core)
ROOT_GENERATE_ROOTMAP(Core LINKDEF ${LibCore_LINKDEF})

include_directories( ${INCLUDE_DIRECTORIES})

# Here all objects ar put together into one share library
Add_Library(Core SHARED ${LibCore_SRCS} ${CORE_DICTIONARIES})
Target_Link_Libraries(Core ${PCRE_LIBRARIES} ${ZLIB_LIBRARY} ${CURSES_NCURSES_LIBRARY} ${CURSES_CURSES_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
Set_Target_properties(Core PROPERTIES ${ROOT_LIBRARY_PROPERTIES})
Add_Dependencies(Core CLIB_DICTIONARY CONT_DICTIONARY EDITLINE_DICTIONARY META_DICTIONARY METAUTILS_DICTIONARY UNIX_DICTIONARY BASE_DICTIONARY ${PCRE_VERSION})
Install(TARGETS Core DESTINATION ${LIB_INSTALL_DIR})

Set(RootConfigFeatures ${RootConfigFeatures} PARENT_SCOPE)
