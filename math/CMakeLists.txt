ROOT_USE_PACKAGE(math/mathcore)
ROOT_USE_PACKAGE(math/matrix)

add_subdirectory(mathcore)
if(enable_mathmore)
  add_subdirectory(mathmore)
endif()
add_subdirectory(matrix)
add_subdirectory(minuit)
add_subdirectory(fumili)
add_subdirectory(physics)
add_subdirectory(mlp)
add_subdirectory(quadp)
add_subdirectory(foam)
add_subdirectory(smatrix)   #special CMakeList.txt
add_subdirectory(splot)
add_subdirectory(mathcore/test EXCLUDE_FROM_ALL)
add_subdirectory(genvector)

