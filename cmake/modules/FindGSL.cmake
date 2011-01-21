#set(GSL_native_version ${GSL_config_version})
#set(GSL_home ${LCG_external}/GSL/${GSL_native_version}/${LCG_system})

set(GSL_home /afs/cern.ch/sw/lcg/external/GSL/1.10/x86_64-slc5-gcc43-opt)

set(GSL_FOUND 1)
set(GSL_INCLUDE_DIRS ${GSL_home}/include)
set(GSL_LIBRARY_DIRS ${GSL_home}/lib)
set(GSL_LIBRARIES gsl gslcblas)

if(WIN32)
  add_definitions(-DGSL_DLL)
  set(GSL_environment PATH+=${GSL_home}/lib)
else()
  set(GSL_environment LD_LIBRARY_PATH+=${GSL_home}/lib)
endif()