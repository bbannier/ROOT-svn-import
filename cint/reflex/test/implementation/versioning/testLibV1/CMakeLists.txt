INCLUDE_DIRECTORIES(${REFLEX_INCLUDE_DIR})
REFLEX_ADD_DICTIONARY(test_LibV1 TEST WITH_PREFIX Library.h Value.h Zombie.h
                      OPTIONS --quiet EXTRA_FILES ${CMAKE_CURRENT_BINARY_DIR}/ReflexVersioning.cxx Library.h Library.cxx Value.h Value.cxx Zombie.h Zombie.cxx)

CONFIGURE_VERSIONING_CXX(test_LibV1)

ADD_DEPENDENCIES(test_LibV1 test_LibCommon)
TARGET_LINK_LIBRARIES(test_LibV1 test_LibCommon)
