/* Copyright 2012 MaidSafe.net limited

This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or later,
and The General Public License (GPL), version 3. By contributing code to this project You agree to
the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in the root directory
of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available at:

http://www.novinet.com/license

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing permissions and limitations under the
License.
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

std::error_code make_error_code(EncryptErrors code) {
  return std::error_code(static_cast<int>(code), GetEncryptCategory());
}

std::error_condition make_error_condition(EncryptErrors code) {
  return std::error_condition(static_cast<int>(code), GetEncryptCategory());
}

encrypt_error MakeError(EncryptErrors code) {
  return encrypt_error(make_error_code(code));
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



std::error_code make_error_code(DriveErrors code) {
  return std::error_code(static_cast<int>(code), GetDriveCategory());
}

std::error_condition make_error_condition(DriveErrors code) {
  return std::error_condition(static_cast<int>(code), GetDriveCategory());
}

drive_error MakeError(DriveErrors code) {
  return drive_error(make_error_code(code));
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

const std::error_category& GetEncryptCategory() {
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

const std::error_category& GetDriveCategory() {
  static detail::DriveCategory instance;
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
