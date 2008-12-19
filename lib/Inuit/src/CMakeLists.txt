set(LLVM_NO_RTTI 1)

set(CLING_INUIT_PLATFORM_SRC InputDriverUnix.cxx TerminalDriverUnix.cxx)

if( CMAKE_HOST_WIN32 )
  set(CLING_INUIT_PLATFORM_SRC "Unix" "Win" ${CLING_INUIT_PLATFORM_SRC})
endif( CMAKE_HOST_WIN32 )


add_cling_library(clingInuit
  EditLine.cxx
  Input.cxx
  InputDriver.cxx
  InputFormat.cxx
  TerminalDriver.cxx
  UI.cxx
  Widget.cxx
  ${CLING_INUIT_PLATFORM_SRC}
  )

