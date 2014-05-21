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

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake_modules/standard_setup.cmake")
  cmake_minimum_required(VERSION 2.8)  # To suppress warning cluttering error message
  set(Msg "\nThis project can currently only be build as part of the MaidSafe super-project.  For")
  set(Msg "${Msg} full details, see https://github.com/maidsafe/MaidSafe/wiki/Build-Instructions\n")
  message(FATAL_ERROR "${Msg}")
endif()

project(common)

include(../../cmake_modules/standard_setup.cmake)


#==================================================================================================#
# Set up all files as GLOBs                                                                        #
#==================================================================================================#
set(CommonSourcesDir ${PROJECT_SOURCE_DIR}/src/maidsafe/common)
ms_glob_dir(Common ${CommonSourcesDir} Common)
ms_glob_dir(CommonTests ${CommonSourcesDir}/tests Tests)
ms_glob_dir(BoostTests ${CommonSourcesDir}/tests/boost "Boost Tests")

ms_glob_dir(CommonAuthentication ${CommonSourcesDir}/authentication "Authentication")
ms_glob_dir(CommonAuthenticationDetail ${CommonSourcesDir}/authentication/detail "Authentication Detail")
ms_glob_dir(CommonAuthenticationTests ${CommonSourcesDir}/authentication/tests "Authentication Tests")

ms_glob_dir(CommonDataStores ${CommonSourcesDir}/data_stores "Data Stores")
ms_glob_dir(CommonDataStoresTests ${CommonSourcesDir}/data_stores/tests "Data Stores Tests")
set(CommonDataStoresTestsUtilsFiles
    ${CommonSourcesDir}/data_stores/tests/test_utils.cc
    ${CommonSourcesDir}/data_stores/tests/test_utils.h)
list(REMOVE_ITEM CommonDataStoresTestsAllFiles ${CommonDataStoresTestsUtilsFiles})

ms_glob_dir(CommonDataTypes ${CommonSourcesDir}/data_types "Data Types")
ms_glob_dir(CommonDataTypesTests ${CommonSourcesDir}/data_types/tests "Data Types Tests")

ms_glob_dir(CommonTools ${CommonSourcesDir}/tools Tools)
ms_glob_dir(CommonToolsTests ${CommonSourcesDir}/tools/tests "Tools Tests")

set(TestsMain ${CommonSourcesDir}/tests/tests_main.cc)
set(CommonGTestsAllFiles ${CommonTestsAllFiles})
set(CommonTestsAllFiles ${CommonSourcesDir}/tests/asio_service_test.cc
                        ${CommonSourcesDir}/tests/bounded_string_test.cc
                        ${CommonSourcesDir}/tests/errors_test.cc
                        ${CommonSourcesDir}/tests/ipc_test.cc
                        ${CommonSourcesDir}/tests/make_unique_test.cc
                        ${CommonSourcesDir}/tests/node_id_test.cc
                        ${CommonSourcesDir}/tests/on_scope_exit_test.cc
                        ${CommonSourcesDir}/tests/utils_test.cc
                        ${CommonSourcesDir}/tests/cli_test.cc
                        ${CommonSourcesDir}/tests/visualiser_log_test.cc)
set(IpcChildProcess ${CommonSourcesDir}/tests/ipc_child_process.cc)
set(ConfigTestFile ${CommonSourcesDir}/tests/config_test.cc)
list(REMOVE_ITEM CommonGTestsAllFiles ${CommonTestsAllFiles} ${IpcChildProcess} ${ConfigTestFile})
list(APPEND CommonTestsAllFiles ${TestsMain})


#==================================================================================================#
# Define MaidSafe libraries and executables                                                        #
#==================================================================================================#
# Common library
ms_add_static_library(maidsafe_common
    ${CommonAllFiles}
    ${CommonAuthenticationAllFiles}
    ${CommonAuthenticationDetailAllFiles}
    ${CommonDataStoresAllFiles}
    ${CommonDataTypesAllFiles})
add_dependencies(maidsafe_common boost_process catch)
target_include_directories(maidsafe_common
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/GeneratedProtoFiles
  PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)
ms_target_include_system_dirs(maidsafe_common PUBLIC ${BoostSourceDir} ${CatchSourceDir})
if(JustThread_FOUND)
  ms_target_include_system_dirs(maidsafe_common PUBLIC ${JustThread_INCLUDE_DIR})
endif()
if(MSVC12 AND CMAKE_CL_64)
  message(AUTHOR_WARNING "Remove the \"/GS-\" flag from ipc.cc once https://svn.boost.org/trac/boost/ticket/9666 is resolved.")
  set_property(SOURCE ${CommonSourcesDir}/ipc.cc APPEND_STRING PROPERTY COMPILE_FLAGS " /GS-")
endif()
target_link_libraries(maidsafe_common
    maidsafe_passport
    cryptopp
    protobuf
    gtest
    ${BoostChronoLibs}
    ${BoostDateTimeLibs}
    ${BoostFilesystemLibs}
    ${BoostLocaleLibs}
    ${BoostRegexLibs}
    ${BoostSystemLibs}
    ${BoostThreadLibs}
    ${BoostProgramOptionsLibs}
    ${JustThread_LIBRARIES}
    ${SYS_LIB})
if(TARGET check_sanitizer_blacklist)
  add_dependencies(maidsafe_common check_sanitizer_blacklist)
endif()

# Network Viewer library
ms_add_static_library(maidsafe_network_viewer
    "${CommonSourcesDir}/tools/network_viewer.cc"
    "${PROJECT_SOURCE_DIR}/include/maidsafe/common/tools/network_viewer.h"
    "${CMAKE_BINARY_DIR}/GeneratedProtoFiles/maidsafe/common/tools/network_viewer.pb.cc"
    "${CMAKE_BINARY_DIR}/GeneratedProtoFiles/maidsafe/common/tools/network_viewer.pb.h"
    "${CommonSourcesDir}/tools/network_viewer.proto")
target_include_directories(maidsafe_network_viewer PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(maidsafe_network_viewer maidsafe_common)

# Signing tool
ms_add_executable(signing_tool "Tools/Common" "${CommonSourcesDir}/tools/signing_tool.cc")
target_link_libraries(signing_tool maidsafe_common maidsafe_passport)

# Bootstrap file tool
ms_add_executable(bootstrap_file_tool "Tools/Common"
    "${CommonSourcesDir}/tools/bootstrap_file_tool.cc"
    "${CMAKE_BINARY_DIR}/GeneratedProtoFiles/maidsafe/common/tools/bootstrap_file_tool.pb.cc"
    "${CMAKE_BINARY_DIR}/GeneratedProtoFiles/maidsafe/common/tools/bootstrap_file_tool.pb.h")
target_link_libraries(bootstrap_file_tool maidsafe_common)

# Address space tool
ms_add_executable(address_space_tool "Tools/Common" "${CommonSourcesDir}/tools/address_space_tool.cc")
target_link_libraries(address_space_tool maidsafe_common)

# Tests
if(MaidsafeTesting)
  ms_add_executable(test_common "Tests/Common" ${CommonTestsAllFiles} ${CommonAuthenticationTestsAllFiles})
  target_link_libraries(test_common maidsafe_common)

  ms_add_executable(ipc_child_process "Tests/Common" ${IpcChildProcess})
  target_link_libraries(ipc_child_process maidsafe_common)
  add_dependencies(test_common ipc_child_process)

  ms_add_executable(test_config "Tests/Common" ${ConfigTestFile})
  target_link_libraries(test_config maidsafe_common)

  ms_add_executable(TESTcommon "Tests/Common" ${CommonGTestsAllFiles})
  target_link_libraries(TESTcommon maidsafe_common)

  macro(CommonTests Group)
    foreach(TestSource ${ARGN})
      get_filename_component(Exe ${TestSource} NAME_WE)
      string(REGEX REPLACE "(.*)_test" "test_\\1" Exe ${Exe})
      set(TestSources ${TestSource} ${TestsMain})
      ms_add_executable(${Exe} "Tests/Common/${Group}" ${TestSources})
      target_link_libraries(${Exe} ${${Exe}AdditionalLibs} maidsafe_common)
      ms_add_catch_tests(${Exe})
    endforeach()
  endmacro()

  ms_add_static_library(maidsafe_data_stores_test_utils ${CommonDataStoresTestsUtilsFiles})
  set_target_properties(maidsafe_data_stores_test_utils PROPERTIES FOLDER "MaidSafe/Executables/Tests/Common/Data Store Tests")
  target_include_directories(maidsafe_data_stores_test_utils PUBLIC ${PROJECT_SOURCE_DIR}/src)
  target_link_libraries(maidsafe_data_stores_test_utils maidsafe_common)
  foreach(TestExe test_data_store test_memory_buffer test_permanent_store)
    set(${TestExe}AdditionalLibs maidsafe_data_stores_test_utils)
  endforeach()
  CommonTests("Data Store Tests" ${CommonDataStoresTestsAllFiles})

  CommonTests("Data Types Tests" ${CommonDataTypesTestsAllFiles})
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
