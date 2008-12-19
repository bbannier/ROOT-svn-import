set(LLVM_NO_RTTI 1)

set( LLVM_USED_LIBS
  clingInterpreter
  clangDriver
  clangLex
  clangCodeGen
  clangSema
  clangParse
  clangAST
  clangBasic
  )

set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  bitreader
  bitwriter
  codegen
  ipo
  selectiondag
  executionengine
  linker
  )

add_clang_executable(cling
  cling.cpp
  )
