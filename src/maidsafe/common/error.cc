/* Copyright (c) 2012 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "maidsafe/common/error.h"

#include "boost/throw_exception.hpp"

#include "maidsafe/common/error_categories.h"


namespace maidsafe {

std::error_code make_error_code(CommonErrors code) {
  return std::error_code(static_cast<int>(code), GetCommonCategory());
}

std::error_condition make_error_condition(CommonErrors code) {
  return std::error_condition(static_cast<int>(code), GetCommonCategory());
}

common_error MakeError(CommonErrors code) {
  return common_error(make_error_code(code));
}



std::error_code make_error_code(AsymmErrors code) {
  return std::error_code(static_cast<int>(code), GetAsymmCategory());
}

std::error_condition make_error_condition(AsymmErrors code) {
  return std::error_condition(static_cast<int>(code), GetAsymmCategory());
}

asymm_error MakeError(AsymmErrors code) {
  return asymm_error(make_error_code(code));
}



std::error_code make_error_code(PassportErrors code) {
  return std::error_code(static_cast<int>(code), GetPassportCategory());
}

std::error_condition make_error_condition(PassportErrors code) {
  return std::error_condition(static_cast<int>(code), GetPassportCategory());
}

passport_error MakeError(PassportErrors code) {
  return passport_error(make_error_code(code));
}



std::error_code make_error_code(NfsErrors code) {
  return std::error_code(static_cast<int>(code), GetNfsCategory());
}

std::error_condition make_error_condition(NfsErrors code) {
  return std::error_condition(static_cast<int>(code), GetNfsCategory());
}

nfs_error MakeError(NfsErrors code) {
  return nfs_error(make_error_code(code));
}



std::error_code make_error_code(RoutingErrors code) {
  return std::error_code(static_cast<int>(code), GetRoutingCategory());
}

std::error_condition make_error_condition(RoutingErrors code) {
  return std::error_condition(static_cast<int>(code), GetRoutingCategory());
}

routing_error MakeError(RoutingErrors code) {
  return routing_error(make_error_code(code));
}



std::error_code make_error_code(VaultErrors code) {
  return std::error_code(static_cast<int>(code), GetVaultCategory());
}

std::error_condition make_error_condition(VaultErrors code) {
  return std::error_condition(static_cast<int>(code), GetVaultCategory());
}

vault_error MakeError(VaultErrors code) {
  return vault_error(make_error_code(code));
}



std::error_code make_error_code(LifeStuffErrors code) {
  return std::error_code(static_cast<int>(code), GetLifeStuffCategory());
}

std::error_condition make_error_condition(LifeStuffErrors code) {
  return std::error_condition(static_cast<int>(code), GetLifeStuffCategory());
}

lifestuff_error MakeError(LifeStuffErrors code) {
  return lifestuff_error(make_error_code(code));
}



const std::error_category& GetCommonCategory() {
  static detail::CommonCategory instance;
  return instance;
}

const std::error_category& GetAsymmCategory() {
  static detail::AsymmCategory instance;
  return instance;
}

const std::error_category& GetPassportCategory() {
  static detail::PassportCategory instance;
  return instance;
}

const std::error_category& GetNfsCategory() {
  static detail::NfsCategory instance;
  return instance;
}

const std::error_category& GetRoutingCategory() {
  static detail::RoutingCategory instance;
  return instance;
}

const std::error_category& GetVaultCategory() {
  static detail::VaultCategory instance;
  return instance;
}

const std::error_category& GetLifeStuffCategory() {
  static detail::LifeStuffCategory instance;
  return instance;
}

}  // namespace maidsafe
