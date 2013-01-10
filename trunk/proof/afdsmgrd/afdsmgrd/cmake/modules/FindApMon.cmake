#
# FindApMon.cmake -- by Dario Berzano <dario.berzano@cern.ch>
#
# This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
#
# CMake module to search for ApMon (MonALISA client library) installation.
#

#
# Search for the ApMon_PREFIX variable
#

# Read ALIEN_DIR envvar; if not found, read GSHELL_ROOT envvar instead
if (DEFINED ENV{ALIEN_DIR})
  set (ALIEN_DIR $ENV{ALIEN_DIR})
else ()
  set (ALIEN_DIR $ENV{GSHELL_ROOT}/..)
endif ()

find_path (ApMon_INCDIR
  ApMon.h
  PATHS ${ApMon_PREFIX}/include ${ALIEN_DIR}/api/include ${ALIEN_DIR}/include
  NO_DEFAULT_PATH
)

find_library (ApMon_LIBRARY
  apmoncpp
  PATHS ${ApMon_PREFIX}/lib ${ALIEN_DIR}/api/lib ${ALIEN_DIR}/api/lib64 ${ALIEN_DIR}/lib ${ALIEN_DIR}/lib64
  NO_DEFAULT_PATH
)

if ((NOT ApMon_INCDIR) OR (NOT ApMon_LIBRARY))
  set (ApMon_FOUND FALSE)
  if (ApMon_FIND_REQUIRED)
    message (FATAL_ERROR "ApMon not found!")
  else ()
    message (STATUS "[ApMon] Not found")
  endif ()
else ()
  message (STATUS "[ApMon] Include path: ${ApMon_INCDIR}")
  message (STATUS "[ApMon] Dynamic library: ${ApMon_LIBRARY}")
  set (ApMon_FOUND TRUE)
endif ()
