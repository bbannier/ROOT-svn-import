# TODO: Check if we have to install the buildin freetype version
# or if we can use the system version of frretype. 
# Up to now we don't check and install the buildin version anyway.
##is_root_install_option_enabled(builtin_f)
##if(option_builtin_pcre_is_enabled)
###  MESSAGE("BLA: ${option_builtin_pcre_is_enabled}")
###  add_subdirectory (core/pcre)
##endif(option_builtin_pcre_is_enabled)

# This is not a verry clean solution, but the problem is that the pcre has its
# own tarfile and its own buildsystem. So we have to unpack the tarfile and
# then call the build system of pcre. The created library is imported into
# the scope of cmake, so even make clean works.

# Define package related variables 
set(FREETYPE_SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(FREETYPE_VERSION "freetype-2.3.12")
set(FREETYPE_VERSION ${FREETYPE_VERSION} PARENT_SCOPE)
set(FREETYPE_TARGZFILE ${FREETYPE_SRCDIR}/${FREETYPE_VERSION}.tar.gz)
set(FREETYPE_TARFILE ${FREETYPE_SRCDIR}/${FREETYPE_VERSION}.tar)

# Define special compiler settings for freetype
set(FREETYPE_CC ${CMAKE_C_COMPILER})
set(FREETYPE_CFLAGS "-O")
if(${ROOT_ARCHITECTURE} MATCHES alphacxx6)
  set(FREETYPE_CC "cc")  
endif(${ROOT_ARCHITECTURE} MATCHES alphacxx6)
if(${ROOT_ARCHITECTURE} MATCHES linux)
  set(FREETYPE_CFLAGS "-m32 -O")
endif(${ROOT_ARCHITECTURE} MATCHES linux)
if(${ROOT_ARCHITECTURE} MATCHES linuxx8664gcc)
  set(FREETYPE_CFLAGS "-m64 -O")
endif(${ROOT_ARCHITECTURE} MATCHES linuxx8664gcc)
if(${ROOT_ARCHITECTURE} MATCHES linuxicc)
  set(FREETYPE_CFLAGS "-m32 -O")
endif(${ROOT_ARCHITECTURE} MATCHES linuxicc)
if(${ROOT_ARCHITECTURE} MATCHES linuxx8664icc)
  set(FREETYPE_CFLAGS "-m64 -O")
endif(${ROOT_ARCHITECTURE} MATCHES linuxx8664icc)
if(${ROOT_ARCHITECTURE} MATCHES macosx)
  set(FREETYPE_CFLAGS "-m32 -O")
endif(${ROOT_ARCHITECTURE} MATCHES macosx)
if(${ROOT_ARCHITECTURE} MATCHES macosx64)
  set(FREETYPE_CFLAGS "-m64 -O")
endif(${ROOT_ARCHITECTURE} MATCHES macosx64)
if(${ROOT_ARCHITECTURE} MATCHES solaris64CC5)
  set(FREETYPE_CFLAGS "-m64 -O")
endif(${ROOT_ARCHITECTURE} MATCHES solaris64CC5)

if(${ROOT_ARCHITECTURE} MATCHES sgicc64)
  set(FREETYPE_CC "cc")  
  set(FREETYPE_CFLAGS "-64 -O")
endif(${ROOT_ARCHITECTURE} MATCHES sgicc64)
if(${ROOT_ARCHITECTURE} MATCHES linuxppc64gcc)
#  set(FREETYPE_CC "cc")  
  set(FREETYPE_CFLAGS "-m64 -O")
endif(${ROOT_ARCHITECTURE} MATCHES linuxppc64gcc)
if(${ROOT_ARCHITECTURE} MATCHES hpuxia64acc)
  set(FREETYPE_CC "cc")  
  set(FREETYPE_CFLAGS "+DD64 -Ae -O +W863")
endif(${ROOT_ARCHITECTURE} MATCHES hpuxia64acc)
if(${ROOT_ARCHITECTURE} MATCHES aix5)
  Set(FREETYPE_ZLIB "--without-zlib")
endif(${ROOT_ARCHITECTURE} MATCHES aix5)
if(${ROOT_ARCHITECTURE} MATCHES aixgcc)
  Set(FREETYPE_ZLIB "--without-zlib")
endif(${ROOT_ARCHITECTURE} MATCHES aixgcc)

# copy tarfile from source directory to build directory, unpack the tarfile
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${FREETYPE_VERSION}/unpacked
                   COMMAND cmake -E copy ${FREETYPE_TARGZFILE} .
                   COMMAND gunzip ${FREETYPE_VERSION}.tar.gz
                   COMMAND tar xf ${FREETYPE_VERSION}.tar
                   COMMAND touch ${FREETYPE_VERSION}/unpacked
                   COMMAND cmake -E remove -f ${FREETYPE_VERSION}.tar
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} 
                   )
# configure and make freetype, this command depends on the unpacked package 
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${FREETYPE_VERSION}/objs/.libs/libfreetype.a  
                   COMMAND CC=${FREETYPE_CC} CFLAGS=${FREETYPE_CFLAGS} ./configure --with-pic --disable-shared ${FREETYPE_ZLIB} > /dev/null 2>&1
                   COMMAND make modules all > /dev/null 2>&1
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${FREETYPE_VERSION} 
                   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${FREETYPE_VERSION}/unpacked
                  )

# copy the created library into the library directory in the build directory
# This command depends on the created libfreetype.a
if(${ROOT_PLATFORM} MATCHES macosx)
  add_custom_command(OUTPUT ${LIBRARY_OUTPUT_PATH}/libfreetype.a
                     COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/${FREETYPE_VERSION}/objs/.libs/libfreetype.a ${LIBRARY_OUTPUT_PATH}
                     COMMAND ranlib ${LIBRARY_OUTPUT_PATH}/libfreetype.a
                     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${FREETYPE_VERSION} 
                     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${FREETYPE_VERSION}/objs/.libs/libfreetype.a
                   )
else(${ROOT_PLATFORM} MATCHES macosx)
  add_custom_command(OUTPUT ${LIBRARY_OUTPUT_PATH}/libfreetype.a
                     COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/${FREETYPE_VERSION}/objs/.libs/libfreetype.a ${LIBRARY_OUTPUT_PATH}
                     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${FREETYPE_VERSION}/objs/.libs/libfreetype.a
                    )
endif(${ROOT_PLATFORM} MATCHES macosx)
                
# create a target which will always be build and does actually nothing. The target is only
# needed that the dependencies are build, f they are not up to date. If everything is up to
# dte nothing is done. This target depends on the libfreetype.a in the library directory of the 
# build directory.
add_custom_target(${FREETYPE_VERSION} ALL
                  DEPENDS ${LIBRARY_OUTPUT_PATH}/libfreetype.a
                 )

# import libfreetype.a which is not in the scope of CMake. This step makes it possible that
# make clean will remove the library  
#add_library(${LIBRARY_OUTPUT_PATH}/libfreetype.a STATIC IMPORTED)
add_library(freetype STATIC IMPORTED)
#INSTALL(TARGET ${LIBRARY_OUTPUT_PATH}/libpcre.a DESTINATION ${LIB_INSTALL_DIR})
INSTALL(FILES ${LIBRARY_OUTPUT_PATH}/libfreetype.a DESTINATION ${LIB_INSTALL_DIR})
