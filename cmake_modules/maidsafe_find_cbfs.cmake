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
#  Module used to locate Callback File System (CBFS) lib and header.           #
#                                                                              #
#  Settable variables to aid with finding CBFS are:                            #
#    CBFS_ROOT_DIR                                                             #
#                                                                              #
#  Variables set and cached by this module are:                                #
#    Cbfs_INCLUDE_DIR, Cbfs_LIBRARY_DIR, Cbfs_LIBRARY                          #
#                                                                              #
#==============================================================================#


UNSET(Cbfs_INCLUDE_DIR CACHE)
UNSET(Cbfs_LIBRARY_DIR CACHE)
UNSET(Cbfs_LIBRARY CACHE)

IF(CBFS_ROOT_DIR)
  SET(CBFS_ROOT_DIR ${CBFS_ROOT_DIR} CACHE PATH "Path to Callback File System library directory" FORCE)
ENDIF()
FIND_LIBRARY(Cbfs_LIBRARY NAMES cbfs cbfs.lib PATHS ${CBFS_ROOT_DIR})
IF(NOT Cbfs_LIBRARY)
  SET(ERROR_MESSAGE "\nCould not find Callback File System.  NO CBFS LIBRARY - ")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}If Cbfs is already installed, run:\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DCBFS_ROOT_DIR=<Path to Cbfs lib directory>\n\n")
  MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
ENDIF()
GET_FILENAME_COMPONENT(CBFS_ROOT_DIR ${Cbfs_LIBRARY} PATH)
SET(Cbfs_LIBRARY_DIR ${CBFS_ROOT_DIR} CACHE PATH "Path to Callback File System library directory" FORCE)

FIND_PATH(Cbfs_INCLUDE_DIR cbfs.h PATHS ${CBFS_ROOT_DIR})
IF(NOT Cbfs_INCLUDE_DIR)
  SET(ERROR_MESSAGE "\nCould not find Callback File System.  NO CBFS.H - ")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}If Cbfs is already installed, run:\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DCBFS_ROOT_DIR=<Path to Cbfs include directory>\n\n")
  MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
ELSE()
  INCLUDE_DIRECTORIES(${Cbfs_INCLUDE_DIR})
  IF(CMAKE_INCLUDE_DIRECTORIES_BEFORE)
    SET(INCLUDE_DIRS ${Cbfs_INCLUDE_DIR} ${INCLUDE_DIRS})
  ELSE()
    SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${Cbfs_INCLUDE_DIR})
  ENDIF()
ENDIF()

MESSAGE("-- Found library ${Cbfs_LIBRARY}")
