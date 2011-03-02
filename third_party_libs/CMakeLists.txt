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
#  Module used to build Third Party libs.                                      #
#                                                                              #
#==============================================================================#


CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)

PROJECT(BuildThirdPartyLibs)

GET_FILENAME_COMPONENT(CURRENT_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)

# Build GoogleMock and GoogleTest
SET(GMOCK_BUILD_DIR ${CURRENT_DIR}/build_googlemock)
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${GMOCK_BUILD_DIR})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} ../src/googlemock WORKING_DIRECTORY ${GMOCK_BUILD_DIR})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . --config Debug WORKING_DIRECTORY ${GMOCK_BUILD_DIR})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . --config Release WORKING_DIRECTORY ${GMOCK_BUILD_DIR})
FILE(COPY ${CURRENT_DIR}/src/googlemock/include DESTINATION ${GMOCK_BUILD_DIR})
FILE(COPY ${CURRENT_DIR}/src/googlemock/gtest/include DESTINATION ${GMOCK_BUILD_DIR})

# Build Cryptopp
SET(CRYPTOPP_BUILD_DIR ${CURRENT_DIR}/build_cryptopp)
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${CRYPTOPP_BUILD_DIR})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} ../src/cryptopp WORKING_DIRECTORY ${CRYPTOPP_BUILD_DIR})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . --config Debug WORKING_DIRECTORY ${CRYPTOPP_BUILD_DIR})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . --config Release WORKING_DIRECTORY ${CRYPTOPP_BUILD_DIR})
FILE(GLOB CRYPTOPP_HEADERS ${CURRENT_DIR}/src/cryptopp/ "${CURRENT_DIR}/src/cryptopp/*.h")
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${CRYPTOPP_BUILD_DIR}/include/cryptopp)
FOREACH(CRYPTOPP_HEADER ${CRYPTOPP_HEADERS})
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${CRYPTOPP_HEADER} ${CRYPTOPP_BUILD_DIR}/include/cryptopp/)
ENDFOREACH()

# Build Google Protocol Buffers
IF(MSVC)
  # Make a copy of .sln file to work with to avoid modified .sln being accidentally committed to repository
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${CURRENT_DIR}/src/protobuf/vsprojects/protobuf.sln ${CURRENT_DIR}/src/protobuf/vsprojects/protobuf_copy.sln)
  FIND_FILE(PROTOBUF_SLN NAMES protobuf_copy.sln PATHS ${CURRENT_DIR}/src/protobuf/vsprojects NO_DEFAULT_PATH)
  GET_FILENAME_COMPONENT(PROTOBUF_VSPROJECTS_DIR ${PROTOBUF_SLN} PATH)
  MESSAGE("-- Upgrading Google Protocol Buffers solution")
  EXECUTE_PROCESS(COMMAND devenv ${PROTOBUF_SLN} /Upgrade OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}")
  ENDIF()
  MESSAGE("-- Building Google Protocol Buffers debug libraries")
  EXECUTE_PROCESS(COMMAND devenv ${PROTOBUF_SLN} /Build "Debug|Win32" /Project libprotobuf OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}")
  ENDIF()
  MESSAGE("-- Building Google Protocol Buffers release libraries and compiler")
  EXECUTE_PROCESS(COMMAND devenv ${PROTOBUF_SLN} /Build "Release|Win32" /Project protoc OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}")
  ENDIF()
  EXECUTE_PROCESS(COMMAND CMD /C CALL extract_includes.bat WORKING_DIRECTORY ${PROTOBUF_VSPROJECTS_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}")
  ENDIF()
ELSE()
  UNSET(PROTOBUF_AUTOGEN CACHE)
  UNSET(PROTOBUF_CONFIGURE CACHE)
  SET(PROTOBUF_BUILD_DIR ${CURRENT_DIR}/build_protobuf)
  # Make a copy of protobuf to work with to avoid modified files being accidentally committed to repository
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CURRENT_DIR}/src/protobuf ${PROTOBUF_BUILD_DIR})
  FIND_FILE(PROTOBUF_AUTOGEN autogen.sh PATHS ${PROTOBUF_BUILD_DIR} NO_DEFAULT_PATH)
  MESSAGE("-- Generating configure file for Google Protocol Buffers")
  MESSAGE("     This may take a few minutes...")
  EXECUTE_PROCESS(COMMAND sh ${PROTOBUF_AUTOGEN} WORKING_DIRECTORY ${PROTOBUF_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
  IF(${RESVAR} EQUAL 0)
    FIND_FILE(PROTOBUF_CONFIGURE configure PATHS ${PROTOBUF_BUILD_DIR} NO_DEFAULT_PATH)
  ENDIF()
  IF(NOT PROTOBUF_CONFIGURE)
    SET(ERROR_MESSAGE "${OUTVAR}\n${ERRVAR}\nCould not create configure file for Google Protocol Buffers.\n\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}*** This needs autoconf and libtool to complete successfully. ***\n")
    MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
  ENDIF()
  MESSAGE("-- Configuring Google Protocol Buffers")
  MESSAGE("     This may take a few minutes...")
  EXECUTE_PROCESS(COMMAND sh ${PROTOBUF_CONFIGURE} --prefix=${PROTOBUF_BUILD_DIR} --enable-shared=no WORKING_DIRECTORY ${PROTOBUF_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}")
  ENDIF()
  MESSAGE("-- Making Google Protocol Buffers")
  MESSAGE("     This may take a few minutes...")
  EXECUTE_PROCESS(COMMAND make install WORKING_DIRECTORY ${PROTOBUF_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}\n${ERRVAR}")
  ENDIF()
ENDIF()

# Build Google Logging library
IF(MSVC)
  # Make a copy of .sln file to work with to avoid modified .sln being accidentally committed to repository
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${CURRENT_DIR}/src/glog/google-glog.sln ${CURRENT_DIR}/src/glog/google-glog_copy.sln)
  FIND_FILE(GLOG_SLN NAMES google-glog_copy.sln PATHS ${CURRENT_DIR}/src/glog NO_DEFAULT_PATH)
  GET_FILENAME_COMPONENT(GLOG_ROOT_DIR ${GLOG_SLN} PATH)
  MESSAGE("-- Upgrading Google Logging solution")
  EXECUTE_PROCESS(COMMAND devenv ${GLOG_SLN} /Upgrade OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}")
  ENDIF()
  MESSAGE("-- Building Google Logging debug libraries")
  EXECUTE_PROCESS(COMMAND devenv ${GLOG_SLN} /Build "Debug|Win32" /Project libglog_static OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}")
  ENDIF()
  MESSAGE("-- Building Google Logging release libraries")
  EXECUTE_PROCESS(COMMAND devenv ${GLOG_SLN} /Build "Release|Win32" /Project libglog_static OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}")
  ENDIF()
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}")
  ENDIF()
ELSE()
  UNSET(GLOG_CONFIGURE CACHE)
  SET(GLOG_BUILD_DIR ${CURRENT_DIR}/build_glog)
  # Make a copy of glog to work with to avoid modified files being accidentally committed to repository
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CURRENT_DIR}/src/glog ${GLOG_BUILD_DIR})
  FIND_FILE(GLOG_CONFIGURE configure PATHS ${GLOG_BUILD_DIR} NO_DEFAULT_PATH)
  IF(NOT GLOG_CONFIGURE)
    MESSAGE(FATAL_ERROR "${OUTVAR}\n${ERRVAR}\nCould not configure Google Logging.\n\n")
  ENDIF()
  MESSAGE("-- Configuring Google Logging library")
  MESSAGE("     This may take a few minutes...")
  EXECUTE_PROCESS(COMMAND sh ${GLOG_CONFIGURE} --prefix=${GLOG_BUILD_DIR} --enable-shared=no WORKING_DIRECTORY ${GLOG_BUILD_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}")
  ENDIF()
  MESSAGE("-- Making Google Logging library")
  MESSAGE("     This may take a few minutes...")
  EXECUTE_PROCESS(COMMAND make -i -C ${GLOG_BUILD_DIR} install OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}\n${ERRVAR}")
  ENDIF()
ENDIF()
