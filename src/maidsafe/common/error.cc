/*  Copyright 2012 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */

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

const std::error_category& GetCommonCategory() {
  static detail::CommonCategory instance;
  return instance;
}

common_error MakeError(CommonErrors code) { return common_error(make_error_code(code)); }

std::error_code make_error_code(AsymmErrors code) {
  return std::error_code(static_cast<int>(code), GetAsymmCategory());
}

std::error_condition make_error_condition(AsymmErrors code) {
  return std::error_condition(static_cast<int>(code), GetAsymmCategory());
}

const std::error_category& GetAsymmCategory() {
  static detail::AsymmCategory instance;
  return instance;
}

asymm_error MakeError(AsymmErrors code) { return asymm_error(make_error_code(code)); }

std::error_code make_error_code(PassportErrors code) {
  return std::error_code(static_cast<int>(code), GetPassportCategory());
}

std::error_condition make_error_condition(PassportErrors code) {
  return std::error_condition(static_cast<int>(code), GetPassportCategory());
}

const std::error_category& GetPassportCategory() {
  static detail::PassportCategory instance;
  return instance;
}

passport_error MakeError(PassportErrors code) { return passport_error(make_error_code(code)); }

std::error_code make_error_code(EncryptErrors code) {
  return std::error_code(static_cast<int>(code), GetEncryptCategory());
}

std::error_condition make_error_condition(EncryptErrors code) {
  return std::error_condition(static_cast<int>(code), GetEncryptCategory());
}

const std::error_category& GetEncryptCategory() {
  static detail::EncryptCategory instance;
  return instance;
}

encrypt_error MakeError(EncryptErrors code) { return encrypt_error(make_error_code(code)); }

std::error_code make_error_code(RoutingErrors code) {
  return std::error_code(static_cast<int>(code), GetRoutingCategory());
}

std::error_condition make_error_condition(RoutingErrors code) {
  return std::error_condition(static_cast<int>(code), GetRoutingCategory());
}

const std::error_category& GetRoutingCategory() {
  static detail::RoutingCategory instance;
  return instance;
}

routing_error MakeError(RoutingErrors code) { return routing_error(make_error_code(code)); }

std::error_code make_error_code(NfsErrors code) {
  return std::error_code(static_cast<int>(code), GetNfsCategory());
}

std::error_condition make_error_condition(NfsErrors code) {
  return std::error_condition(static_cast<int>(code), GetNfsCategory());
}

const std::error_category& GetNfsCategory() {
  static detail::NfsCategory instance;
  return instance;
}

nfs_error MakeError(NfsErrors code) { return nfs_error(make_error_code(code)); }

std::error_code make_error_code(DriveErrors code) {
  return std::error_code(static_cast<int>(code), GetDriveCategory());
}

std::error_condition make_error_condition(DriveErrors code) {
  return std::error_condition(static_cast<int>(code), GetDriveCategory());
}

const std::error_category& GetDriveCategory() {
  static detail::DriveCategory instance;
  return instance;
}

drive_error MakeError(DriveErrors code) { return drive_error(make_error_code(code)); }

std::error_code make_error_code(VaultErrors code) {
  return std::error_code(static_cast<int>(code), GetVaultCategory());
}

std::error_condition make_error_condition(VaultErrors code) {
  return std::error_condition(static_cast<int>(code), GetVaultCategory());
}

const std::error_category& GetVaultCategory() {
  static detail::VaultCategory instance;
  return instance;
}

vault_error MakeError(VaultErrors code) { return vault_error(make_error_code(code)); }

std::error_code make_error_code(VaultManagerErrors code) {
  return std::error_code(static_cast<int>(code), GetVaultManagerCategory());
}

std::error_condition make_error_condition(VaultManagerErrors code) {
  return std::error_condition(static_cast<int>(code), GetVaultManagerCategory());
}

const std::error_category& GetVaultManagerCategory() {
  static detail::VaultManagerCategory instance;
  return instance;
}

vault_manager_error MakeError(VaultManagerErrors code) {
  return vault_manager_error(make_error_code(code));
}

std::error_code make_error_code(ApiErrors code) {
  return std::error_code(static_cast<int>(code), GetApiCategory());
}

std::error_condition make_error_condition(ApiErrors code) {
  return std::error_condition(static_cast<int>(code), GetApiCategory());
}

const std::error_category& GetApiCategory() {
  static detail::ApiCategory instance;
  return instance;
}

api_error MakeError(ApiErrors code) { return api_error(make_error_code(code)); }

}  // namespace maidsafe
