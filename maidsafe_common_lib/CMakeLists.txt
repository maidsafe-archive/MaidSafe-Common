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


CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)
MESSAGE("================================================================================\n")
SET(CMAKE_CACHEFILE "${CMAKE_CACHEFILE_DIR}/CMakeCache.txt")
IF(WIN32)
  STRING(REPLACE "/" "\\" CMAKE_CACHEFILE ${CMAKE_CACHEFILE})
ENDIF()

SET(ERROR_MESSAGE "No generator previously specified.  Run:\n")
SET(ERROR_MESSAGE_CMAKE_PATH "cmake ../../..")
GET_FILENAME_COMPONENT(CMAKE_BUILD_TYPE_DIR ${CMAKE_BINARY_DIR} NAME)

# Variable MSVC is not set until after PROJECT command below
IF(CMAKE_BUILD_TYPE_DIR MATCHES "Win_MSVC")
  SET(CMAKE_BUILD_TYPE_DIR Release)
  SET(ERROR_MESSAGE_CMAKE_PATH "   cmake ..\\..")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}\tdel ${CMAKE_CACHEFILE}\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}then run e.g:\n\t${ERROR_MESSAGE_CMAKE_PATH} -G\"Visual Studio 9 2008\"\n\n")
ELSEIF(UNIX)
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}\trm ${CMAKE_CACHEFILE}\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}then run e.g:\n\t${ERROR_MESSAGE_CMAKE_PATH} -G\"CodeBlocks - Unix Makefiles\"\n\n")
ELSE()
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}\tdel ${CMAKE_CACHEFILE}\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}then run e.g:\n\t${ERROR_MESSAGE_CMAKE_PATH} -G\"CodeBlocks - MinGW Makefiles\"\n\n")
ENDIF()

IF(CMAKE_BUILD_TYPE_DIR MATCHES "kdev")
  SET(CMAKE_BUILD_TYPE_DIR Debug)
  SET(KDEV 1)
ENDIF()


SET(ERROR_MESSAGE "${ERROR_MESSAGE}To see a full list of generators, run:\n\t${ERROR_MESSAGE_CMAKE_PATH} --help\n")
SET(CMAKE_GEN ${CMAKE_GENERATOR} CACHE INTERNAL "" FORCE)
IF(NOT ${CMAKE_C_COMPILER_WORKS})
  MESSAGE(FATAL_ERROR ${ERROR_MESSAGE})
ENDIF()

IF((CMAKE_BUILD_TYPE_DIR MATCHES "Release") OR (DEFINED ${KDEV}))
  SET(PROJECT_NAME maidsafe_common)
ELSE()
  SET(PROJECT_NAME maidsafe_common_${CMAKE_BUILD_TYPE_DIR})
ENDIF()
OPTION(BUILD_SHARED_LIBS "Build shared libraries." OFF)


# This call resets CMAKE_BUILD_TYPE, hence it's copied from CMAKE_BUILD_TYPE_DIR which is set above.
PROJECT(${PROJECT_NAME})
SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_DIR} CACHE INTERNAL "Except for MSVC, the build type is set by running cmake from the appropriate build subdirectory e.g. for a Debug build, \"cd build/Linux/Debug && cmake ../../..\"." FORCE)

SET(${PROJECT_NAME}_ROOT ${PROJECT_SOURCE_DIR})
SET(${PROJECT_NAME}_SOURCE_DIR ${${PROJECT_NAME}_ROOT}/src)
SET(SOURCE_DIR ${${PROJECT_NAME}_ROOT}/src)

IF(NOT MAIDSAFE_TEST_TYPE)
  SET(MAIDSAFE_TEST_TYPE "ALL" CACHE STRING "Choose the type of TEST, options are: ALL, BEH, FUNC" FORCE)
  SET(MAIDSAFE_TEST_TYPE_MESSAGE "Tests included: All")
ELSE()
  IF(${MAIDSAFE_TEST_TYPE} MATCHES _)
    SET(MAIDSAFE_TEST_TYPE_MESSAGE "Tests included: All")
  ELSEIF(${MAIDSAFE_TEST_TYPE} MATCHES BEH)
    SET(MAIDSAFE_TEST_TYPE_MESSAGE "Tests included: Behavioural")
  ELSEIF(${MAIDSAFE_TEST_TYPE} MATCHES FUNC)
    SET(MAIDSAFE_TEST_TYPE_MESSAGE "Tests included: Functional")
  ELSE()
    SET(MAIDSAFE_TEST_TYPE "_" CACHE STRING "Choose the type of TEST, options are: _ BEH FUNC" FORCE)
    SET(MAIDSAFE_TEST_TYPE_MESSAGE "Tests included: All")
  ENDIF()
ENDIF()

ENABLE_TESTING()

IF(APPLE)
  SET(CMAKE_OSX_SYSROOT "/")
ENDIF()

IF(CMAKE_BUILD_TYPE MATCHES "Release")
  IF(MSVC)
    MESSAGE("---------------------------")
  ELSE()
    MESSAGE("Building a package which is OK to release.")
    MESSAGE("-----------------------------------------------------------------------")
  ENDIF()
ELSE()
  IF(MSVC)
    MESSAGE("---------------------------")
  ELSE()
    MESSAGE("Building a package which is NOT OK to release.")
    MESSAGE("---------------------------------------------------------------------------")
  ENDIF()
  SET(pddate "DEVELOPER_${pddate}")
ENDIF()

SET(CMAKE_include_directories_BEFORE ON)

GET_FILENAME_COMPONENT(CMAKE_INSTALL_ROOT ${CMAKE_INSTALL_PREFIX} PATH)
SET(OLD_CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_ROOT}/maidsafe/common_lib)
IF(WIN32)
  STRING(REPLACE "/" "\\" OLD_CMAKE_INSTALL_PREFIX ${OLD_CMAKE_INSTALL_PREFIX})
ENDIF()
SET(CMAKE_INSTALL_PREFIX ${${PROJECT_NAME}_ROOT}/build/common_lib)
message("\${CMAKE_INSTALL_PREFIX} - ${CMAKE_INSTALL_PREFIX}")

IF(NOT DEFINED ${KDEV})
  SET(CMAKE_DEBUG_POSTFIX _d)
  SET(CMAKE_RELWITHDEBINFO_POSTFIX _rwdi)
  SET(CMAKE_MINSIZEREL_POSTFIX _msr)

  IF(CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(TEST_POSTFIX ${CMAKE_DEBUG_POSTFIX})
  ELSEIF(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    SET(TEST_POSTFIX ${CMAKE_RELWITHDEBINFO_POSTFIX})
  ELSEIF(CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
    SET(TEST_POSTFIX ${CMAKE_MINSIZEREL_POSTFIX})
  ENDIF()
ENDIF()
# Create CTestCustom.cmake to avoid inclusion of coverage results from third-party code
FILE(WRITE ${${PROJECT_NAME}_BINARY_DIR}/CTestCustom.cmake "SET(CTEST_CUSTOM_COVERAGE_EXCLUDE \${CTEST_CUSTOM_COVERAGE_EXCLUDE} \"cryptopp/\" \"protobuf/\" \"tests/\")\n\n")

###################################################################################################
# Set default directories to be searched for included files and libs                              #
###################################################################################################
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${${PROJECT_NAME}_ROOT}/../cmake)
SET(TOOLS_PATH ${${PROJECT_NAME}_ROOT}/../tools)
SET(DEFAULT_THIRD_PARTY_ROOT ${${PROJECT_NAME}_ROOT}/..)

IF(DEFINED ADD_LIBRARY_DIR)
  IF(DEFAULT_LIBRARY_DIR)
    LIST(REMOVE_DUPLICATES DEFAULT_LIBRARY_DIR)
  ENDIF()
  SET(DEFAULT_LIBRARY_DIR ${DEFAULT_LIBRARY_DIR} ${ADD_LIBRARY_DIR} CACHE PATH "Path to libraries directories" FORCE)
ENDIF()

###################################################################################################
# Boost libraries search                                                                          #
###################################################################################################
SET(REQUIRED_BOOST_COMPONENTS date_time filesystem program_options regex system thread unit_test_framework)
INCLUDE(maidsafe_FindBoost)
SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
#SET(LIBS_DIRS ${LIBS_DIRS} ${Boost_LIBRARY_DIRS})
#SET(LIBS_DIRS_DEBUG ${LIBS_DIRS_DEBUG} ${Boost_LIBRARY_DIRS})

###################################################################################################
# Cryptopp library search                                                                         #
###################################################################################################
INCLUDE(maidsafe_FindCryptopp)
SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${Cryptopp_INCLUDE_DIR})
#SET(LIBS_DIRS ${LIBS_DIRS} ${Cryptopp_LIBRARY_DIR})
#SET(LIBS_DIRS_DEBUG ${LIBS_DIRS_DEBUG} ${Cryptopp_LIBRARY_DIR_DEBUG})


###################################################################################################
# Google Test library search                                                                      #
###################################################################################################
INCLUDE(maidsafe_FindGtest)
SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${Gtest_INCLUDE_DIR})
#SET(LIBS_DIRS ${LIBS_DIRS} ${Gtest_LIBRARY_DIR})
#SET(LIBS_DIRS_DEBUG ${LIBS_DIRS_DEBUG} ${Gtest_LIBRARY_DIR_DEBUG})
IF(NOT WIN32)
  SET(Gtest_LIBRARY "${Gtest_LIBRARY};pthread")
ENDIF()


###################################################################################################
# Google Log library search                                                                       #
###################################################################################################
INCLUDE(maidsafe_FindGlog)
SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${Glog_INCLUDE_DIR})
#SET(LIBS_DIRS ${LIBS_DIRS} ${Glog_LIBRARY_DIR})
#SET(LIBS_DIRS_DEBUG ${LIBS_DIRS_DEBUG} ${Glog_LIBRARY_DIR_DEBUG})

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
  SET(SYS_LIB dl pthread)
  IF(NOT APPLE)
    SET(SYS_LIB rt c m ${SYS_LIB})
  ENDIF()
ELSEIF(WIN32)
  IF(MSVC)
    SET(SYS_LIB ws2_32 odbc32 odbccp32 WSock32 IPHlpApi)
  ELSE()
    SET(SYS_LIB advapi32 kernel32 ws2_32 iphlpapi mswsock)
  ENDIF()
ENDIF()
FOREACH(library ${SYS_LIB})
  FIND_LIBRARY(CURRENT_LIB ${library})
  IF(CURRENT_LIB)
    MESSAGE("-- Found library ${CURRENT_LIB}")
    GET_FILENAME_COMPONENT(CURRENT_LIBRARY_DIR ${CURRENT_LIB} PATH)
    SET(LIBS_DIRS ${LIBS_DIRS} ${CURRENT_LIBRARY_DIR})
    SET(LIBS_DIRS_DEBUG ${LIBS_DIRS_DEBUG} ${CURRENT_LIBRARY_DIR})
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

#LIST(REMOVE_DUPLICATES LIBS_DIRS)
#LIST(REMOVE_DUPLICATES LIBS_DIRS_DEBUG)
#LINK_DIRECTORIES(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

INCLUDE_DIRECTORIES(BEFORE ${${PROJECT_NAME}_SOURCE_DIR})
LIST(REMOVE_DUPLICATES INCLUDE_DIRS)
INCLUDE_DIRECTORIES(BEFORE SYSTEM ${INCLUDE_DIRS})

MESSAGE("================================================================================")

IF(MSVC)
  SET(CMAKE_CXX_FLAGS "")
  SET(CMAKE_CXX_FLAGS_INIT "")
  SET(CMAKE_CXX_FLAGS_RELEASE "")
  SET(CMAKE_CXX_FLAGS_DEBUG "")
ENDIF()

###################################################################################################
# Set up all files as GLOBs                                                                       #
###################################################################################################
FILE(GLOB SOURCE_FILES_COMMON_DIR "${${PROJECT_NAME}_SOURCE_DIR}/maidsafe/common/*.cc")
FILE(GLOB HEADER_FILES_COMMON_DIR "${${PROJECT_NAME}_SOURCE_DIR}/maidsafe/common/*.h")
SET(ALL_FILES_COMMON_DIR ${SOURCE_FILES_COMMON_DIR} ${HEADER_FILES_COMMON_DIR})
SOURCE_GROUP("common Source Files" FILES ${SOURCE_FILES_COMMON_DIR})
SOURCE_GROUP("common Header Files" FILES ${HEADER_FILES_COMMON_DIR})

SET(TEST_MAIN "${${PROJECT_NAME}_SOURCE_DIR}/maidsafe/common/tests/main.cc")

FILE(GLOB SOURCE_FILES_COMMON_TESTS_DIR "${${PROJECT_NAME}_SOURCE_DIR}/maidsafe/common/tests/*.cc")
FILE(GLOB HEADER_FILES_COMMON_TESTS_DIR "${${PROJECT_NAME}_SOURCE_DIR}/maidsafe/common/tests/*.h")
SET(ALL_FILES_COMMON_TESTS_DIR ${SOURCE_FILES_COMMON_TESTS_DIR} ${HEADER_FILES_COMMON_TESTS_DIR})
SOURCE_GROUP("common tests Source Files" FILES ${SOURCE_FILES_COMMON_TESTS_DIR})
SOURCE_GROUP("common tests Header Files" FILES ${HEADER_FILES_COMMON_TESTS_DIR})

FILE(GLOB SOURCE_FILES_COMMON_BOOST_TESTS_DIR "${${PROJECT_NAME}_SOURCE_DIR}/maidsafe/common/tests/boost/*.cc")
FILE(GLOB HEADER_FILES_COMMON_BOOST_TESTS_DIR "${${PROJECT_NAME}_SOURCE_DIR}/maidsafe/common/tests/boost/*.h")
SET(ALL_FILES_COMMON_BOOST_TESTS_DIR ${SOURCE_FILES_COMMON_BOOST_TESTS_DIR} ${HEADER_FILES_COMMON_BOOST_TESTS_DIR})
SOURCE_GROUP("common boost tests Source Files" FILES ${SOURCE_FILES_COMMON_BOOST_TESTS_DIR})
SOURCE_GROUP("common boost tests Header Files" FILES ${HEADER_FILES_COMMON_BOOST_TESTS_DIR})

###################################################################################################
# All Platforms:  Add Test Exes and Create all maidsafe_dht libs                                  #
###################################################################################################
ADD_LIBRARY(maidsafe_common_static STATIC ${ALL_FILES_COMMON_DIR})
SET_TARGET_PROPERTIES(maidsafe_common_static PROPERTIES OUTPUT_NAME maidsafe_common)

ADD_EXECUTABLE(TESTcommon ${TEST_MAIN} ${ALL_FILES_COMMON_TESTS_DIR})
ADD_EXECUTABLE(TESTboost ${TEST_MAIN} ${ALL_FILES_COMMON_BOOST_TESTS_DIR})

IF(NOT MSVC)
  SET_TARGET_PROPERTIES(TESTcommon PROPERTIES
                          DEBUG_OUTPUT_NAME TESTcommon${CMAKE_DEBUG_POSTFIX}
                          RELWITHDEBINFO_OUTPUT_NAME TESTcommon${CMAKE_RELWITHDEBINFO_POSTFIX}
                          MINSIZEREL_OUTPUT_NAME TESTcommon${CMAKE_MINSIZEREL_POSTFIX})
  SET_TARGET_PROPERTIES(TESTboost PROPERTIES
                          DEBUG_OUTPUT_NAME TESTboost${CMAKE_DEBUG_POSTFIX}
                          RELWITHDEBINFO_OUTPUT_NAME TESTboost${CMAKE_RELWITHDEBINFO_POSTFIX}
                          MINSIZEREL_OUTPUT_NAME TESTboost${CMAKE_MINSIZEREL_POSTFIX})
ENDIF()

# When this target is built, it removes all .gcda files from the build directory and its subdirectories
IF(UNIX)
  FIND_FILE(CLEAN_COVERAGE clean_coverage.cmake ${CMAKE_MODULE_PATH})
  IF(CLEAN_COVERAGE)
    ADD_CUSTOM_TARGET(CleanCoverage COMMAND ${CMAKE_COMMAND} -DSEARCH_DIR=${CMAKE_BINARY_DIR} -P ${CLEAN_COVERAGE})
  ENDIF()
ENDIF()

TARGET_LINK_LIBRARIES(maidsafe_common_static
                        ${Cryptopp_LIBRARY}
                        ${Glog_LIBRARY}
                        ${Boost_DATE_TIME_LIBRARIES}
                        ${Boost_FILESYSTEM_LIBRARIES}
                        ${Boost_THREAD_LIBRARIES}
                        ${Boost_SYSTEM_LIBRARIES}
                        ${Boost_REGEX_LIBRARIES}
                        ${SYS_LIB})
TARGET_LINK_LIBRARIES(TESTcommon maidsafe_common_static ${Gtest_LIBRARY})
TARGET_LINK_LIBRARIES(TESTboost ${Glog_LIBRARY} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} ${Gtest_LIBRARY} ${SYS_LIB})

###################################################################################################
# Builds                                                                                          #
###################################################################################################
ADD_DEFINITIONS(-DSTATICLIB)
ADD_DEFINITIONS(-DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_FILESYSTEM_VERSION=3)

IF(CMAKE_BUILD_TYPE MATCHES "Debug")
  ADD_DEFINITIONS(-DDEBUG)
ENDIF()

IF(WIN32)
  ADD_DEFINITIONS(-DWIN32 -D_WIN32 -D__WINDOWS__ -D__WIN32__)
ENDIF()

IF(MSVC)
  ADD_DEFINITIONS(-D__MSVC__ -DWIN32_LEAN_AND_MEAN -D_WIN32_WINNT=0x501 -DUDT_STATIC)
  ADD_DEFINITIONS(-D_CONSOLE -D_UNICODE -DUNICODE -D_BIND_TO_CURRENT_VCLIBS_VERSION=1)

  # prevents std::min() and std::max() to be overwritten
  ADD_DEFINITIONS(-DNOMINMAX)

  # flag to link to static version of Google Glog
  ADD_DEFINITIONS(-DGOOGLE_GLOG_DLL_DECL=)

  # prevents from automatic linking of boost libraries
  ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)

  SET(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /Ot /Oy /GL /D \"NDEBUG\" /EHsc /MT /Gy /W3 /Zi")
  SET(CMAKE_CXX_FLAGS_DEBUG "/Od /Ot /Oy /D \"_DEBUG\" /EHsc /MTd  /c /ZI /TP")
  SET(CMAKE_CXX_FLAGS_MINSIZEREL "/EHsc /MT")
  SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/EHsc /MT")

  # CMake defaults to passing stack size in LINKER_FLAGS of 10MB.  Set this to windows default of 1MB
  STRING(REGEX REPLACE "/STACK:[0-9]+" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
  STRING(REGEX REPLACE "/STACK:[0-9]+" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")

  # Given a link dir of "a/b/c", MSVC adds "a/b/c/" AND "a/b/c/CMAKE_BUILD_TYPE" as link dirs, so we
  # can't just use "LINK_DIRECTORIES" as some Google debug libs have the same name as the release version.
  FOREACH(LIBS_DIR ${LIBS_DIRS})
    STRING(REPLACE "\\" "\\\\" LIBS_DIR ${LIBS_DIR})
    SET(LINKER_LIBS_DIRS_RELEASE "${LINKER_LIBS_DIRS_RELEASE} /LIBPATH:\"${LIBS_DIR}\"")
  ENDFOREACH()
  FOREACH(LIBS_DIR_DEBUG ${LIBS_DIRS_DEBUG})
    STRING(REPLACE "\\" "\\\\" LIBS_DIR_DEBUG ${LIBS_DIR_DEBUG})
    SET(LINKER_LIBS_DIRS_DEBUG "${LINKER_LIBS_DIRS_DEBUG} /LIBPATH:\"${LIBS_DIR_DEBUG}\"")
  ENDFOREACH()
  SET_TARGET_PROPERTIES(maidsafe_common_static PROPERTIES
                          STATIC_LIBRARY_FLAGS_RELEASE "/LTCG")
  SET_TARGET_PROPERTIES(TESTcommon TESTboost PROPERTIES
                          LINK_FLAGS "/STACK:1048576"
                          LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF /LTCG /INCREMENTAL:NO ${LINKER_LIBS_DIRS_RELEASE}"
                          LINK_FLAGS_DEBUG "${LINKER_LIBS_DIRS_DEBUG}"
                          LINK_FLAGS_RELWITHDEBINFO "${LINKER_LIBS_DIRS_DEBUG}"
                          LINK_FLAGS_MINSIZEREL "${LINKER_LIBS_DIRS_DEBUG}")
ENDIF()


IF(UNIX)
  SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g -ggdb -pg -fprofile-arcs -ftest-coverage")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem ${CMAKE_INCLUDE_PATH} -isystem -W -Wall -Wextra -D_FORTIFY_SOURCE=2 -D_FILE_OFFSET_BITS=64")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -ggdb -pg -fprofile-arcs -ftest-coverage")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
  SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -pg -fprofile-arcs -ftest-coverage")
  SET_SOURCE_FILES_PROPERTIES(${${PROJECT_NAME}_SOURCE_DIR}/maidsafe/common/crypto.cc
                              ${${PROJECT_NAME}_SOURCE_DIR}/maidsafe/common/utils.cc
                                PROPERTIES COMPILE_FLAGS "-w")
  IF(APPLE)
    ADD_DEFINITIONS(-DMAIDSAFE_APPLE -DBSD)
    SET(CMAKE_AR "/usr/bin/libtool")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DARWIN_C_SOURCE")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finline-functions -funswitch-loops -fgcse-after-reload -ftree-vectorize -D__FreeBSD__=10 -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -Wuninitialized")
    SET(CMAKE_CXX_CREATE_STATIC_LIBRARY "<CMAKE_AR> -static -o <TARGET> <LINK_FLAGS> <OBJECTS>" "<CMAKE_RANLIB> <TARGET>")
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -lgcov")
  ELSE()
    ADD_DEFINITIONS(-DMAIDSAFE_LINUX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pedantic -pedantic-errors -std=c++0x -Weffc++")
  ENDIF()
ENDIF()


###################################################################################################
# Tests                                                                                           #
###################################################################################################
INCLUDE(CTest)
INCLUDE(maidsafe_AddGtests)

SET(CTEST_CUSTOM_MAXIMUM_PASSED_TEST_OUTPUT_SIZE 50000)
SET(CTEST_CUSTOM_MAXIMUM_FAILED_TEST_OUTPUT_SIZE 50000)
SET(CTEST_CONTINUOUS_DURATION 600)
SET(CTEST_CONTINUOUS_MINIMUM_INTERVAL 10)
SET(CTEST_START_WITH_EMPTY_BINARY_DIRECTORY true)
IF(PYTHONINTERP_FOUND)
  IF(UNIX)
    FIND_FILE(LINT_CHECKER cpplint.py ${TOOLS_PATH})
    IF(LINT_CHECKER)
      ADD_TEST(STYLE_CHECK_KAD python ${LINT_CHECKER} ${STYLE_KADEMLIA} ${ALL_FILES_TEST_KADEMLIA_DIR} ${ALL_FILES_TEST_FUNCTIONALKADEMLIA_DIR})
      SET_PROPERTY(TEST STYLE_CHECK_KAD PROPERTY LABELS Functional Kademlia CodingStyle)
      ADD_TEST(STYLE_CHECK_COMMON python ${LINT_CHECKER} ${ALL_FILES_COMMON_DIR} ${ALL_FILES_TEST_COMMON_DIR})
      SET_PROPERTY(TEST STYLE_CHECK_COMMON PROPERTY LABELS Functional Common CodingStyle)
      ADD_TEST(STYLE_CHECK_TRANSPORT python ${LINT_CHECKER} ${ALL_FILES_TRANSPORT_DIR} ${ALL_FILES_TEST_TRANSPORT_DIR})
      SET_PROPERTY(TEST STYLE_CHECK_TRANSPORT PROPERTY LABELS Functional Transport CodingStyle)
    ENDIF()
  ELSE()
    FIND_FILE(LINT_CHECKER run_cpplint.bat ${TOOLS_PATH})
    IF(LINT_CHECKER)
      STRING(REPLACE "/" "\\\\" STYLE_CHECK_SOURCE ${${PROJECT_NAME}_ROOT})
      ADD_TEST(STYLE_CHECK ${LINT_CHECKER} ${STYLE_CHECK_SOURCE})
      SET_PROPERTY(TEST STYLE_CHECK PROPERTY LABELS Functional CodingStyle)
    ENDIF()
  ENDIF()
ENDIF()

ADD_GTESTS(TESTcommon)
ADD_GTESTS(TESTboost)

IF(UNIX)
  SET(MEMORYCHECK_COMMAND "/usr/bin/valgrind --show-reachable=no")
ENDIF()

IF(INSTALL_PREFIX)
  message("\${INSTALL_PREFIX} - ${INSTALL_PREFIX}")
  IF(WIN32)
    STRING(REPLACE "\\" "/" INSTALL_PREFIX ${INSTALL_PREFIX})
  ENDIF()
  SET(CMAKE_INSTALL_PREFIX "${INSTALL_PREFIX}")
ENDIF()

###################################################################################################
MESSAGE("")
LIST(LENGTH ALL_GTESTS GTEST_COUNT)
MESSAGE("${MAIDSAFE_TEST_TYPE_MESSAGE}.   ${GTEST_COUNT} Google Tests.")
MESSAGE("")
MESSAGE("    To include all tests,                ${ERROR_MESSAGE_CMAKE_PATH} -DMAIDSAFE_TEST_TYPE=ALL")
MESSAGE("    To include behavioural tests,        ${ERROR_MESSAGE_CMAKE_PATH} -DMAIDSAFE_TEST_TYPE=BEH")
MESSAGE("    To include functional tests,        ${ERROR_MESSAGE_CMAKE_PATH} -DMAIDSAFE_TEST_TYPE=FUNC")
MESSAGE("================================================================================")
MESSAGE("")
MESSAGE("Install prefix is set to:\n")
MESSAGE("        \"${CMAKE_INSTALL_PREFIX}\"")
MESSAGE("")
MESSAGE("    To alter this (e.g. to ${OLD_CMAKE_INSTALL_PREFIX}),\n")
MESSAGE("          ${ERROR_MESSAGE_CMAKE_PATH} -DINSTALL_PREFIX=\"${OLD_CMAKE_INSTALL_PREFIX}\"")
MESSAGE("================================================================================")
MESSAGE("")

INSTALL(TARGETS maidsafe_common_static EXPORT maidsafe_common ARCHIVE DESTINATION lib)
INSTALL(FILES ${HEADER_FILES_COMMON_DIR} DESTINATION include/maidsafe/common)
INSTALL(EXPORT maidsafe_common DESTINATION cmake)
