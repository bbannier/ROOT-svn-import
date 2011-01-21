# Create libThread
# Define all header and source files which are needed
# to build the Thread library

# Set all header files 
Set(THREAD_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/TCondition.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/TConditionImp.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/TMutex.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/TMutexImp.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/TRWLock.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/TSemaphore.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/TThread.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/TThreadFactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/TThreadImp.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/TAtomicCount.h 
   )
    #${CMAKE_CURRENT_SOURCE_DIR}/inc/TLockFile.h

If(NOT ${ROOT_ARCHITECTURE} MATCHES win32)
  Set(THREAD_HEADERS
      ${THREAD_HEADERS}
      ${CMAKE_CURRENT_SOURCE_DIR}/inc/TPosixCondition.h 
      ${CMAKE_CURRENT_SOURCE_DIR}/inc/TPosixMutex.h 
      ${CMAKE_CURRENT_SOURCE_DIR}/inc/TPosixThread.h    
      ${CMAKE_CURRENT_SOURCE_DIR}/inc/TPosixThreadFactory.h 
      ${CMAKE_CURRENT_SOURCE_DIR}/inc/PosixThreadInc.h
     )
  Set(THREAD_HEADERS_EXT  
      ${CMAKE_CURRENT_SOURCE_DIR}/inc/TAtomicCountGcc.h 
      ${CMAKE_CURRENT_SOURCE_DIR}/inc/TAtomicCountPthread.h
     )
Else(NOT ${ROOT_ARCHITECTURE} MATCHES win32)
  Set(THREAD_HEADERS
      ${THREAD_HEADERS}
      ${CMAKE_CURRENT_SOURCE_DIR}/inc/TWin32Condition.h 
      ${CMAKE_CURRENT_SOURCE_DIR}/inc/TWin32Mutex.h 
      ${CMAKE_CURRENT_SOURCE_DIR}/inc/TWin32Thread.h    
      ${CMAKE_CURRENT_SOURCE_DIR}/inc/TWin32ThreadFactory.h
     )
  Set(THREAD_HEADERS_EXT 
      ${CMAKE_CURRENT_SOURCE_DIR}/inc/TAtomicCountWin32.h
     )
EndIf(NOT ${ROOT_ARCHITECTURE} MATCHES win32)

Set(THREAD_ALL_HEADERS
    ${THREAD_HEADERS}
    ${THREAD_HEADERS_EXT}
)
INSTALL_HEADERS("THREAD_ALL_HEADERS")

# Set all source files
Set(THREAD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TCondition.cxx 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TConditionImp.cxx 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TMutex.cxx 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TMutexImp.cxx 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TRWLock.cxx 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TSemaphore.cxx 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TThread.cxx 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TThreadFactory.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TThreadImp.cxx 
   )
    #${CMAKE_CURRENT_SOURCE_DIR}/src/TLockFile.cxx
If(NOT ${ROOT_ARCHITECTURE} MATCHES win32)
  Set(THREAD_SOURCES
      ${THREAD_SOURCES}
      ${CMAKE_CURRENT_SOURCE_DIR}/src/TPosixCondition.cxx 
      ${CMAKE_CURRENT_SOURCE_DIR}/src/TPosixMutex.cxx 
      ${CMAKE_CURRENT_SOURCE_DIR}/src/TPosixThread.cxx    
      ${CMAKE_CURRENT_SOURCE_DIR}/src/TPosixThreadFactory.cxx 
     )
Else(NOT ${ROOT_ARCHITECTURE} MATCHES win32)
  Set(THREAD_SOURCES
      ${THREAD_SOURCES}
      ${CMAKE_CURRENT_SOURCE_DIR}/src/TWin32Condition.cxx 
      ${CMAKE_CURRENT_SOURCE_DIR}/src/TWin32Mutex.cxx 
      ${CMAKE_CURRENT_SOURCE_DIR}/src/TWin32Thread.cxx    
      ${CMAKE_CURRENT_SOURCE_DIR}/src/TWin32ThreadFactory.cxx
     )
EndIf(NOT ${ROOT_ARCHITECTURE} MATCHES win32)

# Define the LinkDef and the output file which are needed
# to create the dictionary.
# Definde the needed include directories and create the Dictionary
Set(THREAD_LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef.h)
Set(THREAD_DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/G__Thread.cxx)

Set(INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/core/rint/inc
    ${CMAKE_SOURCE_DIR}/cint/cint/include
    ${CMAKE_SOURCE_DIR}/cint/cint/stl
    ${CMAKE_SOURCE_DIR}/cint/cint/lib
    ${CMAKE_SOURCE_DIR}/cint/cint/inc
    ${CMAKE_SOURCE_DIR}/core/base/inc
    ${CMAKE_SOURCE_DIR}/core/clib/inc
    ${CMAKE_SOURCE_DIR}/core/meta/inc
    ${CMAKE_SOURCE_DIR}/core/thread/inc
)

GENERATE_DICTIONARY("${THREAD_HEADERS}"
                    "${THREAD_LINKDEF}" "${THREAD_DICTIONARY}"
                    "${INCLUDE_DIRS}" ""
                   )

Set(DEPENDENCIES RIO)

GENERATE_ROOTMAP("${THREAD_LINKDEF}" Thread ${DEPENDENCIES})

Set(INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_SOURCE_DIR}/cint/cint/inc
  ${CMAKE_SOURCE_DIR}/core/base/inc
  ${CMAKE_SOURCE_DIR}/core/meta/inc
  ${CMAKE_SOURCE_DIR}/io/io/inc
  ${ROOT_INCLUDE_DIR}
)
Include_Directories( ${INCLUDE_DIRECTORIES})

Add_Library(Thread SHARED ${THREAD_SOURCES} ${THREAD_DICTIONARY})
Set_Target_properties(Thread PROPERTIES ${ROOT_LIBRARY_PROPERTIES})
Target_Link_Libraries(Thread ${DEPENDENCIES})
Install(TARGETS Thread DESTINATION ${LIB_INSTALL_DIR})
