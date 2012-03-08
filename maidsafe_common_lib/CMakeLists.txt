#==============================================================================#
#                                                                              #
#  Copyright (c) 2011 MaidSafe.net limited                                     #
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


SET(MS_PROJECT_NAME maidsafe_common)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4 FATAL_ERROR)
MESSAGE("================================================================================\n")

GET_FILENAME_COMPONENT(CMAKE_BUILD_TYPE_DIR ${CMAKE_BINARY_DIR} NAME)

# Variable MSVC is not set until after "PROJECT" command below
IF(CMAKE_BUILD_TYPE_DIR MATCHES "Win_MSVC")
  SET(CMAKE_BUILD_TYPE_DIR Release)
ELSEIF(CMAKE_BUILD_TYPE_DIR MATCHES "kdev")
  SET(CMAKE_BUILD_TYPE_DIR Debug)
  SET(KDEV 1)
ENDIF()

IF(NOT ${CMAKE_C_COMPILER_WORKS})
  MESSAGE(FATAL_ERROR "No generator previously specified.\nTo see a full list of generators, run:\n\tcmake --help\n")
ENDIF()

# This "PROJECT" command resets CMAKE_BUILD_TYPE, hence it's copied from CMAKE_BUILD_TYPE_DIR which is set above.
IF((CMAKE_BUILD_TYPE_DIR MATCHES "Release") OR (DEFINED ${KDEV}))
  PROJECT(${MS_PROJECT_NAME})
ELSE()
  STRING(TOLOWER ${CMAKE_BUILD_TYPE_DIR} MS_PROJECT_NAME_APPENDIX)
  PROJECT(${MS_PROJECT_NAME}_${MS_PROJECT_NAME_APPENDIX})
ENDIF()

SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_DIR} CACHE INTERNAL "Except for MSVC, the build type is set by running cmake from the appropriate build subdirectory e.g. for a Debug build, \"cd build/Linux/Debug && cmake ../../..\"." FORCE)

IF(MSVC)
  IF(${MSVC_VERSION} LESS 1600)  # i.e for MSVC < Visual Studio 10.0
    MESSAGE(FATAL_ERROR "In order to use C++0x features, this library cannot be built using a version of Visual Studio less than 10.0 (i.e. VS 2010)")
  ENDIF()
ENDIF()

# Check compiler can handle required C++11 features
MESSAGE("-- Checking required C++11 features supported")
SET(ERROR_LOG ${PROJECT_BINARY_DIR}/C++11_features_compile_message.txt)
SET(TRY_COMPILE_FILE ${PROJECT_SOURCE_DIR}/build/C++11_features.cc)
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E remove ${ERROR_LOG})
IF(WIN32)
  TRY_COMPILE(C++11_CAPABLE ${PROJECT_BINARY_DIR} ${TRY_COMPILE_FILE} OUTPUT_VARIABLE OUT_VAR)
ELSE()
  TRY_COMPILE(C++11_CAPABLE ${PROJECT_BINARY_DIR} ${TRY_COMPILE_FILE} COMPILE_DEFINITIONS -std=c++0x OUTPUT_VARIABLE OUT_VAR)
ENDIF()
IF(NOT C++11_CAPABLE)
  FILE(WRITE ${ERROR_LOG} "${OUT_VAR}")
  SET(MSG "\n\nYour selected compiler does not implement all of the C++11 features required for this project.\n")
  SET(MSG "${MSG}To see all the features required, look in ${TRY_COMPILE_FILE}\n")
  SET(MSG "${MSG}To see the output from trying to compile this file, look in ${ERROR_LOG}\n\n")
  MESSAGE(FATAL_ERROR ${MSG})
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
SET(MaidSafeCommon_INCLUDE_DIR ${MAIDSAFE_COMMON_INSTALL_PREFIX}/include)

SET(PROTO_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
FILE(GLOB PROTO_FILES_COMMON RELATIVE ${PROTO_SOURCE_DIR} "${PROTO_SOURCE_DIR}/maidsafe/common/*.proto")
SET(PROTO_FILES "${PROTO_FILES_COMMON}")


###################################################################################################
# Standard setup                                                                                  #
###################################################################################################
INCLUDE(maidsafe_standard_setup)
# Need to overwrite defaults for this project
SET(MaidSafeCommon_TOOLS_DIR ${PROJECT_SOURCE_DIR}/../tools)
HANDLE_VERSIONS(${PROJECT_SOURCE_DIR}/src/maidsafe/common/version.h)

###################################################################################################
# Boost libraries search                                                                          #
###################################################################################################
SET(REQUIRED_BOOST_COMPONENTS filesystem system thread date_time unit_test_framework)
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
FILE(GLOB PROTO_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/*.proto")
FILE(GLOB SOURCE_PROTO_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/*.pb.cc")
FILE(GLOB HEADER_PROTO_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/*.pb.h")
FILE(GLOB MS_HEADER_PROTO_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/*_pb.h")

IF(APPLE)
  # Currently not supporting breakpad using OSX, so remove breakpad header and source
  LIST(REMOVE_ITEM SOURCE_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/breakpad.cc")
  LIST(REMOVE_ITEM HEADER_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/breakpad.h")
ENDIF()
SET(ALL_FILES_COMMON_DIR ${SOURCE_FILES_COMMON_DIR} ${HEADER_FILES_COMMON_DIR} ${PROTO_FILES_COMMON_DIR})
LIST(REMOVE_ITEM SOURCE_FILES_COMMON_DIR ${SOURCE_PROTO_FILES_COMMON_DIR})
LIST(REMOVE_ITEM HEADER_FILES_COMMON_DIR ${HEADER_PROTO_FILES_COMMON_DIR} ${MS_HEADER_PROTO_FILES_COMMON_DIR})
SOURCE_GROUP("common Source Files" FILES ${SOURCE_FILES_COMMON_DIR})
SOURCE_GROUP("common Header Files" FILES ${HEADER_FILES_COMMON_DIR})
SOURCE_GROUP("Common Proto Files" FILES ${PROTO_FILES_COMMON_DIR} ${SOURCE_PROTO_FILES_COMMON_DIR} ${HEADER_PROTO_FILES_COMMON_DIR} ${MS_HEADER_PROTO_FILES_COMMON_DIR})

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
# Define MaidSafe libraries and executables                                                       #
###################################################################################################
MS_ADD_STATIC_LIBRARY(${MS_PROJECT_NAME} ${ALL_FILES_COMMON_DIR})
MS_ADD_EXECUTABLE(TESTcommon Tests ${TEST_MAIN} "${ALL_FILES_COMMON_TESTS_DIR}")
MS_ADD_EXECUTABLE(TESTboost Tests ${TEST_MAIN} ${ALL_FILES_COMMON_BOOST_TESTS_DIR})

TARGET_LINK_LIBRARIES(maidsafe_common_static cryptopp_static protobuf_static glog_static boost_date_time boost_filesystem boost_serialization boost_system boost_thread ${SYS_LIB})
TARGET_LINK_LIBRARIES(TESTcommon maidsafe_common_static gtest_static)
TARGET_LINK_LIBRARIES(TESTboost maidsafe_common_static glog_static boost_unit_test_framework gtest_static ${SYS_LIB})

###################################################################################################
# Set compiler and linker flags                                                                   #
###################################################################################################
INCLUDE(maidsafe_standard_flags)
IF(MSVC)
  # Warnings switched off for protocol buffer generated files
  SET_SOURCE_FILES_PROPERTIES(${PROJECT_SOURCE_DIR}/src/maidsafe/common/safe_encrypt.pb.cc
                                PROPERTIES COMPILE_FLAGS "/W0")
ELSEIF(UNIX)
  SET_SOURCE_FILES_PROPERTIES(${PROJECT_SOURCE_DIR}/src/maidsafe/common/safe_encrypt.pb.cc
                                PROPERTIES COMPILE_FLAGS "-w")
  SET_SOURCE_FILES_PROPERTIES(${PROJECT_SOURCE_DIR}/src/maidsafe/common/tests/threadsafe_chunk_store_test.cc
                                PROPERTIES COMPILE_FLAGS "-Wstrict-overflow=2")
ENDIF()

###################################################################################################
# Tests                                                                                           #
###################################################################################################
SET(STYLE_TESTS ${ALL_FILES_COMMON_DIR} ${ALL_FILES_COMMON_TESTS_DIR})
LIST(REMOVE_ITEM STYLE_TESTS "${PROJECT_SOURCE_DIR}/src/maidsafe/common/safe_encrypt.pb.cc")
LIST(REMOVE_ITEM STYLE_TESTS "${PROJECT_SOURCE_DIR}/src/maidsafe/common/safe_encrypt.pb.h")
ADD_STYLE_TEST(${STYLE_TESTS})
ADD_GTESTS(TESTcommon)
ADD_GTESTS(TESTboost)
INCLUDE(${CRYPTO_TEST_CMAKE})

TEST_SUMMARY_OUTPUT()

# Remove files from coverage stats
ADD_COVERAGE_EXCLUDE(cryptopp/)
ADD_COVERAGE_EXCLUDE(third_party_libs/)
ADD_COVERAGE_EXCLUDE(utils\\\\.h)

# Remove third-party tests from MemCheck runs.
ADD_MEMCHECK_IGNORE(boost.BEH_BOOST_bind_function_simple)
ADD_MEMCHECK_IGNORE(boost.BEH_BOOST_bind_function_object1)
ADD_MEMCHECK_IGNORE(boost.BEH_BOOST_bind_function_object2)
ADD_MEMCHECK_IGNORE(boost.BEH_BOOST_bind_member_function_non_void)
ADD_MEMCHECK_IGNORE(boost.BEH_BOOST_bind_member_function_void)
ADD_MEMCHECK_IGNORE(boost.BEH_BOOST_nested_bind)
ADD_MEMCHECK_IGNORE(boost.BEH_BOOST_smart_ptr_AtomicCount1)
ADD_MEMCHECK_IGNORE(boost.BEH_BOOST_smart_ptr_AtomicCount2)
ADD_MEMCHECK_IGNORE(boost.BEH_BOOST_smart_ptr_make_shared)
ADD_MEMCHECK_IGNORE(boost.BEH_BOOST_formattest1)
ADD_MEMCHECK_IGNORE(boost.BEH_BOOST_formattest2)
ADD_MEMCHECK_IGNORE(CrypoppTest.BEH_FULL)
ADD_MEMCHECK_IGNORE(CrypoppTest.BEH_AES)
ADD_MEMCHECK_IGNORE(CrypoppTest.BEH_SHA)
ADD_MEMCHECK_IGNORE(CrypoppTest.BEH_RSA_PSS)
ADD_MEMCHECK_IGNORE(CrypoppTest.BEH_RSA_OAEP)
ADD_MEMCHECK_IGNORE(CrypoppTest.BEH_RSA_PKCS1_1_5)


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
MESSAGE(" ${ERROR_MESSAGE_CMAKE_PATH} -DFORCE_THIRD_PARTY_BUILD=GLOG")
MESSAGE(" ${ERROR_MESSAGE_CMAKE_PATH} -DFORCE_THIRD_PARTY_BUILD=BREAKPAD\n\n")
MESSAGE("To build and install this project now, run:\n")
IF(MSVC)
  MESSAGE("    cmake --build . --config Release --target install")
  MESSAGE("    cmake --build . --config Debug --target install")
ELSE()
  MESSAGE("    cmake --build . --target install")
ENDIF()
MESSAGE("\n\n================================================================================"\n)

RENAME_OUTDATED_BUILT_EXES()

FILE(STRINGS ${PROJECT_SOURCE_DIR}/src/maidsafe/common/version.h MAIDSAFE_COMMON_VERSION
       REGEX "VERSION [0-9]+$")
STRING(REGEX MATCH "[0-9]+$" MAIDSAFE_COMMON_VERSION ${MAIDSAFE_COMMON_VERSION})

MS_INSTALL_LIBS(maidsafe_common_static)
LIST(REMOVE_ITEM HEADER_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/safe_encrypt.pb.h")
LIST(REMOVE_ITEM HEADER_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/safe_encrypt_pb.h")
MS_INSTALL_HEADERS(common ${HEADER_FILES_COMMON_DIR})

INSTALL(FILES ${MODULE_FILES} DESTINATION share/maidsafe/cmake_modules)
INSTALL(FILES ${TOOL_FILES} DESTINATION share/maidsafe/tools)
INSTALL(FILES ${EXPORT_BOOST_CMAKE} DESTINATION share/maidsafe)
MS_INSTALL_EXPORT()

FIND_FILE(CONINUOUS_SCRIPT_CMAKE maidsafe_dashboard.cmake PATHS ${CMAKE_MODULE_PATH})
IF(CONINUOUS_SCRIPT_CMAKE)
  CONFIGURE_FILE(${CONINUOUS_SCRIPT_CMAKE} ${CMAKE_BINARY_DIR}/maidsafe_dashboard.cmake @ONLY)
  INSTALL(FILES ${CMAKE_BINARY_DIR}/maidsafe_dashboard.cmake DESTINATION share/maidsafe/cmake_modules)
ENDIF()
