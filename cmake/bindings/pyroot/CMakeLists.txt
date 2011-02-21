############################################################################
# CMakeLists.txt file for building ROOT bindings/pyroot package
############################################################################
ROOT_USE_PACKAGE(math/mathcore)
ROOT_USE_PACKAGE(tree/tree)
include_directories(${PYTHON_INCLUDE_DIRS})

if(reflex)
  add_definitions(-DPYROOT_USE_REFLEX )
  ROOT_USE_PACKAGE(cint/reflex)
endif()

ROOT_GENERATE_DICTIONARY(G__PyROOT *.h LINKDEF LinkDef.h)
ROOT_GENERATE_ROOTMAP(PyROOT LINKDEF LinkDef.h)

ROOT_LINKER_LIBRARY(PyROOT *.cxx G__PyROOT.cxx LIBRARIES RIO MathCore ${PYTHON_LIBRARIES})
if(MACOSX_MINOR EQUAL 5)
  ROOT_EXECUTABLE(python64 python64.c LIBRARIES  ${PYTHON_LIBRARIES})
endif()

#---Install python modules--------------------------------------------------
install(FILES ROOT.py DESTINATION lib)
install(CODE "execute_process(COMMAND python -m py_compile ${CMAKE_INSTALL_PREFIX}/lib/ROOT.py )")
install(CODE "execute_process(COMMAND python -O -m py_compile ${CMAKE_INSTALL_PREFIX}/lib/ROOT.py )")

#---Install headers----------------------------------------------------------
ROOT_INSTALL_HEADERS()

