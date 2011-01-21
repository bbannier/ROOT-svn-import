#
Set(BuildFreeType TRUE)
ROOT_USE_PACKAGE(graf2d/gpad)
ROOT_USE_PACKAGE(graf2d/graf)

If(BuildFreeType)
  Add_Subdirectory(freetype)
  Set(FREETYPE_INCLUDE_DIR ${CMAKE_BINARY_DIR}/graf2d/freetype/${FREETYPE_VERSION}/include)
  Set(FREETYPE_LIBRARIES "-L${LIBRARY_OUTPUT_PATH} -lfreetype")
EndIf(BuildFreeType)

Add_Subdirectory(gpad)
Add_Subdirectory(graf)
Add_Subdirectory(postscript)
Add_Subdirectory(x11)         # special CMakeLists.txt
Add_Subdirectory(x11ttf)      # special CMakeLists.txt
Add_Subdirectory(asimage)     # special CMakeLists.txt

Set(RootConfigFeatures ${RootConfigFeatures} PARENT_SCOPE)
