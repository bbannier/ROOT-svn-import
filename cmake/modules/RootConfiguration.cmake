#---Define a function to do not polute the top level namespace with unneeded variables-----------------------
function(RootConfigure)
  
#---Define all sort of variables to bridge between the old Module.mk and the new CMake equivalents-----------
set(ROOT_DICTTYPE cint)
set(ROOT_CONFIGARGS "")
set(bindir ${CMAKE_INSTALL_PREFIX}/bin)
set(libdir ${CMAKE_INSTALL_PREFIX}/lib)
set(mandir ${CMAKE_INSTALL_PREFIX}/man)

#---CINT Configuration---------------------------------------------------------------------------------------
configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/cint-config.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cint-config @ONLY)

#---Create the configcint.h header file and copy the file first to the cint/cint directory. Copy it to the 
#   temporary include directory only if there is a change. Otherwise you have to recompile unneccessarily some files 
#   depending on configcint.h all the time. 
include(WriteConfigCint)
WRITE_CONFIG_CINT( ${CMAKE_CURRENT_BINARY_DIR}/tmp/configcint.h)
execute_process(COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/tmp/configcint.h ${HEADER_OUTPUT_PATH})
install(FILES ${HEADER_OUTPUT_PATH}/configcint.h DESTINATION include)

#---RConfigure.h---------------------------------------------------------------------------------------------
configure_file(${PROJECT_SOURCE_DIR}/cmake/scripts/RConfigure.in include/RConfigure.h)
install(FILES ${CMAKE_BINARY_DIR}/include/RConfigure.h DESTINATION include)

#---Configure and install various files----------------------------------------------------------------------
execute_Process(COMMAND uname -a OUTPUT_VARIABLE BuildNodeInfo OUTPUT_STRIP_TRAILING_WHITESPACE )

configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/root-config.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/root-config @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/rootrc.in ${CMAKE_BINARY_DIR}/etc/system.rootrc @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/RConfigOptions.in include/RConfigOptions.h)
configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/compiledata.in include/compiledata.h)
configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/Makefile-comp.in config/Makefile.comp)
configure_file(${PROJECT_SOURCE_DIR}/config/mimes.unix.in ${CMAKE_BINARY_DIR}/etc/root.mimes)


configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/genreflex.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/genreflex @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/genreflex-rootcint.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/genreflex-rootcint @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/memprobe.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/memprobe @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/config/thisroot.sh ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/thisroot.sh @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/config/thisroot.csh ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/thisroot.csh @ONLY)

install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/genreflex
              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/genreflex-rootcint
              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/memprobe
              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/thisroot.sh
              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/thisroot.csh
              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/root-config
              PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ 
              DESTINATION bin)

install(FILES ${CMAKE_BINARY_DIR}/include/RConfigOptions.h
              ${CMAKE_BINARY_DIR}/include/compiledata.h 
              DESTINATION include)

install(FILES ${CMAKE_BINARY_DIR}/etc/root.mimes 
              ${CMAKE_BINARY_DIR}/etc/system.rootrc
              DESTINATION etc)

install(FILES ${CMAKE_BINARY_DIR}/config/Makefile.comp
              DESTINATION config)

endfunction()
RootConfigure()
