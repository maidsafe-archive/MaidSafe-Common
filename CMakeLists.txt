#==================================================================================================#
# Copyright 2012 MaidSafe.net limited                                                              #
#                                                                                                  #
# This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or     #
# later, and The General Public License (GPL), version 3. By contributing code to this project You #
# agree to the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in the     #
# root directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also         #
# available at:                                                                                    #
#                                                                                                  #
# http://www.novinet.com/license                                                                   #
#                                                                                                  #
# Unless required by applicable law or agreed to in writing, software distributed under the        #
# License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either #
# express or implied. See the License for the specific language governing permissions and          #
# limitations under the License.                                                                   #
#                                                                                                  #
#==================================================================================================#

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

cmake_minimum_required(VERSION 2.8.10 FATAL_ERROR)
project(common)

include(../../cmake_modules/standard_setup.cmake)


#==================================================================================================#
# Set up all files as GLOBs                                                                        #
#==================================================================================================#
set(CommonSourcesDir ${PROJECT_SOURCE_DIR}/src/maidsafe/common)
glob_dir(Common ${CommonSourcesDir} Common)
glob_dir(CommonTools ${CommonSourcesDir}/tools Tools)
if(APPLE)
  # Currently not supporting breakpad using OSX, so remove breakpad header and source
  list(REMOVE_ITEM CommonAllFiles ${CommonSourcesDir}/breakpad.cc)
  list(REMOVE_ITEM CommonAllFiles ${CommonSourcesDir}/breakpad.h)
endif()
set(TestsMain ${CommonSourcesDir}/tests/tests_main.cc)
list(REMOVE_ITEM CommonAllFiles ${TestsMain})

glob_dir(CommonTests ${CommonSourcesDir}/tests Tests)
glob_dir(BoostTests ${CommonSourcesDir}/tests/boost "Boost Tests")


#==================================================================================================#
# Define MaidSafe libraries and executables                                                        #
#==================================================================================================#
ms_add_static_library(common ${CommonAllFiles})
ms_add_static_library(network_viewer ${CommonToolsAllFiles})
add_dependencies(maidsafe_common boost boost_process)
# target_include_directories(maidsafe_common INTERFACE ${PROJECT_SOURCE_DIR}/include)
# target_include_directories(maidsafe_network_viewer INTERFACE ${PROJECT_SOURCE_DIR}/include)

if(MaidsafeTesting)
  ms_add_executable(TESTcommon "Tests/Common" ${TestsMain} ${CommonTestsAllFiles})
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
  add_gtests(TESTcommon)
  # add_gtests(TESTboost)
  add_custom_command(TARGET TESTcommon
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory $<TARGET_FILE_DIR:cryptest>/TestData ${CMAKE_CURRENT_BINARY_DIR}/TestData)
  add_custom_command(TARGET TESTcommon
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory $<TARGET_FILE_DIR:cryptest>/TestVectors ${CMAKE_CURRENT_BINARY_DIR}/TestVectors)
  add_test(NAME CryptoppTest.BEH_FULL WORKING_DIRECTORY $<TARGET_FILE_DIR:cryptest> COMMAND $<TARGET_FILE:cryptest> v)
  add_test(NAME CryptoppTest.BEH_AES WORKING_DIRECTORY $<TARGET_FILE_DIR:cryptest> COMMAND $<TARGET_FILE:cryptest> tv aes)
  add_test(NAME CryptoppTest.BEH_SHA WORKING_DIRECTORY $<TARGET_FILE_DIR:cryptest> COMMAND $<TARGET_FILE:cryptest> tv sha)
  add_test(NAME CryptoppTest.BEH_RSA_PSS WORKING_DIRECTORY $<TARGET_FILE_DIR:cryptest> COMMAND $<TARGET_FILE:cryptest> tv rsa_pss)
  add_test(NAME CryptoppTest.BEH_RSA_OAEP WORKING_DIRECTORY $<TARGET_FILE_DIR:cryptest> COMMAND $<TARGET_FILE:cryptest> tv rsa_oaep)
  add_test(NAME CryptoppTest.BEH_RSA_PKCS1_1_5 WORKING_DIRECTORY $<TARGET_FILE_DIR:cryptest> COMMAND $<TARGET_FILE:cryptest> tv rsa_pkcs1_1_5)

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

  label_as_critical_tests(AsioServiceTest.BEH_All
                          UtilsTest.BEH_VersionToString
                          UtilsTest.BEH_VersionToInt
                          UtilsTest.BEH_Cpu_Size
                          UtilsTest.BEH_Names
                          UtilsTest.BEH_RandomStringMultiThread
                          UtilsTest.BEH_RandomStringGenerator
                          UtilsTest.BEH_RandomStringSingleThread
                          UtilsTest.BEH_Base32EncodeDecode
                          UtilsTest.BEH_TimeFunctions
                          UtilsTest.BEH_RandomNumberGen
                          UtilsTest.BEH_ReadWriteFile
                          UtilsTest.BEH_Sleep
                          UtilsTest.BEH_AppDir
                          UtilsTest.BEH_CreateTestPath
                          RSATest.BEH_RsaEncodeKeys
                          RSATest.BEH_AsymEncryptDecrypt
                          RSATest.FUNC_SignValidate
                          RSATest.BEH_RsaKeysComparing
                          BoundedStringTest.BEH_DefaultConstructor
                          BoundedStringTest.BEH_Getters
                          BoundedStringTest.BEH_StringConstructor
                          BoundedStringTest.BEH_Swap
                          BoundedStringTest.BEH_Copy
                          BoundedStringTest.BEH_Move
                          BoundedStringTest.BEH_Assignment
                          BoundedStringTest.BEH_OtherBoundedStringConstructor
                          BoundedStringTest.BEH_OtherBoundedStringAssignment
                          BoundedStringTest.BEH_ConcatenationOperators
                          NodeIdTest.BEH_DefaultConstructor
                          NodeIdTest.BEH_DistanceCheck
                          NodeIdTest.BEH_BitToByteCount
                          NodeIdTest.BEH_DefaultCtr
                          NodeIdTest.BEH_CopyCtr
                          NodeIdTest.BEH_KadIdTypeCtr
                          NodeIdTest.BEH_StringCtr
                          NodeIdTest.BEH_EncodingCtr
                          NodeIdTest.BEH_OperatorEqual
                          NodeIdTest.BEH_OperatorDifferent
                          NodeIdTest.BEH_OperatorGreaterThan
                          NodeIdTest.BEH_OperatorLessThan
                          NodeIdTest.BEH_OperatorGreaterEqual
                          NodeIdTest.BEH_OperatorLessEqual
                          NodeIdTest.BEH_OperatorXOR
                          CryptoTest.BEH_Xor
                          CryptoTest.BEH_SecurePasswordGeneration
                          CryptoTest.BEH_Hash
                          CryptoTest.BEH_SymmEncrypt
                          CryptoTest.BEH_Compress
                          CryptoTest.BEH_GzipSHA512Deterministic
                          CryptoTest.BEH_AESTigerDeterministic
                          CryptoTest.BEH_SecretSharing
                          KeyValueBufferTest.BEH_Constructor
                          KeyValueBufferTest.BEH_SetMaxDiskMemoryUsage
                          KeyValueBufferTest.BEH_RemoveDiskBuffer
                          KeyValueBufferTest.BEH_SuccessfulStore
                          KeyValueBufferTest.BEH_UnsuccessfulStore
                          KeyValueBufferTest.BEH_DeleteOnDiskBufferOverfill
                          KeyValueBufferTest.BEH_PopOnDiskBufferOverfill
                          KeyValueBufferTest.BEH_AsyncPopOnDiskBufferOverfill
                          KeyValueBufferTest.BEH_AsyncNonPopOnDiskBufferOverfill
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Store/0
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Store/1
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Store/2
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Store/3
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Store/4
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Store/5
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Store/6
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Store/7
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Store/8
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Delete/0
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Delete/1
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Delete/2
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Delete/3
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Delete/4
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Delete/5
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Delete/6
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Delete/7
                          TestKeyValueBuffer/KeyValueBufferTestDiskMemoryUsage.BEH_Delete/8
                          )

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
