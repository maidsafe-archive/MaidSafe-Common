#==================================================================================================#
#                                                                                                  #
#  Copyright 2012 MaidSafe.net limited                                                             #
#                                                                                                  #
#  This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,        #
#  version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which    #
#  licence you accepted on initial access to the Software (the "Licences").                        #
#                                                                                                  #
#  By contributing code to the MaidSafe Software, or to this project generally, you agree to be    #
#  bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root        #
#  directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available   #
#  at: http://www.maidsafe.net/licenses                                                            #
#                                                                                                  #
#  Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed    #
#  under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF   #
#  ANY KIND, either express or implied.                                                            #
#                                                                                                  #
#  See the Licences for the specific language governing permissions and limitations relating to    #
#  use of the MaidSafe Software.                                                                   #
#                                                                                                  #
#==================================================================================================#


set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(common)

include(../../cmake_modules/standard_setup.cmake)


#==================================================================================================#
# Set up all files as GLOBs                                                                        #
#==================================================================================================#
set(CommonSourcesDir ${PROJECT_SOURCE_DIR}/src/maidsafe/common)
glob_dir(Common ${CommonSourcesDir} Common)
glob_dir(CommonTools ${CommonSourcesDir}/tools Tools)
glob_dir(CommonTests ${CommonSourcesDir}/tests Tests)
glob_dir(BoostTests ${CommonSourcesDir}/tests/boost "Boost Tests")
set(TestsMain ${CommonSourcesDir}/tests/tests_main.cc)
set(CommonGTestsAllFiles ${CommonTestsAllFiles})
set(CommonTestsAllFiles ${CommonSourcesDir}/tests/asio_service_test.cc
                        ${CommonSourcesDir}/tests/bounded_string_test.cc
                        ${CommonSourcesDir}/tests/errors_test.cc
                        ${CommonSourcesDir}/tests/ipc_test.cc
                        ${CommonSourcesDir}/tests/node_id_test.cc
                        ${CommonSourcesDir}/tests/on_scope_exit_test.cc)
set(IpcChildProcess ${CommonSourcesDir}/tests/ipc_child_process.cc)
set(LocationFile ${CMAKE_CURRENT_BINARY_DIR}/ipc_test/ipc_child_process_location.h)
list(REMOVE_ITEM CommonGTestsAllFiles ${CommonTestsAllFiles} ${IpcChildProcess})
list(APPEND CommonTestsAllFiles ${TestsMain})


#==================================================================================================#
# Define MaidSafe libraries and executables                                                        #
#==================================================================================================#
ms_add_static_library(common ${CommonAllFiles})
ms_add_static_library(network_viewer ${CommonToolsAllFiles})
add_dependencies(maidsafe_common boost boost_process catch)
# target_include_directories(maidsafe_common INTERFACE ${PROJECT_SOURCE_DIR}/include)
# target_include_directories(maidsafe_network_viewer INTERFACE ${PROJECT_SOURCE_DIR}/include)

if(MaidsafeTesting)
  ms_add_executable(test_common "Tests/Common" ${CommonTestsAllFiles} ${LocationFile})
  ms_add_executable(ipc_child_process "Tests/Common" ${IpcChildProcess})
  ms_add_executable(TESTcommon "Tests/Common" ${CommonGTestsAllFiles})
  # ms_add_executable(TESTboost "Tests/Common" ${TestsMain} ${BoostTestsAllFiles})
endif()

target_link_libraries(maidsafe_common
                        cryptopp
                        protobuf
                        gmock
                        gtest
                        ${BoostChronoLibs}
                        ${BoostDateTimeLibs}
                        ${BoostFilesystemLibs}
                        ${BoostSystemLibs}
                        ${BoostThreadLibs}
                        ${BoostProgramOptionsLibs}
                        ${JustThread_LIBRARIES})

target_link_libraries(maidsafe_common ${SYS_LIB})
target_link_libraries(maidsafe_network_viewer maidsafe_common)

if(MaidsafeTesting)
  target_link_libraries(test_common maidsafe_common)
  target_link_libraries(ipc_child_process maidsafe_common)
  add_process_location(ipc_child_process "${LocationFile}")
  add_dependencies(test_common ipc_child_process)
  get_target_property(IncDirs test_common INCLUDE_DIRECTORIES)
  set_target_properties(test_common PROPERTIES INCLUDE_DIRECTORIES "${IncDirs};${CMAKE_CURRENT_BINARY_DIR}/ipc_test")
  target_link_libraries(TESTcommon maidsafe_common)
  # target_link_libraries(TESTboost maidsafe_common boost_unit_test_framework-static gtest ${SYS_LIB})
endif()
rename_outdated_built_exes()

set(AllExesForCurrentProject ${AllExesForCurrentProject} cryptest)


#==================================================================================================#
# Set compiler and linker flags                                                                    #
#==================================================================================================#
include(standard_flags)


#==================================================================================================#
# Tests                                                                                            #
#==================================================================================================#
if(MaidsafeTesting)
  add_style_test()
  add_catch_tests(test_common)
  add_gtests(TESTcommon)
  # add_gtests(TESTboost)
  add_custom_command(TARGET TESTcommon
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory $<TARGET_FILE_DIR:cryptest>/TestData ${CMAKE_CURRENT_BINARY_DIR}/TestData)
  add_custom_command(TARGET TESTcommon
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory $<TARGET_FILE_DIR:cryptest>/TestVectors ${CMAKE_CURRENT_BINARY_DIR}/TestVectors)
  add_test(NAME CryptoppTest.BEH_FULL COMMAND cryptest v)
  add_test(NAME CryptoppTest.BEH_AES COMMAND cryptest tv aes)
  add_test(NAME CryptoppTest.BEH_SHA COMMAND cryptest tv sha)
  add_test(NAME CryptoppTest.BEH_RSA_PSS COMMAND cryptest tv rsa_pss)
  add_test(NAME CryptoppTest.BEH_RSA_OAEP COMMAND cryptest tv rsa_oaep)
  add_test(NAME CryptoppTest.BEH_RSA_PKCS1_1_5 COMMAND cryptest tv rsa_pkcs1_1_5)

  set_property(TEST CryptoppTest.BEH_FULL PROPERTY LABELS Common Behavioural CryptoPP ${CMAKE_BUILD_TYPE})
  set_property(TEST CryptoppTest.BEH_FULL PROPERTY TIMEOUT 60)
  set_property(TEST CryptoppTest.BEH_AES PROPERTY LABELS Common Behavioural CryptoPP ${CMAKE_BUILD_TYPE})
  set_property(TEST CryptoppTest.BEH_AES PROPERTY TIMEOUT 60)
  set_property(TEST CryptoppTest.BEH_SHA PROPERTY LABELS Common Behavioural CryptoPP ${CMAKE_BUILD_TYPE})
  set_property(TEST CryptoppTest.BEH_SHA PROPERTY TIMEOUT 60)
  set_property(TEST CryptoppTest.BEH_RSA_PSS PROPERTY LABELS Common Behavioural CryptoPP ${CMAKE_BUILD_TYPE})
  set_property(TEST CryptoppTest.BEH_RSA_PSS PROPERTY TIMEOUT 60)
  set_property(TEST CryptoppTest.BEH_RSA_OAEP PROPERTY LABELS Common Behavioural CryptoPP ${CMAKE_BUILD_TYPE})
  set_property(TEST CryptoppTest.BEH_RSA_OAEP PROPERTY TIMEOUT 60)
  set_property(TEST CryptoppTest.BEH_RSA_PKCS1_1_5 PROPERTY LABELS Common Behavioural CryptoPP ${CMAKE_BUILD_TYPE})
  set_property(TEST CryptoppTest.BEH_RSA_PKCS1_1_5 PROPERTY TIMEOUT 60)

  add_project_experimental()
  test_summary_output()
endif()


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
