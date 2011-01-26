############################################################################
# CMakeLists.txt file for building ROOT core/newdelete package
############################################################################

ROOT_USE_PACKAGE(io/io)
ROOT_LINKER_LIBRARY(New *.cxx)
ROOT_INSTALL_HEADERS()

