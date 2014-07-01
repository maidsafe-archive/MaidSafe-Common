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
set(TestLibAllFiles ${CommonSourcesDir}/test.cc ${PROJECT_SOURCE_DIR}/include/maidsafe/common/test.h)
list(REMOVE_ITEM CommonAllFiles ${TestLibAllFiles})
ms_glob_dir(CommonTests ${CommonSourcesDir}/tests "Tests")
ms_glob_dir(BoostTests ${CommonSourcesDir}/tests/boost "Boost Tests")

ms_glob_dir(CommonAuthentication ${CommonSourcesDir}/authentication "Authentication")
ms_glob_dir(CommonAuthenticationDetail ${CommonSourcesDir}/authentication/detail "Authentication Detail")
ms_glob_dir(CommonAuthenticationTests ${CommonSourcesDir}/authentication/tests "Authentication Tests")

ms_glob_dir(CommonDataTypes ${CommonSourcesDir}/data_types "Data Types")
ms_glob_dir(CommonDataTypesTests ${CommonSourcesDir}/data_types/tests "Data Types Tests")

ms_glob_dir(CommonTools ${CommonSourcesDir}/tools Tools)
ms_glob_dir(CommonToolsTests ${CommonSourcesDir}/tools/tests "Tools Tests")

set(TestsMain ${CommonSourcesDir}/tests/tests_main.cc)
set(IpcChildProcess ${CommonSourcesDir}/tests/ipc_child_process.cc)
set(ConfigTestFile ${CommonSourcesDir}/tests/config_test.cc)
list(REMOVE_ITEM CommonTestsAllFiles ${IpcChildProcess} ${ConfigTestFile} ${CommonAuthenticationTestsAllFiles})


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
add_dependencies(maidsafe_common boost_process)
target_include_directories(maidsafe_common
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/GeneratedProtoFiles
  PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)
ms_target_include_system_dirs(maidsafe_common PUBLIC ${BoostSourceDir})
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

# Qa tool
ms_add_executable(qa_tool "Tools/Common" "${CommonSourcesDir}/tools/qa_tool.cc")
target_link_libraries(qa_tool maidsafe_common maidsafe_passport)

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
  ms_add_static_library(maidsafe_test ${TestLibAllFiles})
  target_link_libraries(maidsafe_test maidsafe_common gtest)

  ms_add_executable(TESTconfig "Tests/Common" ${ConfigTestFile})
  target_link_libraries(TESTconfig maidsafe_common)

  ms_add_executable(TESTcommon "Tests/Common" ${CommonTestsAllFiles} ${CommonAuthenticationTestsAllFiles})
  target_link_libraries(TESTcommon maidsafe_test)
  
  ms_add_executable(ipc_child_process "Tests/Common" ${IpcChildProcess})
  target_link_libraries(ipc_child_process maidsafe_common)
  add_dependencies(TESTcommon ipc_child_process)

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
  ms_add_gtests(TESTconfig)
  ms_add_gtests(TESTcommon)
  ms_add_project_experimental()
  ms_test_summary_output()
endif()


#==================================================================================================#
# Package                                                                                          #
#==================================================================================================#
install(TARGETS maidsafe_common maidsafe_network_viewer COMPONENT Development CONFIGURATIONS Debug Release ARCHIVE DESTINATION lib)
install(TARGETS signing_tool qa_tool bootstrap_file_tool address_space_tool COMPONENT Tools CONFIGURATIONS Debug RUNTIME DESTINATION bin/debug)
install(TARGETS signing_tool qa_tool bootstrap_file_tool address_space_tool COMPONENT Tools CONFIGURATIONS Release RUNTIME DESTINATION bin)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ COMPONENT Development DESTINATION include)

if(MaidsafeTesting)
  install(TARGETS ipc_child_process TESTconfig TESTcommon COMPONENT Tests CONFIGURATIONS Debug RUNTIME DESTINATION bin/debug)
  install(TARGETS ipc_child_process TESTconfig TESTcommon COMPONENT Tests CONFIGURATIONS Release RUNTIME DESTINATION bin)
endif()
