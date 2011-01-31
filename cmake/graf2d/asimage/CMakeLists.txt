############################################################################
# CMakeLists.txt file for building ROOT graf2d/asimage package
# @author Pere Mato, CERN
############################################################################

ROOT_USE_PACKAGE(graf2d/graf)
ROOT_USE_PACKAGE(graf2d/x11)
ROOT_USE_PACKAGE(gui/gui)

include_directories(${CMAKE_SOURCE_DIR}/hist/hist/inc)  # This is to avoid a circular dependency graf <--> hist 

set(BuildAfterImage TRUE)
if(BuildAfterImage)
  include(BuildAfterImage.cmake)
  set(AFTERIMAGE_INCLUDE_DIR ${CMAKE_BINARY_DIR}/graf2d/asimage/libAfterImage)
  set(AFTERIMAGE_LIBRARIES "-L${LIBRARY_OUTPUT_PATH} -lAfterImage")
  set(RootConfigFeatures "${RootConfigFeatures} builtin_afterimage astiff")
endif()
include_directories(${FREETYPE_INCLUDE_DIR} ${AFTERIMAGE_INCLUDE_DIR})


set(ASEXTRALIBS jpeg tiff ungif png z)
set(XLIBS Xext X11)


ROOT_GENERATE_DICTIONARY(G__ASImage TASImage.h TASImagePlugin.h TASPluginGS.h LINKDEF LinkDef.h)
ROOT_GENERATE_ROOTMAP(ASImage LINKDEF LinkDef.h)
ROOT_LINKER_LIBRARY(ASImage TASImage.cxx TASPluginGS.cxx  G__ASImage.cxx 
                    LIBRARIES  ${AFTERIMAGE_LIBRARIES} ${FREETYPE_LIBRARIES} ${ASEXTRALIBS} ${XLIBS})
add_dependencies(ASImage ${AFTER_VERSION})

ROOT_GENERATE_DICTIONARY(G__ASImageGui TASPaletteEditor.h LINKDEF LinkDefGui.h)
ROOT_GENERATE_ROOTMAP(ASImageGui LINKDEF LinkDefGui.h)
ROOT_LINKER_LIBRARY(ASImageGui TASPaletteEditor.cxx G__ASImageGui.cxx LIBRARIES  ASImage )

ROOT_INSTALL_HEADERS()
