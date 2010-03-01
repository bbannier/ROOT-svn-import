MACRO_GET_TARGET_DIRECTORY(Reflex REFLEX_OUT_DIR)

IF(WIN32)
   ADD_EXECUTABLE(reflex-ldd reflex-ldd.cpp bindeps.h bindeps.cpp)
   GET_TARGET_PROPERTY(REFLEX_LDD_OUTPUT reflex-ldd LOCATION)
   ADD_CUSTOM_COMMAND(TARGET reflex-ldd POST_BUILD 
                      COMMAND ${CMAKE_COMMAND} -E copy ${REFLEX_LDD_OUTPUT} ${REFLEX_OUT_DIR}
                      COMMENT "copying reflex-ldd next to reflex")
   INSTALL(TARGETS reflex-ldd RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
ELSE(WIN32)
   ADD_CUSTOM_TARGET(reflex-ldd ALL
                     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/reflex-ldd ${REFLEX_OUT_DIR}
                     COMMAND chmod a+x ${REFLEX_OUT_DIR}/reflex-ldd
                     COMMENT "copying reflex-ldd to ${REFLEX_OUT_DIR}")
   INSTALL(PROGRAMS reflex-ldd DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
ENDIF(WIN32)
