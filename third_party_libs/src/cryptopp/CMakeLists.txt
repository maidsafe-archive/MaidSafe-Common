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

CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)
PROJECT(Cryptopp)
ENABLE_TESTING()

SET(CMAKE_DEBUG_POSTFIX _d)
SET(CMAKE_RELWITHDEBINFO_POSTFIX _rwdi)
SET(CMAKE_MINSIZEREL_POSTFIX _msr)

IF(CMAKE_BUILD_TYPE MATCHES "Debug")
  SET(TEST_POSTFIX ${CMAKE_DEBUG_POSTFIX})
ELSEIF(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
  SET(TEST_POSTFIX ${CMAKE_RELWITHDEBINFO_POSTFIX})
ELSEIF(CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
  SET(TEST_POSTFIX ${CMAKE_MINSIZEREL_POSTFIX})
ENDIF()

INCLUDE(CTest)

SET(CTEST_CUSTOM_MAXIMUM_PASSED_TEST_OUTPUT_SIZE 50000)
SET(CTEST_CUSTOM_MAXIMUM_FAILED_TEST_OUTPUT_SIZE 50000)
SET(CTEST_CONTINUOUS_DURATION 600)
SET(CTEST_CONTINUOUS_MINIMUM_INTERVAL 10)
SET(CTEST_START_WITH_EMPTY_BINARY_DIRECTORY true)

FILE(GLOB cryptopp_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE(GLOB cryptopp_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

IF(CMAKE_CL_64)
  SET(cryptopp_SOURCES ${cryptopp_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/x64dll.asm ${CMAKE_CURRENT_SOURCE_DIR}/x64masm.asm)
  ADD_CUSTOM_COMMAND(OUTPUT $(IntDir)x64dll.obj COMMAND ml64.exe /c /nologo /Fo$(IntDir)x64dll.obj /Zi "${CMAKE_CURRENT_SOURCE_DIR}/x64dll.asm" MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/x64dll.asm VERBATIM)
  ADD_CUSTOM_COMMAND(OUTPUT $(IntDir)x64masm.obj COMMAND ml64.exe /c /nologo /Fo$(IntDir)x64masm.obj /Zi "${CMAKE_CURRENT_SOURCE_DIR}/x64masm.asm" MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/x64masm.asm VERBATIM)
ENDIF()

# Not used
LIST(REMOVE_ITEM cryptopp_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/eccrypto.cpp")
LIST(REMOVE_ITEM cryptopp_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/eprecomp.cpp")
LIST(REMOVE_ITEM cryptopp_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/cryptlib_bds.cpp")

# Set up test
LIST(REMOVE_ITEM cryptopp_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/bench.cpp")
LIST(REMOVE_ITEM cryptopp_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/bench2.cpp")
LIST(REMOVE_ITEM cryptopp_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/datatest.cpp")
LIST(REMOVE_ITEM cryptopp_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/dlltest.cpp")
LIST(REMOVE_ITEM cryptopp_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/fipsalgt.cpp")
LIST(REMOVE_ITEM cryptopp_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/regtest.cpp")
LIST(REMOVE_ITEM cryptopp_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test.cpp")
LIST(REMOVE_ITEM cryptopp_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/validat1.cpp")
LIST(REMOVE_ITEM cryptopp_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/validat2.cpp")
LIST(REMOVE_ITEM cryptopp_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/validat3.cpp")

SET(cryptopp_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/bench.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/bench2.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/datatest.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/dlltest.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/fipsalgt.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/regtest.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/validat1.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/validat2.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/validat3.cpp)

LIST(REMOVE_ITEM cryptopp_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/bench.h")
LIST(REMOVE_ITEM cryptopp_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/factory.h")
LIST(REMOVE_ITEM cryptopp_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/validate.h")

SET(cryptopp_TEST_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/bench.h
                          ${CMAKE_CURRENT_SOURCE_DIR}/factory.h
                          ${CMAKE_CURRENT_SOURCE_DIR}/validate.h)
ADD_EXECUTABLE(cryptest ${cryptopp_TEST_SOURCES} ${cryptopp_TEST_HEADERS})

ADD_LIBRARY(cryptopp_static STATIC ${cryptopp_SOURCES} ${cryptopp_HEADERS})
SET_TARGET_PROPERTIES(cryptopp_static PROPERTIES OUTPUT_NAME cryptopp)

IF(NOT MSVC)
  SET_TARGET_PROPERTIES(cryptest PROPERTIES
                          DEBUG_OUTPUT_NAME cryptest${CMAKE_DEBUG_POSTFIX}
                          RELWITHDEBINFO_OUTPUT_NAME cryptest${CMAKE_RELWITHDEBINFO_POSTFIX}
                          MINSIZEREL_OUTPUT_NAME cryptest${CMAKE_MINSIZEREL_POSTFIX})
ENDIF()

INSTALL(TARGETS cryptopp_static ARCHIVE DESTINATION lib)

IF(NOT WIN32)
  ADD_DEFINITIONS(-DNDEBUG)
ENDIF()

TARGET_LINK_LIBRARIES(cryptest cryptopp_static)
IF(APPLE)
  ADD_DEFINITIONS(-DCRYPTOPP_DISABLE_ASM -DCRYPTOPP_DISABLE_AESNI)
  SET_TARGET_PROPERTIES(cryptopp_static PROPERTIES COMPILE_FLAGS "-O2 -fPIC -w -finline-functions")
ELSEIF(UNIX)
  SET_TARGET_PROPERTIES(cryptopp_static PROPERTIES COMPILE_FLAGS "-O2 -fPIC -w -pipe -finline-functions")
ELSEIF(WIN32 AND MSVC)
  CMAKE_POLICY(SET CMP0008 NEW)
  SET_TARGET_PROPERTIES(cryptopp_static PROPERTIES COMPILE_FLAGS "/D_WINDOWS /DUSE_PRECOMPILED_HEADERS /DWIN32 /D_VC80_UPGRADE=0x0710 /EHsc /W3")
  SET_TARGET_PROPERTIES(cryptest PROPERTIES COMPILE_FLAGS "/D_CONSOLE /DWIN32 /D_VC80_UPGRADE=0x0710 /D_MBCS /EHsc /W3 /Zm400"
                                            LINK_FLAGS "/STACK:1048576"
                                            LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF /LTCG"
                                            LINK_FLAGS_RELWITHDEBINFO "/LTCG"
                                            LINK_FLAGS_MINSIZEREL "/LTCG")
  SET(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /Oi /Oy /GL /DNDEBUG /GF /MT /Gy /Zi")
  SET(CMAKE_CXX_FLAGS_DEBUG "/Od /Oi /D_DEBUG /MTd /Zi")
  LIST(REMOVE_ITEM cryptopp_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/dll.cpp") # this file doesn't use precompiled headers
  LIST(REMOVE_ITEM cryptopp_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/iterhash.cpp") # this file doesn't use precompiled headers
  LIST(REMOVE_ITEM cryptopp_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp") # this file is used to create precompiled headers
  SET_SOURCE_FILES_PROPERTIES(${cryptopp_SOURCES} PROPERTIES
                               COMPILE_FLAGS "/Yu\"pch.h\"")
  SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp PROPERTIES
                               COMPILE_FLAGS "/Yc\"pch.h\"")
  TARGET_LINK_LIBRARIES(cryptest odbc32.lib odbccp32.lib Ws2_32.lib)
ENDIF()
