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
#  Module used to set standard compiler and linker flags.                      #
#                                                                              #
#==============================================================================#


ADD_DEFINITIONS(-DSTATICLIB)
ADD_DEFINITIONS(-DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_FILESYSTEM_VERSION=3)

IF(CMAKE_BUILD_TYPE MATCHES "Debug")
  ADD_DEFINITIONS(-DDEBUG)
ENDIF()

IF(WIN32)
  ADD_DEFINITIONS(-DWIN32 -D_WIN32 -D__WINDOWS__ -D__WIN32__ -DMAIDSAFE_WIN32)
ENDIF()

IF(MSVC)
  ADD_DEFINITIONS(-D__MSVC__ -DWIN32_LEAN_AND_MEAN -D_WIN32_WINNT=0x501)
  ADD_DEFINITIONS(-D_CONSOLE -D_UNICODE -DUNICODE -D_BIND_TO_CURRENT_VCLIBS_VERSION=1)

  # prevents std::min() and std::max() to be overwritten
  ADD_DEFINITIONS(-DNOMINMAX)

  # flag to link to static version of Google Glog
  ADD_DEFINITIONS(-DGOOGLE_GLOG_DLL_DECL=)

  # prevents from automatic linking of boost libraries
  ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)

  SET(CMAKE_CXX_FLAGS "/EHsc /W4 /WX")
  SET(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /Ot /Oy /GL /D \"NDEBUG\" /MT /Gy /Zi")
  IF(CMAKE_CL_64)
    SET(CMAKE_CXX_FLAGS_DEBUG "/Od /Ot /Oy /D \"_DEBUG\" /D \"DEBUG\" /MTd /c /Zi /TP")
  ELSE()
    SET(CMAKE_CXX_FLAGS_DEBUG "/Od /Ot /Oy /D \"_DEBUG\" /D \"DEBUG\" /MTd /c /ZI /TP")
  ENDIF()
  SET(CMAKE_CXX_FLAGS_MINSIZEREL "/MT")
  SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MT")

  # CMake defaults to passing stack size in LINKER_FLAGS of 10MB.  Set this to windows default of 1MB
  #STRING(REGEX REPLACE "/STACK:[0-9]+" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
  #STRING(REGEX REPLACE "/STACK:[0-9]+" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")

  # Given a link dir of "a/b/c", MSVC adds "a/b/c/" AND "a/b/c/CMAKE_BUILD_TYPE" as link dirs, so we
  # can't just use "LINK_DIRECTORIES" as some Google debug libs have the same name as the release version.
  FOREACH(LIBS_DIR ${LIBS_DIRS})
    STRING(REPLACE "\\" "\\\\" LIBS_DIR ${LIBS_DIR})
    SET(LINKER_LIBS_DIRS_RELEASE "${LINKER_LIBS_DIRS_RELEASE} /LIBPATH:\"${LIBS_DIR}\"")
  ENDFOREACH()
  FOREACH(LIBS_DIR_DEBUG ${LIBS_DIRS_DEBUG})
    STRING(REPLACE "\\" "\\\\" LIBS_DIR_DEBUG ${LIBS_DIR_DEBUG})
    SET(LINKER_LIBS_DIRS_DEBUG "${LINKER_LIBS_DIRS_DEBUG} /LIBPATH:\"${LIBS_DIR_DEBUG}\"")
  ENDFOREACH()
ELSEIF(UNIX)
  IF(DEFINED COVERAGE)
    IF(${COVERAGE})
      SET(COVERAGE_FLAGS "-pg -fprofile-arcs -ftest-coverage")
    ELSE()
      SET(COVERAGE_FLAGS)
    ENDIF()
  ELSE()
    IF($ENV{COVERAGE})
      SET(COVERAGE_FLAGS "-pg -fprofile-arcs -ftest-coverage")
      SET(COVERAGE ON)
    ELSE()
      SET(COVERAGE_FLAGS)
    ENDIF()
  ENDIF()
  IF(${COVERAGE})
    MESSAGE("-- Coverage ON")
    MESSAGE("================================================================================")
  ELSE()
    MESSAGE("-- Coverage OFF.  To enable, do:   export COVERAGE=ON   and re-run CMake")
    MESSAGE("================================================================================")
  ENDIF()
  SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g -ggdb ${COVERAGE_FLAGS}")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -Wlogical-op -Wunused-parameter -Wno-system-headers -Wno-deprecated")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wwrite-strings -Wundef -std=c++0x -D_FORTIFY_SOURCE=2 -D_FILE_OFFSET_BITS=64")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -ggdb ${COVERAGE_FLAGS}")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
  SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${COVERAGE_FLAGS}")
  IF(APPLE)
    ADD_DEFINITIONS(-DMAIDSAFE_APPLE -DBSD)
    SET(CMAKE_AR "/usr/bin/libtool")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DARWIN_C_SOURCE")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finline-functions -funswitch-loops -fgcse-after-reload -ftree-vectorize -D__FreeBSD__=10")
    SET(CMAKE_CXX_CREATE_STATIC_LIBRARY "<CMAKE_AR> -static -o <TARGET> <LINK_FLAGS> <OBJECTS>" "<CMAKE_RANLIB> <TARGET>")
    IF(${COVERAGE})
      SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -lgcov")
    ENDIF()
  ELSE()
    ADD_DEFINITIONS(-DMAIDSAFE_LINUX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pedantic -pedantic-errors -Weffc++")
  ENDIF()
  UNSET(COVERAGE CACHE)
ENDIF()
