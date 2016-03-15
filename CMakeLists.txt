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
ms_glob_dir(Common ${CommonSourcesDir} Common)
ms_glob_dir(CommonTools ${CommonSourcesDir}/tools Tools)
ms_glob_dir(CommonTests ${CommonSourcesDir}/tests Tests)
ms_glob_dir(BoostTests ${CommonSourcesDir}/tests/boost "Boost Tests")
set(TestsMain ${CommonSourcesDir}/tests/tests_main.cc)
set(CommonGTestsAllFiles ${CommonTestsAllFiles})
set(CommonTestsAllFiles ${CommonSourcesDir}/tests/asio_service_test.cc
                        ${CommonSourcesDir}/tests/bounded_string_test.cc
                        ${CommonSourcesDir}/tests/errors_test.cc
                        ${CommonSourcesDir}/tests/ipc_test.cc
                        ${CommonSourcesDir}/tests/node_id_test.cc
                        ${CommonSourcesDir}/tests/on_scope_exit_test.cc
                        ${CommonSourcesDir}/tests/utils_test.cc)
set(IpcChildProcess ${CommonSourcesDir}/tests/ipc_child_process.cc)
set(ConfigTestFile ${CommonSourcesDir}/tests/config_test.cc)
list(REMOVE_ITEM CommonGTestsAllFiles ${CommonTestsAllFiles} ${IpcChildProcess} ${ConfigTestFile})
list(APPEND CommonTestsAllFiles ${TestsMain})


#==================================================================================================#
# Define MaidSafe libraries and executables                                                        #
#==================================================================================================#
ms_add_static_library(maidsafe_common ${CommonAllFiles})
ms_add_static_library(maidsafe_network_viewer ${CommonToolsAllFiles})
add_dependencies(maidsafe_common boost_process catch)
target_include_directories(maidsafe_common
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/GeneratedProtoFiles
  PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)
ms_target_include_system_dirs(maidsafe_common
  PUBLIC
    ${BoostSourceDir}
    ${CatchSourceDir}
    $<$<BOOL:${JustThread_FOUND}>:${JustThread_INCLUDE_DIR}>
)
target_include_directories(maidsafe_network_viewer PUBLIC ${PROJECT_SOURCE_DIR}/include)

if(MaidsafeTesting)
  ms_add_executable(test_common "Tests/Common" ${CommonTestsAllFiles})
  ms_add_executable(ipc_child_process "Tests/Common" ${IpcChildProcess})
  add_dependencies(test_common ipc_child_process)
  ms_add_executable(test_config "Tests/Common" ${ConfigTestFile})
  ms_add_executable(TESTcommon "Tests/Common" ${CommonGTestsAllFiles})
endif()

target_link_libraries(maidsafe_common
                        cryptopp
                        protobuf
                        gtest
                        ${BoostChronoLibs}
                        ${BoostDateTimeLibs}
                        ${BoostFilesystemLibs}
                        ${BoostLocaleLibs}
                        ${BoostSystemLibs}
                        ${BoostThreadLibs}
                        ${BoostProgramOptionsLibs}
                        ${JustThread_LIBRARIES}
                        ${SYS_LIB})

target_link_libraries(maidsafe_network_viewer maidsafe_common)

set(surefile_crackerName SureFile)
ms_add_executable(surefile_cracker "Tools/Common" ${CommonSourcesDir}/tools/main.cpp)
target_link_libraries(surefile_cracker maidsafe_common)

if(MaidsafeTesting)
  target_link_libraries(test_common maidsafe_common)
  target_link_libraries(ipc_child_process maidsafe_common)
  target_link_libraries(test_config maidsafe_common)
  target_link_libraries(TESTcommon maidsafe_common)
endif()
ms_rename_outdated_built_exes()


#==================================================================================================#
# Set compiler and linker flags                                                                    #
#==================================================================================================#
include(${CMAKE_CURRENT_SOURCE_DIR}/common_flags.cmake)
include(standard_flags)


#==================================================================================================#
# Tests                                                                                            #
#==================================================================================================#
if(MaidsafeTesting)
  ms_add_style_test()
  ms_add_catch_tests(test_common)
  ms_add_catch_tests(test_config)
  ms_add_gtests(TESTcommon)
  ms_add_project_experimental()
  ms_test_summary_output()
endif()

# Remove files from coverage stats
ms_add_coverage_exclude(cryptopp/)
ms_add_coverage_exclude(third_party_libs/)
ms_add_coverage_exclude(utils\\\\.h)
