add_subdirectory(net)
add_subdirectory(auth)  # special CMakeListst.txt
add_subdirectory(netx)
if(bonjour)
  add_subdirectory(bonjour)
endif()
if(krb5)
  add_subdirectory(krb5auth)
endif()
