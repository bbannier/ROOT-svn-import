set(LLVM_NO_RTTI 1)

SET(CURSES_NEED_NCURSES TRUE)
FIND_PACKAGE(Curses)
FIND_LIBRARY(READLINE_LIBRARY NAMES readline) 

set( LLVM_USED_LIBS
  clingUserInterface
  clingMetaProcessor
  clingInterpreter
  clingEditLine
  clangFrontend
  clangDriver
  clangCodeGen
  clangSema
  clangChecker
  clangAnalysis
  clangRewrite
  clangAST
  clangParse
  clangLex
  clangBasic
  ${CURSES_CURSES_LIBRARY}
  ${READLINE_LIBRARY}
  )

set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  all
  )

add_clang_executable(cling
  cling.cpp
  )
