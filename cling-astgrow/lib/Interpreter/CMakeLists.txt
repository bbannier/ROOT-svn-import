set(LLVM_NO_RTTI 1)

add_cling_library(clingInterpreter
  Interpreter.cpp
  )
