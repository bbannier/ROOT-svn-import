############################################################################
# CMakeLists.txt file for building ROOT core/editline package
############################################################################

file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx)

ROOT_GENERATE_DICTIONARY(G__Editline *.h LINKDEF LinkDef.h)

set(EDITLINE_DICTIONARY  ${CMAKE_CURRENT_BINARY_DIR}/G__Editline.cxx PARENT_SCOPE)

set(LibCore_SRCS ${LibCore_SRCS} ${sources} PARENT_SCOPE)
set(LibCore_LINKDEF ${LibCore_LINKDEF} ${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef.h PARENT_SCOPE)
set(EDITLINE_CXX_SOURCES  ${sources} PARENT_SCOPE)
add_custom_target(EDITLINE_DICTIONARY DEPENDS G__Editline.cxx)

ROOT_INSTALL_HEADERS()
