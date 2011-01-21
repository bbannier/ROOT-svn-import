#---------------------------------------------------------------------------------------------------
#  RootNewMacros.cmake
#---------------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.4.6)
cmake_policy(SET CMP0003 NEW) # See "cmake --help-policy CMP0003" for more details
cmake_policy(SET CMP0011 NEW) # See "cmake --help-policy CMP0011" for more details
cmake_policy(SET CMP0009 NEW) # See "cmake --help-policy CMP0009" for more details


set(lib lib)
set(bin bin)
if(WIN32)
  set(ssuffix .bat)
  set(scomment rem)
  set(libprefix "")
  set(ld_library_path PATH)
elseif(APPLE)
  set(ld_library_path DYLD_LIBRARY_PATH)
  set(ssuffix .csh)
  set(scomment \#)
  set(libprefix lib)
else()
  set(ld_library_path LD_LIBRARY_PATH)
  set(ssuffix .csh)
  set(scomment \#)
  set(libprefix lib)
endif()

set(CMAKE_VERBOSE_MAKEFILES OFF)
set(CMAKE_INCLUDE_CURRENT_DIR OFF)

include(CMakeMacroParseArguments)


if(CMAKE_PROJECT_NAME STREQUAL ROOT)
  set(merge_rootmap_cmd ${python_cmd} ${CMAKE_SOURCE_DIR}/GaudiPolicy/scripts/merge_files.py)
  set(merge_conf_cmd ${python_cmd} ${CMAKE_SOURCE_DIR}/GaudiPolicy/scripts/merge_files.py)
  set(genconf_cmd ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/genconf.exe)
  set(versheader_cmd ${python_cmd} ${CMAKE_SOURCE_DIR}/GaudiPolicy/scripts/createProjVersHeader.py)
  set(gaudirun ${CMAKE_SOURCE_DIR}/Gaudi/scripts/gaudirun.py)
  set(zippythondir_cmd ${python_cmd} ${CMAKE_SOURCE_DIR}/GaudiPolicy/scripts/ZipPythonDir.py)
  set(cmdwrap_cmd ${CMAKE_SOURCE_DIR}/cmake/cmdwrap.bat)
else()
  set(merge_rootmap_cmd ${python_cmd}  ${GAUDI_installation}/GaudiPolicy/scripts/merge_files.py)
  set(merge_conf_cmd ${python_cmd}  ${GAUDI_installation}/GaudiPolicy/scripts/merge_files.py)
  set(genconf_cmd ${GAUDI_binaryarea}/bin/genconf.exe)
  set(versheader_cmd ${python_cmd} ${GAUDI_installation}/GaudiPolicy/scripts/createProjVersHeader.py)
  set(GAUDI_SOURCE_DIR ${GAUDI_installation})
  set(gaudirun ${GAUDI_installarea}/scripts/gaudirun.py)
  set(zippythondir_cmd ${python_cmd} ${GAUDI_installation}/GaudiPolicy/scripts/ZipPythonDir.py)
  set(cmdwrap_cmd ${GAUDI_binaryarea}/scripts/cmdwrap.bat)
endif()


#---------------------------------------------------------------------------------------------------
#---REFLEX_GENERATE_DICTIONARY( dictionary headerfiles selectionfile OPTIONS opt1 opt2 ...)
#---------------------------------------------------------------------------------------------------
macro(REFLEX_GENERATE_DICTIONARY dictionary _headerfiles _selectionfile)  
  find_package(GCCXML)
  find_package(ROOT)
  PARSE_ARGUMENTS(ARG "OPTIONS" "" ${ARGN})  
  if( IS_ABSOLUTE ${_selectionfile}) 
   set( selectionfile ${_selectionfile})
  else() 
   set( selectionfile ${CMAKE_CURRENT_SOURCE_DIR}/${_selectionfile}) 
  endif()
  if( IS_ABSOLUTE ${_headerfiles}) 
    set( headerfiles ${_headerfiles})
  else()
    set( headerfiles ${CMAKE_CURRENT_SOURCE_DIR}/${_headerfiles})
  endif()
 
  set(gensrcdict ${dictionary}_dict.cpp)

  if(MSVC)
    set(gccxmlopts "--gccxmlopt=\"--gccxml-compiler cl\"")
  else()
    #set(gccxmlopts "--gccxmlopt=\'--gccxml-cxxflags -m64 \'")
    set(gccxmlopts)
  endif()
  
  set(rootmapname ${dictionary}Dict.rootmap)
  set(rootmapopts --rootmap=${rootmapname} --rootmap-lib=${libprefix}${dictionary}Dict)

  set(include_dirs -I${CMAKE_CURRENT_SOURCE_DIR})
  get_directory_property(_incdirs INCLUDE_DIRECTORIES)
  foreach( d ${_incdirs})    
   set(include_dirs ${include_dirs} -I${d})
  endforeach()

  get_directory_property(_defs COMPILE_DEFINITIONS)
  foreach( d ${_defs})    
   set(definitions ${definitions} -D${d})
  endforeach()

  add_custom_command(
    OUTPUT ${gensrcdict} ${rootmapname}     
    COMMAND ${ROOT_genreflex_cmd}       
    ARGS ${headerfiles} -o ${gensrcdict} ${gccxmlopts} ${rootmapopts} --select=${selectionfile}
         --gccxmlpath=${GCCXML_home}/bin ${ARG_OPTIONS} ${include_dirs} ${definitions}
    DEPENDS ${headerfiles} ${selectionfile})  

  # Creating this target at ALL level enables the possibility to generate dictionaries (genreflex step)
  # well before the dependent libraries of the dictionary are build  
  add_custom_target(${dictionary}Gen ALL DEPENDS ${gensrcdict})
 
endmacro()

#---------------------------------------------------------------------------------------------------
#---ROOT_GENERATE_DICTIONARY( dictionary headerfiles LINKDEF linkdef OPTIONS opt1 opt2 ...)
#---------------------------------------------------------------------------------------------------
function(ROOT_GENERATE_DICTIONARY dictionary)
  #---Get the list of header files-------------------------
  PARSE_ARGUMENTS(ARG "LINKDEF;OPTIONS" "" ${ARGN})
  set(headerfiles)
  foreach( fp ${ARG_DEFAULT_ARGS})
    file(GLOB files inc/${fp})
    if(files)
      foreach( f ${files}) 
        if(NOT f MATCHES LinkDef)
          set( headerfiles ${headerfiles} ${f})
        endif()
      endforeach()
    else()
      set( headerfiles ${headerfiles} ${fp})
    endif()
  endforeach()
  #---Get the list of include directories------------------
  get_directory_property(incdirs INCLUDE_DIRECTORIES)
  set(includedirs -I${CMAKE_CURRENT_SOURCE_DIR}/inc 
                  -I${CMAKE_BINARY_DIR}/include
                  -I${CMAKE_SOURCE_DIR}/cint/cint/include 
                  -I${CMAKE_SOURCE_DIR}/cint/cint/stl 
                  -I${CMAKE_SOURCE_DIR}/cint/cint/lib) 
  foreach( d ${incdirs})    
   set(includedirs ${includedirs} -I${d})
  endforeach()
  #---Get LinkDef.h file------------------------------------
  foreach( f ${ARG_LINKDEF})
    if( IS_ABSOLUTE ${f})
      set(_linkdef ${_linkdef} ${f})
    else() 
      if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/inc/${f})
        set(_linkdef ${_linkdef} ${CMAKE_CURRENT_SOURCE_DIR}/inc/${f})
      else()
        set(_linkdef ${_linkdef} ${CMAKE_CURRENT_SOURCE_DIR}/${f})
      endif()
    endif()
  endforeach()
  #---call rootcint------------------------------------------
  add_custom_command(OUTPUT ${dictionary}.cxx ${dictionary}.h
                     COMMAND rootcint_tmp -cint -f  ${dictionary}.cxx 
                                          -c ${ARG_OPTIONS} ${includedirs} ${headerfiles} ${_linkdef} 
                     DEPENDS ${headerfiles} ${_linkdef} rootcint_tmp )
endfunction()


#---------------------------------------------------------------------------------------------------
#---ROOT_LINKER_LIBRARY( <name> source1 source2 ... [DLLEXPORT] LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(ROOT_LINKER_LIBRARY library)
  PARSE_ARGUMENTS(ARG "LIBRARIES" "DLLEXPORT" ${ARGN})
  set(lib_srcs)
  foreach( fp ${ARG_DEFAULT_ARGS})
    file(GLOB files src/${fp})
    if(files) 
      set( lib_srcs ${lib_srcs} ${files})
    else()
      set( lib_srcs ${lib_srcs} ${fp})
    endif()
  endforeach()
  include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/inc ${CMAKE_BINARY_DIR}/include )
  if(WIN32 AND NOT BUILD_DLLEXPORT_LIBS AND NOT ARG_DLLEXPORT)
	add_library( ${library}-arc STATIC EXCLUDE_FROM_ALL ${lib_srcs})
    set_target_properties(${library}-arc PROPERTIES COMPILE_FLAGS -DGAUDI_LINKER_LIBRARY )
    add_custom_command( 
      OUTPUT ${library}.def
	  COMMAND ${genwindef_cmd} -o ${library}.def -l ${library} ${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/${library}-arc.lib
	  DEPENDS ${library}-arc genwindef)
	#---Needed to create a dummy source file to please Windows IDE builds with the manifest
	file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/${library}.cpp "// empty file\n" )
    add_library( ${library} SHARED ${library}.cpp ${library}.def)
    target_link_libraries(${library} ${library}-arc ${ARG_LIBRARIES})
    set_target_properties(${library} PROPERTIES LINK_INTERFACE_LIBRARIES "${ARG_LIBRARIES}" )
  else()
    add_library( ${library} SHARED ${lib_srcs})
    set_target_properties(${library} PROPERTIES  ${ROOT_LIBRARY_PROPERTIES} )
    target_link_libraries(${library} ${ARG_LIBRARIES})
  endif()
  #----Installation details-------------------------------------------------------
  #install(TARGETS ${library} EXPORT ${CMAKE_PROJECT_NAME}Exports DESTINATION ${lib})
  install(TARGETS ${library} DESTINATION ${lib})
  #install(EXPORT ${CMAKE_PROJECT_NAME}Exports DESTINATION cmake) 
endfunction()


#---------------------------------------------------------------------------------------------------
#---ROOT_USE_PACKAGE( package )
#---------------------------------------------------------------------------------------------------
macro( ROOT_USE_PACKAGE package )
  if( EXISTS ${CMAKE_SOURCE_DIR}/${package}/CMakeLists.txt)
    include_directories( ${CMAKE_SOURCE_DIR}/${package}/inc ) 
    file(READ ${CMAKE_SOURCE_DIR}/${package}/CMakeLists.txt file_contents)
    string( REGEX MATCHALL "ROOT_USE_PACKAGE[ ]*[(][ ]*([^ )])+" vars ${file_contents})
    foreach( var ${vars})
      string(REGEX REPLACE "ROOT_USE_PACKAGE[ ]*[(][ ]*([^ )])" "\\1" p ${var})
      ROOT_USE_PACKAGE(${p})
    endforeach()
  else()
    find_package(${package})
    GET_PROPERTY(parent DIRECTORY PROPERTY PARENT_DIRECTORY)
    if(parent)
      set(${package}_environment  ${${package}_environment} PARENT_SCOPE)
    else()
      set(${package}_environment  ${${package}_environment} )
    endif()
    include_directories( ${${package}_INCLUDE_DIRS} ) 
    link_directories( ${${package}_LIBRARY_DIRS} ) 
  endif()
endmacro()


#---------------------------------------------------------------------------------------------------
#---ROOT_GENERATE_ROOTMAP( library LINKDEF linkdef DEPENDENCIES lib1 lib2 )
#---------------------------------------------------------------------------------------------------
function(ROOT_GENERATE_ROOTMAP library)
  PARSE_ARGUMENTS(ARG "LINKDEF;DEPENDENCIES" "" ${ARGN})
  get_filename_component(libname ${library} NAME_WE)
  get_filename_component(path ${library} PATH)
  set(outfile ${LIBRARY_OUTPUT_PATH}/${libprefix}${libname}.rootmap)
  foreach( f ${ARG_LINKDEF})
    if( IS_ABSOLUTE ${f})
      set(_linkdef ${_linkdef} ${f})
    else() 
      set(_linkdef ${_linkdef} ${CMAKE_CURRENT_SOURCE_DIR}/inc/${f})
    endif()
  endforeach()
  #---Build the rootmap file--------------------------------------
  add_custom_command(OUTPUT ${outfile}
                     COMMAND rlibmap -o ${outfile} -l ${libprefix}${library}${CMAKE_SHARED_LIBRARY_SUFFIX} -d ${ARG_DEPENDENCIES} -c ${_linkdef} 
                     DEPENDS ${library} ${_linkdef} rlibmap )
  add_custom_target( ${libprefix}${library}.rootmap ALL DEPENDS  ${outfile})
  #---Install the rootmap file------------------------------------
  install(FILES ${outfile} DESTINATION lib)
endfunction()


#---------------------------------------------------------------------------------------------------
#---ROOT_INSTALL_HEADERS([dir1 dir2 ...])
#---------------------------------------------------------------------------------------------------
function(ROOT_INSTALL_HEADERS)
  if( ARGN )
    set(dirs ${ARGN})
  else()
    set(dirs inc/)
  endif()
  foreach(d ${dirs})  
    install(DIRECTORY ${d} DESTINATION include 
                           PATTERN ".svn" EXCLUDE
                           REGEX "LinkDef" EXCLUDE )
  endforeach()
endfunction()

#---------------------------------------------------------------------------------------------------
#---ROOT_STANDARD_LIBRARY_PACKAGE(libname DEPENDENCIES lib1 lib2)
#---------------------------------------------------------------------------------------------------
function(ROOT_STANDARD_LIBRARY_PACKAGE libname)
  PARSE_ARGUMENTS(ARG "DEPENDENCIES" "" ${ARGN})
  ROOT_GENERATE_DICTIONARY(G__${libname} *.h LINKDEF LinkDef.h)
  ROOT_GENERATE_ROOTMAP(${libname} LINKDEF LinkDef.h DEPENDENCIES ${ARG_DEPENDENCIES})
  ROOT_LINKER_LIBRARY(${libname} *.cxx G__${libname}.cxx LIBRARIES ${ARG_DEPENDENCIES})
  ROOT_INSTALL_HEADERS()
endfunction()


#---------------------------------------------------------------------------------------------------
#---ROOT_EXECUTABLE( <name> source1 source2 ... LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(ROOT_EXECUTABLE executable)
  PARSE_ARGUMENTS(ARG "LIBRARIES" "" ${ARGN})
  set(exe_srcs)
  foreach( fp ${ARG_DEFAULT_ARGS})  
    file(GLOB files src/${fp})
    if(files) 
      set( exe_srcs ${exe_srcs} ${files})
    else()
      set( exe_srcs ${exe_srcs} ${fp})
    endif()
  endforeach()
  include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/inc ${CMAKE_BINARY_DIR}/include )
  add_executable( ${executable} ${exe_srcs})
  target_link_libraries(${executable} ${ARG_LIBRARIES} )
  #set_target_properties(${executable} PROPERTIES SUFFIX .exe)
  #----Installation details-------------------------------------------------------
  install(TARGETS ${executable} RUNTIME DESTINATION ${bin})
endfunction()



#---------------------------------------------------------------------------------------------------
#---REFLEX_BUILD_DICTIONARY( dictionary headerfiles selectionfile OPTIONS opt1 opt2 ...  LIBRARIES lib1 lib2 ... )
#---------------------------------------------------------------------------------------------------
function(REFLEX_BUILD_DICTIONARY dictionary headerfiles selectionfile )
  PARSE_ARGUMENTS(ARG "LIBRARIES;OPTIONS" "" ${ARGN})
  REFLEX_GENERATE_DICTIONARY(${dictionary} ${headerfiles} ${selectionfile} OPTIONS ${ARG_OPTIONS})
  add_library(${dictionary}Dict MODULE ${gensrcdict})
  target_link_libraries(${dictionary}Dict ${ARG_LIBRARIES} ${ROOT_Reflex_LIBRARY})
  #----Installation details-------------------------------------------------------
  install(TARGETS ${dictionary}Dict LIBRARY DESTINATION ${lib})
  set(mergedRootMap ${CMAKE_INSTALL_PREFIX}/${lib}/${CMAKE_PROJECT_NAME}Dict.rootmap)
  set(srcRootMap ${CMAKE_CURRENT_BINARY_DIR}/${rootmapname})
  install(CODE "EXECUTE_PROCESS(COMMAND ${merge_rootmap_cmd} --do-merge --input-file ${srcRootMap} --merged-file ${mergedRootMap})")
endfunction()

#---------------------------------------------------------------------------------------------------
#---SET_RUNTIME_PATH( var [LD_LIBRARY_PATH | PATH] )
#---------------------------------------------------------------------------------------------------
function( SET_RUNTIME_PATH var pathname)
  set( dirs ${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR})
  get_property(found_packages GLOBAL PROPERTY PACKAGES_FOUND)
  get_property(found_projects GLOBAL PROPERTY PROJECTS_FOUND)
  foreach( package ${found_projects} ${found_packages} )
     foreach( env ${${package}_environment})
         if(env MATCHES "^${pathname}[+]=.*")
            string(REGEX REPLACE "^${pathname}[+]=(.+)" "\\1"  val ${env})
            set(dirs ${dirs} ${val})
         endif()
     endforeach()
  endforeach()
  if(WIN32)
    string(REPLACE ";" "[:]" dirs "${dirs}")
  else()
    string(REPLACE ";" ":" dirs "${dirs}")
  endif()
  set(${var} "${dirs}" PARENT_SCOPE)
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_GENERATE_ROOTMAP( library )
#---------------------------------------------------------------------------------------------------
function(GAUDI_GENERATE_ROOTMAP library)
  find_package(ROOT)
  set(rootmapfile ${library}.rootmap)
  if(WIN32)
    set(fulllibname ${library})
    SET_RUNTIME_PATH(path PATH)
    set(genmap_command ${cmdwrap_cmd} ${path} ${ROOT_genmap_cmd} )
  else()
    set(fulllibname lib${library}.so)
    SET_RUNTIME_PATH(path LD_LIBRARY_PATH)
    set(genmap_command ${ld_library_path}=.:${path}:$ENV{${ld_library_path}} ${ROOT_genmap_cmd} )
  endif()

  add_custom_command( OUTPUT ${rootmapfile}
                      COMMAND ${genmap_command} -i ${fulllibname} -o ${rootmapfile} 
                      DEPENDS ${library} )
  add_custom_target( ${library}Rootmap ALL DEPENDS  ${rootmapfile})
  #----Installation details-------------------------------------------------------
  set(mergedRootMap ${CMAKE_INSTALL_PREFIX}/${lib}/${CMAKE_PROJECT_NAME}.rootmap)
  set(srcRootMap ${CMAKE_CURRENT_BINARY_DIR}/${library}.rootmap)
  install(CODE "EXECUTE_PROCESS(COMMAND ${merge_rootmap_cmd} --do-merge --input-file ${srcRootMap} --merged-file ${mergedRootMap})")
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_GENERATE_CONFIGURATION( library )
#---------------------------------------------------------------------------------------------------
function(GAUDI_GENERATE_CONFIGURATION library)
  get_filename_component(package ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  set(library_preload)  # TODO....
  set(outdir ${CMAKE_CURRENT_BINARY_DIR}/genConf/${package})
  file(MAKE_DIRECTORY ${outdir})
  set(confModuleName GaudiKernel.Proxy)
  set(confDefaultName Configurable.DefaultName)
  set(confAlgorithm ConfigurableAlgorithm)
  set(confAlgTool ConfigurableAlgTool)
  set(confAuditor ConfigurableAuditor)
  set(confService ConfigurableService)
  if( TARGET GaudiSvc)
	  set(GaudiSvc_dependency GaudiSvc)
  endif()
  if(WIN32)
    SET_RUNTIME_PATH(path PATH)
    set(genconf_command ${cmdwrap_cmd} ${path} ${genconf_cmd} )
  else()
    SET_RUNTIME_PATH(path LD_LIBRARY_PATH)  
    set(genconf_command ${ld_library_path}=.:${path}:$ENV{${ld_library_path}} ${genconf_cmd} )
  endif()
  add_custom_command( 
    OUTPUT ${outdir}/${library}_confDb.py
		COMMAND ${genconf_command} ${library_preload} -o ${outdir} -p ${package} 
				--configurable-module=${confModuleName}
				--configurable-default-name=${confDefaultName}
				--configurable-algorithm=${confAlgorithm}
				--configurable-algtool=${confAlgTool}
				--configurable-auditor=${confAuditor}
				--configurable-service=${confService}
				-i lib${library}.so
		DEPENDS ${library} ${GaudiSvc_dependency} )
  add_custom_target( ${library}Conf ALL DEPENDS  ${outdir}/${library}_confDb.py )
  #----Installation details-------------------------------------------------------
  set(mergedConf ${CMAKE_INSTALL_PREFIX}/python/${CMAKE_PROJECT_NAME}_merged_confDb.py)
  set(srcConf ${outdir}/${library}_confDb.py)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/genConf/ DESTINATION python PATTERN "*.stamp" EXCLUDE PATTERN "*.pyc" EXCLUDE )
  install(CODE "EXECUTE_PROCESS(COMMAND ${merge_conf_cmd} --do-merge --input-file ${srcConf} --merged-file ${mergedConf})")
  GAUDI_INSTALL_PYTHON_INIT()  
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_LINKER_LIBRARY( <name> source1 source2 ... LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_LINKER_LIBRARY library)
  PARSE_ARGUMENTS(ARG "LIBRARIES" "" ${ARGN})
  set(lib_srcs)
  foreach( fp ${ARG_DEFAULT_ARGS})
    file(GLOB files src/${fp})
    if(files) 
      set( lib_srcs ${lib_srcs} ${files})
    else()
      set( lib_srcs ${lib_srcs} ${fp})
    endif()
  endforeach()
  add_library( ${library} ${lib_srcs})
  set_target_properties(${library} PROPERTIES 
         COMPILE_FLAGS -DGAUDI_LINKER_LIBRARY )
  target_link_libraries(${library} ${ARG_LIBRARIES})
  if(TARGET ${library}Obj2doth)
    add_dependencies( ${library} ${library}Obj2doth) 
  endif()
  #----Installation details-------------------------------------------------------
  install(TARGETS ${library} EXPORT ${CMAKE_PROJECT_NAME}Exports DESTINATION  ${lib})
  install(EXPORT ${CMAKE_PROJECT_NAME}Exports DESTINATION cmake) 

endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_COMPONENT_LIBRARY( <name> source1 source2 ... LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_COMPONENT_LIBRARY library)
  PARSE_ARGUMENTS(ARG "LIBRARIES" "" ${ARGN})
  set(lib_srcs)
  foreach( fp ${ARG_DEFAULT_ARGS})  
    file(GLOB files src/${fp})
    if(files) 
      set( lib_srcs ${lib_srcs} ${files})
    else()
      set( lib_srcs ${lib_srcs} ${fp})
    endif()
  endforeach()
  add_library( ${library} MODULE ${lib_srcs})
  GAUDI_GENERATE_ROOTMAP(${library})
  GAUDI_GENERATE_CONFIGURATION(${library})
  target_link_libraries(${library} ${ROOT_Reflex_LIBRARY} ${ARG_LIBRARIES})
  #----Installation details-------------------------------------------------------
  install(TARGETS ${library} LIBRARY DESTINATION ${lib})
endfunction()


#---------------------------------------------------------------------------------------------------
#---GAUDI_PYTHON_MODULE( <name> source1 source2 ... LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_PYTHON_MODULE module)
  PARSE_ARGUMENTS(ARG "LIBRARIES" "" ${ARGN})
  set(lib_srcs)
  foreach( fp ${ARG_DEFAULT_ARGS})  
    file(GLOB files src/${fp})
    if(files) 
      set( lib_srcs ${lib_srcs} ${files})
    else()
      set( lib_srcs ${lib_srcs} ${fp})
    endif()
  endforeach()
  add_library( ${module} MODULE ${lib_srcs})
  if(win32)
    set_target_properties( ${module} PROPERTIES SUFFIX .pyd PREFIX "")
  else()
    set_target_properties( ${module} PROPERTIES SUFFIX .so PREFIX "")
  endif() 
  target_link_libraries(${module} ${Python_LIBRARIES} ${ARG_LIBRARIES})
  #----Installation details-------------------------------------------------------
  install(TARGETS ${module} LIBRARY DESTINATION python-bin)
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_EXECUTABLE( <name> source1 source2 ... LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_EXECUTABLE executable)
  PARSE_ARGUMENTS(ARG "LIBRARIES" "" ${ARGN})
  set(exe_srcs)
  foreach( fp ${ARG_DEFAULT_ARGS})  
    file(GLOB files src/${fp})
    if(files) 
      set( exe_srcs ${exe_srcs} ${files})
    else()
      set( exe_srcs ${exe_srcs} ${fp})
    endif()
  endforeach()
  add_executable( ${executable} ${exe_srcs})
  target_link_libraries(${executable} ${ARG_LIBRARIES} )
  set_target_properties(${executable} PROPERTIES SUFFIX .exe)
  #----Installation details-------------------------------------------------------
  install(TARGETS ${executable} RUNTIME DESTINATION ${bin})
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_UNIT_TEST( <name> source1 source2 ... LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_UNIT_TEST executable)
  PARSE_ARGUMENTS(ARG "LIBRARIES" "" ${ARGN})
  set(exe_srcs)
  foreach( fp ${ARG_DEFAULT_ARGS})  
    file(GLOB files src/${fp})
    if(files) 
      set( exe_srcs ${exe_srcs} ${files})
    else()
      set( exe_srcs ${exe_srcs} ${fp})
    endif()
  endforeach()
  if(BUILD_TESTS)
    add_executable( ${executable} ${exe_srcs})
    target_link_libraries(${executable} ${ARG_LIBRARIES} )
	if(WIN32)
	  SET_RUNTIME_PATH(path PATH)
      add_test(${executable} ${cmdwrap_cmd} ${path} ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/${executable}.exe )
    else()
      add_test(${executable} ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/${executable}.exe)
	endif()
    #----Installation details-------------------------------------------------------
    set_target_properties(${executable} PROPERTIES SUFFIX .exe)
    install(TARGETS ${executable} RUNTIME DESTINATION ${bin})
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_FRAMEWORK_TEST( <name> conf1 conf2 ... ENVIRONMENT env=val ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_FRAMEWORK_TEST name)
  if(BUILD_TESTS)
    PARSE_ARGUMENTS(ARG "ENVIRONMENT" "" ${ARGN})
    foreach( optfile  ${ARG_DEFAULT_ARGS} )
      if( IS_ABSOLUTE ${optfile}) 
        set( optfiles ${optfiles} ${optfile})
      else() 
        set( optfiles ${optfiles} ${CMAKE_CURRENT_SOURCE_DIR}/${optfile}) 
      endif()
    endforeach()
    add_test(${name} ${CMAKE_INSTALL_PREFIX}/scripts/testwrap${ssuffix} ${CMAKE_INSTALL_PREFIX}/setup${ssuffix} "." ${gaudirun} ${optfiles})
    set_property(TEST ${name} PROPERTY ENVIRONMENT 
      LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}:$ENV{LD_LIBRARY_PATH}
      ${ARG_ENVIRONMENT})
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_QMTEST_TEST( <name> TESTS qmtest1 qmtest2 ... ENVIRONMENT env=val ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_QMTEST_TEST name)
  if(BUILD_TESTS)
    PARSE_ARGUMENTS(ARG "TESTS;ENVIRONMENT" "" ${ARGN})
    foreach(arg ${ARG_TESTS})
	  set(tests ${tests} ${arg})
    endforeach()
    if( NOT tests )
      set(tests ${name})
    endif()
    GAUDI_USE_PACKAGE(QMtest)
    add_test(${name} ${CMAKE_INSTALL_PREFIX}/scripts/testwrap${ssuffix} ${CMAKE_INSTALL_PREFIX}/setup${ssuffix} 
                     ${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest 
                     qmtest run ${tests})
    set_property(TEST ${name} PROPERTY ENVIRONMENT 
      LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}:$ENV{LD_LIBRARY_PATH}
      QMTEST_CLASS_PATH=${CMAKE_SOURCE_DIR}/GaudiPolicy/qmtest_classes
      ${ARG_ENVIRONMENT})
    install(CODE "if( NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest/QMTest) 
                    execute_process(COMMAND  ${CMAKE_INSTALL_PREFIX}/scripts/testwrap${ssuffix}  
                                             ${CMAKE_INSTALL_PREFIX}/setup${ssuffix}
                                             ${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest 
                                             qmtest create-tdb )
                  endif()")
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_HEADERS([dir1 dir2 ...])
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_HEADERS)
  if( ARGN )
    set( dirs ${ARGN} )
  else()
    get_filename_component(dirs ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  endif()
  foreach( inc ${dirs})  
    install(DIRECTORY ${inc} DESTINATION include PATTERN ".svn" EXCLUDE )
  endforeach()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_PYTHON_MODULES( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_PYTHON_MODULES)  
  get_filename_component(package ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  install(DIRECTORY python/ DESTINATION python 
          PATTERN ".svn" EXCLUDE
          PATTERN "*.pyc" EXCLUDE )
  GAUDI_INSTALL_PYTHON_INIT()
endfunction()


#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_PYTHON_INIT( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_PYTHON_INIT)    
  get_filename_component(package ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  install(CODE "if (NOT EXISTS \"${CMAKE_INSTALL_PREFIX}/python/${package}/__init__.py\")
                  file(INSTALL DESTINATION \"${CMAKE_INSTALL_PREFIX}/python/${package}\"
                               TYPE FILE 
                               FILES \"${GAUDI_SOURCE_DIR}/GaudiPolicy/cmt/fragments/__init__.py\"  )
                endif()" )
  GAUDI_ZIP_PYTHON_MODULES()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_ZIP_PYTHON_MODULES( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_ZIP_PYTHON_MODULES)
  install(CODE "execute_process(COMMAND  ${zippythondir_cmd} --quiet ${CMAKE_INSTALL_PREFIX}/python)")  
endfunction()    

#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_SCRIPTS( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_SCRIPTS)
  install(DIRECTORY scripts/ DESTINATION scripts 
          FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                           GROUP_EXECUTE GROUP_READ 
          PATTERN ".svn" EXCLUDE
          PATTERN "*.pyc" EXCLUDE )
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_JOBOPTIONS( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_JOBOPTIONS)
  get_filename_component(package ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  install(FILES ${ARGN} DESTINATION jobOptions/${package}) 
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_PROJECT_VERSION_HEADER( )
#---------------------------------------------------------------------------------------------------
function( GAUDI_PROJECT_VERSION_HEADER )
  set(project ${CMAKE_PROJECT_NAME})
  set(version ${${CMAKE_PROJECT_NAME}_VERSION})
  set(ProjectVersionHeader_output  ${CMAKE_INSTALL_PREFIX}/include/${project}_VERSION.h)
  add_custom_target( ${project}VersionHeader ALL
                     ${versheader_cmd} ${project} ${version} ${ProjectVersionHeader_output} )
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_USE_PACKAGE( package )
#---------------------------------------------------------------------------------------------------
macro( GAUDI_USE_PACKAGE package )
  if( EXISTS ${CMAKE_SOURCE_DIR}/${package}/CMakeLists.txt)
    include_directories( ${CMAKE_SOURCE_DIR}/${package} ) 
    file(READ ${CMAKE_SOURCE_DIR}/${package}/CMakeLists.txt file_contents)
    string( REGEX MATCHALL "GAUDI_USE_PACKAGE[ ]*[(][ ]*([^ )])+" vars ${file_contents})
    foreach( var ${vars})
      string(REGEX REPLACE "GAUDI_USE_PACKAGE[ ]*[(][ ]*([^ )])" "\\1" p ${var})
      GAUDI_USE_PACKAGE(${p})
    endforeach()
  else()
    find_package(${package})
    GET_PROPERTY(parent DIRECTORY PROPERTY PARENT_DIRECTORY)
    if(parent)
      set(${package}_environment  ${${package}_environment} PARENT_SCOPE)
    else()
      set(${package}_environment  ${${package}_environment} )
    endif()
    include_directories( ${${package}_INCLUDE_DIRS} ) 
    link_directories( ${${package}_LIBRARY_DIRS} ) 
  endif()
endmacro()

#---------------------------------------------------------------------------------------------------
#---GAUDI_BUILD_SETUP( )
#---------------------------------------------------------------------------------------------------
function( GAUDI_BUILD_SETUP )
  GAUDI_BUILD_PROJECT_SETUP()
  get_property(found_packages GLOBAL PROPERTY PACKAGES_FOUND)
  message("Found packages = ${found_packages}")
  foreach( package ${found_packages} )
    GAUDI_BUILD_PACKAGE_SETUP( "" ${package} "${${package}_environment}")
  endforeach()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_BUILD_PROJECT_SETUP( )
#---------------------------------------------------------------------------------------------------
function( GAUDI_BUILD_PROJECT_SETUP )
  set( setup  ${CMAKE_BINARY_DIR}/setup${ssuffix} )
  file(WRITE  ${setup} "${scomment} ${CMAKE_PROJECT_NAME} Setup file\n")
  if(WIN32)
	file(APPEND ${setup} "@echo off\n")
    file(APPEND ${setup} "set PATH=${CMAKE_INSTALL_PREFIX}/${bin};${CMAKE_INSTALL_PREFIX}/${lib};${CMAKE_INSTALL_PREFIX}/scripts;%PATH%\n")
    file(APPEND ${setup} "set PYTHONPATH=${CMAKE_INSTALL_PREFIX}/python;%PYTHONPATH%\n")
  else()
    file(APPEND ${setup} "setenv PATH  ${CMAKE_INSTALL_PREFIX}/${bin}:${CMAKE_INSTALL_PREFIX}/scripts:\${PATH}\n")
    file(APPEND ${setup} "setenv LD_LIBRARY_PATH  ${CMAKE_INSTALL_PREFIX}/${lib}:\${LD_LIBRARY_PATH}\n")
    file(APPEND ${setup} "setenv PYTHONPATH  ${CMAKE_INSTALL_PREFIX}/python:\${PYTHONPATH}\n")
  endif()

  #----Get the setup fro each external package
  get_property(found_packages GLOBAL PROPERTY PACKAGES_FOUND)
  get_property(found_projects GLOBAL PROPERTY PROJECTS_FOUND)
  foreach( package ${found_projects} ${found_packages} )
    GAUDI_BUILD_PACKAGE_SETUP( ${setup} ${package} "${${package}_environment}")
  endforeach()

  #---Get the setup for each package (directory)
  file(APPEND  ${setup} "\n${scomment} Standard variables for each package\n")
  file(GLOB_RECURSE cmakelist_files  ${CMAKE_SOURCE_DIR} CMakeLists.txt)
  foreach( file ${cmakelist_files} )
    GET_FILENAME_COMPONENT(path ${file} PATH)
    if (NOT path STREQUAL ${CMAKE_SOURCE_DIR})
      GET_FILENAME_COMPONENT(directory ${path} NAME)
      string(TOUPPER ${directory} DIRECTORY)
      set( ${directory}_environment ${${directory}_environment} ${DIRECTORY}ROOT=${path})
      GAUDI_BUILD_PACKAGE_SETUP( ${setup} ${directory} "${${directory}_environment}")
    endif()
  endforeach()
  #---Installation---------------------------------------------------------------------------------
  install(FILES ${setup}  DESTINATION . 
                          PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ 
                                      GROUP_EXECUTE GROUP_READ 
                                      WORLD_EXECUTE WORLD_READ )
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_BUILD_PACKAGE_SETUP( setupfile package envlist )
#---------------------------------------------------------------------------------------------------
function( GAUDI_BUILD_PACKAGE_SETUP setup package envlist )
  if ( NOT setup )
    set( setup ${CMAKE_INSTALL_PREFIX}/${package}_setup.csh )
    file(WRITE  ${setup} "${scomment} Package ${package} setup file\n")
  else()
    file(APPEND  ${setup} "\n${scomment} Package ${package} setup file\n")
  endif()
  foreach( env ${envlist} )
    if(env MATCHES ".*[+]=.*")
      string(REGEX REPLACE "([^=+]+)[+]=.*" "\\1" var ${env})
      string(REGEX REPLACE ".*[+]=(.+)" "\\1"  val ${env})
	  if(WIN32)
        file(APPEND ${setup} "set ${var}=${val};%${var}%\n")
	  else()
        file(APPEND ${setup} "if \$?${var} then\n")
        file(APPEND ${setup} "  setenv ${var} ${val}:\${${var}}\n")
        file(APPEND ${setup} "else\n")
        file(APPEND ${setup} "  setenv ${var} ${val}\n")
        file(APPEND ${setup} "endif\n")      
	  endif()
    elseif ( env MATCHES ".*=.*")
      string(REGEX REPLACE "([^=+]+)=.*" "\\1" var ${env})
      string(REGEX REPLACE ".*=(.+)" "\\1"  val ${env})
	  if(WIN32)
        file(APPEND ${setup} "set ${var}=${val}\n")
	  else()
        file(APPEND ${setup} "setenv ${var} ${val}\n")
	  endif()
   endif() 
  endforeach()
endfunction()
