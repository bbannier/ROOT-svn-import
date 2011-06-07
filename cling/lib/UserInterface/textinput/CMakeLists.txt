#set(LLVM_LINK_COMPONENTS mc)
#set(LLVM_USED_LIBS clangBasic clangAST clangParse)


add_cling_library(clingUITextInput
  Editor.cpp
  History.cpp
  KeyBinding.cpp
  Range.cpp
  SignalHandler.cpp
  StreamReader.cpp
  StreamReaderUnix.cpp
  StreamReaderWin.cpp
  TerminalConfigUnix.cpp
  TerminalDisplay.cpp
  TerminalDisplayUnix.cpp
  TerminalDisplayWin.cpp
  TextInput.cpp
  TextInputContext.cpp)

#add_dependencies(clangDriver ClangAttrList ClangDiagnosticDriver
#                 ClangDriverOptions ClangCC1Options ClangCC1AsOptions)
