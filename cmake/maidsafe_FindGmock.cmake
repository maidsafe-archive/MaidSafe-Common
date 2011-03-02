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
#  Module used to locate GoogleMock libs and headers.                          #
#                                                                              #
#  Settable variables to aid with finding Gmock are:                           #
#    GMOCK_LIB_DIR, GMOCK_INC_DIR and GMOCK_ROOT_DIR                           #
#                                                                              #
#  Variables set and cached by this module are:                                #
#    Gmock_INCLUDE_DIR, Gmock_LIBRARY_DIR, Gmock_LIBRARY                       #
#                                                                              #
#  For MSVC, Gmock_LIBRARY_DIR_DEBUG is also set and cached.                   #
#                                                                              #
#==============================================================================#


UNSET(Gmock_INCLUDE_DIR CACHE)
UNSET(Gmock_LIBRARY_DIR CACHE)
UNSET(Gmock_LIBRARY_DIR_DEBUG CACHE)
UNSET(Gmock_LIBRARY CACHE)
UNSET(Gmock_LIBRARY_DEBUG CACHE)
UNSET(Gmock_LIBRARY_RELEASE CACHE)

IF(GMOCK_LIB_DIR)
  SET(GMOCK_LIB_DIR ${GMOCK_LIB_DIR} CACHE PATH "Path to GoogleMock libraries directory" FORCE)
ENDIF()
IF(GMOCK_INC_DIR)
  SET(GMOCK_INC_DIR ${GMOCK_INC_DIR} CACHE PATH "Path to GoogleMock include directory" FORCE)
ENDIF()
IF(GMOCK_ROOT_DIR)
  SET(GMOCK_ROOT_DIR ${GMOCK_ROOT_DIR} CACHE PATH "Path to GoogleMock root directory" FORCE)
ELSEIF(DEFAULT_THIRD_PARTY_ROOT)
  FIND_THIRD_PARTY_PROJECT(GMOCK_ROOT_DIR gmock ${DEFAULT_THIRD_PARTY_ROOT})
  IF(GMOCK_ROOT_DIR_CACHED)
    SET(GMOCK_ROOT_DIR ${GMOCK_ROOT_DIR_CACHED})
  ENDIF()
ENDIF()

IF(MSVC)
  SET(GMOCK_LIBPATH_SUFFIX msvc/Release)
ELSE()
  SET(GMOCK_LIBPATH_SUFFIX lib)
ENDIF()

FIND_LIBRARY(Gmock_LIBRARY_RELEASE NAMES gmock PATHS ${GMOCK_LIB_DIR} ${GMOCK_ROOT_DIR} PATH_SUFFIXES ${GMOCK_LIBPATH_SUFFIX})
IF(MSVC)
  SET(GMOCK_LIBPATH_SUFFIX msvc/Debug)
  FIND_LIBRARY(Gmock_LIBRARY_DEBUG NAMES gmock PATHS ${GMOCK_LIB_DIR} ${GMOCK_ROOT_DIR} PATH_SUFFIXES ${GMOCK_LIBPATH_SUFFIX})
ENDIF()

FIND_PATH(Gmock_INCLUDE_DIR gmock/gmock.h PATHS ${GMOCK_INC_DIR} ${GMOCK_ROOT_DIR}/include)

GET_FILENAME_COMPONENT(GMOCK_LIBRARY_DIR ${Gmock_LIBRARY_RELEASE} PATH)
SET(Gmock_LIBRARY_DIR ${GMOCK_LIBRARY_DIR} CACHE PATH "Path to GoogleMock libraries directory" FORCE)
IF(MSVC)
  GET_FILENAME_COMPONENT(GMOCK_LIBRARY_DIR_DEBUG ${Gmock_LIBRARY_DEBUG} PATH)
  SET(Gmock_LIBRARY_DIR_DEBUG ${GMOCK_LIBRARY_DIR_DEBUG} CACHE PATH "Path to GoogleMock debug libraries directory" FORCE)
ENDIF()

IF(NOT Gmock_LIBRARY_RELEASE)
  SET(ERROR_MESSAGE "\nCould not find Google Mock.  NO GMOCK LIBRARY - ")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}You can download it at http://code.google.com/p/googlemock\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}If Google Mock is already installed, run:\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DGMOCK_LIB_DIR=<Path to gmock lib directory> and/or")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}\n${ERROR_MESSAGE_CMAKE_PATH} -DGMOCK_ROOT_DIR=<Path to gmock root directory>")
  MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
ELSE()
  SET(Gmock_LIBRARY ${Gmock_LIBRARY_RELEASE} CACHE PATH "Path to GoogleMock library" FORCE)
ENDIF()

IF(MSVC)
  IF(NOT Gmock_LIBRARY_DEBUG)
    SET(ERROR_MESSAGE "\nCould not find Google Mock.  NO *DEBUG* GMOCK LIBRARY - ")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}You can download it at http://code.google.com/p/googlemock\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}If Google Mock is already installed, run:\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DGMOCK_ROOT_DIR=<Path to gmock root directory>")
    MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
  ELSE()
    SET(Gmock_LIBRARY debug ${Gmock_LIBRARY_DEBUG} optimized ${Gmock_LIBRARY} CACHE PATH "Path to GoogleMock libraries" FORCE)
  ENDIF()
ENDIF()

IF(NOT Gmock_INCLUDE_DIR)
  SET(ERROR_MESSAGE "\nCould not find Google Mock.  NO GMOCK.H - ")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}You can download it at http://code.google.com/p/googlemock\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}If Google Mock is already installed, run:\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DGMOCK_INC_DIR=<Path to gmock include directory> and/or")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}\n${ERROR_MESSAGE_CMAKE_PATH} -DGMOCK_ROOT_DIR=<Path to gmock root directory>")
  MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
ENDIF()

MESSAGE("-- Found Google Mock library")
IF(MSVC)
  MESSAGE("-- Found Google Mock Debug library")
ENDIF()
