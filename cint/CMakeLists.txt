# To generate the dictonaries rootcint_tmp has to run in $ROOTSYS with the
# include files in $ROOTSYS/include.
# This can't be done in a separate build directory, but what can be done is
# to simulate the setup by symbolic links for some cint directories. It looks
# like this is enough.
#Execute_Process(COMMAND cmake -E create_symlink ${CMAKE_SOURCE_DIR}/cint/cint/include ${CMAKE_BINARY_DIR}/cint/cint/include 
#                COMMAND cmake -E create_symlink ${CMAKE_SOURCE_DIR}/cint/cint/lib ${CMAKE_BINARY_DIR}/cint/cint/lib 
#                COMMAND cmake -E create_symlink ${CMAKE_SOURCE_DIR}/cint/cint/stl ${CMAKE_BINARY_DIR}/cint/cint/stl 
#)
Execute_Process(COMMAND cmake -E make_directory ${CMAKE_BINARY_DIR}/cint/cint 
                COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/cint/cint/include ${CMAKE_BINARY_DIR}/cint/cint/include 
                COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/cint/cint/lib ${CMAKE_BINARY_DIR}/cint/cint/lib
                COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/cint/cint/stl ${CMAKE_BINARY_DIR}/cint/cint/stl 
)


Add_Subdirectory(cint)
#Add_Subdirectory(test)
Add_Subdirectory(ROOT)
## TODO: Add cling support
#Add_Subdirectory(cling)

# The following line is needed to transport the definition
# to the root scope, because the variable is needed also in
# the core subdirectory
# A other solution would be to call cint/cint directly from
# the main CMakeLists.txt.
# Don't know which alternative is the better one.
Set(CINT_TMP_SRCS ${CINT_TMP_SRCS} PARENT_SCOPE)
Set(CINT_SRCS ${CINT_SRCS} PARENT_SCOPE)
