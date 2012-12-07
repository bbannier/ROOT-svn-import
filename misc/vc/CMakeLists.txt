include(cmake/VcMacros.cmake)
vc_determine_compiler()
vc_set_preferred_compiler_flags(WARNING_FLAGS BUILDTYPE_FLAGS)
add_definitions("${Vc_DEFINITIONS}")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(libvc_compile_flags "-DVC_COMPILE_LIB")
vc_compile_for_all_implementations(_objs src/trigonometric.cpp FLAGS ${libvc_compile_flags} EXCLUDE Scalar)
set(_srcs src/const.cpp src/cpuid.cpp src/support.cpp ${_objs})
if(USE_AVX)
   list(APPEND _srcs src/avx_sorthelper.cpp)
else()
   if(NOT Vc_AVX_INTRINSICS_BROKEN)
      # we'd still like to have avx_sorthelper.cpp built in, but that requires compilation with -mavx (or a comparable flag)
      foreach(_flag "-xAVX" "-mavx" "/arch:AVX")
         AddCompilerFlag("${_flag}" CXX_RESULT _flag_works)
         if(_flag_works)
            if(_flag STREQUAL "-xAVX")
               set(_flag "${_flag} -diag-disable 10121") # disable the warning "overriding -xSSE4.2 with -xAVX"
            endif()
            list(APPEND _srcs src/avx_sorthelper.cpp)
            set_source_files_properties(src/avx_sorthelper.cpp PROPERTIES COMPILE_FLAGS "${_flag}")
            break()
         endif()
      endforeach()
   endif()
endif()
add_library(Vc STATIC ${_srcs})
set_target_properties(Vc PROPERTIES COMPILE_FLAGS ${libvc_compile_flags})

set_property(GLOBAL APPEND PROPERTY ROOT_EXPORTED_TARGETS Vc)

install(TARGETS Vc RUNTIME DESTINATION bin
                   LIBRARY DESTINATION lib
                   ARCHIVE DESTINATION lib
                   COMPONENT libraries)
ROOT_INSTALL_HEADERS(include/)
