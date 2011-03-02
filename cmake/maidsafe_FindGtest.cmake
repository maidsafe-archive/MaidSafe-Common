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
#  Module used to locate GoogleTest libs and headers.                          #
#                                                                              #
#  If using MSVC, finds only Gtest libs which have been compiled using         #
#  dynamically-linked C runtime library (i.e. with /MD set rather than /MT)    #
#                                                                              #
#  Settable variables to aid with finding Gtest are:                           #
#    GTEST_LIB_DIR, GTEST_INC_DIR and GTEST_ROOT_DIR                           #
#                                                                              #
#  Variables set and cached by this module are:                                #
#    Gtest_INCLUDE_DIR, Gtest_LIBRARY_DIR, Gtest_LIBRARY                       #
#                                                                              #
#  For MSVC, Gtest_LIBRARY_DIR_DEBUG is also set and cached.                   #
#                                                                              #
#==============================================================================#


UNSET(Gtest_INCLUDE_DIR CACHE)
UNSET(Gtest_LIBRARY_DIR CACHE)
UNSET(Gtest_LIBRARY_DIR_DEBUG CACHE)
UNSET(Gtest_LIBRARY CACHE)
UNSET(Gtest_LIBRARY_DEBUG CACHE)
UNSET(Gtest_LIBRARY_RELEASE CACHE)

IF(GTEST_LIB_DIR)
  SET(GTEST_LIB_DIR ${GTEST_LIB_DIR} CACHE PATH "Path to GoogleTest libraries directory" FORCE)
ENDIF()
IF(GTEST_INC_DIR)
  SET(GTEST_INC_DIR ${GTEST_INC_DIR} CACHE PATH "Path to GoogleTest include directory" FORCE)
ENDIF()
IF(GTEST_ROOT_DIR)
  SET(GTEST_ROOT_DIR ${GTEST_ROOT_DIR} CACHE PATH "Path to GoogleTest root directory" FORCE)
ELSEIF(DEFAULT_THIRD_PARTY_ROOT)
  FIND_THIRD_PARTY_PROJECT(GTEST_ROOT_DIR gtest ${DEFAULT_THIRD_PARTY_ROOT})
  IF(GTEST_ROOT_DIR_CACHED)
    SET(GTEST_ROOT_DIR ${GTEST_ROOT_DIR_CACHED})
  ENDIF()
ENDIF()

IF(MSVC)
  IF(CMAKE_CL_64)
    SET(GTEST_LIBPATH_SUFFIX msvc/x64/Release)
  ELSE()
    SET(GTEST_LIBPATH_SUFFIX msvc/gtest-md/Release)
  ENDIF()
ELSE()
  SET(GTEST_LIBPATH_SUFFIX lib lib64)
ENDIF()

FIND_LIBRARY(Gtest_LIBRARY_RELEASE NAMES gtest gtest-md PATHS ${GTEST_LIB_DIR} ${GTEST_ROOT_DIR} PATH_SUFFIXES ${GTEST_LIBPATH_SUFFIX})
IF(MSVC)
  IF(CMAKE_CL_64)
    SET(GTEST_LIBPATH_SUFFIX msvc/x64/Debug)
  ELSE()
    SET(GTEST_LIBPATH_SUFFIX msvc/gtest-md/Debug)
  ENDIF()
  FIND_LIBRARY(Gtest_LIBRARY_DEBUG NAMES gtestd gtest-mdd PATHS ${GTEST_LIB_DIR} ${GTEST_ROOT_DIR} PATH_SUFFIXES ${GTEST_LIBPATH_SUFFIX})
ENDIF()

FIND_PATH(Gtest_INCLUDE_DIR gtest/gtest.h PATHS ${GTEST_INC_DIR} ${GTEST_ROOT_DIR}/include)

GET_FILENAME_COMPONENT(GTEST_LIBRARY_DIR ${Gtest_LIBRARY_RELEASE} PATH)
SET(Gtest_LIBRARY_DIR ${GTEST_LIBRARY_DIR} CACHE PATH "Path to GoogleTest libraries directory" FORCE)
IF(MSVC)
  GET_FILENAME_COMPONENT(GTEST_LIBRARY_DIR_DEBUG ${Gtest_LIBRARY_DEBUG} PATH)
  SET(Gtest_LIBRARY_DIR_DEBUG ${GTEST_LIBRARY_DIR_DEBUG} CACHE PATH "Path to GoogleTest debug libraries directory" FORCE)
ENDIF()

IF(NOT Gtest_LIBRARY_RELEASE)
  SET(ERROR_MESSAGE "\nCould not find Google Test.  NO GTEST LIBRARY - ")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}You can download it at http://code.google.com/p/googletest\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}If Google Test is already installed, run:\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DGTEST_LIB_DIR=<Path to gtest lib directory> and/or")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}\n${ERROR_MESSAGE_CMAKE_PATH} -DGTEST_ROOT_DIR=<Path to gtest root directory>")
  MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
ELSE()
  SET(Gtest_LIBRARY ${Gtest_LIBRARY_RELEASE} CACHE PATH "Path to GoogleTest library" FORCE)
ENDIF()

IF(MSVC)
  IF(NOT Gtest_LIBRARY_DEBUG)
    SET(ERROR_MESSAGE "\nCould not find Google Test.  NO *DEBUG* GTEST LIBRARY - ")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}You can download it at http://code.google.com/p/googletest\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}If Google Test is already installed, run:\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DGTEST_ROOT_DIR=<Path to gtest root directory>")
    MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
  ELSE()
    SET(Gtest_LIBRARY debug ${Gtest_LIBRARY_DEBUG} optimized ${Gtest_LIBRARY} CACHE PATH "Path to GoogleTest libraries" FORCE)
  ENDIF()
ENDIF()

IF(NOT Gtest_INCLUDE_DIR)
  SET(ERROR_MESSAGE "\nCould not find Google Test.  NO GTEST.H - ")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}You can download it at http://code.google.com/p/googletest\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}If Google Test is already installed, run:\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DGTEST_INC_DIR=<Path to gtest include directory> and/or")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}\n${ERROR_MESSAGE_CMAKE_PATH} -DGTEST_ROOT_DIR=<Path to gtest root directory>")
  MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
ENDIF()

MESSAGE("-- Found Google Test library")
IF(MSVC)
  MESSAGE("-- Found Google Test Debug library")
ENDIF()
