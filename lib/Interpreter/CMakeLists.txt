set(LLVM_NO_RTTI 1)

add_cling_library(clingInterp
  Interpreter.cpp
  )
