macro(add_cling_library name)
  set(srcs ${ARGN})
  if(MSVC_IDE)
    file( GLOB_RECURSE headers *.h)
    set(srcs ${srcs} ${headers})
    string( REGEX MATCHALL "/[^/]+" split_path ${CMAKE_CURRENT_SOURCE_DIR})
    list( GET split_path -1 dir)
    file( GLOB_RECURSE headers ../../include/cling${dir}/*.h)
    set(srcs ${srcs} ${headers})
  endif(MSVC_IDE)
  add_library( ${name} ${srcs} )
  if( LLVM_COMMON_DEPENDS )
    add_dependencies( ${name} ${LLVM_COMMON_DEPENDS})
  endif( LLVM_COMMON_DEPENDS )
  if(MSVC)
    get_target_property(cflag ${name} COMPILE_FLAGS)
    if(NOT cflag)
      set(cflag "")
    endif(NOT cflag)
    set(cflag "${cflag} /Za")
    set_target_properties(${name} PROPERTIES COMPILE_FLAGS ${cflag})
  endif(MSVC)
  install(TARGETS ${name}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
endmacro(add_cling_library)

macro(add_cling_executable name)
  set(srcs ${ARGN})
  if(MSVC_IDE)
    file( GLOB_RECURSE headers *.h)
    set(srcs ${srcs} ${headers})
  endif(MSVC_IDE)
  add_llvm_executable( ${name} ${srcs} )
  install(TARGETS ${name}
    RUNTIME DESTINATION bin)
endmacro(add_cling_executable)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../clang/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../clang/lib/
  ${CMAKE_CURRENT_SOURCE_DIR}/../cling/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../cling/lib/Inuit/include
  )

install(DIRECTORY include
  DESTINATION .
  PATTERN ".svn" EXCLUDE
  )

set(CLANG_SYS_HEADERS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../Headers")
add_definitions( -D_GNU_SOURCE -DCLANG_SYS_HEADERS="${CLANG_SYS_HEADERS}" -DHOST_TARGET="${LLVM_HOSTTRIPLE}")

add_subdirectory(lib)
add_subdirectory(Driver)

# TODO: docs.
# exec_program(llvm-config ARGS --libs OUTPUT_VARIABLE LLVM_LIBS1)
# hand crafted:
#set(LLVM_LIBS1 "-lclangDriver -lclangLex -lclangCodeGen -lclangSema -lclangParse -lclangAST -lclangBasic -lLLVMLinker /build/axel/build/llvm/inst/lib/LLVMExecutionEngine.o /build/axel/build/llvm/inst/lib/LLVMJIT.o -lLLVMBitWriter -lLLVMBitReader -lLLVMCodeGen -lLLVMAnalysis -lLLVMTarget -lLLVMCore -lLLVMSupport -lLLVMSystem")
set(LLVM_LIBS "-ldl ${LLVM_LIBS1}")
#exec_program(llvm-config ARGS --ldflags OUTPUT_VARIABLE LLVM_LDFLAGS)
#exec_program(llvm-config ARGS --includedir OUTPUT_VARIABLE LLVM_INCLUDE_DIR)
#set(CLANG_LIBS "-lclangDriver -lclangLex -lclangCodeGen -lclangSema -lclangParse -lclangAST -lclangBasic" )
#exec_program(llvm-config ARGS --prefix OUTPUT_VARIABLE LLVM_PREFIX)
#set(CLANG_SYS_HEADERS "${LLVM_PREFIX}/Headers")
#exec_program(llvm-config ARGS --host-target OUTPUT_VARIABLE LLVM_HOST_TARGET)
