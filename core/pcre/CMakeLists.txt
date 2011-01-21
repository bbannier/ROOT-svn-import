# TODO: Check if we have to install the buildin pcre version
# or if we can use the system version of pcre. We have to create 
# a FindPCRE.cmake script and search for the system version of
# pcre if not set buildin version of pcre.
# Up to now we don't check and install the buildin version anyway.
##is_root_install_option_enabled(builtin_pcre)
##if(option_builtin_pcre_is_enabled)
###  MESSAGE("BLA: ${option_builtin_pcre_is_enabled}")
###  add_subdirectory (core/pcre)
##endif(option_builtin_pcre_is_enabled)

# This is not a verry clean solution, but the problem is that the pcre has its
# own tarfile and its own buildsystem. So we have to unpack the tarfile and
# then call the build system of pcre. The created library is imported into
# the scope of cmake, so even make clean works.

# Define package related variables 
set(PCRE_SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PCRE_VERSION "pcre-7.8")
set(PCRE_VERSION ${PCRE_VERSION} PARENT_SCOPE)
set(PCRE_TARGZFILE ${PCRE_SRCDIR}/${PCRE_VERSION}.tar.gz)
set(PCRE_TARFILE ${PCRE_SRCDIR}/${PCRE_VERSION}.tar)

# Define special compiler settings for prce
set(PCRE_CC ${CMAKE_C_COMPILER})
set(PCRE_CFLAGS "-O")
if(${ROOT_ARCHITECTURE} MATCHES alphacxx6)
  set(PCRE_CC "cc")  
endif(${ROOT_ARCHITECTURE} MATCHES alphacxx6)
if(${ROOT_ARCHITECTURE} MATCHES linux)
  set(PCRE_CFLAGS "-m32 -O")
endif(${ROOT_ARCHITECTURE} MATCHES linux)
if(${ROOT_ARCHITECTURE} MATCHES linuxx8664gcc)
  set(PCRE_CFLAGS "-m64 -O")
endif(${ROOT_ARCHITECTURE} MATCHES linuxx8664gcc)
if(${ROOT_ARCHITECTURE} MATCHES linuxicc)
  set(PCRE_CFLAGS "-m32 -O")
endif(${ROOT_ARCHITECTURE} MATCHES linuxicc)
if(${ROOT_ARCHITECTURE} MATCHES linuxx8664icc)
  set(PCRE_CFLAGS "-m64 -O")
endif(${ROOT_ARCHITECTURE} MATCHES linuxx8664icc)
if(${ROOT_ARCHITECTURE} MATCHES macosx)
  set(PCRE_CFLAGS "-m32 -O")
endif(${ROOT_ARCHITECTURE} MATCHES macosx)
if(${ROOT_ARCHITECTURE} MATCHES macosx64)
  set(PCRE_CFLAGS "-m64 -O")
endif(${ROOT_ARCHITECTURE} MATCHES macosx64)
if(${ROOT_ARCHITECTURE} MATCHES solaris64CC5)
  set(PCRE_CFLAGS "-m64 -O")
endif(${ROOT_ARCHITECTURE} MATCHES solaris64CC5)

if(${ROOT_ARCHITECTURE} MATCHES sgicc64)
  set(PCRE_CC "cc")  
  set(PCRE_CFLAGS "-64 -O")
endif(${ROOT_ARCHITECTURE} MATCHES sgicc64)
if(${ROOT_ARCHITECTURE} MATCHES linuxppc64gcc)
  set(PCRE_CFLAGS "-m64 -O")
endif(${ROOT_ARCHITECTURE} MATCHES linuxppc64gcc)
if(${ROOT_ARCHITECTURE} MATCHES hpuxia64acc)
  set(PCRE_CC "cc")  
  set(PCRE_CFLAGS "+DD64 -Ae -O")
endif(${ROOT_ARCHITECTURE} MATCHES hpuxia64acc)

# copy tarfile from source directory to build directory, unpack the tarfile
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PCRE_VERSION}/unpacked
                   COMMAND cmake -E copy ${PCRE_TARGZFILE} .
                   COMMAND gunzip ${PCRE_VERSION}.tar.gz
                   COMMAND tar xf ${PCRE_VERSION}.tar
                   COMMAND touch ${PCRE_VERSION}/unpacked
                   COMMAND cmake -E remove -f ${PCRE_VERSION}.tar
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} 
                   )
# configure and make pcre, this command depends on the unpacked package 
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PCRE_VERSION}/.libs/libpcre.a  
                   COMMAND CC=${PCRE_CC} CFLAGS=${PCRE_CFLAGS} ./configure --with-pic --disable-shared > /dev/null 2>& 1
                   COMMAND make libpcre.la > /dev/null 2>& 1
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PCRE_VERSION} 
                   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PCRE_VERSION}/unpacked
                  )

# copy the created library into the library directory in the build directory
# This command depends on the created libpcre.a
if(${ROOT_PLATFORM} MATCHES macosx)
  add_custom_command(OUTPUT ${LIBRARY_OUTPUT_PATH}/libpcre.a
                     COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/${PCRE_VERSION}/.libs/libpcre.a ${LIBRARY_OUTPUT_PATH}
                     COMMAND ranlib ${LIBRARY_OUTPUT_PATH}/libpcre.a
                     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PCRE_VERSION} 
                     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PCRE_VERSION}/.libs/libpcre.a
                   )
else(${ROOT_PLATFORM} MATCHES macosx)
  add_custom_command(OUTPUT ${LIBRARY_OUTPUT_PATH}/libpcre.a
                     COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/${PCRE_VERSION}/.libs/libpcre.a ${LIBRARY_OUTPUT_PATH}
                     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PCRE_VERSION}/.libs/libpcre.a
                    )
endif(${ROOT_PLATFORM} MATCHES macosx)
                
# create a target which will always be build and does actually nothing. The target is only
# needed that the dependencies are build, f they are not up to date. If everything is up to
# dte nothing is done. This target depends on the libpcre.a in the library directory of the 
# build directory.
add_custom_target(${PCRE_VERSION} 
                  DEPENDS ${LIBRARY_OUTPUT_PATH}/libpcre.a
                 )

# import libpcre.a which is not in the scope of CMake. This step makes it possible that
# make clean will remove the library  
#add_library(${LIBRARY_OUTPUT_PATH}/libpcre.a STATIC IMPORTED)
add_library(pcre STATIC IMPORTED)
#INSTALL(TARGET ${LIBRARY_OUTPUT_PATH}/libpcre.a DESTINATION ${LIB_INSTALL_DIR})
INSTALL(FILES ${LIBRARY_OUTPUT_PATH}/libpcre.a DESTINATION ${LIB_INSTALL_DIR})
