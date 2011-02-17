############################################################################
# CMakeLists.txt file for building ROOT graf3d/gl package
############################################################################


ROOT_GENERATE_DICTIONARY(G__Eve1 ${headers1} LINKDEF src/SolarisCCDictHack.h LinkDef1.h)
ROOT_GENERATE_DICTIONARY(G__Eve2 ${headers2} LINKDEF src/SolarisCCDictHack.h LinkDef2.h)

ROOT_GENERATE_ROOTMAP(Eve LINKDEF LinkDef1.h LinkDef2.h DEPENDENCIES Geom GeomPainter Graf3D Gui)
ROOT_LINKER_LIBRARY(Eve *.cxx G__Eve1.cxx G__Eve2.cxx LIBRARIES)

ROOT_INSTALL_HEADERS()
