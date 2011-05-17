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
#  Module used to clone MaidSafe and Sigmoid repositories on GitHub.           #
#                                                                              #
#  Modules to be cloned should be provided in variable ALL_REPOSITORIES.  Path #
#  to root directory of all clones should be set in REPOSITORIES_ROOT_DIR.     #
#                                                                              #
#==============================================================================#


IF(NOT ALL_REPOSITORIES)
  SET(ALL_REPOSITORIES
        MaidSafe-Common
        MaidSafe-Encrypt
        MaidSafe-DHT
        PKI
        Passport
        MaidSafe-PD
        File-Browser
        LifeStuff
        LifeStuff-GUI
        SigmoidCore
        SigmoidPro
        Deduplicator-Gauge
        )
ENDIF()

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
INCLUDE(maidsafe_find_git)

FILE(MAKE_DIRECTORY ${REPOSITORIES_ROOT_DIR})

FOREACH(REPO ${ALL_REPOSITORIES})
  MESSAGE("-- Cloning ${REPO}")
  EXECUTE_PROCESS(COMMAND ${Git_EXECUTABLE} clone git@github.com:maidsafe/${REPO}.git
                  WORKING_DIRECTORY ${REPOSITORIES_ROOT_DIR}
                  RESULT_VARIABLE RES_VAR OUTPUT_VARIABLE OUT_VAR ERROR_VARIABLE ERR_VAR)
  IF(NOT RES_VAR EQUAL 0)
    MESSAGE("  Error cloning ${REPO}.  Result: ${RES_VAR}")
    MESSAGE("  ${OUT_VAR}")
  ENDIF()
ENDFOREACH()
