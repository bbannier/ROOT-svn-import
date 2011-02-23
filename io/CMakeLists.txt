add_subdirectory(io)  # special CMakeListst.txt 
add_subdirectory(sql)
if(xml)
  add_subdirectory(xml)
  add_subdirectory(xmlparser)
endif()
