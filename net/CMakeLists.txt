add_subdirectory(net)
add_subdirectory(auth)  # special CMakeListst.txt
if(bonjour)
  add_subdirectory(bonjour)
endif()