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


CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4 FATAL_ERROR)

PROJECT(BuildThirdPartyLibs)

GET_FILENAME_COMPONENT(CURRENT_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)

IF(NOT BUILD_TARGET)
  SET(BUILD_TARGET "ALL")
ENDIF()

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

EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_PREFIX_RELEASE})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_PREFIX_DEBUG})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${INC_DIR})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${BIN_DIR})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${SHARE_DIR})
FILE(WRITE ${EXPORT_FILE} "\n")

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
  IF(MSVC)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} ../src/googlemock -G ${CMAKE_GENERATOR} WORKING_DIRECTORY ${GMOCK_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(GMOCK_GTEST_OUTVAR "${OUTVAR}")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . --config Debug WORKING_DIRECTORY ${GMOCK_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(GMOCK_GTEST_OUTVAR "${GMOCK_GTEST_OUTVAR}${SEPARATOR}${OUTVAR}")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . --config Release WORKING_DIRECTORY ${GMOCK_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(GMOCK_GTEST_OUTVAR "${GMOCK_GTEST_OUTVAR}${SEPARATOR}${OUTVAR}")
  ELSE()
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} ../src/googlemock -G ${CMAKE_GENERATOR} -DCMAKE_BUILD_TYPE=Release WORKING_DIRECTORY ${GMOCK_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(GMOCK_GTEST_OUTVAR "${OUTVAR}")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${GMOCK_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(GMOCK_GTEST_OUTVAR "${GMOCK_GTEST_OUTVAR}${SEPARATOR}${OUTVAR}")
  ENDIF()
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
  IF(MSVC)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} ../src/cryptopp -G ${CMAKE_GENERATOR} WORKING_DIRECTORY ${CRYPTOPP_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(CRYPTOPP_OUTVAR "${OUTVAR}")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . --config Debug --target cryptopp_static WORKING_DIRECTORY ${CRYPTOPP_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(CRYPTOPP_OUTVAR "${CRYPTOPP_OUTVAR}${SEPARATOR}${OUTVAR}")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . --config Release WORKING_DIRECTORY ${CRYPTOPP_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(CRYPTOPP_OUTVAR "${CRYPTOPP_OUTVAR}${SEPARATOR}${OUTVAR}")
  ELSE()
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} ../src/cryptopp -G ${CMAKE_GENERATOR} -DCMAKE_BUILD_TYPE=Debug WORKING_DIRECTORY ${CRYPTOPP_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(CRYPTOPP_OUTVAR "${OUTVAR}")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . --target cryptopp_static WORKING_DIRECTORY ${CRYPTOPP_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(CRYPTOPP_OUTVAR "${CRYPTOPP_OUTVAR}${SEPARATOR}${OUTVAR}")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} ../src/cryptopp -G ${CMAKE_GENERATOR} -DCMAKE_BUILD_TYPE=Release WORKING_DIRECTORY ${CRYPTOPP_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(CRYPTOPP_OUTVAR "${OUTVAR}")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${CRYPTOPP_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(CRYPTOPP_OUTVAR "${CRYPTOPP_OUTVAR}${SEPARATOR}${OUTVAR}")
  ENDIF()
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
APPEND_TO_CRYPTO_TEST_FILE("IF(NOT \${MEMORY_CHECK})")
APPEND_TO_CRYPTO_TEST_FILE("  ADD_TEST(CrypoppTest.BEH_FULL \"${Cryptopp_TEST_EXE}\" v)")
APPEND_TO_CRYPTO_TEST_FILE("  ADD_TEST(CrypoppTest.BEH_AES \"${Cryptopp_TEST_EXE}\" tv aes)")
APPEND_TO_CRYPTO_TEST_FILE("  ADD_TEST(CrypoppTest.BEH_SHA \"${Cryptopp_TEST_EXE}\" tv sha)")
APPEND_TO_CRYPTO_TEST_FILE("  ADD_TEST(CrypoppTest.BEH_RSA_PSS \"${Cryptopp_TEST_EXE}\" tv rsa_pss)")
APPEND_TO_CRYPTO_TEST_FILE("  ADD_TEST(CrypoppTest.BEH_RSA_OAEP \"${Cryptopp_TEST_EXE}\" tv rsa_oaep)")
APPEND_TO_CRYPTO_TEST_FILE("  ADD_TEST(CrypoppTest.BEH_RSA_PKCS1_1_5 \"${Cryptopp_TEST_EXE}\" tv rsa_pkcs1_1_5)")
APPEND_TO_CRYPTO_TEST_FILE("  SET_PROPERTY(TEST CrypoppTest.BEH_FULL PROPERTY LABELS Behavioural CryptoPP ${CMAKE_BUILD_TYPE})")
APPEND_TO_CRYPTO_TEST_FILE("  SET_PROPERTY(TEST CrypoppTest.BEH_FULL PROPERTY TIMEOUT 60)")
APPEND_TO_CRYPTO_TEST_FILE("  SET_PROPERTY(TEST CrypoppTest.BEH_AES PROPERTY LABELS Behavioural CryptoPP ${CMAKE_BUILD_TYPE})")
APPEND_TO_CRYPTO_TEST_FILE("  SET_PROPERTY(TEST CrypoppTest.BEH_AES PROPERTY TIMEOUT 60)")
APPEND_TO_CRYPTO_TEST_FILE("  SET_PROPERTY(TEST CrypoppTest.BEH_SHA PROPERTY LABELS Behavioural CryptoPP ${CMAKE_BUILD_TYPE})")
APPEND_TO_CRYPTO_TEST_FILE("  SET_PROPERTY(TEST CrypoppTest.BEH_SHA PROPERTY TIMEOUT 60)")
APPEND_TO_CRYPTO_TEST_FILE("  SET_PROPERTY(TEST CrypoppTest.BEH_RSA_PSS PROPERTY LABELS Behavioural CryptoPP ${CMAKE_BUILD_TYPE})")
APPEND_TO_CRYPTO_TEST_FILE("  SET_PROPERTY(TEST CrypoppTest.BEH_RSA_PSS PROPERTY TIMEOUT 60)")
APPEND_TO_CRYPTO_TEST_FILE("  SET_PROPERTY(TEST CrypoppTest.BEH_RSA_OAEP PROPERTY LABELS Behavioural CryptoPP ${CMAKE_BUILD_TYPE})")
APPEND_TO_CRYPTO_TEST_FILE("  SET_PROPERTY(TEST CrypoppTest.BEH_RSA_OAEP PROPERTY TIMEOUT 60)")
APPEND_TO_CRYPTO_TEST_FILE("  SET_PROPERTY(TEST CrypoppTest.BEH_RSA_PKCS1_1_5 PROPERTY LABELS Behavioural CryptoPP ${CMAKE_BUILD_TYPE})")
APPEND_TO_CRYPTO_TEST_FILE("  SET_PROPERTY(TEST CrypoppTest.BEH_RSA_PKCS1_1_5 PROPERTY TIMEOUT 60)")
APPEND_TO_CRYPTO_TEST_FILE("ENDIF()")


###################################################################################################
# Google Protocol Buffers                                                                         #
###################################################################################################
IF((${BUILD_TARGET} MATCHES "PROTOBUF") OR (${BUILD_TARGET} MATCHES "ALL"))
  IF(MSVC)
    MESSAGE("-- Building Google Protocol Buffers Debug libraries")
    MESSAGE("     This may take a few minutes...")
    FIND_FILE(LIBPROTOBUF_PROJECT NAMES libprotobuf.vcxproj PATHS ${CURRENT_DIR}/src/protobuf/vsprojects NO_DEFAULT_PATH)
    FIND_FILE(LIBPROTOC_PROJECT NAMES libprotoc.vcxproj PATHS ${CURRENT_DIR}/src/protobuf/vsprojects NO_DEFAULT_PATH)
    FIND_FILE(PROTOC_PROJECT NAMES protoc.vcxproj PATHS ${CURRENT_DIR}/src/protobuf/vsprojects NO_DEFAULT_PATH)
    # Build libprotobuf Debug
    EXECUTE_PROCESS(COMMAND msbuild ${LIBPROTOBUF_PROJECT} /property:Configuration=Debug OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
    SET(PROTOBUF_OUTVAR "${OUTVAR}")
    MESSAGE("-- Building Google Protocol Buffers Release libraries and compiler")
    MESSAGE("     This may take a few minutes...")
    # Build libprotobuf Release
    EXECUTE_PROCESS(COMMAND msbuild ${LIBPROTOBUF_PROJECT} /property:Configuration=Release OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
    SET(PROTOBUF_OUTVAR "${PROTOBUF_OUTVAR}${SEPARATOR}${OUTVAR}")
    # Build libprotoc Release
    EXECUTE_PROCESS(COMMAND msbuild ${LIBPROTOC_PROJECT} /property:Configuration=Release OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
    SET(PROTOBUF_OUTVAR "${PROTOBUF_OUTVAR}${SEPARATOR}${OUTVAR}")
    # Build protoc Release
    EXECUTE_PROCESS(COMMAND msbuild ${PROTOC_PROJECT} /property:Configuration=Release OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
    SET(PROTOBUF_OUTVAR "${PROTOBUF_OUTVAR}${SEPARATOR}${OUTVAR}")
    # Extract headers
    EXECUTE_PROCESS(COMMAND CMD /C CALL extract_includes.bat WORKING_DIRECTORY ${CURRENT_DIR}/src/protobuf/vsprojects OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
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
    MESSAGE("-- Building Google Logging Debug libraries")
    MESSAGE("     This may take a few minutes...")
    FIND_FILE(LIBGLOG_PROJECT NAMES libglog_static.vcxproj PATHS ${CURRENT_DIR}/src/glog/vsprojects/libglog_static NO_DEFAULT_PATH)
    IF(CMAKE_CL_64)
      EXECUTE_PROCESS(COMMAND msbuild ${LIBGLOG_PROJECT} "/property:Configuration=Debug;Platform=x64" OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
    ELSE()
      EXECUTE_PROCESS(COMMAND msbuild ${LIBGLOG_PROJECT} "/property:Configuration=Debug;Platform=Win32" OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
    ENDIF()
    SET(GLOG_OUTVAR "${OUTVAR}")
    MESSAGE("-- Building Google Logging Release libraries")
    MESSAGE("     This may take a few minutes...")
    IF(CMAKE_CL_64)
      EXECUTE_PROCESS(COMMAND msbuild ${LIBGLOG_PROJECT} "/property:Configuration=Release;Platform=x64" OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
    ELSE()
      EXECUTE_PROCESS(COMMAND msbuild ${LIBGLOG_PROJECT} "/property:Configuration=Release;Platform=Win32" OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
    ENDIF()
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
    EXECUTE_PROCESS(COMMAND chmod u+x configure install-sh WORKING_DIRECTORY ${GLOG_BUILD_DIR})
    EXECUTE_PROCESS(COMMAND make -i -C ${GLOG_BUILD_DIR} install-libLTLIBRARIES OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
    SET(GLOG_OUTVAR "${GLOG_OUTVAR}${SEPARATOR}${OUTVAR}")
  ENDIF()
ENDIF()

FIND_LIBRARY(Glog_LIBRARY_RELEASE NAMES libglog.a libglog_static.lib
               PATHS ${CURRENT_DIR}/build_glog/lib ${CURRENT_DIR}/src/glog/vsprojects/libglog_static/Release
               NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
FIND_LIBRARY(Glog_LIBRARY_DEBUG NAMES libglog.a libglog_static.lib
               PATHS ${CURRENT_DIR}/build_glog/lib ${CURRENT_DIR}/src/glog/vsprojects/libglog_static/Debug
               NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
IF(WIN32)
  FIND_PATH(Glog_INCLUDE_DIR glog/logging.h
              PATHS ${CURRENT_DIR}/src/glog/src/windows
              NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
ELSE()
  FIND_PATH(Glog_INCLUDE_DIR glog/logging.h
              PATHS ${CURRENT_DIR}/build_glog/src
              NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
ENDIF()

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
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${Glog_INCLUDE_DIR}/glog/logging.h ${INC_DIR}/glog/logging.h)
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${Glog_INCLUDE_DIR}/glog/log_severity.h ${INC_DIR}/glog/log_severity.h)
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${Glog_INCLUDE_DIR}/glog/vlog_is_on.h ${INC_DIR}/glog/vlog_is_on.h)
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

###################################################################################################
# Google Breakpad                                                                                 #
###################################################################################################

# Currently not supporting breakpad using OSX
IF (NOT APPLE)
IF((${BUILD_TARGET} MATCHES "BREAKPAD") OR (${BUILD_TARGET} MATCHES "ALL"))
  MESSAGE("-- Building Google Breakpad Client")
  MESSAGE("     This may take a few minutes...")
  SET(BREAKPAD_BUILD_DIR ${CURRENT_DIR}/build_breakpad)
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${BREAKPAD_BUILD_DIR})
  IF(MSVC)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} ../src/google-breakpad -G ${CMAKE_GENERATOR} WORKING_DIRECTORY ${BREAKPAD_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(BREAKPAD_OUTVAR "${OUTVAR}")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . --config Debug WORKING_DIRECTORY ${BREAKPAD_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(BREAKPAD_OUTVAR "${BREAKPAD_OUTVAR}${SEPARATOR}${OUTVAR}")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . --config Release WORKING_DIRECTORY ${BREAKPAD_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(BREAKPAD_OUTVAR "${BREAKPAD_OUTVAR}${SEPARATOR}${OUTVAR}")
  ELSE()
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} ../src/google-breakpad -G ${CMAKE_GENERATOR} -DCMAKE_BUILD_TYPE=Release WORKING_DIRECTORY ${BREAKPAD_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(BREAKPAD_OUTVAR "${OUTVAR}")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${BREAKPAD_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
    SET(BREAKPAD_OUTVAR "${BREAKPAD_OUTVAR}${SEPARATOR}${OUTVAR}")
  ENDIF()
  IF(WIN32)
    SET(BREAKPAD_HEADERS
          client/windows/handler/exception_handler.h
          client/windows/common/ipc_protocol.h
          client/windows/crash_generation/crash_generation_client.h
          common/windows/string_utils-inl.h)
  ELSEIF(UNIX)
    IF(APPLE)
      SET(BREAKPAD_HEADERS
            client/mac/handler/exception_handler.h
            client/mac/crash_generation/crash_generation_client.h
            common/mac/MachIPC.h)
    ELSE()
      SET(BREAKPAD_HEADERS
            client/linux/handler/exception_handler.h
            client/linux/android_ucontext.h
            client/linux/crash_generation/crash_generation_client.h
            client/linux/minidump_writer/minidump_writer.h)
    ENDIF()
  ENDIF()
  SET(BREAKPAD_HEADERS ${BREAKPAD_HEADERS}
        google_breakpad/common/breakpad_types.h
        google_breakpad/common/minidump_cpu_amd64.h
        google_breakpad/common/minidump_cpu_arm.h
        google_breakpad/common/minidump_cpu_ppc.h
        google_breakpad/common/minidump_cpu_ppc64.h
        google_breakpad/common/minidump_cpu_sparc.h
        google_breakpad/common/minidump_cpu_x86.h
        google_breakpad/common/minidump_exception_linux.h
        google_breakpad/common/minidump_exception_mac.h
        google_breakpad/common/minidump_exception_solaris.h
        google_breakpad/common/minidump_exception_win32.h
        google_breakpad/common/minidump_format.h
        processor/scoped_ptr.h)
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${BREAKPAD_BUILD_DIR}/include/breakpad OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  FOREACH(BREAKPAD_HEADER ${BREAKPAD_HEADERS})
    SET(INSTALL_PATH ${BREAKPAD_BUILD_DIR}/include/breakpad/${BREAKPAD_HEADER})
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${CURRENT_DIR}/src/google-breakpad/src/${BREAKPAD_HEADER} ${INSTALL_PATH} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  ENDFOREACH()
ENDIF()

FIND_LIBRARY(Breakpad_LIBRARY_RELEASE NAMES breakpad PATHS ${CURRENT_DIR}/build_breakpad
               PATH_SUFFIXES Release NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
FIND_LIBRARY(Breakpad_LIBRARY_DEBUG NAMES breakpad_d breakpad PATHS ${CURRENT_DIR}/build_breakpad
               PATH_SUFFIXES Debug NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
FIND_PATH(Breakpad_INCLUDE_DIR breakpad/google_breakpad/common/minidump_format.h PATHS ${CURRENT_DIR}/build_breakpad/include
            NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)

IF(NOT Breakpad_LIBRARY_RELEASE OR NOT Breakpad_LIBRARY_DEBUG OR NOT Breakpad_INCLUDE_DIR)
  MESSAGE(FATAL_ERROR "${BREAKPAD_OUTVAR}")
ENDIF()

MESSAGE("-- Found Google Breakpad Client")
GET_FILENAME_COMPONENT(Breakpad_LIBRARY_RELEASE_NAME ${Breakpad_LIBRARY_RELEASE} NAME)
GET_FILENAME_COMPONENT(Breakpad_LIBRARY_DEBUG_NAME ${Breakpad_LIBRARY_DEBUG} NAME)
SET(Breakpad_LIBRARY_RELEASE_INSTALLED ${LIB_PREFIX_RELEASE}${Breakpad_LIBRARY_RELEASE_NAME})
SET(Breakpad_LIBRARY_DEBUG_INSTALLED ${LIB_PREFIX_DEBUG}${Breakpad_LIBRARY_DEBUG_NAME})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${Breakpad_LIBRARY_RELEASE} ${LIB_PREFIX_RELEASE})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${Breakpad_LIBRARY_DEBUG} ${LIB_PREFIX_DEBUG})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_directory ${Breakpad_INCLUDE_DIR} ${INC_DIR})
MESSAGE("-- Installed ${Breakpad_LIBRARY_RELEASE_INSTALLED}")
MESSAGE("-- Installed ${Breakpad_LIBRARY_DEBUG_INSTALLED}")
FILE(GLOB_RECURSE HEADERS ${INC_DIR} "${INC_DIR}breakpad/*")
FOREACH(HEADER ${HEADERS})
  MESSAGE("-- Installed ${HEADER}")
ENDFOREACH()

APPEND_TO_EXPORT_FILE("ADD_LIBRARY(breakpad_static STATIC IMPORTED)")
APPEND_TO_EXPORT_FILE("SET_TARGET_PROPERTIES(breakpad_static PROPERTIES")
APPEND_TO_EXPORT_FILE("                        IMPORTED_CONFIGURATIONS \"RELEASE;DEBUG\"")
APPEND_TO_EXPORT_FILE("                        IMPORTED_LOCATION_RELEASE \"${Breakpad_LIBRARY_RELEASE_INSTALLED}\"")
APPEND_TO_EXPORT_FILE("                        IMPORTED_LOCATION_DEBUG \"${Breakpad_LIBRARY_DEBUG_INSTALLED}\"")
APPEND_TO_EXPORT_FILE("                        IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE \"${Breakpad_LIBRARY_RELEASE_INSTALLED}\"")
APPEND_TO_EXPORT_FILE("                        IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG \"${Breakpad_LIBRARY_DEBUG_INSTALLED}\")\n")

ENDIF()

UNSET(BUILD_TARGET CACHE)

FILE(WRITE ${CURRENT_DIR}/install_prefix.cmake "SET(MAIDSAFE_COMMON_INSTALL_PREFIX \"${CMAKE_INSTALL_PREFIX}\")")
