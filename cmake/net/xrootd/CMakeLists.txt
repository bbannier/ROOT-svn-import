############################################################################
# CMakeLists.txt file for building ROOT net/xrootd package
# @author Pere Mato, CERN
############################################################################

set(xrootddir ${CMAKE_CURRENT_SOURCE_DIR}/src/xrootd)
#---Xrootd headers, sources, config dependences -----------------------------
file(GLOB_RECURSE xrootdsrcdep ${xrootddir}/src/*.hh
                               ${xrootddir}/src/*.cc
                               ${xrootddir}/src/*.c
                               ${xrootddir}/src/*.h )

file(GLOB xrootdconfdep ${xrootddir}/config/*
                        ${xrootddir}/config/test/*
                        ${xrootddir}/configure.classic)
set(xrootdmake  ${xrootddir}/GNUmakefile)

#---We need to know the final list of libraries and executables (targets)----
set(xrootdexes  xrootd xrdcp xrd xrdpwdadmin cmsd xrdstagetool xprep)
set(xrootdlibs  libXrdBwm libXrdCrypto libXrdRootd libXrdSeckrb5	libXrdSecssl libXrdSecunix
                libXrdClient libXrdCryptossl libXrdSec libXrdSecpwd libXrdSecsss libXrdSut)
foreach(lib ${xrootdlibs})
  set(makelibs ${makelibs} ${xrootddir}/lib/${lib}.${SOEXT})
  set(finallibs ${finallibs} ${LIBRARY_OUTPUT_PATH}/${lib}.${SOEXT})
endforeach()
foreach(bin ${xrootdexes})
  set(makeexes ${makeexes} ${xrootddir}/bin/${bin})
  set(finalexes ${finalexes} ${EXECUTABLE_OUTPUT_PATH}/${bin})
endforeach()

add_custom_target(XROOTD_LOCAL_TARGET ALL DEPENDS ${finalexes} ${finallibs})

if(NOT WIN32)
  execute_process(COMMAND uname -r OUTPUT_VARIABLE RELE)
  execute_process(COMMAND -m | tr '[A-Z]' '[a-z]' OUTPUT_VARIABLE CHIP)
  execute_process(COMMAND uname -p OUTPUT_VARIABLE PROC)
  if(ROOT_ARCHITECTURE MATCHES freebsd)
    set(xopt --ccflavour=gcc)
  elseif(ROOT_ARCHITECTURE MATCHES linuxicc)
    set(xopt --ccflavour=icc --use-xrd-strlcpy)
  elseif(ROOT_ARCHITECTURE MATCHES linuxia64ecc)
    set(xopt --ccflavour=icc --use-xrd-strlcpy)
  elseif(ROOT_ARCHITECTURE MATCHES linuxia64gcc)
    set(xopt --ccflavour=gccia64 --use-xrd-strlcpy)
  elseif(ROOT_ARCHITECTURE MATCHES linuxx8664gcc)
    set(xopt --ccflavour=gccx8664 --use-xrd-strlcpy)
  elseif(ROOT_ARCHITECTURE MATCHES linuxx8664icc)
    set(xopt --ccflavour=iccx8664 --use-xrd-strlcpy)
  elseif(ROOT_ARCHITECTURE MATCHES linuxppc64gcc)
    set(xopt --ccflavour=gccppc64 --use-xrd-strlcpy)
  elseif(ROOT_ARCHITECTURE MATCHES linuxalphagcc)
    set(xopt --ccflavour=gcc --use-xrd-strlcpy)
  elseif(ROOT_ARCHITECTURE MATCHES linux)
    set(xarch i386_linux)
    set(xopt --ccflavour=gcc --use-xrd-strlcpy)
  elseif(ROOT_ARCHITECTURE MATCHES macosx64)
    set(xopt --ccflavour=macos64)
  elseif(ROOT_ARCHITECTURE MATCHES macosxicc)
    set(xopt --ccflavour=icc)
  elseif(ROOT_ARCHITECTURE MATCHES macosx)
    set(xopt --ccflavour=macos)
  elseif(ROOT_ARCHITECTURE MATCHES solaris64 AND CHIP MATCHES i86pc)
    set(xopt --ccflavour=sunCCamd64 --use-xrd-strlcpy)
  elseif(ROOT_ARCHITECTURE MATCHES solaris AND RELE MATCHES 5.11 AND CHIP MATCHES i86pc)
    set(xopt --ccflavour=sunCCi86pc --use-xrd-strlcpy)
  elseif(ROOT_ARCHITECTURE MATCHES solaris AND RELE MATCHES 5.1 AND CHIP MATCHES i86pc)
    set(xopt --use-xrd-strlcpy)
  elseif(ROOT_ARCHITECTURE MATCHES solaris AND CHIP MATCHES i86pc)
    set(xopt --ccflavour=sunCCi86pc --use-xrd-strlcpy)
  elseif(ROOT_ARCHITECTURE MATCHES solarisgcc AND RELE MATCHES 5.8)
    set(xopt --ccflavour=gcc)
  elseif(ROOT_ARCHITECTURE MATCHES solaris AND RELE MATCHES 5.8)
    set(xopt --ccflavour=sunCC)
  elseif(ROOT_ARCHITECTURE MATCHES solarisgcc AND RELE MATCHES 5.9)
    set(xopt --ccflavour=gcc)
  elseif(ROOT_ARCHITECTURE MATCHES solaris AND RELE MATCHES 5.9)
    set(xopt --ccflavour=sunCC)
  elseif(ROOT_ARCHITECTURE MATCHES solarisgcc)
    set(xopt --ccflavour=gcc --use-xrd-strlcpy)
  elseif(ROOT_ARCHITECTURE MATCHES solaris)
    set(xopt --ccflavour=sunCC --use-xrd-strlcpy)
  elseif(ROOT_ARCHITECTURE MATCHES win32gcc)
    set(xopt win32gcc)
  else()
    set(xopt)
  endif()
  #---Kerberos 5 options----------------------------------------------------------------------
  if(KRB5_LIBRARIES)
    foreach(l ${KRB5_LIBRARIES})
      get_filename_component(xlib ${l} PATH)
      break()
    endforeach()
    set(xopt ${xopt} --with-krb5-libdir=${xlib} --with-krb5-incdir=${KRB5_INCLUDE_DIR})
  endif()
  if(NOT BUILDXRDGSI)
    set(xopt ${xopt} --disable-gsi)    
  endif()
  if(bonjour)
    set(xopt ${xopt} --enable-bonjour)        
  endif()
  #---OpenSSL options----------------------------------------------------------------------
  if(OPENSSL_LIBRARIES)
    foreach(l ${OPENSSL_LIBRARIES})
      get_filename_component(xlib ${l} PATH)
      get_filename_component(ext ${l} EXT)
      break()
    endforeach()
    set(xopt ${xopt} --with-ssl-libdir=${xlib} --with-ssl-incdir=${OPENSSL_INCLUDE_DIR})
    if(ext MATCHES so)
      set(xopt ${xopt} --with-ssl-shared=yes)
    endif()
  endif()
  #---shadowpw option-----------------------------------------------------------------------
  if(shadowpw)
      set(xopt ${xopt} --enable-shadowpw)    
  endif()
  #---AFS lib and options---------------------------------------------------------------------------
  if(AFS_LIBRARIES)
    foreach(l ${AFS_LIBRARIES})
      get_filename_component(xlib ${l} PATH)
      get_filename_component(ext ${l} EXT)
      break()
    endforeach()
    set(xopt ${xopt} --with-afs-libdir=${xlib} --with-afs-incdir=${AFS_INCLUDE_DIR})
    if(ext MATCHES so)
      set(xopt ${xopt} --with-afs-shared=yes)
    endif()
  endif()
  #---Extra options----------------------------------------------------------------------------
  if(xrootd-opts)
    set(xopt ${xopt} ${xrootd-opts})
  endif()
	set( xopt ${xopt} --disable-krb4 --enable-echo --no-arch-subdirs --disable-mon 
	                  --with-cxx=${CMAKE_CXX_COMPILER} --with-ld=${CMAKE_CXX_COMPILER})
	message(STATUS "Options to Xrootd-configure: ${xarch} ${xopt}")
	
	add_custom_command(OUTPUT ${xrootdmake}
                     COMMAND ./configure.classic ${xarch} ${xopt}
                     WORKING_DIRECTORY ${xrootddir}
                     DEPENDS ${xrootdconfdep})
else(NOT WIN32)
  #----TODO-----------
endif(NOT WIN32)  

if(NOT WIN32)
	add_custom_command(OUTPUT ${makelibs} ${makeexes}
	                   COMMAND echo "Building xrootd with make ..."
                     COMMAND make
                     WORKING_DIRECTORY ${xrootddir}
                     DEPENDS ${xrootdmake} ${xrootdsrcdep})
else()
  
endif()

#---Copy the binaries and libraries to destination----------------------------------------------
foreach(lib ${xrootdlibs})
  add_custom_command(OUTPUT ${LIBRARY_OUTPUT_PATH}/${lib}.${SOEXT}
                     COMMAND cmake -E copy_if_different ${xrootddir}/lib/${lib}.${SOEXT} ${LIBRARY_OUTPUT_PATH}
                     DEPENDS ${xrootddir}/lib/${lib}.${SOEXT} )
endforeach()
foreach(bin ${xrootdexes})
  add_custom_command(OUTPUT ${EXECUTABLE_OUTPUT_PATH}/${bin}
                     COMMAND cmake -E copy_if_different ${xrootddir}/bin/${bin} ${EXECUTABLE_OUTPUT_PATH}
                     DEPENDS ${xrootddir}/bin/${bin} )
endforeach()

#---Installation--------------------------------------------------------------------------------
install(FILES ${finallibs} DESTINATION lib)
install(PROGRAMS ${finalexes} DESTINATION bin)





