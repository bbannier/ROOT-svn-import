ROOT_USE_PACKAGE(core/base)
ROOT_USE_PACKAGE(core/meta)
ROOT_USE_PACKAGE(core/clib)
ROOT_USE_PACKAGE(core/cont)
ROOT_USE_PACKAGE(cint/cint)

Set(CINTSTLDLLNAMES
    vector 
    vectorbool 
    list 
    deque 
    map 
    map2 
    set 
    multimap 
    multimap2 
    multiset 
    stack 
    queue 
    exception 
    stdexcept 
    climits 
    complex
    stdcxxfunc
   )
Set(CINTDLLDIR ${CMAKE_SOURCE_DIR}/cint/cint/lib/dll_stl/)
Set(CINTSTLDLLHEADERS
    ${CINTDLLDIR}/vec.h
    ${CINTDLLDIR}/vecbool.h 
    ${CINTDLLDIR}/lst.h 
    ${CINTDLLDIR}/dqu.h 
    ${CINTDLLDIR}/mp.h 
    ${CINTDLLDIR}/mp.h 
    ${CINTDLLDIR}/st.h 
    ${CINTDLLDIR}/multmp.h 
    ${CINTDLLDIR}/multmp.h 
    ${CINTDLLDIR}/multst.h 
    ${CINTDLLDIR}/stk.h 
    ${CINTDLLDIR}/que.h 
    ${CINTDLLDIR}/cinteh.h 
    ${CINTDLLDIR}/se.h 
    ${CINTDLLDIR}/clim.h 
    ${CINTDLLDIR}/cmplx.h
    ${CMAKE_SOURCE_DIR}/cint/cint/lib/stdstrct/stdcxxfunc.h
  )
Set(CINTINCDLLNAMES 
    stdfunc 
#    stdcxxfunc
   )
Set(CINTINCDLLHEADERS
    ${CMAKE_SOURCE_DIR}/cint/cint/lib/stdstrct/stdfunc.h 
#    ${CMAKE_SOURCE_DIR}/cint/cint/lib/stdstrct/stdcxxfunc.h
   )
If(NOT ${ROOT_ARCHITECTURE} MATCHES win32)
  # FIX THEM!
  Set(CINTSTLDLLNAMES
      ${CINTSTLDLLNAMES} 
      valarray
     )
  Set(CINTSTLDLLHEADERS
      ${CINTSTLDLLHEADERS} 
      ${CINTDLLDIR}/vary.h
     )
  Set(CINTINCDLLNAMES
      ${CINTINCDLLNAMES} 
      posix 
      ipc
     )
  Set(CINTINCDLLHEADERS
      ${CINTINCDLLHEADERS} 
      ${CMAKE_SOURCE_DIR}/cint/cint/lib/posix/exten.h 
      ${CMAKE_SOURCE_DIR}/cint/cint/lib/ipc/ipcif.h
     )
EndIf(NOT ${ROOT_ARCHITECTURE} MATCHES win32)

Set(CINTBUILDLOADER
    vector 
    list 
    deque 
    map 
    map2 
    set 
    multimap 
    multimap2 
    multiset 
    valarray
)

Set(CINTDLLCXXFLAGS "${CINT_CXX_DEFINITIONS} -DG__HAVE_CONFIG -DG__NOMAKEINFO")
Set(CINTDLLCFLAGS "${CINT_C_DEFINITIONS} -DG__HAVE_CONFIG -DG__NOMAKEINFO -DG__SYSTYPES")

# To generate the dictonaries rootcint_tmp has to run in $ROOTSYS with the
# include files in $ROOTSYS/include.
# This can't be done in a separate build directory, but what can be done is
# to simulate the setup by symbolic links for some cint directories. It looks
# like this is enough.
#Execute_Process(COMMAND cmake -E create_symlink ${CMAKE_SOURCE_DIR}/cint/cint/include ${CMAKE_BINARY_DIR}/cint/cint/include 
#                COMMAND cmake -E create_symlink ${CMAKE_SOURCE_DIR}/cint/cint/lib ${CMAKE_BINARY_DIR}/cint/cint/lib 
#                COMMAND cmake -E create_symlink ${CMAKE_SOURCE_DIR}/cint/cint/stl ${CMAKE_BINARY_DIR}/cint/cint/stl 
#)

Set(AdditionalHeaderFiles ${HEADER_OUTPUT_PATH}/cintdictversion.h)
#Set(IncludeDirs -I${CMAKE_SOURCE_DIR}/cint/cint/lib -I${CMAKE_SOURCE_DIR}/cint/cint/lib/dll_stl)
Set(IncludeDirs -I${CMAKE_SOURCE_DIR}/cint/cint/inc)
Set(AllFiles)
ForEach(_name ${CINTSTLDLLNAMES}) 
  Set(OutFileName ${CMAKE_CURRENT_BINARY_DIR}/G__cpp_${_name}.cxx)
  Set(LoaderName ${CMAKE_CURRENT_BINARY_DIR}/stlLoader_${_name}.cc)
  List(FIND CINTSTLDLLNAMES ${_name} _position)
  List(GET CINTSTLDLLHEADERS ${_position} InFileName)
  If(${_name} MATCHES map2)
    Set(AdditionalFlags "-DG__MAP2")
  Else(${_name} MATCHES map2)
    Set(AdditionalFlags "")
  EndIf(${_name} MATCHES map2)



  Add_Custom_Command(OUTPUT ${OutFileName} 
                     COMMAND ${EXECUTABLE_OUTPUT_PATH}/cint_tmp -w1 -z${_name} -n${OutFileName} ${AdditionalFlags} -D__MAKECINT__ -DG__MAKECINT ${IncludeDirs} -c-1 -A -Z0 ${InFileName} ${AdditionalHeaderFiles}
                     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                     DEPENDS cint_tmp
                    ) 

  List(FIND CINTBUILDLOADER ${_name} _position)
  If(NOT ${_position} EQUAL -1)
    Add_Custom_Command(OUTPUT ${LoaderName}
                       COMMAND cmake -E copy_if_different ${CMAKE_SOURCE_DIR}/core/metautils/src/stlLoader.cc  ${LoaderName}
                      )

    Set(_compile_flags "${CINTDLLCXXFLAGS} -DWHAT=\\\"${_name}\\\" -I${CMAKE_SOURCE_DIR}/cint/cint/inc")
    Set_Source_Files_Properties(${LoaderName} 
                                COMPILE_FLAGS ${_compile_flags}
                               )
    Set(DictSource ${CMAKE_CURRENT_BINARY_DIR}/rootcint_${_name}.cxx)
    Set(DictHeader ${CMAKE_CURRENT_BINARY_DIR}/rootcint_${_name}.h)

    If(${_name} MATCHES map)
      Set(_name1 map)
    Else(${_name} MATCHES map)
      If(${_name} MATCHES set)
        Set(_name1 set)
      Else(${_name} MATCHES set)
        Set(_name1 ${_name})
      EndIf(${_name} MATCHES set)
    EndIf(${_name} MATCHES map)

    Set(LINKDEF_FILE "${CMAKE_SOURCE_DIR}/core/metautils/src/${_name}Linkdef.h")
    get_directory_property(incdirs INCLUDE_DIRECTORIES)
    set(includedirs -I${CMAKE_CURRENT_SOURCE_DIR}/inc 
                    -I${CMAKE_BINARY_DIR}/include
                    -I${CMAKE_SOURCE_DIR}/cint/cint/include 
                    -I${CMAKE_SOURCE_DIR}/cint/cint/stl 
                    -I${CMAKE_SOURCE_DIR}/cint/cint/lib) 
    foreach( d ${incdirs})    
     set(includedirs ${includedirs} -I${d})
    endforeach()


    Add_Custom_Command(OUTPUT ${DictSource} ${DictHeader}
                       COMMAND rootcint_tmp  -f ${DictSource} -c ${includedirs} ${_name1} ${LINKDEF_FILE}
                       WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                       DEPENDS rootcint_tmp
                      )

    Set_Source_Files_Properties(${DictSource} 
                                COMPILE_FLAGS "${CINTDLLCXXFLAGS} -DG__CINTBODY -I${HEADER_OUTPUT_PATH}"
                               )
    Add_Library(lib${_name}Dict SHARED ${DictSource} )
    Set_Target_properties(lib${_name}Dict PROPERTIES SUFFIX ".so")
#  Install(TARGETS lib${_name}Dict DESTINATION ${LIB_INSTALL_DIR})

    Set(LIBNAME lib${_name}Dict) 
    Set(MAP_OUTFILE "${LIBRARY_OUTPUT_PATH}/${LIBNAME}.rootmap")
#    Set(MAP_INFILE "${LIBRARY_OUTPUT_PATH}/lib${_name}.dll")

    Add_Custom_Command(OUTPUT ${MAP_OUTFILE}
                       COMMAND rlibmap -o ${MAP_OUTFILE} -l ${_name}.dll -c ${LINKDEF_FILE} 
                       DEPENDS ${LIBNAME} ${LINKDEF_FILE} rlibmap
                      )

  Install(FILES ${MAP_OUTFILE} DESTINATION ${INCLUDE_INSTALL_DIR})

  Add_Custom_Target(${LIBNAME}.rootmap ALL
                    DEPENDS ${MAP_OUTFILE}
                   )

  Else(NOT ${_position} EQUAL -1)
    Set(LoaderName)
  EndIf(NOT ${_position} EQUAL -1)

  Set_Source_Files_Properties(${OutFileName} 
                              COMPILE_FLAGS "-I${CMAKE_SOURCE_DIR}/cint/cint/inc"
                             )
  Set(INCLUDE_DIRECTORIES 
      ${CMAKE_SOURCE_DIR}/cint/cint/inc 
     ) 

#  include_directories( ${INCLUDE_DIRECTORIES})
  Add_Library(${_name} ${LoaderName} ${OutFileName} )
  Set_Target_properties(${_name} PROPERTIES SUFFIX ".dll")
#  Install(TARGETS ${_name} DESTINATION ${LIB_INSTALL_DIR})
   

EndForEach(_name ${CINTSTLDLLNAMES})

ForEach(_name ${CINTINCDLLNAMES}) 
  Set(OutFileName ${CMAKE_CURRENT_BINARY_DIR}/G__c_${_name}.c)
 
  List(FIND CINTINCDLLNAMES ${_name} _position)
  List(GET CINTINCDLLHEADERS ${_position} InFileName)

  If(${_name} MATCHES stdfunc)
    Add_Custom_Command(OUTPUT ${OutFileName} 
                       COMMAND ${EXECUTABLE_OUTPUT_PATH}/cint_tmp -K -w1 -z${_name} -n${OutFileName} -D__MAKECINT__ -DG__MAKECINT ${IncludeDirs} -c-2 -Z0 ${InFileName} ${AdditionalHeaderFiles}
                       WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                       DEPENDS cint_tmp
                      ) 

    Set_Source_Files_Properties(${OutFileName} 
                                COMPILE_FLAGS "${CINTDLLCFLAGS} -I${CMAKE_SOURCE_DIR}/cint/cint/inc"
                               )

    Set(INCLUDE_DIRECTORIES 
        ${CMAKE_SOURCE_DIR}/cint/cint/inc 
       ) 

#    include_directories( ${INCLUDE_DIRECTORIES})
    Add_Library(${_name} ${OutFileName})
    Set_Target_properties(${_name} PROPERTIES SUFFIX ".dll")
#  Install(TARGETS ${_name} DESTINATION ${LIB_INSTALL_DIR})
  EndIf(${_name} MATCHES stdfunc)

  If(${_name} MATCHES ipc)
    Add_Custom_Command(OUTPUT ${OutFileName} 
                       COMMAND ${EXECUTABLE_OUTPUT_PATH}/cint_tmp -K -w1 -z${_name} -n${OutFileName} -D__MAKECINT__ -DG__MAKECINT ${IncludeDirs} -c-2 -Z0 ${InFileName} ${AdditionalHeaderFiles}
                       WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                       DEPENDS ${CMAKE_BINARY_DIR}/cint/cint/include/sys/types.h
                               cint_tmp
#                       DEPENDS cint_tmp
                      ) 

    Set_Source_Files_Properties(${OutFileName} 
                                COMPILE_FLAGS "${CINTDLLCFLAGS} -I${CMAKE_SOURCE_DIR}/cint/cint/inc"
                               )

    Set(INCLUDE_DIRECTORIES 
        ${CMAKE_SOURCE_DIR}/cint/cint/inc 
       ) 

#    include_directories( ${INCLUDE_DIRECTORIES})
    Add_Library(${_name} ${OutFileName})
    Set_Target_properties(${_name} PROPERTIES SUFFIX ".dll")
#  Install(TARGETS ${_name} DESTINATION ${LIB_INSTALL_DIR})
  EndIf(${_name} MATCHES ipc)


  If(${_name} MATCHES posix)
    
    Add_Custom_Command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mktypes.c 
                       COMMAND cmake -E copy_if_different 
                               ${CMAKE_SOURCE_DIR}/cint/cint/lib/posix/mktypes.c
                               ${CMAKE_CURRENT_BINARY_DIR}/mktypes.c
                      ) 
    Add_Executable(mktypes ${CMAKE_CURRENT_BINARY_DIR}/mktypes.c)
    Add_Custom_Command(OUTPUT ${HEADER_OUTPUT_PATH}/systypes.h 
                       COMMAND ${EXECUTABLE_OUTPUT_PATH}/mktypes
                       DEPENDS mktypes
                      ) 
    Add_Custom_Command(OUTPUT ${CMAKE_BINARY_DIR}/cint/cint/include/sys/types.h 
                       COMMAND cmake -E copy_if_different 
                               ${HEADER_OUTPUT_PATH}/systypes.h
                               ${CMAKE_BINARY_DIR}/cint/cint/include/sys/types.h
                       COMMAND cmake -E copy_if_different 
                               ${HEADER_OUTPUT_PATH}/systypes.h
                               ${CMAKE_BINARY_DIR}/cint/cint/include/systypes.h
                       DEPENDS ${HEADER_OUTPUT_PATH}/systypes.h
                      ) 

    If(macosx105)
      Set(_ExtraFlag "-D__DARWIN_UNIX03")
    EndIf(macosx105)

    Add_Custom_Command(OUTPUT ${OutFileName} 
                       COMMAND ${EXECUTABLE_OUTPUT_PATH}/cint_tmp -K -w1 -z${_name} -n${OutFileName} -D__MAKECINT__ -DG__MAKECINT ${_ExtraFlag} -c-2 -Z0 ${InFileName} ${AdditionalHeaderFiles} ${CMAKE_BINARY_DIR}/cint/cint/include/sys/types.h ${CMAKE_SOURCE_DIR}/cint/cint/lib/posix/posix.h
                       WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                       DEPENDS ${CMAKE_BINARY_DIR}/cint/cint/include/sys/types.h
                               cint_tmp
                      ) 
    Set_Source_Files_Properties(${OutFileName} 
                                COMPILE_FLAGS "${CINT_C_DEFINITIONS}  -DG__HAVE_CONFIG -DG__NOMAKEINFO -I${CMAKE_SOURCE_DIR}/cint/cint/inc -I${CMAKE_SOURCE_DIR}/cint/cint/src -I${CMAKE_SOURCE_DIR}/cint/cint/src/dict -DG__SYSTYPES_H" 
                               )
    Set_Source_Files_Properties(${CMAKE_SOURCE_DIR}/cint/cint/lib/posix/exten.c
                                COMPILE_FLAGS " -DG__HAVE_CONFIG -DG__NOMAKEINFO -DG__CINTBODY"
                               )

    Add_Library(${_name} ${OutFileName} ${CMAKE_SOURCE_DIR}/cint/cint/lib/posix/exten.c)
    Set_Target_properties(${_name} PROPERTIES SUFFIX ".dll")
#  Install(TARGETS ${_name} DESTINATION ${LIB_INSTALL_DIR})
  EndIf(${_name} MATCHES posix)
   


EndForEach(_name ${CINTINCDLLNAMES}) 
