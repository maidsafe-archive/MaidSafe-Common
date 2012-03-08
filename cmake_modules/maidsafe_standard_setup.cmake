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

IF(WIN32)
  SET(ERROR_MESSAGE_CMAKE_PATH "   cmake ..\\..")
ELSE()
  SET(ERROR_MESSAGE_CMAKE_PATH "cmake ../../..")
ENDIF()

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

INCLUDE_DIRECTORIES(${MaidSafeCommon_INCLUDE_DIR}/maidsafe)
IF(CMAKE_COMPILER_IS_GNUCC)
  # Need to set these CMake variables as they're not correctly set when using g++-mp-4.6 on OSX.
  SET(CMAKE_INCLUDE_SYSTEM_FLAG_C "-isystem ")
  SET(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")
ENDIF()
INCLUDE_DIRECTORIES(SYSTEM ${MaidSafeCommon_INCLUDE_DIR} ${MaidSafeCommon_INCLUDE_DIR}/breakpad ${Boost_INCLUDE_DIR})

IF(DEFINED ADD_LIBRARY_DIR)
  SET(DEFAULT_LIBRARY_DIR ${DEFAULT_LIBRARY_DIR} ${ADD_LIBRARY_DIR})
  LIST(REMOVE_DUPLICATES DEFAULT_LIBRARY_DIR)
  SET(DEFAULT_LIBRARY_DIR ${DEFAULT_LIBRARY_DIR} CACHE PATH "Path to libraries directories" FORCE)
ENDIF()

SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${DEFAULT_LIBRARY_DIR})
INCLUDE(maidsafe_utils)

# Create CTestCustom.cmake to avoid inclusion of coverage results from test files, protocol buffer files and main.cc files
FILE(WRITE ${PROJECT_BINARY_DIR}/CTestCustom.cmake "\n")
ADD_COVERAGE_EXCLUDE(\\\\.pb\\\\.)
ADD_COVERAGE_EXCLUDE(tests/)
ADD_COVERAGE_EXCLUDE(main\\\\.cc)

# Avoid running MemCheck on STYLE_CHECK tests
ADD_MEMCHECK_IGNORE(STYLE_CHECK)

INCLUDE(maidsafe_run_protoc)


###################################################################################################
# Python library search                                                                           #
###################################################################################################
UNSET(PYTHON_EXECUTABLE CACHE)
INCLUDE(FindPythonInterp)
SET(Python_ADDITIONAL_VERSIONS 3.9 3.8 3.7 3.6 3.5 3.4 3.3 3.2 3.1 3.0)
FIND_PACKAGE(PythonInterp)
IF(PYTHONINTERP_FOUND)
  EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -V ERROR_VARIABLE PYTHON_VERSION ERROR_STRIP_TRAILING_WHITESPACE)
  STRING(REPLACE "Python " "" PYTHON_VERSION ${PYTHON_VERSION})
  MESSAGE("-- Found python executable v${PYTHON_VERSION} - style checking enabled.")
ELSE()
  MESSAGE("-- Didn't find python executable: style checking disabled.")
ENDIF()


###################################################################################################
# All other libraries search                                                                      #
###################################################################################################
INCLUDE(maidsafe_find_openmp)

IF(UNIX)
  SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /usr/lib/i386-linux-gnu/ /usr/lib/x86_64-linux-gnu/)
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

IF(NOT DEFINED MEMORY_CHECK)
  IF($ENV{MEMORY_CHECK})
    SET(MEMORY_CHECK ON)
  ENDIF()
ENDIF()

IF(UNIX)
  UNSET(MEMORYCHECK_SUPPRESSIONS_FILE CACHE)
  FIND_FILE(MEMORYCHECK_SUPPRESSIONS_FILE NAMES MemCheck.supp PATHS ${PROJECT_SOURCE_DIR} DOC "File that contains suppressions for the memory checker")
  SET(MEMORYCHECK_COMMAND_OPTIONS "--tool=memcheck --quiet --verbose --trace-children=yes --demangle=yes --num-callers=50 --show-below-main=yes --leak-check=full --show-reachable=yes --track-origins=yes --gen-suppressions=all")
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

CLEANUP_TEMP_DIR()
SET(CPACK_STRIP_FILES TRUE)
