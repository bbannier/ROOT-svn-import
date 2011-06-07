set(SOURCES
  testlibrary.cpp)

if( LLVM_ENABLE_PIC )
  set(SHARED_LIBRARY TRUE)
  add_cling_library(libcling-test-library ${SOURCES})

  set_target_properties(libcling-test-library
    PROPERTIES
    OUTPUT_NAME "libcling-test-library")

  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(LIBCLINGTL_LINK_FLAGS
      "-Wl,-compatibility_version -Wl,1 -Wl,-dead_strip -Wl,-seg1addr -Wl,0xE0000000")
    set_target_properties(libcling-test-library
      PROPERTIES
      LINK_FLAGS "${LIBCLINGTL_LINK_FLAGS}"
      INSTALL_NAME_DIR "@executable_path/../lib")
  endif()

  if(MSVC)
    # windows.h doesn't compile with /Za
    get_target_property(NON_ANSI_COMPILE_FLAGS libcling-test-library COMPILE_FLAGS)
    string(REPLACE "/Za" "" NON_ANSI_COMPILE_FLAGS ${NON_ANSI_COMPILE_FLAGS})
    set_target_properties(libcling-test-library PROPERTIES
      COMPILE_FLAGS ${NON_ANSI_COMPILE_FLAGS})
  endif()

  set(LIBCLINGTL_STATIC_TARGET_NAME libcling-test-library_static)
else()
  set(LIBCLINGTL_STATIC_TARGET_NAME libcling-test-library)
endif()

if( NOT BUILD_SHARED_LIBS AND NOT WIN32 )
  add_cling_library(${LIBCLINGTL_STATIC_TARGET_NAME} STATIC ${SOURCES})

  set_target_properties(${LIBCLINGTL_STATIC_TARGET_NAME}
    PROPERTIES
    OUTPUT_NAME "libcling-test-library")
endif()
