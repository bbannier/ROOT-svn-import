#
# Uninstall.cmake -- by Dario Berzano <dario.berzano@cern.ch>
#
# This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
#
# CMake module for uninstalling everything that was installed by a previous
# call of the install target. This module relies on two things:
#
#  1. CMake creates automatically the install_manifest.txt file with the list of
#     installed destination files
#
#  2. Every install () directive is given with *full destination path* in this
#     project! This is not true in general for every CMake project!
#
# Errors on files not found are not fatal; errors on deletion are fatal,
# instead.
#

message (STATUS "Reading uninstall list from: ${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt")

# Check for install manifest
if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt")
  message(FATAL_ERROR "Cannot find install manifest: ${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt")
endif ()

# Read install manifest into a list in memory
file (READ "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt" LIST_FILES)
string(REGEX REPLACE "\n" ";" LIST_FILES "${LIST_FILES}")

# For each element, uninstall it
foreach (ONE_FILE ${LIST_FILES})

  message (STATUS "Uninstalling: ${ONE_FILE}")

  if (EXISTS "${ONE_FILE}")

    execute_process (
      COMMAND ${CMAKE_COMMAND} -E remove "${ONE_FILE}"
      OUTPUT_VARIABLE RM_Out
      RESULT_VARIABLE RM_RetVal
    )

    if (NOT ${RM_RetVal} EQUAL 0)
      message (FATAL_ERROR "  >> Cannot delete, check permissions")
    endif ()

  else ()
    message (STATUS "  >> File does not exist" )
  endif ()

endforeach ()
