set( LLVM_USED_LIBS
  libcling)

add_cling_executable(cling
  cling.cpp)

if(UNIX)
  set(CLINGXX_LINK_OR_COPY create_symlink)
else()
  set(CLINGXX_LINK_OR_COPY copy)
endif()

install(TARGETS cling
  RUNTIME DESTINATION bin)
