Set(LibCore_SRCS)
Set(LibCore_INCLUDE_DIRS)
Set(LibCore_LINKDEF)

ROOT_USE_PACKAGE(core/base)
ROOT_USE_PACKAGE(core/clib)
ROOT_USE_PACKAGE(core/cont)
ROOT_USE_PACKAGE(core/meta)
ROOT_USE_PACKAGE(core/editline)
ROOT_USE_PACKAGE(core/unix)
ROOT_USE_PACKAGE(core/winnt)
ROOT_USE_PACKAGE(core/zip)
ROOT_USE_PACKAGE(cint/cint)


if(builtin_pcre)
  add_subdirectory(pcre)
endif()
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
if(WIN32)
  add_subdirectory(winnt)
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
    ${WINNT_DICTIONARY} 
    ${BASE1_DICTIONARY} 
    ${BASE2_DICTIONARY} 
    ${BASE3_DICTIONARY} 
    ${BASE4_DICTIONARY} 
   ) 

Set_Source_Files_Properties(${METAUTILS_DICTIONARY} GENERATED)
Set_Source_Files_Properties(${CLIB_DICTIONARY} GENERATED)
Set_Source_Files_Properties(${CONT_DICTIONARY} GENERATED)
if(editline)
  Set_Source_Files_Properties(${EDITLINE_DICTIONARY} GENERATED)
endif()
Set_Source_Files_Properties(${META_DICTIONARY} GENERATED)
if(UNIX)
  Set_Source_Files_Properties(${UNIX_DICTIONARY} GENERATED)
endif()
if(WIN32)
  Set_Source_Files_Properties(${WINNT_DICTIONARY} GENERATED)
endif()
Set_Source_Files_Properties(${BASE1_DICTIONARY} GENERATED)
Set_Source_Files_Properties(${BASE2_DICTIONARY} GENERATED)
Set_Source_Files_Properties(${BASE3_DICTIONARY} GENERATED)
Set_Source_Files_Properties(${BASE4_DICTIONARY} GENERATED)

Set_Source_Files_Properties(${EDITLINE_CXX_SOURCES} PROPERTIES
                            COMPILE_DEFINITIONS R__CURSESHDR="${CURSES_HEADER_FILE}"
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

if(WIN32)
   set(corelinklibs Cint shell32.lib WSock32.lib Oleaut32.lib Iphlpapi.lib)
endif()

ROOT_LINKER_LIBRARY(Core ${LibCore_SRCS} ${CORE_DICTIONARIES} 
                    LIBRARIES ${PCRE_LIBRARIES} ${ZLIB_LIBRARY} ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${corelinklibs})
 
# Here all objects ar put together into one share library
#=if(WIN32)
#=	foreach(src ${LibCore_SRCS} ${CORE_DICTIONARIES})
#=	  string (REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "" lsrc ${src})
#=	  string (REPLACE ${CMAKE_CURRENT_BINARY_DIR} "" msrc ${lsrc})
#=	  string (REGEX REPLACE "\\.cxx" ".obj" pobj ${msrc})
#=	  string (REGEX REPLACE "\\.c" ".obj" obj ${pobj})
#=	  set(lib_objs ${lib_objs} ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Core.dir/${obj})
#=	endforeach()
#=	add_library(Core SHARED ${LibCore_SRCS} ${CORE_DICTIONARIES} Core.def)
#=    add_custom_command(TARGET Core PRE_LINK
#=                       COMMAND bindexplib
#=                       ARGS -o Core.def libCore ${lib_objs} 
#=					   DEPENDS bindexplib )
#=    set_source_files_properties(Core.def PROPERTIES GENERATED 1)
#=    set(corelinklibs Cint shell32.lib WSock32.lib Oleaut32.lib Iphlpapi.lib)
#=else()
#=   add_library(Core SHARED ${LibCore_SRCS} ${CORE_DICTIONARIES})
#=endif()
#=Target_Link_Libraries(Core ${PCRE_LIBRARIES} ${ZLIB_LIBRARY} ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${corelinklibs})
#=Set_Target_properties(Core PROPERTIES ${ROOT_LIBRARY_PROPERTIES})

add_Dependencies(Core CLIB_DICTIONARY CONT_DICTIONARY  META_DICTIONARY METAUTILS_DICTIONARY BASE_DICTIONARY)
if(UNIX)
  add_dependencies(Core UNIX_DICTIONARY)
endif()
if(WIN32)
  add_dependencies(Core WINNT_DICTIONARY)
endif()
if(editline)
  add_dependencies(Core EDITLINE_DICTIONARY)
endif()
if(builtin_pcre)
  add_dependencies(Core PCRE)
endif()


#install(TARGETS Core RUNTIME DESTINATION bin
#                     LIBRARY DESTINATION lib
#                     ARCHIVE DESTINATION lib)
#Set(RootConfigFeatures ${RootConfigFeatures} PARENT_SCOPE)
