#==============================================================================#
#                                                                              #
#  Copyright (c) 2011 maidsafe.net limited                                     #
#  All rights reserved.                                                        #
#                                                                              #
#  Redistribution and use in source and binary forms, with or without          #
#  modification, are permitted provided that the following conditions are met: #
#                                                                              #
#      * Redistributions of source code must retain the above copyright        #
#        notice, this list of conditions and the following disclaimer.         #
#      * Redistributions in binary form must reproduce the above copyright     #
#        notice, this list of conditions and the following disclaimer in the   #
#        documentation and/or other materials provided with the distribution.  #
#      * Neither the name of the maidsafe.net limited nor the names of its     #
#        contributors may be used to endorse or promote products derived from  #
#        this software without specific prior written permission.              #
#                                                                              #
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" #
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   #
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  #
#  ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE  #
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         #
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF        #
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    #
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN     #
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)     #
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  #
#  POSSIBILITY OF SUCH DAMAGE.                                                 #
#                                                                              #
#==============================================================================#
#                                                                              #
#  Written by maidsafe.net team                                                #
#                                                                              #
#==============================================================================#
#                                                                              #
#  Module used to create standard setup of main CMakeLists.txt                 #
#                                                                              #
#==============================================================================#


SET(MAIDSAFE_TEST_TYPE_MESSAGE "Tests included: All")
IF(NOT MAIDSAFE_TEST_TYPE)
  SET(MAIDSAFE_TEST_TYPE "ALL" CACHE STRING "Choose the type of TEST, options are: ALL, BEH, FUNC" FORCE)
ELSE()
  IF(${MAIDSAFE_TEST_TYPE} MATCHES BEH)
    SET(MAIDSAFE_TEST_TYPE_MESSAGE "Tests included: Behavioural")
  ELSEIF(${MAIDSAFE_TEST_TYPE} MATCHES FUNC)
    SET(MAIDSAFE_TEST_TYPE_MESSAGE "Tests included: Functional")
  ELSE()
    SET(MAIDSAFE_TEST_TYPE "ALL" CACHE STRING "Choose the type of TEST, options are: ALL BEH FUNC" FORCE)
  ENDIF()
ENDIF()

ENABLE_TESTING()
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

IF(UNIX)
  EXECUTE_PROCESS(COMMAND date +%a%d%m%y OUTPUT_VARIABLE pddate OUTPUT_STRIP_TRAILING_WHITESPACE)
ELSE()
  EXECUTE_PROCESS(COMMAND CMD /C DATE /T OUTPUT_VARIABLE pddate OUTPUT_STRIP_TRAILING_WHITESPACE)
  STRING(REPLACE "/" "" pddate ${pddate})
  STRING(SUBSTRING ${pddate} 0 4 pddateddmm)
  STRING(SUBSTRING ${pddate} 6 2 pddateyy)
  SET(pddate ${pddateddmm}${pddateyy})
  STRING(REPLACE " " "_" pddate ${pddate})
ENDIF()

IF(APPLE)
  SET(CMAKE_OSX_SYSROOT "/")
ENDIF()

IF(NOT DEFINED ${KDEV})
  SET(CMAKE_DEBUG_POSTFIX -d)
  SET(CMAKE_RELWITHDEBINFO_POSTFIX -rwdi)
  SET(CMAKE_MINSIZEREL_POSTFIX -msr)

  IF(CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(TEST_POSTFIX ${CMAKE_DEBUG_POSTFIX})
  ELSEIF(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    SET(TEST_POSTFIX ${CMAKE_RELWITHDEBINFO_POSTFIX})
  ELSEIF(CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
    SET(TEST_POSTFIX ${CMAKE_MINSIZEREL_POSTFIX})
  ENDIF()
ENDIF()

INCLUDE_DIRECTORIES(BEFORE SYSTEM ${MaidSafeCommon_INCLUDE_DIR} ${MaidSafeCommon_INCLUDE_DIR}/maidsafe)
SET(INCLUDE_DIRS ${MaidSafeCommon_INCLUDE_DIR} ${MaidSafeCommon_INCLUDE_DIR}/maidsafe ${INCLUDE_DIRS})
IF(Boost_INCLUDE_DIR)
  INCLUDE_DIRECTORIES(AFTER SYSTEM ${Boost_INCLUDE_DIR})
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
ENDIF()

# Create CTestCustom.cmake to avoid inclusion of coverage results from third-party code
FILE(WRITE ${PROJECT_BINARY_DIR}/CTestCustom.cmake "SET(CTEST_CUSTOM_COVERAGE_EXCLUDE")
FILE(APPEND ${PROJECT_BINARY_DIR}/CTestCustom.cmake " \".pb.\"")
FILE(APPEND ${PROJECT_BINARY_DIR}/CTestCustom.cmake " \"cryptopp/\"")
FILE(APPEND ${PROJECT_BINARY_DIR}/CTestCustom.cmake " \"distributed_network/\"")
FILE(APPEND ${PROJECT_BINARY_DIR}/CTestCustom.cmake " \"libupnp/\"")
FILE(APPEND ${PROJECT_BINARY_DIR}/CTestCustom.cmake " \"nat-pmp/\"")
FILE(APPEND ${PROJECT_BINARY_DIR}/CTestCustom.cmake " \"tests/\"")
FILE(APPEND ${PROJECT_BINARY_DIR}/CTestCustom.cmake " \"third_party_libs/\"")
FILE(APPEND ${PROJECT_BINARY_DIR}/CTestCustom.cmake " \"upnp/\"")
FILE(APPEND ${PROJECT_BINARY_DIR}/CTestCustom.cmake " \"main.cc\"")
FILE(APPEND ${PROJECT_BINARY_DIR}/CTestCustom.cmake ")\n\n")

IF(DEFINED ADD_LIBRARY_DIR)
  SET(DEFAULT_LIBRARY_DIR ${DEFAULT_LIBRARY_DIR} ${ADD_LIBRARY_DIR})
  LIST(REMOVE_DUPLICATES DEFAULT_LIBRARY_DIR)
  SET(DEFAULT_LIBRARY_DIR ${DEFAULT_LIBRARY_DIR} CACHE PATH "Path to libraries directories" FORCE)
ENDIF()

SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${DEFAULT_LIBRARY_DIR})
INCLUDE(maidsafe_run_protoc)

###################################################################################################
# MySQL++ library search                                                                          #
###################################################################################################
#INCLUDE(maidsafe_find_mysqlpp)
#IF(Mysqlpp_FOUND)
#  SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${Mysqlpp_INCLUDE_DIR} ${Mysql_INCLUDE_DIR})
#ENDIF()

###################################################################################################
# Python library search                                                                           #
###################################################################################################
UNSET(PYTHON_EXECUTABLE CACHE)
INCLUDE(FindPythonInterp)
FIND_PACKAGE(PythonInterp)
IF(PYTHONINTERP_FOUND)
  MESSAGE("-- Found python executable: style checking enabled.")
ELSE()
  MESSAGE("-- Didn't find python executable: style checking disabled.")
ENDIF()

###################################################################################################
# All other libraries search                                                                      #
###################################################################################################
IF(UNIX)
  SET(CMAKE_THREAD_PREFER_PTHREAD true)
  FIND_PACKAGE(Threads REQUIRED)
  SET(SYS_LIB ${CMAKE_THREAD_LIBS_INIT})
ELSEIF(WIN32)
  IF(MSVC)
    SET(SYS_LIB ws2_32 odbc32 odbccp32 WSock32 IPHlpApi)
  ELSE()
    SET(SYS_LIB advapi32 kernel32 ws2_32 iphlpapi mswsock)
  ENDIF()
  FOREACH(library ${SYS_LIB})
    FIND_LIBRARY(CURRENT_LIB ${library})
    IF(CURRENT_LIB)
      MESSAGE("-- Found library ${CURRENT_LIB}")
    ELSE()
      SET(ERROR_MESSAGE "\nCould not find library ${library}.")
      IF(MSVC)
        SET(ERROR_MESSAGE "${ERROR_MESSAGE}\nRun cmake from a Visual Studio Command Prompt.")
      ELSE()
        SET(ERROR_MESSAGE "${ERROR_MESSAGE}  Run\n${ERROR_MESSAGE_CMAKE_PATH} -DADD_LIBRARY_DIR=<Path to ${library} directory>")
      ENDIF()
      MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
    ENDIF()
    UNSET(CURRENT_LIB CACHE)
  ENDFOREACH()
ENDIF()

INCLUDE_DIRECTORIES(BEFORE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR})
SET(INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR} ${INCLUDE_DIRS})

LINK_DIRECTORIES(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

MESSAGE("================================================================================")

IF(MSVC)
  SET(CMAKE_CXX_FLAGS "")
  SET(CMAKE_CXX_FLAGS_INIT "")
  SET(CMAKE_CXX_FLAGS_RELEASE "")
  SET(CMAKE_CXX_FLAGS_DEBUG "")
ENDIF()

# When this target is built, it removes all .gcda files from the build directory and its subdirectories
IF(UNIX)
  FIND_FILE(CLEAN_COVERAGE clean_coverage.cmake ${CMAKE_MODULE_PATH})
  IF(CLEAN_COVERAGE)
    ADD_CUSTOM_TARGET(CleanCoverage COMMAND ${CMAKE_COMMAND} -DSEARCH_DIR=${CMAKE_BINARY_DIR} -P ${CLEAN_COVERAGE})
  ENDIF()
ENDIF()

SET(CTEST_CUSTOM_MAXIMUM_PASSED_TEST_OUTPUT_SIZE 50000)
SET(CTEST_CUSTOM_MAXIMUM_FAILED_TEST_OUTPUT_SIZE 50000)
SET(CTEST_CONTINUOUS_DURATION 600)
SET(CTEST_CONTINUOUS_MINIMUM_INTERVAL 10)
SET(CTEST_START_WITH_EMPTY_BINARY_DIRECTORY true)

IF(UNIX)
  SET(MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --quiet --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=50 --verbose --demangle=yes")
ENDIF()

INCLUDE(CTest)
INCLUDE(maidsafe_add_gtests)

IF(MAIDSAFE_COMMON_INSTALL_DIR)
  FILE(TO_CMAKE_PATH ${MAIDSAFE_COMMON_INSTALL_DIR} CMAKE_INSTALL_PREFIX)
ENDIF()
IF(INSTALL_PREFIX)
  FILE(TO_CMAKE_PATH ${INSTALL_PREFIX} INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "${INSTALL_PREFIX}")
ENDIF()
FILE(TO_NATIVE_PATH ${CMAKE_INSTALL_PREFIX} CMAKE_INSTALL_PREFIX_MESSAGE)


###################################################################################################
# Cleanup of temporary test folders                                                               #
###################################################################################################
IF(WIN32)
  IF(NOT CLEAN_TEMP)
    SET(CLEAN_TEMP "OFF" CACHE INTERNAL "Cleanup of temp test folders, options are: ONCE, OFF, ALWAYS" FORCE)
  ENDIF(NOT CLEAN_TEMP)
  EXECUTE_PROCESS(COMMAND CMD /C ECHO %TEMP% OUTPUT_VARIABLE temp_path OUTPUT_STRIP_TRAILING_WHITESPACE)
  STRING(REPLACE "\\" "/" temp_path ${temp_path})
  FILE(GLOB maidsafe_temp_dirs ${temp_path}/MaidSafe_Test*)
  FILE(GLOB sigmoid_temp_dirs ${temp_path}/Sigmoid_Test*)
  SET(temp_dirs ${maidsafe_temp_dirs} ${sigmoid_temp_dirs})
  LIST(LENGTH temp_dirs temp_dir_count)
  IF(NOT ${temp_dir_count} EQUAL 0)
    MESSAGE("")
    IF(CLEAN_TEMP MATCHES ONCE OR CLEAN_TEMP MATCHES ALWAYS)
      MESSAGE("Cleaning up temporary test folders.\n")
      FOREACH(temp_dir ${temp_dirs})
        FILE(REMOVE_RECURSE ${temp_dir})
        MESSAGE("-- Removed ${temp_dir}")
      ENDFOREACH()
    ELSE()
      MESSAGE("The following temporary test folders could be cleaned up:\n")
      FOREACH(temp_dir ${temp_dirs})
        MESSAGE("-- Found ${temp_dir}")
      ENDFOREACH()
      MESSAGE("")
      MESSAGE("To cleanup, run cmake ../.. -DCLEAN_TEMP=ONCE or cmake ../.. -DCLEAN_TEMP=ALWAYS")
    ENDIF()
    MESSAGE("================================================================================")
  ENDIF()
  IF(NOT CLEAN_TEMP MATCHES ALWAYS)
    SET(CLEAN_TEMP "OFF" CACHE INTERNAL "Cleanup of temp test folders, options are: ONCE, OFF, ALWAYS" FORCE)
  ENDIF()
ENDIF()

SET(CPACK_STRIP_FILES TRUE)

###################################################################################################
# Helper functions                                                                                #
###################################################################################################

# Force renaming of exes to match standard CMake library renaming policy
FUNCTION(RENAME_EXECUTABLE EXE)
  IF(NOT MSVC)
    SET_TARGET_PROPERTIES(${EXE} PROPERTIES
                            DEBUG_OUTPUT_NAME ${EXE}-d
                            RELWITHDEBINFO_OUTPUT_NAME ${EXE}-rwdi
                            MINSIZEREL_OUTPUT_NAME ${EXE}-msr)
  ENDIF()
ENDFUNCTION()

FUNCTION(TEST_SUMMARY_OUTPUT)
  LIST(LENGTH ALL_GTESTS GTEST_COUNT)
  MESSAGE("\n${MAIDSAFE_TEST_TYPE_MESSAGE}.   ${GTEST_COUNT} Google Tests.\n")
  MESSAGE("    To include all tests,                ${ERROR_MESSAGE_CMAKE_PATH} -DMAIDSAFE_TEST_TYPE=ALL")
  MESSAGE("    To include behavioural tests,        ${ERROR_MESSAGE_CMAKE_PATH} -DMAIDSAFE_TEST_TYPE=BEH")
  MESSAGE("    To include functional tests,        ${ERROR_MESSAGE_CMAKE_PATH} -DMAIDSAFE_TEST_TYPE=FUNC")
  MESSAGE("================================================================================")
ENDFUNCTION()

# Checks VERSION_H file for dependent MaidSafe library versions.  For a project
# with e.g. "#define MAIDSAFE_COMMON_VERSION 010215" in its version.h then the following
# variables are set:
# ${THIS_VERSION_FILE}            maidsafe/common/version.h
# ${THIS_VERSION}                 MAIDSAFE_COMMON_VERSION
# ${${THIS_VERSION}}              10215
# ${CPACK_PACKAGE_VERSION_MAJOR}  1
# ${CPACK_PACKAGE_VERSION_MINOR}  02
# ${CPACK_PACKAGE_VERSION_PATCH}  15
# ${CPACK_PACKAGE_VERSION}        1.02.15
# ${MAIDSAFE_LIBRARY_POSTFIX}     -1_02_15
# Also, the MAIDSAFE_LIBRARY_POSTFIX is appended to each of the CMAKE_<build type>_POSTFIX variables.
# Finally, a preprocessor definition CMAKE_${THIS_VERSION} is added to force a re-run of CMake
# after changing the version in version.h
FUNCTION(HANDLE_VERSIONS VERSION_H)
  FILE(GLOB THIS_VERSION_FILE RELATIVE ${PROJECT_SOURCE_DIR}/src ${VERSION_H})
  SET(THIS_VERSION_FILE ${THIS_VERSION_FILE} PARENT_SCOPE)
  FILE(STRINGS ${VERSION_H} VERSIONS REGEX "#define [A-Z_]+VERSION [0-9]+$")
  FOREACH(VERSN ${VERSIONS})
    STRING(REPLACE "#define " "" VERSN ${VERSN})
    STRING(REGEX REPLACE "[ ]+" ";" VERSN ${VERSN})
    LIST(GET VERSN 0 VERSION_VARIABLE_NAME)
    LIST(GET VERSN 1 VERSION_VARIABLE_VALUE)
    STRING(REGEX MATCH "THIS_NEEDS_" IS_DEPENDENCY ${VERSION_VARIABLE_NAME})
    IF(IS_DEPENDENCY)
      STRING(REGEX REPLACE "THIS_NEEDS_" "" DEPENDENCY_VARIABLE_NAME ${VERSION_VARIABLE_NAME})
      IF(NOT ${${DEPENDENCY_VARIABLE_NAME}} MATCHES ${VERSION_VARIABLE_VALUE})
        SET(ERROR_MESSAGE "\n\nThis project needs ${DEPENDENCY_VARIABLE_NAME} ${VERSION_VARIABLE_VALUE}\n")
        SET(ERROR_MESSAGE "${ERROR_MESSAGE}Found ${DEPENDENCY_VARIABLE_NAME} ${${DEPENDENCY_VARIABLE_NAME}}\n\n")
        MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
      ENDIF()
    ELSE()
      SET(${VERSION_VARIABLE_NAME} ${VERSION_VARIABLE_VALUE} PARENT_SCOPE)
      SET(THIS_VERSION ${VERSION_VARIABLE_NAME} PARENT_SCOPE)
      MATH(EXPR VERSION_MAJOR ${VERSION_VARIABLE_VALUE}/10000)
      MATH(EXPR VERSION_MINOR ${VERSION_VARIABLE_VALUE}/100%100)
      MATH(EXPR VERSION_PATCH ${VERSION_VARIABLE_VALUE}%100)
      IF(VERSION_MINOR LESS 10)
        SET(VERSION_MINOR 0${VERSION_MINOR})
      ENDIF()
      IF(VERSION_PATCH LESS 10)
        SET(VERSION_PATCH 0${VERSION_PATCH})
      ENDIF()
      SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR} PARENT_SCOPE)
      SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR} PARENT_SCOPE)
      SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH} PARENT_SCOPE)
      SET(CPACK_PACKAGE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH} PARENT_SCOPE)
      SET(MAIDSAFE_LIBRARY_POSTFIX -${VERSION_MAJOR}_${VERSION_MINOR}_${VERSION_PATCH})
      SET(MAIDSAFE_LIBRARY_POSTFIX ${MAIDSAFE_LIBRARY_POSTFIX} PARENT_SCOPE)
      SET(CMAKE_RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX}${MAIDSAFE_LIBRARY_POSTFIX} PARENT_SCOPE)
      SET(CMAKE_DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}${MAIDSAFE_LIBRARY_POSTFIX} PARENT_SCOPE)
      SET(CMAKE_RELWITHDEBINFO_POSTFIX ${CMAKE_RELWITHDEBINFO_POSTFIX}${MAIDSAFE_LIBRARY_POSTFIX} PARENT_SCOPE)
      SET(CMAKE_MINSIZEREL_POSTFIX ${CMAKE_MINSIZEREL_POSTFIX}${MAIDSAFE_LIBRARY_POSTFIX} PARENT_SCOPE)
      ADD_DEFINITIONS(-DCMAKE_${VERSION_VARIABLE_NAME}=${VERSION_VARIABLE_VALUE})
    ENDIF()
  ENDFOREACH()
ENDFUNCTION()

FUNCTION(ADD_VERSION_INFO_TO_INSTALLED_FILE)
  INSTALL(CODE "STRING(TOLOWER \${CMAKE_INSTALL_CONFIG_NAME} CMAKE_INSTALL_CONFIG_NAME_LOWER)")
  INSTALL(CODE "FIND_FILE(INSTALL_FILE ${EXPORT_NAME}-\${CMAKE_INSTALL_CONFIG_NAME_LOWER}.cmake PATHS \${CMAKE_INSTALL_PREFIX}/share/maidsafe)")
  INSTALL(CODE "FILE(APPEND \${INSTALL_FILE} \"\\n\\n# Definition of this library's version\\n\")")
  INSTALL(CODE "FILE(APPEND \${INSTALL_FILE} \"SET(${THIS_VERSION} ${${THIS_VERSION}})\\n\")")
ENDFUNCTION()

FUNCTION(CHECK_INSTALL_HEADER_HAS_VERSION_GUARD HEADER)
  GET_FILENAME_COMPONENT(HEADER_NAME ${HEADER} NAME)
  IF(${HEADER_NAME} STREQUAL "version.h")
    RETURN()
  ENDIF()
  FILE(STRINGS ${HEADER} VERSION_LINE REGEX "#if ${THIS_VERSION} != ${${THIS_VERSION}}")
  IF("${VERSION_LINE}" STREQUAL "")
    SET(ERROR_MESSAGE "\n\n${HEADER} is missing the version guard block.\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}Ensure the following code is included in the file:\n\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}\t#include \"${THIS_VERSION_FILE}\"\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}\t#if ${THIS_VERSION} != ${${THIS_VERSION}}\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}\t#  error This API is not compatible with the installed library.\\\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}\t    Please update the ${EXPORT_NAME} library.\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}\t#endif\n\n\n")
    MESSAGE(FATAL_ERROR ${ERROR_MESSAGE})
  ENDIF()
ENDFUNCTION()

FUNCTION(FINAL_MESSAGE)
  MESSAGE("\nThe library and headers will be installed to:\n")
  MESSAGE("    \"${CMAKE_INSTALL_PREFIX_MESSAGE}\"\n\n")
  MESSAGE("To include this project in any other MaidSafe project, use:\n")
  MESSAGE("    -DMAIDSAFE_COMMON_INSTALL_DIR:PATH=\"${CMAKE_INSTALL_PREFIX_MESSAGE}\"\n\n")
  MESSAGE("To build and install this project now, run:\n")
  IF(MSVC)
    MESSAGE("    cmake --build . --config Release --target install")
    MESSAGE("    cmake --build . --config Debug --target install")
  ELSE()
    MESSAGE("    cmake --build . --target install")
  ENDIF()
  MESSAGE("\n\n================================================================================"\n)
ENDFUNCTION()
