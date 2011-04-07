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
#  Module used to build and install Third Party libs.                          #
#                                                                              #
#==============================================================================#


CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)

PROJECT(BuildThirdPartyLibs)

GET_FILENAME_COMPONENT(CURRENT_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)

IF(NOT BUILD_TARGET)
  SET(BUILD_TARGET "ALL")
ENDIF()

IF(${BUILD_TARGET} MATCHES "ALL")
  SET(MSG "\nBuilding ALL targets.\nTo change this, run cmake with one of the following options:\n")
ELSE()
  SET(MSG "\nBuilding target ${BUILD_TARGET} only.\nTo change this, run cmake with one of the following options:\n")
ENDIF()
SET(MSG "${MSG}    -DBUILD_TARGET=ALL\n")
SET(MSG "${MSG}    -DBUILD_TARGET=GMOCK_GTEST\n")
SET(MSG "${MSG}    -DBUILD_TARGET=CRYPTOPP\n")
SET(MSG "${MSG}    -DBUILD_TARGET=PROTOBUF\n")
SET(MSG "${MSG}    -DBUILD_TARGET=GLOG\n")
MESSAGE("${MSG}")
SET(SEPARATOR "\n====================================================================================================\n\n")

GET_FILENAME_COMPONENT(COMMON_ROOT_DIR ${CURRENT_DIR} PATH)
SET(CMAKE_INSTALL_PREFIX ${COMMON_ROOT_DIR}/installed)
IF(INSTALL_PREFIX)
  FILE(TO_CMAKE_PATH ${INSTALL_PREFIX} INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "${INSTALL_PREFIX}")
ENDIF()
FILE(TO_NATIVE_PATH ${CMAKE_INSTALL_PREFIX} CMAKE_INSTALL_PREFIX_MESSAGE)
IF(WIN32)
  SET(LIB_PREFIX_RELEASE "${CMAKE_INSTALL_PREFIX}/lib/Release/")
  SET(LIB_PREFIX_DEBUG "${CMAKE_INSTALL_PREFIX}/lib/Debug/")
ELSE()
  SET(LIB_PREFIX_RELEASE "${CMAKE_INSTALL_PREFIX}/lib/")
  SET(LIB_PREFIX_DEBUG "${CMAKE_INSTALL_PREFIX}/lib/")
ENDIF()
SET(INC_DIR "${CMAKE_INSTALL_PREFIX}/include/")
SET(BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin/")
SET(SHARE_DIR "${CMAKE_INSTALL_PREFIX}/share/maidsafe/")
SET(EXPORT_FILE "${SHARE_DIR}maidsafe_third_party.cmake")

MESSAGE("The third party libraries and headers will be installed to:\n")
MESSAGE("    \"${CMAKE_INSTALL_PREFIX_MESSAGE}\"\n\n")
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_PREFIX_RELEASE})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_PREFIX_DEBUG})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${INC_DIR})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${BIN_DIR})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${SHARE_DIR})
FILE(WRITE ${EXPORT_FILE} "\n")
FILE(WRITE ${CURRENT_DIR}/install_prefix.cmake "SET(MAIDSAFE_COMMON_INSTALL_PREFIX \"${CMAKE_INSTALL_PREFIX}\")")

FUNCTION(APPEND_TO_EXPORT_FILE LINE)
  FILE(APPEND ${EXPORT_FILE} "${LINE}\n")
ENDFUNCTION()

###################################################################################################
# GoogleMock and GoogleTest                                                                       #
###################################################################################################
IF((${BUILD_TARGET} MATCHES "GMOCK_GTEST") OR (${BUILD_TARGET} MATCHES "ALL"))
  MESSAGE("-- Building GoogleMock and GoogleTest")
  MESSAGE("     This may take a few minutes...")
  SET(GMOCK_BUILD_DIR ${CURRENT_DIR}/build_googlemock)
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${GMOCK_BUILD_DIR})
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} ../src/googlemock WORKING_DIRECTORY ${GMOCK_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  SET(GMOCK_GTEST_OUTVAR "${OUTVAR}")
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . --config Debug WORKING_DIRECTORY ${GMOCK_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  SET(GMOCK_GTEST_OUTVAR "${GMOCK_GTEST_OUTVAR}${SEPARATOR}${OUTVAR}")
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . --config Release WORKING_DIRECTORY ${GMOCK_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  SET(GMOCK_GTEST_OUTVAR "${GMOCK_GTEST_OUTVAR}${SEPARATOR}${OUTVAR}")
ENDIF()

FIND_LIBRARY(Gtest_LIBRARY_RELEASE NAMES gtest PATHS ${CURRENT_DIR}/build_googlemock/gtest
               PATH_SUFFIXES Release NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
FIND_LIBRARY(Gtest_LIBRARY_DEBUG NAMES gtest PATHS ${CURRENT_DIR}/build_googlemock/gtest
               PATH_SUFFIXES Debug NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
FIND_PATH(Gtest_INCLUDE_DIR gtest/gtest.h PATHS ${CURRENT_DIR}/src/googlemock/gtest/include
            NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)

FIND_LIBRARY(Gmock_LIBRARY_RELEASE NAMES gmock PATHS ${CURRENT_DIR}/build_googlemock
               PATH_SUFFIXES Release NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
FIND_LIBRARY(Gmock_LIBRARY_DEBUG NAMES gmock PATHS ${CURRENT_DIR}/build_googlemock
               PATH_SUFFIXES Debug NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
FIND_PATH(Gmock_INCLUDE_DIR gmock/gmock.h PATHS ${CURRENT_DIR}/src/googlemock/include
            NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)

IF(NOT Gtest_LIBRARY_RELEASE OR NOT Gtest_LIBRARY_DEBUG OR NOT Gtest_INCLUDE_DIR OR NOT Gmock_LIBRARY_RELEASE OR NOT Gmock_LIBRARY_DEBUG OR NOT Gmock_INCLUDE_DIR)
  MESSAGE(FATAL_ERROR "${GMOCK_GTEST_OUTVAR}")
ENDIF()

MESSAGE("-- Found GoogleTest")
GET_FILENAME_COMPONENT(Gtest_LIBRARY_RELEASE_NAME ${Gtest_LIBRARY_RELEASE} NAME)
GET_FILENAME_COMPONENT(Gtest_LIBRARY_DEBUG_NAME ${Gtest_LIBRARY_DEBUG} NAME)
SET(Gtest_LIBRARY_RELEASE_INSTALLED ${LIB_PREFIX_RELEASE}${Gtest_LIBRARY_RELEASE_NAME})
SET(Gtest_LIBRARY_DEBUG_INSTALLED ${LIB_PREFIX_DEBUG}${Gtest_LIBRARY_DEBUG_NAME})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${Gtest_LIBRARY_RELEASE} ${LIB_PREFIX_RELEASE})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${Gtest_LIBRARY_DEBUG} ${LIB_PREFIX_DEBUG})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_directory ${Gtest_INCLUDE_DIR} ${INC_DIR})
MESSAGE("-- Installed ${Gtest_LIBRARY_RELEASE_INSTALLED}")
MESSAGE("-- Installed ${Gtest_LIBRARY_DEBUG_INSTALLED}")
FILE(GLOB_RECURSE HEADERS ${INC_DIR} "${INC_DIR}gtest/*")
FOREACH(HEADER ${HEADERS})
  MESSAGE("-- Installed ${HEADER}")
ENDFOREACH()

APPEND_TO_EXPORT_FILE("ADD_LIBRARY(gtest_static STATIC IMPORTED)")
APPEND_TO_EXPORT_FILE("SET_TARGET_PROPERTIES(gtest_static PROPERTIES")
APPEND_TO_EXPORT_FILE("                        IMPORTED_CONFIGURATIONS \"RELEASE;DEBUG\"")
APPEND_TO_EXPORT_FILE("                        IMPORTED_LOCATION_RELEASE \"${Gtest_LIBRARY_RELEASE_INSTALLED}\"")
IF(WIN32)
  APPEND_TO_EXPORT_FILE("                        IMPORTED_LOCATION_DEBUG \"${Gtest_LIBRARY_DEBUG_INSTALLED}\")")
ELSE()
  APPEND_TO_EXPORT_FILE("                        IMPORTED_LOCATION_DEBUG \"${Gtest_LIBRARY_DEBUG_INSTALLED}\"")
  APPEND_TO_EXPORT_FILE("                        IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE \"pthread\"")
  APPEND_TO_EXPORT_FILE("                        IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG \"pthread\")\n")
ENDIF()

MESSAGE("-- Found GoogleMock")
GET_FILENAME_COMPONENT(Gmock_LIBRARY_RELEASE_NAME ${Gmock_LIBRARY_RELEASE} NAME)
GET_FILENAME_COMPONENT(Gmock_LIBRARY_DEBUG_NAME ${Gmock_LIBRARY_DEBUG} NAME)
SET(Gmock_LIBRARY_RELEASE_INSTALLED ${LIB_PREFIX_RELEASE}${Gmock_LIBRARY_RELEASE_NAME})
SET(Gmock_LIBRARY_DEBUG_INSTALLED ${LIB_PREFIX_DEBUG}${Gmock_LIBRARY_DEBUG_NAME})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${Gmock_LIBRARY_RELEASE} ${LIB_PREFIX_RELEASE})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${Gmock_LIBRARY_DEBUG} ${LIB_PREFIX_DEBUG})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_directory ${Gmock_INCLUDE_DIR} ${INC_DIR})
MESSAGE("-- Installed ${Gmock_LIBRARY_RELEASE_INSTALLED}")
MESSAGE("-- Installed ${Gmock_LIBRARY_DEBUG_INSTALLED}")
FILE(GLOB_RECURSE HEADERS ${INC_DIR} "${INC_DIR}gmock/*")
FOREACH(HEADER ${HEADERS})
  MESSAGE("-- Installed ${HEADER}")
ENDFOREACH()

APPEND_TO_EXPORT_FILE("ADD_LIBRARY(gmock_static STATIC IMPORTED)")
APPEND_TO_EXPORT_FILE("SET_TARGET_PROPERTIES(gmock_static PROPERTIES")
APPEND_TO_EXPORT_FILE("                        IMPORTED_CONFIGURATIONS \"RELEASE;DEBUG\"")
APPEND_TO_EXPORT_FILE("                        IMPORTED_LOCATION_RELEASE \"${Gmock_LIBRARY_RELEASE_INSTALLED}\"")
APPEND_TO_EXPORT_FILE("                        IMPORTED_LOCATION_DEBUG \"${Gmock_LIBRARY_DEBUG_INSTALLED}\"")
APPEND_TO_EXPORT_FILE("                        IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE \"${Gtest_LIBRARY_RELEASE_INSTALLED}\"")
APPEND_TO_EXPORT_FILE("                        IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG \"${Gtest_LIBRARY_DEBUG_INSTALLED}\")\n")


###################################################################################################
# Cryptopp                                                                                        #
###################################################################################################
IF((${BUILD_TARGET} MATCHES "CRYPTOPP") OR (${BUILD_TARGET} MATCHES "ALL"))
  MESSAGE("-- Building Crypto++")
  MESSAGE("     This may take a few minutes...")
  SET(CRYPTOPP_BUILD_DIR ${CURRENT_DIR}/build_cryptopp)
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${CRYPTOPP_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} ../src/cryptopp WORKING_DIRECTORY ${CRYPTOPP_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  SET(CRYPTOPP_OUTVAR "${OUTVAR}")
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . --config Debug WORKING_DIRECTORY ${CRYPTOPP_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  SET(CRYPTOPP_OUTVAR "${CRYPTOPP_OUTVAR}${SEPARATOR}${OUTVAR}")
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . --config Release WORKING_DIRECTORY ${CRYPTOPP_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  SET(CRYPTOPP_OUTVAR "${CRYPTOPP_OUTVAR}${SEPARATOR}${OUTVAR}")
  FILE(GLOB CRYPTOPP_HEADERS ${CURRENT_DIR}/src/cryptopp/ "${CURRENT_DIR}/src/cryptopp/*.h")
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${CRYPTOPP_BUILD_DIR}/include/cryptopp OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  FOREACH(CRYPTOPP_HEADER ${CRYPTOPP_HEADERS})
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${CRYPTOPP_HEADER} ${CRYPTOPP_BUILD_DIR}/include/cryptopp/ OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  ENDFOREACH()
ENDIF()

FIND_LIBRARY(Cryptopp_LIBRARY_RELEASE NAMES cryptopp PATHS ${CURRENT_DIR}/build_cryptopp
               PATH_SUFFIXES Release NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
FIND_LIBRARY(Cryptopp_LIBRARY_DEBUG NAMES cryptopp_d cryptopp PATHS ${CURRENT_DIR}/build_cryptopp
               PATH_SUFFIXES Debug NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
IF(WIN32)
  FIND_FILE(Cryptopp_PDB_DEBUG NAMES cryptopp_d.pdb PATHS ${CURRENT_DIR}/build_cryptopp
                 PATH_SUFFIXES Debug NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
ENDIF()
FIND_PATH(Cryptopp_INCLUDE_DIR cryptopp/config.h PATHS ${CURRENT_DIR}/build_cryptopp/include
            NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
FIND_PROGRAM(Cryptopp_TEST_EXE cryptest PATHS ${CURRENT_DIR}/build_cryptopp
               PATH_SUFFIXES Release NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)

IF(NOT Cryptopp_LIBRARY_RELEASE OR NOT Cryptopp_LIBRARY_DEBUG OR NOT Cryptopp_INCLUDE_DIR OR NOT Cryptopp_TEST_EXE)
  MESSAGE(FATAL_ERROR "${CRYPTOPP_OUTVAR}")
ENDIF()

MESSAGE("-- Found Crypto++")
GET_FILENAME_COMPONENT(Cryptopp_LIBRARY_RELEASE_NAME ${Cryptopp_LIBRARY_RELEASE} NAME)
GET_FILENAME_COMPONENT(Cryptopp_LIBRARY_DEBUG_NAME ${Cryptopp_LIBRARY_DEBUG} NAME)
SET(Cryptopp_LIBRARY_RELEASE_INSTALLED ${LIB_PREFIX_RELEASE}${Cryptopp_LIBRARY_RELEASE_NAME})
SET(Cryptopp_LIBRARY_DEBUG_INSTALLED ${LIB_PREFIX_DEBUG}${Cryptopp_LIBRARY_DEBUG_NAME})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${Cryptopp_LIBRARY_RELEASE} ${LIB_PREFIX_RELEASE})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${Cryptopp_LIBRARY_DEBUG} ${LIB_PREFIX_DEBUG})
IF(Cryptopp_PDB_DEBUG)
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${Cryptopp_PDB_DEBUG} ${LIB_PREFIX_DEBUG})
ENDIF()
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_directory ${Cryptopp_INCLUDE_DIR} ${INC_DIR})
MESSAGE("-- Installed ${Cryptopp_LIBRARY_RELEASE_INSTALLED}")
MESSAGE("-- Installed ${Cryptopp_LIBRARY_DEBUG_INSTALLED}")
FILE(GLOB_RECURSE HEADERS ${INC_DIR} "${INC_DIR}cryptopp/*")
FOREACH(HEADER ${HEADERS})
  MESSAGE("-- Installed ${HEADER}")
ENDFOREACH()

APPEND_TO_EXPORT_FILE("ADD_LIBRARY(cryptopp_static STATIC IMPORTED)")
APPEND_TO_EXPORT_FILE("SET_TARGET_PROPERTIES(cryptopp_static PROPERTIES")
APPEND_TO_EXPORT_FILE("                        IMPORTED_CONFIGURATIONS \"RELEASE;DEBUG\"")
APPEND_TO_EXPORT_FILE("                        IMPORTED_LOCATION_RELEASE \"${Cryptopp_LIBRARY_RELEASE_INSTALLED}\"")
APPEND_TO_EXPORT_FILE("                        IMPORTED_LOCATION_DEBUG \"${Cryptopp_LIBRARY_DEBUG_INSTALLED}\")\n")

SET(CRYPTO_TEST_FILE "${CURRENT_DIR}/crypto_test.cmake")
FILE(WRITE ${CRYPTO_TEST_FILE} "\n")
FUNCTION(APPEND_TO_CRYPTO_TEST_FILE LINE)
  FILE(APPEND ${CRYPTO_TEST_FILE} "${LINE}\n")
ENDFUNCTION()

APPEND_TO_CRYPTO_TEST_FILE("FILE(COPY \"${CURRENT_DIR}/src/cryptopp/TestData\" DESTINATION . FILES_MATCHING PATTERN \"*.dat\")")
APPEND_TO_CRYPTO_TEST_FILE("FILE(COPY \"${CURRENT_DIR}/src/cryptopp/TestVectors\" DESTINATION . FILES_MATCHING PATTERN \"*.txt\")")
APPEND_TO_CRYPTO_TEST_FILE("ADD_TEST(CrypoppTest.BEH_FULL \"${Cryptopp_TEST_EXE}\" v)")
APPEND_TO_CRYPTO_TEST_FILE("ADD_TEST(CrypoppTest.BEH_AES \"${Cryptopp_TEST_EXE}\" tv aes)")
APPEND_TO_CRYPTO_TEST_FILE("ADD_TEST(CrypoppTest.BEH_SHA \"${Cryptopp_TEST_EXE}\" tv sha)")
APPEND_TO_CRYPTO_TEST_FILE("ADD_TEST(CrypoppTest.BEH_RSA_PSS \"${Cryptopp_TEST_EXE}\" tv rsa_pss)")
APPEND_TO_CRYPTO_TEST_FILE("ADD_TEST(CrypoppTest.BEH_RSA_OAEP \"${Cryptopp_TEST_EXE}\" tv rsa_oaep)")
APPEND_TO_CRYPTO_TEST_FILE("ADD_TEST(CrypoppTest.BEH_RSA_PKCS1_1_5 \"${Cryptopp_TEST_EXE}\" tv rsa_pkcs1_1_5)")
APPEND_TO_CRYPTO_TEST_FILE("SET_PROPERTY(TEST CrypoppTest.BEH_FULL PROPERTY LABELS Behavioural CryptoPP ${CMAKE_BUILD_TYPE})")
APPEND_TO_CRYPTO_TEST_FILE("SET_PROPERTY(TEST CrypoppTest.BEH_FULL PROPERTY TIMEOUT 60)")
APPEND_TO_CRYPTO_TEST_FILE("SET_PROPERTY(TEST CrypoppTest.BEH_AES PROPERTY LABELS Behavioural CryptoPP ${CMAKE_BUILD_TYPE})")
APPEND_TO_CRYPTO_TEST_FILE("SET_PROPERTY(TEST CrypoppTest.BEH_AES PROPERTY TIMEOUT 60)")
APPEND_TO_CRYPTO_TEST_FILE("SET_PROPERTY(TEST CrypoppTest.BEH_SHA PROPERTY LABELS Behavioural CryptoPP ${CMAKE_BUILD_TYPE})")
APPEND_TO_CRYPTO_TEST_FILE("SET_PROPERTY(TEST CrypoppTest.BEH_SHA PROPERTY TIMEOUT 60)")
APPEND_TO_CRYPTO_TEST_FILE("SET_PROPERTY(TEST CrypoppTest.BEH_RSA_PSS PROPERTY LABELS Behavioural CryptoPP ${CMAKE_BUILD_TYPE})")
APPEND_TO_CRYPTO_TEST_FILE("SET_PROPERTY(TEST CrypoppTest.BEH_RSA_PSS PROPERTY TIMEOUT 60)")
APPEND_TO_CRYPTO_TEST_FILE("SET_PROPERTY(TEST CrypoppTest.BEH_RSA_OAEP PROPERTY LABELS Behavioural CryptoPP ${CMAKE_BUILD_TYPE})")
APPEND_TO_CRYPTO_TEST_FILE("SET_PROPERTY(TEST CrypoppTest.BEH_RSA_OAEP PROPERTY TIMEOUT 60)")
APPEND_TO_CRYPTO_TEST_FILE("SET_PROPERTY(TEST CrypoppTest.BEH_RSA_PKCS1_1_5 PROPERTY LABELS Behavioural CryptoPP ${CMAKE_BUILD_TYPE})")
APPEND_TO_CRYPTO_TEST_FILE("SET_PROPERTY(TEST CrypoppTest.BEH_RSA_PKCS1_1_5 PROPERTY TIMEOUT 60)")


###################################################################################################
# Google Protocol Buffers                                                                         #
###################################################################################################
IF((${BUILD_TARGET} MATCHES "PROTOBUF") OR (${BUILD_TARGET} MATCHES "ALL"))
  IF(MSVC)
    # Make a copy of .sln file to work with to avoid modified .sln being accidentally committed to repository
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${CURRENT_DIR}/src/protobuf/vsprojects/protobuf.sln ${CURRENT_DIR}/src/protobuf/vsprojects/protobuf_copy.sln)
    FIND_FILE(PROTOBUF_SLN NAMES protobuf_copy.sln PATHS ${CURRENT_DIR}/src/protobuf/vsprojects NO_DEFAULT_PATH)
    GET_FILENAME_COMPONENT(PROTOBUF_VSPROJECTS_DIR ${PROTOBUF_SLN} PATH)
    FILE(GLOB_RECURSE PREVIOUS_ATTEMPTS ${CURRENT_DIR}/src/protobuf "*.vcxproj*")
    IF(PREVIOUS_ATTEMPTS)
      EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROTOBUF_VSPROJECTS_DIR}/include OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
      FILE(REMOVE ${PREVIOUS_ATTEMPTS})
    ENDIF()
    MESSAGE("-- Upgrading Google Protocol Buffers solution")
    EXECUTE_PROCESS(COMMAND devenv ${PROTOBUF_SLN} /Upgrade OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(PROTOBUF_OUTVAR "${OUTVAR}")
    MESSAGE("-- Building Google Protocol Buffers Debug libraries")
    MESSAGE("     This may take a few minutes...")
    EXECUTE_PROCESS(COMMAND devenv ${PROTOBUF_SLN} /Build "Debug|Win32" /Project libprotobuf OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(PROTOBUF_OUTVAR "${PROTOBUF_OUTVAR}${SEPARATOR}${OUTVAR}")
    MESSAGE("-- Building Google Protocol Buffers Release libraries and compiler")
    MESSAGE("     This may take a few minutes...")
    EXECUTE_PROCESS(COMMAND devenv ${PROTOBUF_SLN} /Build "Release|Win32" /Project protoc OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(PROTOBUF_OUTVAR "${PROTOBUF_OUTVAR}${SEPARATOR}${OUTVAR}")
    EXECUTE_PROCESS(COMMAND CMD /C CALL extract_includes.bat WORKING_DIRECTORY ${PROTOBUF_VSPROJECTS_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(PROTOBUF_OUTVAR "${PROTOBUF_OUTVAR}${SEPARATOR}${OUTVAR}")
  ELSE()
    UNSET(PROTOBUF_AUTOGEN CACHE)
    UNSET(PROTOBUF_CONFIGURE CACHE)
    SET(PROTOBUF_BUILD_DIR ${CURRENT_DIR}/build_protobuf)
    # Make a copy of protobuf to work with to avoid modified files being accidentally committed to repository
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CURRENT_DIR}/src/protobuf ${PROTOBUF_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    FIND_FILE(PROTOBUF_AUTOGEN autogen.sh PATHS ${PROTOBUF_BUILD_DIR} NO_DEFAULT_PATH)
    MESSAGE("-- Generating configure file for Google Protocol Buffers")
    MESSAGE("     This may take a few minutes...")
    EXECUTE_PROCESS(COMMAND sh ${PROTOBUF_AUTOGEN} WORKING_DIRECTORY ${PROTOBUF_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
    SET(PROTOBUF_OUTVAR "${OUTVAR}")
    FIND_FILE(PROTOBUF_CONFIGURE configure PATHS ${PROTOBUF_BUILD_DIR} NO_DEFAULT_PATH)
    IF(NOT PROTOBUF_CONFIGURE)
      SET(ERROR_MESSAGE "${OUTVAR}\n${ERRVAR}\nCould not create configure file for Google Protocol Buffers.\n\n")
      SET(ERROR_MESSAGE "${ERROR_MESSAGE}*** This needs autoconf and libtool to complete successfully. ***\n")
      MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
    ENDIF()
    MESSAGE("-- Configuring Google Protocol Buffers")
    MESSAGE("     This may take a few minutes...")
    EXECUTE_PROCESS(COMMAND sh ${PROTOBUF_CONFIGURE} --prefix=${PROTOBUF_BUILD_DIR} --enable-shared=no WORKING_DIRECTORY ${PROTOBUF_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(PROTOBUF_OUTVAR "${PROTOBUF_OUTVAR}${SEPARATOR}${OUTVAR}")
    MESSAGE("-- Making Google Protocol Buffers")
    MESSAGE("     This may take a few minutes...")
    EXECUTE_PROCESS(COMMAND make install WORKING_DIRECTORY ${PROTOBUF_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
    SET(PROTOBUF_OUTVAR "${PROTOBUF_OUTVAR}${SEPARATOR}${OUTVAR}")
  ENDIF()
ENDIF()

FIND_LIBRARY(Protobuf_LIBRARY_RELEASE NAMES libprotobuf.a libprotobuf.lib
               PATHS ${CURRENT_DIR}/build_protobuf/lib ${CURRENT_DIR}/src/protobuf/vsprojects/Release
               NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
FIND_LIBRARY(Protobuf_LIBRARY_DEBUG NAMES libprotobuf.a libprotobuf.lib
               PATHS ${CURRENT_DIR}/build_protobuf/lib ${CURRENT_DIR}/src/protobuf/vsprojects/Debug
               NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
FIND_PROGRAM(Protoc_EXE NAMES protoc
               PATHS ${CURRENT_DIR}/build_protobuf/bin ${CURRENT_DIR}/src/protobuf/vsprojects/Release
               NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
FIND_PATH(Protobuf_INCLUDE_DIR google/protobuf/service.h
            PATHS ${CURRENT_DIR}/build_protobuf/include ${CURRENT_DIR}/src/protobuf/vsprojects/include
            NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)

IF(NOT Protobuf_LIBRARY_RELEASE OR NOT Protobuf_LIBRARY_DEBUG OR NOT Protoc_EXE OR NOT Protobuf_INCLUDE_DIR)
  MESSAGE(FATAL_ERROR "${PROTOBUF_OUTVAR}")
ENDIF()

MESSAGE("-- Found Google Protocol Buffers")
GET_FILENAME_COMPONENT(Protobuf_LIBRARY_RELEASE_NAME ${Protobuf_LIBRARY_RELEASE} NAME)
GET_FILENAME_COMPONENT(Protobuf_LIBRARY_DEBUG_NAME ${Protobuf_LIBRARY_DEBUG} NAME)
GET_FILENAME_COMPONENT(Protoc_EXE_NAME ${Protoc_EXE} NAME)
SET(Protobuf_LIBRARY_RELEASE_INSTALLED ${LIB_PREFIX_RELEASE}${Protobuf_LIBRARY_RELEASE_NAME})
SET(Protobuf_LIBRARY_DEBUG_INSTALLED ${LIB_PREFIX_DEBUG}${Protobuf_LIBRARY_DEBUG_NAME})
SET(Protoc_EXE_INSTALLED ${BIN_DIR}${Protoc_EXE_NAME})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${Protobuf_LIBRARY_RELEASE} ${LIB_PREFIX_RELEASE})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${Protobuf_LIBRARY_DEBUG} ${LIB_PREFIX_DEBUG})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${Protoc_EXE} ${BIN_DIR})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_directory ${Protobuf_INCLUDE_DIR} ${INC_DIR})
MESSAGE("-- Installed ${Protobuf_LIBRARY_RELEASE_INSTALLED}")
MESSAGE("-- Installed ${Protobuf_LIBRARY_DEBUG_INSTALLED}")
MESSAGE("-- Installed ${Protoc_EXE_INSTALLED}")
FILE(GLOB_RECURSE HEADERS ${INC_DIR} "${INC_DIR}google/*")
FOREACH(HEADER ${HEADERS})
  MESSAGE("-- Installed ${HEADER}")
ENDFOREACH()

APPEND_TO_EXPORT_FILE("ADD_LIBRARY(protobuf_static STATIC IMPORTED)")
APPEND_TO_EXPORT_FILE("SET_TARGET_PROPERTIES(protobuf_static PROPERTIES")
APPEND_TO_EXPORT_FILE("                        IMPORTED_CONFIGURATIONS \"RELEASE;DEBUG\"")
APPEND_TO_EXPORT_FILE("                        IMPORTED_LOCATION_RELEASE \"${Protobuf_LIBRARY_RELEASE_INSTALLED}\"")
APPEND_TO_EXPORT_FILE("                        IMPORTED_LOCATION_DEBUG \"${Protobuf_LIBRARY_DEBUG_INSTALLED}\")\n")

APPEND_TO_EXPORT_FILE("SET(Protoc_EXE \"${Protoc_EXE_INSTALLED}\")\n")


###################################################################################################
# Google Logging library                                                                          #
###################################################################################################
IF((${BUILD_TARGET} MATCHES "GLOG") OR (${BUILD_TARGET} MATCHES "ALL"))
  IF(MSVC)
    # Make a copy of .sln file to work with to avoid modified .sln being accidentally committed to repository
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${CURRENT_DIR}/src/glog/google-glog.sln ${CURRENT_DIR}/src/glog/google-glog_copy.sln)
    FIND_FILE(GLOG_SLN NAMES google-glog_copy.sln PATHS ${CURRENT_DIR}/src/glog NO_DEFAULT_PATH)
    GET_FILENAME_COMPONENT(GLOG_ROOT_DIR ${GLOG_SLN} PATH)
    FILE(GLOB_RECURSE PREVIOUS_ATTEMPTS ${CURRENT_DIR}/src/glog "*.vcxproj*")
    IF(PREVIOUS_ATTEMPTS)
      FILE(REMOVE ${PREVIOUS_ATTEMPTS})
    ENDIF()
    MESSAGE("-- Upgrading Google Logging solution")
    EXECUTE_PROCESS(COMMAND devenv ${GLOG_SLN} /Upgrade OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(GLOG_OUTVAR "${OUTVAR}")
    MESSAGE("-- Building Google Logging Debug libraries")
    MESSAGE("     This may take a few minutes...")
    EXECUTE_PROCESS(COMMAND devenv ${GLOG_SLN} /Build "Debug|Win32" /Project libglog_static OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(GLOG_OUTVAR "${GLOG_OUTVAR}${SEPARATOR}${OUTVAR}")
    MESSAGE("-- Building Google Logging Release libraries")
    MESSAGE("     This may take a few minutes...")
    EXECUTE_PROCESS(COMMAND devenv ${GLOG_SLN} /Build "Release|Win32" /Project libglog_static OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(GLOG_OUTVAR "${GLOG_OUTVAR}${SEPARATOR}${OUTVAR}")
  ELSE()
    UNSET(GLOG_CONFIGURE CACHE)
    SET(GLOG_BUILD_DIR ${CURRENT_DIR}/build_glog)
    # Make a copy of glog to work with to avoid modified files being accidentally committed to repository
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CURRENT_DIR}/src/glog ${GLOG_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    FIND_FILE(GLOG_CONFIGURE configure PATHS ${GLOG_BUILD_DIR} NO_DEFAULT_PATH)
    IF(NOT GLOG_CONFIGURE)
      MESSAGE(FATAL_ERROR "${OUTVAR}\n${ERRVAR}\nCould not configure Google Logging.\n\n")
    ENDIF()
    MESSAGE("-- Configuring Google Logging library")
    MESSAGE("     This may take a few minutes...")
    EXECUTE_PROCESS(COMMAND sh ${GLOG_CONFIGURE} --prefix=${GLOG_BUILD_DIR} --enable-shared=no WORKING_DIRECTORY ${GLOG_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
    SET(GLOG_OUTVAR "${OUTVAR}")
    MESSAGE("-- Making Google Logging library")
    MESSAGE("     This may take a few minutes...")
    EXECUTE_PROCESS(COMMAND make -i -C ${GLOG_BUILD_DIR} install OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
    SET(GLOG_OUTVAR "${GLOG_OUTVAR}${SEPARATOR}${OUTVAR}")
  ENDIF()
ENDIF()

FIND_LIBRARY(Glog_LIBRARY_RELEASE NAMES libglog.a libglog_static.lib
               PATHS ${CURRENT_DIR}/build_glog/lib ${CURRENT_DIR}/src/glog/Release
               NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
FIND_LIBRARY(Glog_LIBRARY_DEBUG NAMES libglog.a libglog_static.lib
               PATHS ${CURRENT_DIR}/build_glog/lib ${CURRENT_DIR}/src/glog/Debug
               NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
FIND_PATH(Glog_INCLUDE_DIR glog/logging.h
            PATHS ${CURRENT_DIR}/build_glog/include ${CURRENT_DIR}/src/glog/src/windows
            NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)

IF(NOT Glog_LIBRARY_RELEASE OR NOT Glog_LIBRARY_DEBUG OR NOT Glog_INCLUDE_DIR)
  MESSAGE(FATAL_ERROR "${GLOG_OUTVAR}")
ENDIF()

MESSAGE("-- Found Google Logging")
GET_FILENAME_COMPONENT(Glog_LIBRARY_RELEASE_NAME ${Glog_LIBRARY_RELEASE} NAME)
GET_FILENAME_COMPONENT(Glog_LIBRARY_DEBUG_NAME ${Glog_LIBRARY_DEBUG} NAME)
SET(Glog_LIBRARY_RELEASE_INSTALLED ${LIB_PREFIX_RELEASE}${Glog_LIBRARY_RELEASE_NAME})
SET(Glog_LIBRARY_DEBUG_INSTALLED ${LIB_PREFIX_DEBUG}${Glog_LIBRARY_DEBUG_NAME})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${Glog_LIBRARY_RELEASE} ${LIB_PREFIX_RELEASE})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${Glog_LIBRARY_DEBUG} ${LIB_PREFIX_DEBUG})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_directory ${Glog_INCLUDE_DIR} ${INC_DIR})
MESSAGE("-- Installed ${Glog_LIBRARY_RELEASE_INSTALLED}")
MESSAGE("-- Installed ${Glog_LIBRARY_DEBUG_INSTALLED}")
FILE(GLOB_RECURSE HEADERS ${INC_DIR} "${INC_DIR}glog/*")
FOREACH(HEADER ${HEADERS})
  MESSAGE("-- Installed ${HEADER}")
ENDFOREACH()

APPEND_TO_EXPORT_FILE("ADD_LIBRARY(glog_static STATIC IMPORTED)")
APPEND_TO_EXPORT_FILE("SET_TARGET_PROPERTIES(glog_static PROPERTIES")
APPEND_TO_EXPORT_FILE("                        IMPORTED_CONFIGURATIONS \"RELEASE;DEBUG\"")
APPEND_TO_EXPORT_FILE("                        IMPORTED_LOCATION_RELEASE \"${Glog_LIBRARY_RELEASE_INSTALLED}\"")
APPEND_TO_EXPORT_FILE("                        IMPORTED_LOCATION_DEBUG \"${Glog_LIBRARY_DEBUG_INSTALLED}\")\n")


UNSET(BUILD_TARGET CACHE)
