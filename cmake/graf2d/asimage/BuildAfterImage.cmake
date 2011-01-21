# TODO: Check if we have to install the buildin version of
# libAfterImage or if we can use the system version of 
# libAfterImage. We have to create a FindAfterImage.cmake 
# script and search for the system version of
# libAfterImage if not set buildin version of libAfterImage.
# Up to now we don't check and install the buildin version anyway.

# This is not a verry clean solution, but the problem is that AfterImage has its
# own tarfile and its own buildsystem. So we have to unpack the tarfile and
# then call the build system of pcre. The created library is imported into
# the scope of cmake, so even make clean works.

# Define package related variables 
set(AFTER_SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/src/libAfterImage)
set(AFTER_VERSION "AfterImage_root")
set(AFTER_VERSION ${AFTER_VERSION} PARENT_SCOPE)

# Define special compiler settings for prce
set(AFTER_CC ${CMAKE_C_COMPILER})
set(AFTER_CFLAGS "-O")
If(${CMAKE_C_COMPILER} MATCHES icc)
  Set(AFTER_CFLAGS "${AFTER_CFLAGS} -wd188 -wd869 -wd2259 -wd1418 -wd1419 -wd593 -wd981 -wd1599 -wd181 -wd177 -wd1572")
EndIf(${CMAKE_C_COMPILER} MATCHES icc)
if(${ROOT_ARCHITECTURE} MATCHES linux)
  set(AFTER_CFLAGS "-m32 -O")
endif(${ROOT_ARCHITECTURE} MATCHES linux)
if(${ROOT_ARCHITECTURE} MATCHES linuxx8664gcc)
  set(AFTER_CFLAGS "-m64 -O")
  set(AFTER_MMX "--enable-mmx-optimization=no")
endif(${ROOT_ARCHITECTURE} MATCHES linuxx8664gcc)
if(${ROOT_ARCHITECTURE} MATCHES linuxicc)
  set(AFTER_CFLAGS "${AFTER_CFLAGS} -m32 -O")
endif(${ROOT_ARCHITECTURE} MATCHES linuxicc)
if(${ROOT_ARCHITECTURE} MATCHES linuxx8664icc)
  set(AFTER_CFLAGS "${AFTER_CFLAGS} -m64 -O")
endif(${ROOT_ARCHITECTURE} MATCHES linuxx8664icc)
if(${ROOT_ARCHITECTURE} MATCHES macosx)
  set(AFTER_CFLAGS "-m32 -O")
endif(${ROOT_ARCHITECTURE} MATCHES macosx)
if(${ROOT_ARCHITECTURE} MATCHES macosx64)
  set(AFTER_CFLAGS "-m64 -O")
endif(${ROOT_ARCHITECTURE} MATCHES macosx64)
if(${ROOT_ARCHITECTURE} MATCHES linuxppc64gcc)
  set(AFTER_CFLAGS "-m64 -O")
endif(${ROOT_ARCHITECTURE} MATCHES linuxppc64gcc)

if(${ROOT_ARCHITECTURE} MATCHES solarisCC5)
  set(AFTER_CFLAGS "${AFTER_CFLAGS} --erroff=E_WHITE_SPACE_IN_DIRECTIVE")
endif(${ROOT_ARCHITECTURE} MATCHES solarisCC5)

if(${ROOT_ARCHITECTURE} MATCHES solaris64CC5)
  set(AFTER_CC "cc")
  set(AFTER_CFLAGS "${AFTER_CFLAGS} -m64 -KPIC --erroff=E_WHITE_SPACE_IN_DIRECTIVE")
endif(${ROOT_ARCHITECTURE} MATCHES solaris64CC5)

if(${ROOT_ARCHITECTURE} MATCHES sgicc64)
  set(AFTER_CC "gcc")  
  set(AFTER_CFLAGS "-mabi=64 -O")
endif(${ROOT_ARCHITECTURE} MATCHES sgicc64)
if(${ROOT_ARCHITECTURE} MATCHES hpuxia64acc)
  set(AFTER_CC "cc")  
  set(AFTER_CFLAGS "+DD64 -Ae +W863 -O")
  set(AFTER_ALT "gcc -mlp64")
endif(${ROOT_ARCHITECTURE} MATCHES hpuxia64acc)

# copy files from source directory to build directory
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/libAfterImage
                   COMMAND cmake -E copy_directory  ${CMAKE_CURRENT_SOURCE_DIR}/src/libAfterImage ${CMAKE_CURRENT_BINARY_DIR}/libAfterImage
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} 
                   )

# configure and make libAfterImage, this command depends on the unpacked package 
If(JPEG_FOUND)
  Set(JPEGINCLUDE "--with-jpeg-includes=-I${JPEG_INCLUDE_DIR}")
EndIf(JPEG_FOUND)
If(PNG_FOUND)
  #Because PNG needs zlib the PNG_INCLUDE_DIR has two entries. This has to be
  # cleaned here
#  Set(PNG_INC_DIR "")
  ForEach(_dir ${PNG_INCLUDE_DIR})
    Set(PNG_INC_DIR ${PNG_INC_DIR} -I${_dir})
  EndForEach(_dir ${PNG_INCLUDE_DIR})
  Set(PNGINCLUDE "--with-png-includes=\"${PNG_INC_DIR}\"")
EndIf(PNG_FOUND)
If(TIFF_FOUND)
  Set(TIFFINCLUDE "--with-tiff-includes=-I${TIFF_INCLUDE_DIR}")
Else(TIFF_FOUND)
  Set(TIFFINCLUDE "--with-tiff=no")
EndIf(TIFF_FOUND)
If(GIF_FOUND)
  Set(GIFINCLUDE "--with-gif-includes=-I${GIF_INCLUDE_DIR}")
EndIf(GIF_FOUND)
Set(TTFINCLUDE "--with-ttf-includes=-I${FREETYPE_INCLUDE_DIR}")

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/libAfterImage/libAfterImage.a  
                   COMMAND GNUMAKE=make CC=${AFTER_CC} CFLAGS=${AFTER_CFLAGS} ./configure --with-ttf ${TTFINCLUDE} --with-afterbase=no --without-svg --disable-glx ${AFTER_MMX} ${AFTER_DBG} --with-builtin-ungif ${GIFINCLUDE} --with-jpeg ${JPEGINCLUDE} --with-png ${PNGINCLUDE} ${TIFFINCLUDE} > /dev/null 2>& 1
                   COMMAND make > /dev/null 2>& 1
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libAfterImage 
                   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/libAfterImage
                  )

# copy the created library into the library directory in the build directory
# This command depends on the created libAfterImage.a
if(${ROOT_PLATFORM} MATCHES macosx)
  add_custom_command(OUTPUT ${LIBRARY_OUTPUT_PATH}/libAfterImage.a
                     COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/libAfterImage/libAfterImage.a ${LIBRARY_OUTPUT_PATH}
                     COMMAND ranlib ${LIBRARY_OUTPUT_PATH}/libAfterImage.a
                     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libAfterImage 
                     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/libAfterImage/libAfterImage.a                   )
else(${ROOT_PLATFORM} MATCHES macosx)
  add_custom_command(OUTPUT ${LIBRARY_OUTPUT_PATH}/libAfterImage.a
                     COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/libAfterImage/libAfterImage.a ${LIBRARY_OUTPUT_PATH}
                     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/libAfterImage/libAfterImage.a
                    )
endif(${ROOT_PLATFORM} MATCHES macosx)
                
# create a target which will always be build and does actually nothing. The target is only
# needed that the dependencies are build, f they are not up to date. If everything is up to
# dte nothing is done. This target depends on the libAfterImage.a in the library directory of the 
# build directory.
add_custom_target(${AFTER_VERSION}
                  DEPENDS ${LIBRARY_OUTPUT_PATH}/libAfterImage.a
                 )

# import libAfterImage.a which is not in the scope of CMake. This step makes it possible that
# make clean will remove the library  
add_library(AfterImage STATIC IMPORTED)
INSTALL(FILES ${LIBRARY_OUTPUT_PATH}/libAfterImage.a DESTINATION ${LIB_INSTALL_DIR})
