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
#  Uses built in CMake module FindQt4 to locate QT libs and headers.  This     #
#  FindQt4 module requires that the Qt4 qmake executable is available via the  #
#  system path.  If this is the case, then the module sets the variable        #
#  QT_USE_FILE which is the path to a CMake file that is included in order to  #
#  compile Qt 4 applications and libraries. It sets up the compilation         #
#  environment for include directories, preprocessor defines and populates a   #
#  QT_LIBRARIES variable.                                                      #
#                                                                              #
#  Settable variable to aid with finding QT is QT_ROOT_DIR                     #
#                                                                              #
#  Since this module uses the option to include(${QT_USE_FILE}), the           #
#  compilation environment is automatically set up with include directories    #
#  and preprocessor definitions.  The requested QT libs are set up as targets, #
#  e.g. QT_QTCORE_LIBRARY.  All libs are added to the variable QT_LIBRARIES.   #
#  See documentation of FindQt4 for further info.                              #
#                                                                              #
#==============================================================================#


UNSET(QMAKE_EXE CACHE)
UNSET(QT_LIBRARIES CACHE)
UNSET(QT_LIBRARY_DIR CACHE)
UNSET(QT_MOC_EXECUTABLE CACHE)
UNSET(QT_INCLUDE_DIR CACHE)
UNSET(QT_INCLUDES CACHE)

SET(QT_USE_IMPORTED_TARGETS TRUE)
IF(QT_ROOT_DIR)
  SET(QT_ROOT_DIR ${QT_ROOT_DIR} CACHE PATH "Path to Qt root directory" FORCE)
  SET(ENV{QTDIR} ${QT_ROOT_DIR})
ELSEIF(DEFAULT_THIRD_PARTY_ROOT)
  FIND_THIRD_PARTY_PROJECT(QT_BASE_DIR Qt ${DEFAULT_THIRD_PARTY_ROOT})
  IF(QT_BASE_DIR_CACHED)
    FIND_THIRD_PARTY_PROJECT(QT_ROOT_DIR "" ${QT_BASE_DIR_CACHED})
    IF(QT_ROOT_DIR_CACHED)
      SET(QT_ROOT_DIR ${QT_ROOT_DIR_CACHED})
      SET(ENV{QTDIR} ${QT_ROOT_DIR})
    ENDIF()
  ENDIF()
ENDIF()

FIND_PROGRAM(QMAKE_EXE NAMES qmake qmake4 qmake-qt4 PATHS ${QT_ROOT_DIR}/bin)
IF(NOT QMAKE_EXE)
  SET(ERROR_MESSAGE "\nCould not find Qt.  NO QMAKE EXECUTABLE ")
  IF(WIN32)
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}(Tried to find qmake.exe in ${QT_ROOT_DIR}\\bin and the system path.)")
  ELSE()
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}(Tried to find qmake in ${QT_ROOT_DIR}/bin and the system path.)")
  ENDIF()
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}\nYou can download Qt at http://qt.nokia.com/downloads\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}If Qt is already installed, run:\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DQT_ROOT_DIR=<Path to Qt root directory>")
  IF(WIN32)
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}\n(such that qmake.exe is in \"<Path to Qt root directory>\\bin\").")
  ELSE()
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}\n(such that qmake is in \"<Path to Qt root directory>/bin\").")
  ENDIF()
  MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
ENDIF()

SET(QT_STATIC 1)
FIND_PACKAGE(Qt4 4.6.0 COMPONENTS QtCore QtGui QtMain QtXml REQUIRED)
INCLUDE(${QT_USE_FILE})

IF(NOT QT_QTCORE_FOUND)
  SET(ERROR_MESSAGE "\nCould not find Qt.  NO QT CORE LIBRARY - ")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}You can download Qt at http://qt.nokia.com/downloads\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}If Qt is already installed, run:\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DQT_ROOT_DIR=<Path to Qt root directory>")
  MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
ENDIF()
IF(QT_LIBRARIES)
  IF(NOT QT_INCLUDES)
    SET(ERROR_MESSAGE "\nCould not find Qt.  AT LEAST ONE HEADER FILE IS MISSING - ")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}You can download Qt at http://qt.nokia.com/downloads\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}If Qt is already installed, run:\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DQT_ROOT_DIR=<Path to Qt root directory>")
    MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
  ENDIF()
ELSE(QT_LIBRARIES)
  SET(ERROR_MESSAGE "\nCould not find Qt.  NO QT LIBRARIES - ")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}You can download Qt at http://qt.nokia.com/downloads\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}If Qt is already installed, run:\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DQT_ROOT_DIR=<Path to Qt root directory>")
  MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
ENDIF(QT_LIBRARIES)
