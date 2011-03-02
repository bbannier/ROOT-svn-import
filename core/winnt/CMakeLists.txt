############################################################################
# CMakeLists.txt file for building ROOT core/winnt package
############################################################################

file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx)

ROOT_GENERATE_DICTIONARY(G__WinNT TWinNTSystem.h LINKDEF LinkDef.h)
set(WINNT_DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/G__WinNT.cxx  PARENT_SCOPE)

set(LibCore_SRCS ${LibCore_SRCS} ${sources} PARENT_SCOPE)
set(LibCore_LINKDEF ${LibCore_LINKDEF} ${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef.h PARENT_SCOPE)
add_custom_target(UNIX_DICTIONARY DEPENDS G__WinNT.cxx)

ROOT_INSTALL_HEADERS()
