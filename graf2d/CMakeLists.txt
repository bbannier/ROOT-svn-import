ROOT_USE_PACKAGE(graf2d/gpad)
ROOT_USE_PACKAGE(graf2d/graf)

if(builtin_fretype)
  add_subdirectory(freetype)
endif()
add_subdirectory(gpad)
add_subdirectory(graf)
add_subdirectory(postscript)
add_subdirectory(x11)         # special CMakeLists.txt
add_subdirectory(x11ttf)      # special CMakeLists.txt
add_subdirectory(asimage)     # special CMakeLists.txt
if(qt)
  add_subdirectory(qt)
endif()

