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

SET(ERROR_MESSAGE_CMAKE_PATH "cmake ../../..")
GET_FILENAME_COMPONENT(CMAKE_BUILD_TYPE_DIR ${CMAKE_BINARY_DIR} NAME)

# Variable MSVC is not set until after PROJECT command below
IF(CMAKE_BUILD_TYPE_DIR MATCHES "Win_MSVC")
  SET(CMAKE_BUILD_TYPE_DIR Release)
  SET(ERROR_MESSAGE_CMAKE_PATH "   cmake ..\\..")
ELSEIF(CMAKE_BUILD_TYPE_DIR MATCHES "kdev")
  SET(CMAKE_BUILD_TYPE_DIR Debug)
  SET(KDEV 1)
ENDIF()

SET(ERROR_MESSAGE "No generator previously specified.\nTo see a full list of generators, run:\n\t${ERROR_MESSAGE_CMAKE_PATH} --help\n")
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

IF(MSVC AND ${MSVC_VERSION} LESS 1600)  # i.e for MSVC < Visual Studio 10.0
  MESSAGE(FATAL_ERROR "In order to use C++0x features, this library cannot be built using a version of Visual Studio less than 10.0 (i.e. VS 2010)")
ENDIF()

###################################################################################################
# Third party library search                                                                      #
###################################################################################################
UNSET(MAIDSAFE_COMMON_INSTALL_PREFIX_CMAKE CACHE)
UNSET(CRYPTO_TEST_CMAKE CACHE)
FIND_FILE(MAIDSAFE_COMMON_INSTALL_PREFIX_CMAKE install_prefix.cmake ${PROJECT_SOURCE_DIR}/../third_party_libs)
FIND_FILE(CRYPTO_TEST_CMAKE crypto_test.cmake ${PROJECT_SOURCE_DIR}/../third_party_libs)
IF(NOT MAIDSAFE_COMMON_INSTALL_PREFIX_CMAKE OR NOT CRYPTO_TEST_CMAKE OR FORCE_THIRD_PARTY_BUILD)
  IF(NOT FORCE_THIRD_PARTY_BUILD)
    SET(FORCE_THIRD_PARTY_BUILD "ALL")
  ENDIF()
  FIND_FILE(THIRD_PARTY_CMAKE CMakeLists.txt ${PROJECT_SOURCE_DIR}/../third_party_libs)
  IF(NOT THIRD_PARTY_CMAKE)
    MESSAGE(FATAL_ERROR "Third Party libs is broken.  Try a fresh checkout!")
  ENDIF()
  IF(${FORCE_THIRD_PARTY_BUILD} MATCHES "ALL")
    MESSAGE("-- Creating third party libs.  Building ALL targets.")
  ELSE()
    MESSAGE("-- Creating third party libs.  Building target ${FORCE_THIRD_PARTY_BUILD} only.")
  ENDIF()
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory BuildThirdPartyLibs)
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} "-DBUILD_TARGET=${FORCE_THIRD_PARTY_BUILD}" ${THIRD_PARTY_CMAKE} -G ${CMAKE_GENERATOR} WORKING_DIRECTORY BuildThirdPartyLibs)# OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  UNSET(FORCE_THIRD_PARTY_BUILD CACHE)
  FIND_FILE(MAIDSAFE_COMMON_INSTALL_PREFIX_CMAKE install_prefix.cmake ${PROJECT_SOURCE_DIR}/../third_party_libs)
  FIND_FILE(CRYPTO_TEST_CMAKE crypto_test.cmake ${PROJECT_SOURCE_DIR}/../third_party_libs)
  IF(NOT MAIDSAFE_COMMON_INSTALL_PREFIX_CMAKE OR NOT CRYPTO_TEST_CMAKE)
    MESSAGE(FATAL_ERROR "Third Party libs is broken.  Try a fresh checkout.")
  ENDIF()
ENDIF()
INCLUDE(${MAIDSAFE_COMMON_INSTALL_PREFIX_CMAKE})
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/build ${PROJECT_SOURCE_DIR}/../cmake_modules)
INCLUDE(${MAIDSAFE_COMMON_INSTALL_PREFIX}/share/maidsafe/maidsafe_third_party.cmake)
INCLUDE_DIRECTORIES(SYSTEM ${MAIDSAFE_COMMON_INSTALL_PREFIX}/include)

###################################################################################################
# Standard setup                                                                                  #
###################################################################################################
INCLUDE(maidsafe_standard_setup)
# Need to overwrite defaults for this project
SET(MaidSafeCommon_TOOLS_DIR ${PROJECT_SOURCE_DIR}/../tools)

###################################################################################################
# Boost libraries search                                                                          #
###################################################################################################
SET(REQUIRED_BOOST_COMPONENTS filesystem system thread unit_test_framework)
INCLUDE(maidsafe_find_boost)
SET(EXPORT_BOOST_CMAKE ${CMAKE_BINARY_DIR}/boost_libs.cmake)
FILE(WRITE ${EXPORT_BOOST_CMAKE} "${EXPORT_BOOST}")
FILE(APPEND ${EXPORT_BOOST_CMAKE} "\nSET(Boost_LIBRARY_DIRS \"${Boost_LIBRARY_DIRS}\")\n")
FILE(APPEND ${EXPORT_BOOST_CMAKE} "SET(Boost_INCLUDE_DIR \"${Boost_INCLUDE_DIR}\")\n")
INCLUDE(${EXPORT_BOOST_CMAKE})
MESSAGE("================================================================================")

###################################################################################################
# Set up all files as GLOBs                                                                       #
###################################################################################################
FILE(GLOB SOURCE_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/*.cc")
FILE(GLOB HEADER_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/*.h")
SET(ALL_FILES_COMMON_DIR ${SOURCE_FILES_COMMON_DIR} ${HEADER_FILES_COMMON_DIR})
SOURCE_GROUP("common Source Files" FILES ${SOURCE_FILES_COMMON_DIR})
SOURCE_GROUP("common Header Files" FILES ${HEADER_FILES_COMMON_DIR})

SET(TEST_MAIN "${PROJECT_SOURCE_DIR}/src/maidsafe/common/tests/main.cc")

FILE(GLOB SOURCE_FILES_COMMON_TESTS_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/tests/*.cc")
FILE(GLOB HEADER_FILES_COMMON_TESTS_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/tests/*.h")
SET(ALL_FILES_COMMON_TESTS_DIR ${SOURCE_FILES_COMMON_TESTS_DIR} ${HEADER_FILES_COMMON_TESTS_DIR})
SOURCE_GROUP("common tests Source Files" FILES ${SOURCE_FILES_COMMON_TESTS_DIR})
SOURCE_GROUP("common tests Header Files" FILES ${HEADER_FILES_COMMON_TESTS_DIR})

FILE(GLOB SOURCE_FILES_COMMON_BOOST_TESTS_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/tests/boost/*.cc")
FILE(GLOB HEADER_FILES_COMMON_BOOST_TESTS_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/tests/boost/*.h")
SET(ALL_FILES_COMMON_BOOST_TESTS_DIR ${SOURCE_FILES_COMMON_BOOST_TESTS_DIR} ${HEADER_FILES_COMMON_BOOST_TESTS_DIR})
SOURCE_GROUP("common boost tests Source Files" FILES ${SOURCE_FILES_COMMON_BOOST_TESTS_DIR})
SOURCE_GROUP("common boost tests Header Files" FILES ${HEADER_FILES_COMMON_BOOST_TESTS_DIR})

FILE(GLOB MODULE_FILES "${PROJECT_SOURCE_DIR}/../cmake_modules/*")
FILE(GLOB TOOL_FILES "${PROJECT_SOURCE_DIR}/../tools/*")

###################################################################################################
# All Platforms:  Add Test Exes and Create all maidsafe_dht libs                                  #
###################################################################################################
ADD_LIBRARY(maidsafe_common_static STATIC ${ALL_FILES_COMMON_DIR})
SET_TARGET_PROPERTIES(maidsafe_common_static PROPERTIES OUTPUT_NAME maidsafe_common)

ADD_EXECUTABLE(TESTcommon ${TEST_MAIN} ${ALL_FILES_COMMON_TESTS_DIR})
ADD_EXECUTABLE(TESTboost ${TEST_MAIN} ${ALL_FILES_COMMON_BOOST_TESTS_DIR})
RENAME_EXECUTABLE(TESTcommon)
RENAME_EXECUTABLE(TESTboost)

TARGET_LINK_LIBRARIES(maidsafe_common_static cryptopp_static glog_static boost_filesystem boost_system boost_thread ${SYS_LIB})
TARGET_LINK_LIBRARIES(TESTcommon maidsafe_common_static gtest_static)
TARGET_LINK_LIBRARIES(TESTboost maidsafe_common_static glog_static boost_unit_test_framework gtest_static ${SYS_LIB})

###################################################################################################
# Builds                                                                                          #
###################################################################################################
INCLUDE(maidsafe_standard_flags)
IF(MSVC)
  # Set warnings as errors
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /WX")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /WX")
  SET(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /W4 /WX")
  SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /W4 /WX")
  SET_TARGET_PROPERTIES(maidsafe_common_static PROPERTIES
                          STATIC_LIBRARY_FLAGS_RELEASE "/LTCG")
  SET_TARGET_PROPERTIES(TESTcommon TESTboost PROPERTIES
                          LINK_FLAGS "/STACK:1048576"
                          LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF /LTCG /INCREMENTAL:NO ${LINKER_LIBS_DIRS_RELEASE}"
                          LINK_FLAGS_DEBUG "${LINKER_LIBS_DIRS_DEBUG}"
                          LINK_FLAGS_RELWITHDEBINFO "${LINKER_LIBS_DIRS_DEBUG} /LTCG /INCREMENTAL:NO"
                          LINK_FLAGS_MINSIZEREL "${LINKER_LIBS_DIRS_DEBUG} /LTCG")
#ELSEIF(UNIX)
 # SET_SOURCE_FILES_PROPERTIES(${PROJECT_SOURCE_DIR}/src/maidsafe/common/crypto.cc
 #                             ${PROJECT_SOURCE_DIR}/src/maidsafe/common/utils.cc
 #                               PROPERTIES COMPILE_FLAGS "-w -isystem")
ENDIF()


###################################################################################################
# Tests                                                                                           #
###################################################################################################
IF(PYTHONINTERP_FOUND)
  IF(UNIX)
    FIND_FILE(LINT_CHECKER cpplint.py ${MaidSafeCommon_TOOLS_DIR})
    IF(LINT_CHECKER)
      ADD_TEST(STYLE_CHECK_COMMON python ${LINT_CHECKER} ${ALL_FILES_COMMON_DIR} ${ALL_FILES_COMMON_TESTS_DIR})
      SET_PROPERTY(TEST STYLE_CHECK_COMMON PROPERTY LABELS Functional Common CodingStyle)
    ENDIF()
  ELSE()
    FIND_FILE(LINT_CHECKER run_cpplint.bat ${MaidSafeCommon_TOOLS_DIR})
    FIND_FILE(LINT_CHECKER_PY cpplint.py ${MaidSafeCommon_TOOLS_DIR})
    IF(LINT_CHECKER AND LINT_CHECKER_PY)
      STRING(REPLACE "/" "\\\\" STYLE_CHECK_SOURCE ${PROJECT_SOURCE_DIR})
      ADD_TEST(STYLE_CHECK ${LINT_CHECKER} ${STYLE_CHECK_SOURCE} ${LINT_CHECKER_PY})
      SET_PROPERTY(TEST STYLE_CHECK PROPERTY LABELS Functional CodingStyle)
    ENDIF()
  ENDIF()
ENDIF()

ADD_GTESTS(TESTcommon)
ADD_GTESTS(TESTboost)
INCLUDE(${CRYPTO_TEST_CMAKE})

TEST_SUMMARY_OUTPUT()


###################################################################################################
# Install details                                                                                 #
###################################################################################################
GET_FILENAME_COMPONENT(COMMON_ROOT_DIR ${PROJECT_SOURCE_DIR} PATH)
SET(CMAKE_INSTALL_PREFIX ${COMMON_ROOT_DIR}/installed)
IF(INSTALL_PREFIX)
  FILE(TO_CMAKE_PATH ${INSTALL_PREFIX} INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "${INSTALL_PREFIX}")
ENDIF()
FILE(TO_NATIVE_PATH ${CMAKE_INSTALL_PREFIX} CMAKE_INSTALL_PREFIX_MESSAGE)

MESSAGE("\nThe libraries, headers, cmake modules and tools will be installed to:\n")
MESSAGE("    \"${CMAKE_INSTALL_PREFIX_MESSAGE}\"\n\n")
MESSAGE("To include this project in any other MaidSafe project, use:\n")
MESSAGE("    -DMAIDSAFE_COMMON_INSTALL_DIR=\"${CMAKE_INSTALL_PREFIX_MESSAGE}\"\n\n")
MESSAGE("To force the rebuild of the third party libs, use one of:\n")
MESSAGE(" ${ERROR_MESSAGE_CMAKE_PATH} -DFORCE_THIRD_PARTY_BUILD=ALL")
MESSAGE(" ${ERROR_MESSAGE_CMAKE_PATH} -DFORCE_THIRD_PARTY_BUILD=GMOCK_GTEST")
MESSAGE(" ${ERROR_MESSAGE_CMAKE_PATH} -DFORCE_THIRD_PARTY_BUILD=CRYPTOPP")
MESSAGE(" ${ERROR_MESSAGE_CMAKE_PATH} -DFORCE_THIRD_PARTY_BUILD=PROTOBUF")
MESSAGE(" ${ERROR_MESSAGE_CMAKE_PATH} -DFORCE_THIRD_PARTY_BUILD=GLOG\n\n")
MESSAGE("To build and install this project now, run:\n")
IF(MSVC OR CMAKE_BUILD_TYPE MATCHES "Release")
  MESSAGE("    cmake --build . --config Release --target install")
ENDIF()
IF(MSVC OR CMAKE_BUILD_TYPE MATCHES "Debug")
  MESSAGE("    cmake --build . --config Debug --target install")
ENDIF()
MESSAGE("\n\n================================================================================"\n)

FILE(STRINGS ${PROJECT_SOURCE_DIR}/src/maidsafe/common/version.h MAIDSAFE_COMMON_VERSION
       REGEX "VERSION [0-9]+$")
STRING(REGEX MATCH "[0-9]+$" MAIDSAFE_COMMON_VERSION ${MAIDSAFE_COMMON_VERSION})

SET(EXPORT_NAME maidsafe_common)
INSTALL(TARGETS maidsafe_common_static EXPORT ${EXPORT_NAME} ARCHIVE DESTINATION lib)
INSTALL(FILES ${HEADER_FILES_COMMON_DIR} DESTINATION include/maidsafe/common)
INSTALL(FILES ${MODULE_FILES} DESTINATION share/maidsafe/cmake_modules)
INSTALL(FILES ${TOOL_FILES} DESTINATION share/maidsafe/tools)
INSTALL(FILES ${EXPORT_BOOST_CMAKE} DESTINATION share/maidsafe)
INSTALL(EXPORT ${EXPORT_NAME} DESTINATION share/maidsafe)
FIND_FILE(MAIDSAFE_INSTALL_VERSION_CMAKE maidsafe_install_version.cmake PATHS ${CMAKE_MODULE_PATH})
IF(MAIDSAFE_INSTALL_VERSION_CMAKE)
  SET(THIS_VERSION MAIDSAFE_COMMON_VERSION)
  CONFIGURE_FILE(${MAIDSAFE_INSTALL_VERSION_CMAKE} ${CMAKE_BINARY_DIR}/maidsafe_install_version.cmake @ONLY)
  CONFIGURE_FILE(${CMAKE_BINARY_DIR}/maidsafe_install_version.cmake ${CMAKE_BINARY_DIR}/maidsafe_install_version.cmake @ONLY)
  INSTALL(SCRIPT ${CMAKE_BINARY_DIR}/maidsafe_install_version.cmake)
ENDIF()
FIND_FILE(CONINUOUS_SCRIPT_CMAKE maidsafe_dashboard.cmake PATHS ${CMAKE_MODULE_PATH})
IF(CONINUOUS_SCRIPT_CMAKE)
  CONFIGURE_FILE(${CONINUOUS_SCRIPT_CMAKE} ${CMAKE_BINARY_DIR}/maidsafe_dashboard.cmake @ONLY)
  INSTALL(FILES ${CMAKE_BINARY_DIR}/maidsafe_dashboard.cmake DESTINATION share/maidsafe/cmake_modules)
ENDIF()
