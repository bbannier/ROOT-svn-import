# - Try to find the freetype library
# Once done this will define
#
#  FREETYPE_FOUND - system has Freetype
#  FREETYPE_INCLUDE_DIRS - the FREETYPE include directories
#  FREETYPE_LIBRARIES - Link these to use FREETYPE
#  FREETYPE_INCLUDE_DIR - internal
#  FREETYPE_VERSION - Freetype version 

# First check if freetype-config exists in the PATH

 find_program(FREETYPECONFIG_EXECUTABLE NAMES freetype-config PATHS
     /opt/local/bin
  )

  #reset vars
  set(FREETYPE_LIBRARIES)
  set(FREETYPE_INCLUDE_DIR)

  # if freetype-config has been found
  if(FREETYPECONFIG_EXECUTABLE)
    execute_process(COMMAND ${FREETYPECONFIG_EXECUTABLE} --version OUTPUT_VARIABLE FREETYPE_VERSION)
    execute_process(COMMAND ${FREETYPECONFIG_EXECUTABLE} --libs OUTPUT_VARIABLE FREETYPE_LIBRARIES)
    execute_process(COMMAND ${FREETYPECONFIG_EXECUTABLE} --cflags OUTPUT_VARIABLE _freetype_pkgconfig_output)
    if(FREETYPE_LIBRARIES AND _freetype_pkgconfig_output)
      set(FREETYPE_FOUND TRUE)
      # freetype-config can print out more than one -I, so we need to chop it up
      # into a list and process each entry separately
      separate_arguments(_freetype_pkgconfig_output)
      foreach(value ${_freetype_pkgconfig_output})
        string(REGEX REPLACE "-I(.+)" "\\1" value "${value}")
        set(FREETYPE_INCLUDE_DIR ${FREETYPE_INCLUDE_DIR} ${value})
      endforeach(value)
    endif(FREETYPE_LIBRARIES AND _freetype_pkgconfig_output)

  else(FREETYPECONFIG_EXECUTABLE)
      # if there is no freetype-config the freetype version is definitely to old
      set(FREETYPE_FOUND FALSE)
  endif(FREETYPECONFIG_EXECUTABLE)

  if (FREETYPE_FOUND)
    if (NOT Freetype_FIND_QUIETLY)
       message(STATUS "Found Freetype: ${FREETYPE_LIBRARIES}")
    endif (NOT Freetype_FIND_QUIETLY)
  else (FREETYPE_FOUND)
    if (Freetype_FIND_REQUIRED)
       message(FATAL_ERROR "Could not find FreeType library")
    endif (Freetype_FIND_REQUIRED)
  endif (FREETYPE_FOUND)

  set(FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIR})



