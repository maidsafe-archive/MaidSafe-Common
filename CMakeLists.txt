#==============================================================================#
#                                                                              #
#  Copyright (c) 2012 MaidSafe.net limited                                     #
#                                                                              #
#  The following source code is property of MaidSafe.net limited and is not    #
#  meant for external use.  The use of this code is governed by the license    #
#  file licence.txt found in the root directory of this project and also on    #
#  www.maidsafe.net.                                                           #
#                                                                              #
#  You are not free to copy, amend or otherwise use this source code without   #
#  the explicit written permission of the board of directors of MaidSafe.net.  #
#                                                                              #
#==============================================================================#


set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
project(common)

set(PROTO_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
file(GLOB PROTO_FILES RELATIVE ${PROTO_SOURCE_DIR} "${PROTO_SOURCE_DIR}/maidsafe/common/*.proto")
include(../../cmake_modules/maidsafe_standard_setup.cmake)


###################################################################################################
# Set up all files as GLOBs                                                                       #
###################################################################################################
file(GLOB SOURCE_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/*.cc")
file(GLOB HEADER_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/*.h")
file(GLOB PROTO_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/*.proto")
file(GLOB SOURCE_PROTO_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/*.pb.cc")
file(GLOB HEADER_PROTO_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/*.pb.h")
file(GLOB MS_HEADER_PROTO_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/*_pb.h")
file(GLOB API_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/include/maidsafe/common/*.h")

if(APPLE)
  # Currently not supporting breakpad using OSX, so remove breakpad header and source
  list(REMOVE_ITEM SOURCE_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/breakpad.cc")
  list(REMOVE_ITEM HEADER_FILES_COMMON_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/breakpad.h")
endif()
set(ALL_FILES_COMMON_DIR ${SOURCE_FILES_COMMON_DIR} ${HEADER_FILES_COMMON_DIR} ${PROTO_FILES_COMMON_DIR} ${API_FILES_COMMON_DIR})
list(REMOVE_ITEM SOURCE_FILES_COMMON_DIR ${SOURCE_PROTO_FILES_COMMON_DIR})
list(REMOVE_ITEM HEADER_FILES_COMMON_DIR ${HEADER_PROTO_FILES_COMMON_DIR} ${MS_HEADER_PROTO_FILES_COMMON_DIR})
source_group("Common Source Files" FILES ${SOURCE_FILES_COMMON_DIR})
source_group("Common Header Files" FILES ${HEADER_FILES_COMMON_DIR})
source_group("Common Proto Files" FILES ${PROTO_FILES_COMMON_DIR} ${SOURCE_PROTO_FILES_COMMON_DIR} ${HEADER_PROTO_FILES_COMMON_DIR} ${MS_HEADER_PROTO_FILES_COMMON_DIR})
source_group("Common API Files" FILES ${API_FILES_COMMON_DIR})

set(TEST_MAIN "${PROJECT_SOURCE_DIR}/src/maidsafe/common/tests/main.cc")

file(GLOB SOURCE_FILES_COMMON_TESTS_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/tests/*.cc")
file(GLOB HEADER_FILES_COMMON_TESTS_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/tests/*.h")
set(ALL_FILES_COMMON_TESTS_DIR ${SOURCE_FILES_COMMON_TESTS_DIR} ${HEADER_FILES_COMMON_TESTS_DIR})
source_group("common tests Source Files" FILES ${SOURCE_FILES_COMMON_TESTS_DIR})
source_group("common tests Header Files" FILES ${HEADER_FILES_COMMON_TESTS_DIR})

file(GLOB SOURCE_FILES_COMMON_BOOST_TESTS_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/tests/boost/*.cc")
file(GLOB HEADER_FILES_COMMON_BOOST_TESTS_DIR "${PROJECT_SOURCE_DIR}/src/maidsafe/common/tests/boost/*.h")
set(ALL_FILES_COMMON_BOOST_TESTS_DIR ${SOURCE_FILES_COMMON_BOOST_TESTS_DIR} ${HEADER_FILES_COMMON_BOOST_TESTS_DIR})
source_group("common boost tests Source Files" FILES ${SOURCE_FILES_COMMON_BOOST_TESTS_DIR})
source_group("common boost tests Header Files" FILES ${HEADER_FILES_COMMON_BOOST_TESTS_DIR})


###################################################################################################
# Define MaidSafe libraries and executables                                                       #
###################################################################################################
ms_add_static_library(common ${ALL_FILES_COMMON_DIR})
ms_add_executable(TESTcommon Tests ${TEST_MAIN} "${ALL_FILES_COMMON_TESTS_DIR}")
#ms_add_executable(TESTboost Tests ${TEST_MAIN} ${ALL_FILES_COMMON_BOOST_TESTS_DIR})

target_link_libraries(maidsafe_common
                        cryptopp
                        protobuf
                        glog
                        gmock
                        gtest
                        boost_date_time-static
                        boost_filesystem-static
                        boost_serialization-static
                        boost_system-static
                        boost_thread-static
                        ${SYS_LIB})
target_link_libraries(TESTcommon maidsafe_common)
#target_link_libraries(TESTboost maidsafe_common boost_unit_test_framework-static gtest ${SYS_LIB})
rename_outdated_built_exes()


###################################################################################################
# Set compiler and linker flags                                                                   #
###################################################################################################
include(maidsafe_standard_flags)
ms_disable_warnings(${PROJECT_SOURCE_DIR}/src/maidsafe/common/safe_encrypt.pb.cc)


###################################################################################################
# Tests                                                                                           #
###################################################################################################
set(STYLE_TESTS ${ALL_FILES_COMMON_DIR} ${ALL_FILES_COMMON_TESTS_DIR})
list(REMOVE_ITEM STYLE_TESTS "${PROJECT_SOURCE_DIR}/src/maidsafe/common/safe_encrypt.pb.cc")
list(REMOVE_ITEM STYLE_TESTS "${PROJECT_SOURCE_DIR}/src/maidsafe/common/safe_encrypt.pb.h")
list(REMOVE_ITEM STYLE_TESTS "${PROJECT_SOURCE_DIR}/src/maidsafe/common/safe_encrypt.proto")
add_style_test(${STYLE_TESTS})
add_gtests(TESTcommon)
#add_gtests(TESTboost)

test_summary_output()

# Remove files from coverage stats
add_coverage_exclude(cryptopp/)
add_coverage_exclude(third_party_libs/)
add_coverage_exclude(utils\\\\.h)

# Remove third-party tests from MemCheck runs.
add_memcheck_ignore(boost.BEH_BOOST_bind_function_simple)
add_memcheck_ignore(boost.BEH_BOOST_bind_function_object1)
add_memcheck_ignore(boost.BEH_BOOST_bind_function_object2)
add_memcheck_ignore(boost.BEH_BOOST_bind_member_function_non_void)
add_memcheck_ignore(boost.BEH_BOOST_bind_member_function_void)
add_memcheck_ignore(boost.BEH_BOOST_nested_bind)
add_memcheck_ignore(boost.BEH_BOOST_smart_ptr_AtomicCount1)
add_memcheck_ignore(boost.BEH_BOOST_smart_ptr_AtomicCount2)
add_memcheck_ignore(boost.BEH_BOOST_smart_ptr_make_shared)
add_memcheck_ignore(boost.BEH_BOOST_formattest1)
add_memcheck_ignore(boost.BEH_BOOST_formattest2)
add_memcheck_ignore(CrypoppTest.BEH_FULL)
add_memcheck_ignore(CrypoppTest.BEH_AES)
add_memcheck_ignore(CrypoppTest.BEH_SHA)
add_memcheck_ignore(CrypoppTest.BEH_RSA_PSS)
add_memcheck_ignore(CrypoppTest.BEH_RSA_OAEP)
add_memcheck_ignore(CrypoppTest.BEH_RSA_PKCS1_1_5)


find_file(CONINUOUS_SCRIPT_CMAKE maidsafe_dashboard.cmake PATHS ${CMAKE_MODULE_PATH})
if(CONINUOUS_SCRIPT_CMAKE)
  configure_file(${CONINUOUS_SCRIPT_CMAKE} ${CMAKE_BINARY_DIR}/maidsafe_dashboard.cmake @ONLY)
  install(FILES ${CMAKE_BINARY_DIR}/maidsafe_dashboard.cmake DESTINATION share/maidsafe/cmake_modules)
endif()
