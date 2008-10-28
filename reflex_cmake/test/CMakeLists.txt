FIND_PACKAGE(CppUnit REQUIRED)
FIND_PACKAGE(PythonInterp REQUIRED)
FIND_PACKAGE(GCCXML REQUIRED)

#####################    config   ####################

SET(REFLEX_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#####################   sub dirs  ####################

MACRO_ADD_SUBDIRECTORIES(genreflex)
MACRO_ADD_SUBDIRECTORIES(implementation)
MACRO_ADD_SUBDIRECTORIES(iso-iec)
MACRO_ADD_SUBDIRECTORIES(regression)
