INCLUDE_DIRECTORIES(${REFLEX_INCLUDE_DIR})
REFLEX_ADD_DICTIONARY(test_LibV2 TEST WITH_PREFIX Library.h Value.h
                      OPTIONS --quiet EXTRA_FILES ${CMAKE_CURRENT_BINARY_DIR}/ReflexVersioning.cxx Library.h Library.cxx Value.h Value.cxx)

CONFIGURE_VERSIONING_CXX(test_LibV2)

ADD_DEPENDENCIES(test_LibV2 test_LibCommon)
TARGET_LINK_LIBRARIES(test_LibV2 test_LibCommon)
