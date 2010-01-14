INCLUDE_DIRECTORIES(${REFLEX_INCLUDE_DIR})

REFLEX_ADD_DICTIONARY(test_LibCommon TEST WITH_PREFIX BaseLibrary.h Dummy.h
                      OPTIONS --quiet EXTRA_FILES ${CMAKE_CURRENT_BINARY_DIR}/ReflexVersioning.cxx BaseLibrary.h BaseLibrary.cxx Dummy.h Dummy.cxx)

CONFIGURE_VERSIONING_CXX(test_LibCommon)
