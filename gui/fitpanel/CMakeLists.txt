############################################################################
# CMakeLists.txt file for building ROOT gui/fitpanel package
############################################################################

ROOT_USE_PACKAGE(gui/gui)
ROOT_USE_PACKAGE(tree/treeplayer)

ROOT_STANDARD_LIBRARY_PACKAGE(FitPanel DEPENDENCIES Hist Gpad Graf Gui Tree MathCore)

