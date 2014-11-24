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

#include "maidsafe/common/serialisation.h"

namespace maidsafe {

namespace {

const int kMultiple(100000);

enum class ErrorCategories : int {
  kCommon = 1 * kMultiple,
  kAsymm = 2 * kMultiple,
  kPassport = 3 * kMultiple,
  kRudp = 4 * kMultiple,
  kEncrypt = 5 * kMultiple,
  kRouting = 6 * kMultiple,
  kNfs = 7 * kMultiple,
  kDrive = 8 * kMultiple,
  kVault = 9 * kMultiple,
  kVaultManager = 10 * kMultiple,
  kApi = 11 * kMultiple
};

}  // unnamed namespace

int ErrorToInt(maidsafe_error error) {
  if (error.code().category() == GetCommonCategory())
    return -static_cast<int>(ErrorCategories::kCommon) - error.code().value();
  if (error.code().category() == GetAsymmCategory())
    return -static_cast<int>(ErrorCategories::kAsymm) - error.code().value();
  if (error.code().category() == GetPassportCategory())
    return -static_cast<int>(ErrorCategories::kPassport) - error.code().value();
  if (error.code().category() == GetRudpCategory())
    return -static_cast<int>(ErrorCategories::kRudp) - error.code().value();
  if (error.code().category() == GetEncryptCategory())
    return -static_cast<int>(ErrorCategories::kEncrypt) - error.code().value();
  if (error.code().category() == GetRoutingCategory())
    return -static_cast<int>(ErrorCategories::kRouting) - error.code().value();
  if (error.code().category() == GetNfsCategory())
    return -static_cast<int>(ErrorCategories::kNfs) - error.code().value();
  if (error.code().category() == GetDriveCategory())
    return -static_cast<int>(ErrorCategories::kDrive) - error.code().value();
  if (error.code().category() == GetVaultCategory())
    return -static_cast<int>(ErrorCategories::kVault) - error.code().value();
  if (error.code().category() == GetVaultManagerCategory())
    return -static_cast<int>(ErrorCategories::kVaultManager) - error.code().value();
  if (error.code().category() == GetApiCategory())
    return -static_cast<int>(ErrorCategories::kApi) - error.code().value();
  BOOST_THROW_EXCEPTION(MakeError(CommonErrors::serialisation_error));
}

maidsafe_error IntToError(int value) {
  value = 0 - value;
  ErrorCategories category_value{static_cast<ErrorCategories>(value - (value % kMultiple))};
  int error_value{value % kMultiple};
  switch (category_value) {
    case ErrorCategories::kCommon:
      return maidsafe_error{error_value, GetCommonCategory()};
    case ErrorCategories::kAsymm:
      return maidsafe_error{error_value, GetAsymmCategory()};
    case ErrorCategories::kPassport:
      return maidsafe_error{error_value, GetPassportCategory()};
    case ErrorCategories::kRudp:
      return maidsafe_error{error_value, GetRudpCategory()};
    case ErrorCategories::kEncrypt:
      return maidsafe_error{error_value, GetEncryptCategory()};
    case ErrorCategories::kRouting:
      return maidsafe_error{error_value, GetRoutingCategory()};
    case ErrorCategories::kNfs:
      return maidsafe_error{error_value, GetNfsCategory()};
    case ErrorCategories::kDrive:
      return maidsafe_error{error_value, GetDriveCategory()};
    case ErrorCategories::kVault:
      return maidsafe_error{error_value, GetVaultCategory()};
    case ErrorCategories::kVaultManager:
      return maidsafe_error{error_value, GetVaultManagerCategory()};
    case ErrorCategories::kApi:
      return maidsafe_error{error_value, GetApiCategory()};
    default:
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
  }
}

maidsafe_error::serialised_type Serialise(maidsafe_error error) {
  error.value_ = ErrorToInt(error);
  return maidsafe_error::serialised_type{ ConvertToString(error) };
}

maidsafe_error Parse(maidsafe_error::serialised_type serialised_error) {
  maidsafe_error copy {std::error_code {}, std::string {}};
  try { ConvertFromString(serialised_error.data, copy); }
  catch(...) { BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));  }
  return IntToError(static_cast<int>(copy.value_));
}

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

std::error_code make_error_code(RudpErrors code) {
  return std::error_code(static_cast<int>(code), GetRudpCategory());
}

std::error_condition make_error_condition(RudpErrors code) {
  return std::error_condition(static_cast<int>(code), GetRudpCategory());
}

const std::error_category& GetRudpCategory() {
  static detail::RudpCategory instance;
  return instance;
}

rudp_error MakeError(RudpErrors code) { return rudp_error(make_error_code(code)); }

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
