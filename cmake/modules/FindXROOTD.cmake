#
# Try to find XROOTD
# Once done this will define
#
#  XROOTD_FOUND - system has XROOTD
#  XROOTD_INCLUDE_DIR - the XROOTD include directory
#  XROOTD_LIBRARIES - The libraries needed to use XROOTD
#  XROOTD_CFLAGS - Additional compilation flags (defines)
#  XROOTD_OLDPACK - old-style packaging for XROOTD libraries
#

if(XROOTD_XrdClient_LIBRARY AND XROOTD_INCLUDE_DIR)
  set(XROOTD_FIND_QUIETLY TRUE)
endif()

find_path(XROOTD_INCLUDE_DIR
  NAMES
  XrdVersion.hh
  PATHS 
  $ENV{XRDSYS}/include/xrootd
  $ENV{XRDSYS}/include
  /opt/xrootd/include/xrootd
  /opt/xrootd/include
  /usr/local/include/xrootd
  /usr/local/include
  /usr/include/xrootd
  /usr/include
)

if (XROOTD_INCLUDE_DIR)
  file(STRINGS ${XROOTD_INCLUDE_DIR}/XrdVersion.hh xrdvers REGEX "^#define XrdVERSION ")
  string(REGEX REPLACE "#define[ ]+XrdVERSION[ ]+" "" xrdvers ${xrdvers})
  string(REGEX REPLACE "[^v\\.]+" "" xrdversdots ${xrdvers})
  if (${xrdversdots} STREQUAL "v..")
    # Regular version string; parse it out
    string(REGEX MATCH "[0-9\\.]+" xrdvers ${xrdvers})
    string(REGEX REPLACE "^([^.]*)\\.(.*)\\.(.*)" "\\1" xrdversmajor ${xrdvers})
    string(REGEX REPLACE "^([^.]*)\\.(.*)\\.(.*)" "\\2" xrdversminor ${xrdvers})
    string(REGEX REPLACE "^([^.]*)\\.(.*)\\.(.*)" "\\3" xrdverspatch ${xrdvers})
    math(EXPR xrdversnum ${xrdversmajor}*100000000+${xrdversminor}*10000+${xrdverspatch})
  else ()
    # Old version string: we keep only the first numerics, i.e. the date
    string(REGEX REPLACE "[v\"]" "" xrdvers ${xrdvers})
    string(SUBSTRING ${xrdvers} 0 8 xrdversnum)
  endif ()
  if ( ${xrdversnum} EQUAL 300030000 )
     SET(XROOTD_FOUND FALSE)
     message(WARNING " >>> Cannot build with XRootD version 3.3.0: please install >=3.3.1 or <= 3.2.x")
  else()
     SET(XROOTD_FOUND TRUE)
  endif ()
endif()

if(XROOTD_FOUND)
  # This we used as a compiler macro variable
  if(NOT XROOTD_FIND_QUIETLY )
    message(STATUS "Found Xrootd version num: ${xrdvers} (setting -DROOTXRDVERS=${xrdversnum})")
  endif()
  SET(XROOTD_CFLAGS "-DROOTXRDVERS=${xrdversnum}")

  if ( ${xrdversnum} LESS 300010000 AND ${xrdversnum} LESS 20111022)
     SET(XROOTD_OLDPACK TRUE)
     message(STATUS "Setting OLDPACK TRUE")
  else()
     SET(XROOTD_OLDPACK FALSE)
  endif()

  # Check for additional headers
  if ( ${xrdversnum} LESS 20070723 )
     # Check for additional headers in old directories
     find_path(XROOTD_INCLUDE_DIR
        NAMES
        XrdNet/XrdNetDNS.hh
        XrdOuc/XrdOucError.hh
        XrdOuc/XrdOucLogger.hh
        XrdOuc/XrdOucPlugin.hh
        XrdOuc/XrdOucPthread.hh
        XrdOuc/XrdOucSemWait.hh
        XrdOuc/XrdOucTimer.hh
        PATHS 
        ${XROOTD_INCLUDE_DIR}
     )
  else()
     if ( ${xrdversnum} LESS 300010000 AND ${xrdversnum} LESS 20111022)
        # DNS stuff was under XrdNet
        find_path(XROOTD_INCLUDE_DIR
           NAMES
           XrdNet/XrdNetDNS.hh
           PATHS 
           ${XROOTD_INCLUDE_DIR}
        )
     else ()
        # DNS stuff is under XrdSys
        find_path(XROOTD_INCLUDE_DIR
           NAMES
           XrdSys/XrdSysDNS.hh
           PATHS 
           ${XROOTD_INCLUDE_DIR}
        )
     endif ()

     if (XROOTD_INCLUDE_DIR)
        # Check for additional headers in new directories
        find_path(XROOTD_INCLUDE_DIR
            NAMES
            XrdSys/XrdSysError.hh
            XrdSys/XrdSysLogger.hh
            XrdSys/XrdSysPlugin.hh
            XrdSys/XrdSysPthread.hh
            XrdSys/XrdSysSemWait.hh
            XrdSys/XrdSysTimer.hh
            PATHS 
            ${XROOTD_INCLUDE_DIR}
        )
     endif()

     SET(XROOTD_PRIVATE_INCLUDE_DIR FALSE)     
     if (XROOTD_INCLUDE_DIR)
        # Check for additional headers in new directories
        find_path(XROOTD_PRIVATE_INCLUDE_DIR
            NAMES
            XrdSys/XrdSysPriv.hh
            PATHS 
            ${XROOTD_INCLUDE_DIR}/private
        )
     endif()
  endif()
  if (XROOTD_INCLUDE_DIR)
     SET(XROOTD_FOUND TRUE)
  else ()
     SET(XROOTD_FOUND FALSE)
  endif ()
else()
  SET(XROOTD_FOUND FALSE)
endif()

if(XROOTD_FOUND)
  # Search for the required libraries; this depends on packaging ...

  if(XROOTD_OLDPACK)
    foreach(l XrdNet XrdOuc XrdSys XrdClient Xrd)
      find_library(XROOTD_${l}_LIBRARY
         NAMES ${l} 
         PATHS $ENV{XRDSYS}/lib
               /opt/xrootd/lib
               /usr/local/lib
               /usr/lib)
      list(APPEND XROOTD_LIBRARIES ${XROOTD_${l}_LIBRARY})
    endforeach()

    if(${xrdversnum} GREATER 20100729)
      find_library(XROOTD_XrdNetUtil_LIBRARY
        NAMES XrdNetUtil
        PATHS $ENV{XRDSYS}/lib
              /opt/xrootd/lib
              /usr/local/lib
              /usr/lib)
      list(APPEND XROOTD_LIBRARIES ${XROOTD_XrdNetUtil_LIBRARY})
    endif ()
  else()
    foreach(l XrdMain XrdUtils XrdClient)
      find_library(XROOTD_${l}_LIBRARY
         NAMES ${l} 
         PATHS $ENV{XRDSYS}/lib
               /opt/xrootd/lib
               /usr/local/lib
               /usr/lib)
      list(APPEND XROOTD_LIBRARIES ${XROOTD_${l}_LIBRARY})
    endforeach()
  endif()

  if(XROOTD_LIBRARIES)
    set(XROOTD_FOUND TRUE)
    if(NOT XROOTD_FIND_QUIETLY )
      if (XROOTD_PRIVATE_INCLUDE_DIR)
         message(STATUS "             include_dir: ${XROOTD_INCLUDE_DIR} ${XROOTD_PRIVATE_INCLUDE_DIR}")
      else ()
         message(STATUS "             include_dir: ${XROOTD_INCLUDE_DIR}")
      endif ()
      message(STATUS "             libraries: ${XROOTD_LIBRARIES}")
    endif() 
  else ()
    set(XROOTD_FOUND FALSE)
  endif ()
endif()

mark_as_advanced(XROOTD_INCLUDE_DIR
                 XROOTD_PRIVATE_INCLUDE_DIR
                 XROOTD_XrdMain_LIBRARY
                 XROOTD_XrdUtils_LIBRARY
                 XROOTD_XrdClient_LIBRARY
                 XROOTD_XrdNetUtils_LIBRARY
                 XROOTD_XrdNet_LIBRARY
                 XROOTD_XrdSys_LIBRARY
                 XROOTD_XrdOuc_LIBRARY
                 XROOTD_Xrd_LIBRARY )

