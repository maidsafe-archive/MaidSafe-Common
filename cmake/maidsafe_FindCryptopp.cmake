# ============================================================================ #
#                                                                              #
# Copyright [2010] maidsafe.net limited                                        #
#                                                                              #
# Description:  See below.                                                     #
# Version:      1.0                                                            #
# Created:      2010-04-15-21.01.30                                            #
# Revision:     none                                                           #
# Compiler:     N/A                                                            #
# Author:       Team                                                           #
# Company:      maidsafe.net limited                                           #
#                                                                              #
# The following source code is property of maidsafe.net limited and is not     #
# meant for external use.  The use of this code is governed by the license     #
# file LICENSE.TXT found in the root of this directory and also on             #
# www.maidsafe.net.                                                            #
#                                                                              #
# You are not free to copy, amend or otherwise use this source code without    #
# the explicit written permission of the board of directors of maidsafe.net.   #
#                                                                              #
# ============================================================================ #
#                                                                              #
#  Module used to locate CryptoPP libs and headers.                            #
#                                                                              #
#  Settable variables to aid with finding CryptoPP are:                        #
#    CRYPTOPP_LIB_DIR, CRYPTOPP_INC_DIR and CRYPTOPP_ROOT_DIR                  #
#                                                                              #
#  Variables set and cached by this module are:                                #
#    Cryptopp_INCLUDE_DIR, Cryptopp_LIBRARY_DIR, Cryptopp_LIBRARY              #
#                                                                              #
#  For MSVC, Cryptopp_LIBRARY_DIR_DEBUG is also set and cached.                #
#                                                                              #
#==============================================================================#


UNSET(Cryptopp_INCLUDE_DIR CACHE)
UNSET(Cryptopp_LIBRARY_DIR CACHE)
UNSET(Cryptopp_LIBRARY_DIR_DEBUG CACHE)
UNSET(Cryptopp_LIBRARY CACHE)
UNSET(Cryptopp_LIBRARY_DEBUG CACHE)
UNSET(Cryptopp_LIBRARY_RELEASE CACHE)

IF(CRYPTOPP_LIB_DIR)
  SET(CRYPTOPP_LIB_DIR ${CRYPTOPP_LIB_DIR} CACHE PATH "Path to CryptoPP libraries directory" FORCE)
ENDIF()
IF(CRYPTOPP_INC_DIR)
  SET(CRYPTOPP_INC_DIR ${CRYPTOPP_INC_DIR} CACHE PATH "Path to CryptoPP include directory" FORCE)
ENDIF()
IF(CRYPTOPP_ROOT_DIR)
  SET(CRYPTOPP_ROOT_DIR ${CRYPTOPP_ROOT_DIR} CACHE PATH "Path to CryptoPP root directory" FORCE)
ELSEIF(DEFAULT_THIRD_PARTY_ROOT)
  SET(CRYPTOPP_ROOT_DIR ${DEFAULT_THIRD_PARTY_ROOT})
ENDIF()

IF(MSVC)
  SET(CRYPTOPP_LIBPATH_SUFFIX third_party_libs/build_cryptopp/Release)
ELSE()
  SET(CRYPTOPP_LIBPATH_SUFFIX third_party_libs/build_cryptopp)
ENDIF()

FIND_LIBRARY(Cryptopp_LIBRARY_RELEASE NAMES cryptopp PATHS ${CRYPTOPP_LIB_DIR} ${CRYPTOPP_ROOT_DIR} PATH_SUFFIXES ${CRYPTOPP_LIBPATH_SUFFIX} NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
IF(MSVC)
  SET(CRYPTOPP_LIBPATH_SUFFIX third_party_libs/build_cryptopp/Debug)
  FIND_LIBRARY(Cryptopp_LIBRARY_DEBUG NAMES cryptopp_d PATHS ${CRYPTOPP_LIB_DIR} ${CRYPTOPP_ROOT_DIR} PATH_SUFFIXES ${CRYPTOPP_LIBPATH_SUFFIX} NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
ENDIF()

FIND_PATH(Cryptopp_INCLUDE_DIR cryptopp/config.h PATHS ${CRYPTOPP_INC_DIR} ${CRYPTOPP_ROOT_DIR}/third_party_libs/build_cryptopp/include NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)

GET_FILENAME_COMPONENT(CRYPTOPP_LIBRARY_DIR ${Cryptopp_LIBRARY_RELEASE} PATH)
SET(Cryptopp_LIBRARY_DIR ${CRYPTOPP_LIBRARY_DIR} CACHE PATH "Path to CryptoPP libraries directory" FORCE)
IF(MSVC)
  GET_FILENAME_COMPONENT(CRYPTOPP_LIBRARY_DIR_DEBUG ${Cryptopp_LIBRARY_DEBUG} PATH)
  SET(Cryptopp_LIBRARY_DIR_DEBUG ${CRYPTOPP_LIBRARY_DIR_DEBUG} CACHE PATH "Path to CryptoPP debug libraries directory" FORCE)
ENDIF()

IF(NOT Cryptopp_LIBRARY_RELEASE)
  SET(ERROR_MESSAGE "\nCould not find CryptoPP.  NO CRYPTOPP LIBRARY - ")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}If CryptoPP is already installed, run:\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DCRYPTOPP_LIB_DIR=<Path to CryptoPP lib directory> and/or")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}\n${ERROR_MESSAGE_CMAKE_PATH} -DCRYPTOPP_ROOT_DIR=<Path to CryptoPP root directory>")
  MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
ELSE()
  SET(Cryptopp_LIBRARY ${Cryptopp_LIBRARY_RELEASE} CACHE PATH "Path to CryptoPP library" FORCE)
ENDIF()

IF(MSVC)
  IF(NOT Cryptopp_LIBRARY_DEBUG)
    SET(ERROR_MESSAGE "\nCould not find CryptoPP.  NO *DEBUG* CRYPTOPP LIBRARY - ")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}If CryptoPP is already installed, run:\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DCRYPTOPP_ROOT_DIR=<Path to CryptoPP root directory>")
    MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
  ELSE()
    SET(Cryptopp_LIBRARY debug ${Cryptopp_LIBRARY_DEBUG} optimized ${Cryptopp_LIBRARY} CACHE PATH "Path to CryptoPP libraries" FORCE)
  ENDIF()
ENDIF()

IF(NOT Cryptopp_INCLUDE_DIR)
  SET(ERROR_MESSAGE "\nCould not find CryptoPP.  NO CONFIG.H - ")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}If CryptoPP is already installed, run:\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DCRYPTOPP_INC_DIR=<Path to CryptoPP include directory> and/or")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}\n${ERROR_MESSAGE_CMAKE_PATH} -DCRYPTOPP_ROOT_DIR=<Path to CryptoPP root directory>")
  MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
ENDIF()

MESSAGE("-- Found CryptoPP library")
IF(MSVC)
  MESSAGE("-- Found CryptoPP Debug library")
ENDIF()
