set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  asmparser
  bitreader
  bitwriter
  codegen
  support
  mc
  executionengine
  target
  jit
  x86info
  x86asmprinter
)

#set(LLVM_USED_LIBS
#  clingWhatever
#)


add_cling_library(clingInterpreter
  ASTDumper.cpp
  ASTUtils.cpp
  ChainedConsumer.cpp
  CIFactory.cpp
  ClangUtils.cpp
  DeclExtractor.cpp
  Diagnostics.cpp
  DynamicLookup.cpp
  ExecutionContext.cpp
  IncrementalParser.cpp
  InputValidator.cpp
  Interpreter.cpp
  InvocationOptions.cpp
  ValuePrinter.cpp
  ValuePrinterInfo.cpp
  ValuePrinterSynthesizer.cpp
  VerifyingSemaConsumer.cpp)

#add_dependencies(clangDriver ClangAttrList ClangDiagnosticDriver
#                 ClangDriverOptions ClangCC1Options ClangCC1AsOptions)
