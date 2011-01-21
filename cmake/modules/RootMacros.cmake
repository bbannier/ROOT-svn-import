  ###########################################
  #
  #       Usefull macros
  #
  ###########################################

  ###############################################################
  #
  # Exchange file extention of LIST from
  # FILE_EXT1 to FILE_EXT2 and assign the
  # newly created list to OUTVAR. The input
  # list LIST is not changed at all
  # Ex: CHANGE_FILE_EXTENSION(*.cxx *.h TRD_HEADERS "${TRD_SRCS}")
  #
  ################################################################

#MACRO (CHANGE_FILE_EXTENSION FILE_EXT1 FILE_EXT2 OUTVAR LIST)
#
#   SET(BLA)
#
#   IF (${FILE_EXT1} MATCHES "^[*][.]+.*$")
#     STRING(REGEX REPLACE "^[*]+([.].*)$" "\\1" FILE_EXT1_NEW ${FILE_EXT1}) 
#   ENDIF  (${FILE_EXT1} MATCHES "^[*][.]+.*$")
#
#   IF (${FILE_EXT2} MATCHES "^[*][.]+.*$")
#     STRING(REGEX REPLACE "^[*]+([.].*)" "\\1" FILE_EXT2_NEW ${FILE_EXT2}) 
#   ENDIF  (${FILE_EXT2} MATCHES "^[*][.]+.*$")
#
#   foreach (_current_FILE ${LIST})
#
#     STRING(REGEX REPLACE "^(.*)${FILE_EXT1_NEW}$" "\\1${FILE_EXT2_NEW}" test ${_current_FILE})
#     SET (BLA ${BLA} ${test})
#
#   endforeach (_current_FILE ${ARGN})
#   
#   SET (${OUTVAR} ${BLA})
#
#
#
#ENDMACRO (CHANGE_FILE_EXTENSION)

  ######################################################
  # 
  # Macro get string with a colon seperated string of
  # pathes or any other colon sperated list.
  # First the string is seperated  and the entries are
  # filled into a list.
  # Loop over the list and searches for the occurence
  # of keywords which are provided as a list.
  # If the keyword occurs this path (entry) is
  # deleted from the list. Returns the list of pathes
  # (entries) wich survives the loop.
  #
  # PATH: colon separated string of pathes or other 
  #       input entries
  # LIST_OF_KEYWORDS: list of the keywords which 
  #                   should be excluded in the output
  # OUTVAR: name of the variable which should be filled
  #         with the resulting output list
  #
  ######################################################

MACRO (CLEAN_PATH_LIST PATH LIST_OF_KEYWORDS OUTVAR)

  SET(BLA "")

  STRING(REGEX MATCHALL "[^:]+" PATH1 ${PATH})

  FOREACH(_current_PATH ${PATH1})
 
    SET(KEYWORD_FOUND FALSE)

    FOREACH(_current_KEYWORD ${LIST_OF_KEYWORDS})

      IF (${_current_PATH} MATCHES "${_current_KEYWORD}")
        SET(KEYWORD_FOUND TRUE)
      ENDIF (${_current_PATH} MATCHES "${_current_KEYWORD}")

    ENDFOREACH(_current_KEYWORD ${LIST_OF_KEYWORDS})
    
    IF (NOT KEYWORD_FOUND)
      SET(BLA ${BLA} ${_current_PATH})
    ENDIF (NOT KEYWORD_FOUND)  

  ENDFOREACH(_current_PATH ${PATH1})

  UNIQUE(${OUTVAR} "${BLA}")

ENDMACRO (CLEAN_PATH_LIST)

  ##########################################################
  #
  # The macro checks if the user wants to build the project
  # in the source directory and if so stop the execution
  # of cmake with an error message.
  #
  ##########################################################

MACRO (CHECK_OUT_OF_SOURCE_BUILD)

   STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" insource)
   IF(insource)
      FILE(REMOVE_RECURSE ${CMAKE_SOURCE_DIR}/Testing)
      FILE(REMOVE ${CMAKE_SOURCE_DIR}/DartConfiguration.tcl)
      MESSAGE(FATAL_ERROR "ROOT should be installed as an out of source build, to keep the source directory clean. Please create a extra build directory and run the command 'cmake <path_to_source_dir>' in this newly created directory. You have also to delete the directory CMakeFiles and the file CMakeCache.txt in the source directory. Otherwise cmake will complain even if you run it from an out-of-source directory.") 
   ENDIF(insource)

ENDMACRO (CHECK_OUT_OF_SOURCE_BUILD)

MACRO(UNIQUE var_name list)

  #######################################################################
  # Make the given list have only one instance of each unique element and
  # store it in var_name.
  #######################################################################

  SET(unique_tmp "")
  FOREACH(l ${list})
    STRING(REGEX REPLACE "[+]" "\\\\+" l1 ${l})
    IF(NOT "${unique_tmp}" MATCHES "(^|;)${l1}(;|$)")
      SET(unique_tmp ${unique_tmp} ${l})
    ENDIF(NOT "${unique_tmp}" MATCHES "(^|;)${l1}(;|$)")
  ENDFOREACH(l)
  SET(${var_name} ${unique_tmp})
ENDMACRO(UNIQUE)


###################################################
# Creates a variable which stores the intersection 
# between two lists
####################################################

MACRO(INTERSECTION var_name list1 list2)
  # Store the intersection between the two given lists in var_name.
  SET(intersect_tmp "")
  FOREACH(l ${list1})
    IF("${list2}" MATCHES "(^|;)${l}(;|$)")
      SET(intersect_tmp ${intersect_tmp} ${l})
    ENDIF("${list2}" MATCHES "(^|;)${l}(;|$)")
  ENDFOREACH(l)
  SET(${var_name} ${intersect_tmp})
ENDMACRO(INTERSECTION)

MACRO(REMOVE_FROM_LIST var_name list1 list2)
  # Remove elements in list2 from list1 and store the result in var_name.
  SET(filter_tmp "")
  FOREACH(l ${list1})
    IF(NOT "${list2}" MATCHES "(^|;)${l}(;|$)")
      SET(filter_tmp ${filter_tmp} ${l})
    ENDIF(NOT "${list2}" MATCHES "(^|;)${l}(;|$)")
  ENDFOREACH(l)
  SET(${var_name} ${filter_tmp})
ENDMACRO(REMOVE_FROM_LIST)

#############################################
# Macro which add all given header files to
# an environment variable. These files will
# be copied in the end to the include directory
#############################################

MACRO(INSTALL_HEADERS target_name)# _directory)
  If(${ARGC} EQUAL 2)
     Set(BLA $ENV{HeaderDirectories})
     List(FIND BLA ${ARGN} _elem_found)
     If(${_elem_found} EQUAL -1)
       Set(BLA ${BLA} ${ARGN})
       Set(ENV{HeaderDirectories} "${BLA}")
     EndIf(${_elem_found} EQUAL -1)   
     Set(BLA $ENV{HeadersToInstall_${ARGN}})
     Set(BLA  ${BLA} ${${target_name}})
     set(ENV{HeadersToInstall_${ARGN}} "${BLA}")
#     Add_Custom_Target(${Target_name}
#                       DEPENDS ${${target_name}}
#                      )
  Else(${ARGC} EQUAL 2)
    Set(BLA $ENV{HeadersToInstall})
    Set(BLA  ${BLA} ${${target_name}})
    set(ENV{HeadersToInstall} "${BLA}")
  EndIf(${ARGC} EQUAL 2)
    
ENDMACRO(INSTALL_HEADERS)

#############################################
# Macro which creates a target to copy all
# Headerfiles given to the include directory
#############################################

MACRO(INSTALL_ALL_HEADERS)
  Set(AllCopiedHeaders)
  ForEach(_value $ENV{HeadersToInstall})
    Get_Filename_Component(_name ${_value} NAME) 
    List(APPEND AllCopiedHeaders ${HEADER_OUTPUT_PATH}/${_name})
  EndForEach(_value $ENV{HeadersToInstall})
  Add_Custom_Command(OUTPUT ${AllCopiedHeaders}
                     COMMAND cp $ENV{HeadersToInstall} 
                             ${HEADER_OUTPUT_PATH}
                    )

  ForEach(_directory $ENV{HeaderDirectories})
    Set(_AllCopiedHeaders)    
    ForEach(_value $ENV{HeadersToInstall_${_directory}})
      Get_Filename_Component(_name ${_value} NAME) 
      List(APPEND _AllCopiedHeaders ${HEADER_OUTPUT_PATH}/${_directory}/${_name})
      List(APPEND AllCopiedHeaders ${HEADER_OUTPUT_PATH}/${_directory}/${_name})
    EndForEach(_value $ENV{HeadersToInstall_${_directory}})
    Add_Custom_Command(OUTPUT ${_AllCopiedHeaders}
                       COMMAND cmake -E make_directory ${HEADER_OUTPUT_PATH}/${_directory}
                       COMMAND cp $ENV{HeadersToInstall_${_directory}} 
                               ${HEADER_OUTPUT_PATH}/${_directory}
                      )
    List(APPEND AllCopiedHeaders ${_AllCopiedHeaders})
  EndForEach(_directory $ENV{HeaderDirectories})

  add_custom_target(INSTALL_HEADERS 
                    DEPENDS ${AllCopiedHeaders} 
                   )
ENDMACRO(INSTALL_ALL_HEADERS)
#############################################
# Macro which creates a target to copy all
# Headerfiles given to the include directory
#############################################

#MACRO(INSTALL_HEADERS1 target_name)# _directory)
#  set(target_name_install "${target_name}_INSTALL")
#  set(BLA $ENV{ALL_HEADERS_INSTALL})
##  MESSAGE("BLA: ${BLA}")
#  set(BLA ${BLA} ${target_name_install})
##  MESSAGE("BLA: ${BLA}")
#  set(ENV{ALL_HEADERS_INSTALL} "${BLA}")
#
#  foreach(_value ${${target_name}})
#    get_filename_component(_name ${_value} NAME) 
#    If(${ARGC} EQUAL 2)
#      Set(_directory ${ARGN} )
#      List(APPEND "${target_name_install}" ${HEADER_OUTPUT_PATH}/${_directory}/${_name})
#      Add_Custom_Command(OUTPUT ${HEADER_OUTPUT_PATH}/${_directory}/${_name}
#                         COMMAND cmake -E copy ${_value} 
#                                 ${HEADER_OUTPUT_PATH}/${_directory}/${_name}
#                        )
#    Else(${ARGC} EQUAL 2)
#      List(APPEND "${target_name_install}" ${HEADER_OUTPUT_PATH}/${_name})
#      Add_Custom_Command(OUTPUT ${HEADER_OUTPUT_PATH}/${_name}
#                         COMMAND cmake -E copy ${_value} 
#                                 ${HEADER_OUTPUT_PATH}/${_name}
#                        )
#    EndIf(${ARGC} EQUAL 2)
#  endforeach(_value ${header_file_list})
#
#  add_custom_target(${target_name_install} 
#                    COMMAND touch ${CMAKE_BINARY_DIR}/${target_name_install}
#                    DEPENDS ${${target_name_install}} 
#                   )
##  set(${target_name_install} ${${target_name_install}} PARENT_SCOPE)
##  MESSAGE("${target_name_install}: ${${target_name_install}}")
#ENDMACRO(INSTALL_HEADERS1)

##MACRO(INSTALL_HEADERS_OLD header_file_list target_name)
##  set(target_name_install "${target_name}_INSTALL")
###  MESSAGE("TNI: ${target_name_install}") 
##  foreach(_value ${header_file_list})
###    MESSAGE("VALUE: ${_value}")
##    get_filename_component(_name ${_value} NAME) 
##    list(APPEND "${target_name_install}" ${HEADER_OUTPUT_PATH}/${_name})
###    MESSAGE("LIST: ${${target_name_install}}")
##    add_custom_command(OUTPUT ${HEADER_OUTPUT_PATH}/${_name}
##                       COMMAND cmake -E copy ${_value} 
##                               ${HEADER_OUTPUT_PATH}/${_name}
##                      )
##
##  endforeach(_value ${header_file_list})
##
##
###    add_custom_command(OUTPUT ${${target_name_install}}
###                       COMMAND cmake -E copy ${header_file_list} 
###                               ${HEADER_OUTPUT_PATH}
###                      )
##
##  add_custom_target(${target_name} ALL
##                    COMMAND touch ${CMAKE_BINARY_DIR}/${target_name}
##                    DEPENDS ${${target_name_install}}
##                   )
##ENDMACRO(INSTALL_HEADERS_OLD)

MACRO (GENERATE_DICTIONARY INFILES LINKDEF_FILE OUTFILE INCLUDE_DIRS_IN MACROS)
 
  set(_INCLUDE_DIRS -I${HEADER_OUTPUT_PATH})

  foreach (_current_FILE ${INCLUDE_DIRS_IN})
    set(_INCLUDE_DIRS ${_INCLUDE_DIRS} -I${_current_FILE})
  endforeach (_current_FILE ${INCLUDE_DIRS_IN})
 

#  MESSAGE("INFILES: ${INFILES}")
#  MESSAGE("OutFILE: ${OUTFILE}")
#  MESSAGE("LINKDEF_FILE: ${LINKDEF_FILE}")
#  MESSAGE("INCLUDE_DIRS: ${_INCLUDE_DIRS}")

  STRING(REGEX REPLACE "^(.*)\\.(.*)$" "\\1.h" bla "${OUTFILE}")
#  MESSAGE("BLA: ${bla}")
  SET (OUTFILES ${OUTFILE} ${bla})
#  MESSAGE("OutFILES: ${OUTFILES}")

  ADD_CUSTOM_COMMAND(OUTPUT ${OUTFILES}
                     COMMAND rootcint_tmp -cint -f ${OUTFILE} -c ${MACROS} ${_INCLUDE_DIRS} ${INFILES} ${LINKDEF_FILE} 
                     DEPENDS ${INFILES} ${LINKDEF_FILE} rootcint_tmp
                     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
   )

ENDMACRO (GENERATE_DICTIONARY)

Macro (GENERATE_ROOTMAP LINKDEF_FILE LIBNAME)
 
  Get_Filename_Component(_libname ${LIBNAME} NAME_WE)
  Get_Filename_Component(_path ${LIBNAME} PATH)
#  Message("_libname: ${_libname}")
  Set(OUTFILE "${LIBRARY_OUTPUT_PATH}/lib${LIBNAME}.rootmap")
  Set(INFILE "${LIBRARY_OUTPUT_PATH}/lib${LIBNAME}.so")
#  Message("OutFILE: ${OUTFILE}")
#  Message("InFILE: ${INFILE}")
 
  If(${ARGC} GREATER 2)
    Set(_dependencies)
    ForEach(_file ${ARGN})
      Set(_dependencies ${_dependencies} ${LIBRARY_OUTPUT_PATH}/lib${_file}.so)
    EndForEach(_file ${ARGN})
  Else(${ARGC} GREATER 2)
    Set(_dependencies "")
  EndIf(${ARGC} GREATER 2)

#  Message("Dependency for ${LIBNAME}: ${_dependencies}")

  Add_Custom_Command(OUTPUT ${OUTFILE}
                     COMMAND rlibmap -o ${OUTFILE} -l ${INFILE} -d ${_dependencies} -c ${LINKDEF_FILE} 
                     DEPENDS ${LIBNAME} ${LINKDEF_FILE} rlibmap ${_dependencies}
                    )
  Install(FILES ${OUTFILE} DESTINATION ${INCLUDE_INSTALL_DIR})

  Add_Custom_Target(${LIBNAME}.rootmap ALL
                    DEPENDS ${OUTFILE}
                   )

EndMacro (GENERATE_ROOTMAP LINKDEF_FILE LIBNAME)



MACRO(filter_out _orig_list _to_remove)
#  MESSAGE("VALUE: ${_orig_list}")  
  list(REMOVE_ITEM ${_orig_list} "${CMAKE_CURRENT_SOURCE_DIR}" )
  foreach(_value ${_to_remove})
#    MESSAGE("VALUE: ${_value}")
    set(_new_value "${CMAKE_CURRENT_SOURCE_DIR}/inc/${_value}")
#    MESSAGE("NEW VALUE: ${_new_value}")
    list(REMOVE_ITEM ${_orig_list} ${_new_value})
  endforeach(_value ${_to_remove})
ENDMACRO(filter_out _list)

Macro(Create_Standard_Root_library)
  # This macro is used to create ROOT libraries and rootmap files
  # which are build out of all files found in the src and inc
  # subdirectory of the current working dir.
  # The user has to define the variables libname and DEPENDENCIES
  # before calling the macro. libname is the name of the library
  # to be created. DEPENDENCIES are the libraries which lib${libname}
  # depends on. In the moment these libraries are explicitly linked
  # to the new library
  # TODO: Only do explicit link if requested.

  #  Message("Create ${libname} with dpendencies ${DEPENDENCIES}")

  # Find all headers except the LinkDef file and copy
  # them to the include subdirectory of the build directory
  # All header files will be also installed to the final destination
  # when doing a "make install"
  File(GLOB ${libname}Headers "${CMAKE_CURRENT_SOURCE_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h"
      )
  List(REMOVE_ITEM ${libname}Headers
                   "${CMAKE_CURRENT_SOURCE_DIR}"
                   "${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef.h"
      )

  INSTALL_HEADERS("${libname}Headers")

  # Find all source files which are needed to build the library
  File(GLOB ${libname}Source "${CMAKE_CURRENT_SOURCE_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
      )
  List(REMOVE_ITEM ${libname}Source
                   "${CMAKE_CURRENT_SOURCE_DIR}"
      )

  # Define the LinkDef and the output file which are needed
  # to create the dictionary. 
  # Definde the needed include directories and create the Dictionary
  Set(LinkDef ${CMAKE_CURRENT_SOURCE_DIR}/inc/LinkDef.h)
  If(SpecialDictionary)
    Set(Dictionary ${CMAKE_CURRENT_BINARY_DIR}/${SpecialDictionary})
  Else(SpecialDictionary)
    Set(Dictionary ${CMAKE_CURRENT_BINARY_DIR}/G__${libname}.cxx)
  EndIf(SpecialDictionary)

  #TODO: Check if this is really needed
  Set(INCLUDE_DIRS
#      ${CMAKE_SOURCE_DIR}/hist/hist/inc
      ${CMAKE_SOURCE_DIR}/cint/cint/include
      ${CMAKE_SOURCE_DIR}/cint/cint/stl
      ${CMAKE_SOURCE_DIR}/cint/cint/lib
     )

  GENERATE_DICTIONARY("${${libname}Headers}"
                      "${LinkDef}" "${Dictionary}"
                      "${INCLUDE_DIRS}" ""
                     )

  Set(INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/inc
      ${ROOT_INCLUDE_DIR}
     )

  GENERATE_ROOTMAP("${LinkDef}" ${libname} ${DEPENDENCIES})

  include_directories( ${INCLUDE_DIRECTORIES})

  # Here all objects ar put together into one share library
  Add_Library(${libname} SHARED ${${libname}Source} ${Dictionary})
  Target_Link_Libraries(${libname} ${DEPENDENCIES} ${CMAKE_THREAD_LIBS_INIT})
  Set_Target_properties(${libname} PROPERTIES ${ROOT_LIBRARY_PROPERTIES})
  Install(TARGETS ${libname} DESTINATION ${LIB_INSTALL_DIR})


  # Define all the header files which should be installed when
  # doing a "make install"
  Install(FILES ${${libname}Headers} DESTINATION ${INCLUDE_INSTALL_DIR})

EndMacro(Create_Standard_Root_library)


