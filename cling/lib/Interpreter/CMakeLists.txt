#set(LLVM_LINK_COMPONENTS mc)
#set(LLVM_USED_LIBS clangBasic clangAST clangParse)


add_cling_library(clingInterpreter
  CIFactory.cpp
  ClangUtils.cpp
  Diagnostics.cpp
  DynamicLookup.cpp
  ExecutionContext.cpp
  IncrementalParser.cpp
  InputValidator.cpp
  Interpreter.cpp
  InvocationOptions.cpp
  Visitors.cpp)

#add_dependencies(clangDriver ClangAttrList ClangDiagnosticDriver
#                 ClangDriverOptions ClangCC1Options ClangCC1AsOptions)
