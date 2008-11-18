cmake_minimum_required(VERSION 2.6)

#-------------------------------------------------------------------------------
# Search for LLVM
#-------------------------------------------------------------------------------
find_program( LLVM_CONFIG_EXECUTABLE NAMES llvm-config PATHS /opt/local/bin )
if( NOT LLVM_CONFIG_EXECUTABLE )
   message( FATAL_ERROR "LLVM was not found!" )
endif( NOT LLVM_CONFIG_EXECUTABLE )

exec_program(llvm-config ARGS --libs OUTPUT_VARIABLE LLVM_LIBS1)
set(LLVM_LIBS "-ldl ${LLVM_LIBS1}")
exec_program(llvm-config ARGS --ldflags OUTPUT_VARIABLE LLVM_LDFLAGS)
exec_program(llvm-config ARGS --includedir OUTPUT_VARIABLE LLVM_INCLUDE_DIR)
set(CLANG_LIBS "-lclangDriver -lclangLex -lclangCodeGen -lclangSema -lclangParse -lclangAST -lclangBasic" )
exec_program(llvm-config ARGS --prefix OUTPUT_VARIABLE LLVM_PREFIX)
set(CLANG_SYS_HEADERS "${LLVM_PREFIX}/Headers")
exec_program(llvm-config ARGS --host-target OUTPUT_VARIABLE LLVM_HOST_TARGET)

#-------------------------------------------------------------------------------
# Define the project
#-------------------------------------------------------------------------------
Project(cling)
ADD_EXECUTABLE(cling cling.cpp) 

include_directories( ${LLVM_INCLUDE_DIR} )
set_target_properties( cling  PROPERTIES LINK_FLAGS "${LLVM_LDFLAGS} -ldl" )
target_link_libraries( cling ${CLANG_LIBS} ${LLVM_LIBS} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHOST_TARGET=\\\"${LLVM_HOST_TARGET}\\\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCLANG_SYS_HEADERS=\\\"${CLANG_SYS_HEADERS}\\\"")
