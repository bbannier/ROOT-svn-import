set(LLVM_NO_RTTI 1)

SET(CURSES_NEED_NCURSES TRUE)
FIND_PACKAGE(Curses)

set( LLVM_USED_LIBS
  clingInterpreter
  clingUserInterface
  clingInuit
  clangDriver
  clangLex
  clangCodeGen
  clangSema
  clangParse
  clangAST
  clangBasic
  ${CURSES_CURSES_LIBRARY}
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
