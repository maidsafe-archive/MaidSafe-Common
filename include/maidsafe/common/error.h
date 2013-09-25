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

#ifndef MAIDSAFE_COMMON_ERROR_H_
#define MAIDSAFE_COMMON_ERROR_H_

#include <string>
#include <system_error>

#include "boost/exception/all.hpp"

#include "maidsafe/common/config.h"

namespace maidsafe {

class maidsafe_error : public std::system_error {
 public:
  maidsafe_error(std::error_code ec, const std::string& what_arg)
      : std::system_error(ec, what_arg) {}
  maidsafe_error(std::error_code ec, const char* what_arg) : std::system_error(ec, what_arg) {}
  explicit maidsafe_error(std::error_code ec) : std::system_error(ec) {}
  maidsafe_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : std::system_error(ev, ecat, what_arg) {}
  maidsafe_error(int ev, const std::error_category& ecat, const char* what_arg)
      : std::system_error(ev, ecat, what_arg) {}
  maidsafe_error(int ev, const std::error_category& ecat) : std::system_error(ev, ecat) {}
};

enum class CommonErrors {
  success = 0,
  pending_result,
  unknown,
  null_pointer,
  invalid_node_id,
  invalid_key_size,
  invalid_string_size,
  invalid_parameter,
  invalid_conversion,
  file_too_large,
  uninitialised,
  hashing_error,
  symmetric_encryption_error,
  symmetric_decryption_error,
  compression_error,
  uncompression_error,
  cannot_invoke_from_this_thread,
  cannot_exceed_limit,
  unable_to_handle_request,
  filesystem_io_error,
  no_such_element,
  serialisation_error,
  parsing_error,
  not_a_directory
};

class common_error : public maidsafe_error {
 public:
  common_error(std::error_code ec, const std::string& what_arg) : maidsafe_error(ec, what_arg) {}
  common_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit common_error(std::error_code ec) : maidsafe_error(ec) {}
  common_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  common_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  common_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(CommonErrors code);
std::error_condition make_error_condition(CommonErrors code);
const std::error_category& GetCommonCategory();
common_error MakeError(CommonErrors code);

enum class AsymmErrors {
  keys_generation_error = 1,
  keys_serialisation_error,
  keys_parse_error,
  invalid_private_key,
  invalid_public_key,
  data_empty,
  invalid_file,
  invalid_signature,
  signature_empty,
  encryption_error,
  decryption_error,
  signing_error
};

class asymm_error : public maidsafe_error {
 public:
  asymm_error(std::error_code ec, const std::string& what_arg) : maidsafe_error(ec, what_arg) {}
  asymm_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit asymm_error(std::error_code ec) : maidsafe_error(ec) {}
  asymm_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  asymm_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  asymm_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(AsymmErrors code);
std::error_condition make_error_condition(AsymmErrors code);
const std::error_category& GetAsymmCategory();
asymm_error MakeError(AsymmErrors code);

enum class PassportErrors {
  fob_serialisation_error = 1,
  fob_parsing_error,
  mid_parsing_error,
  tmid_parsing_error,
  no_confirmed_fob,
  no_pending_fob,
  passport_parsing_error,
  public_id_already_exists,
  no_such_public_id
};

class passport_error : public maidsafe_error {
 public:
  passport_error(std::error_code ec, const std::string& what_arg) : maidsafe_error(ec, what_arg) {}
  passport_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit passport_error(std::error_code ec) : maidsafe_error(ec) {}
  passport_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  passport_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  passport_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(PassportErrors code);
std::error_condition make_error_condition(PassportErrors code);
const std::error_category& GetPassportCategory();
passport_error MakeError(PassportErrors code);

enum class EncryptErrors {
  bad_sequence = 1,
  no_data
};

class encrypt_error : public maidsafe_error {
 public:
  encrypt_error(std::error_code ec, const std::string& what_arg) : maidsafe_error(ec, what_arg) {}
  encrypt_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit encrypt_error(std::error_code ec) : maidsafe_error(ec) {}
  encrypt_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  encrypt_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  encrypt_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(EncryptErrors code);
std::error_condition make_error_condition(EncryptErrors code);
const std::error_category& GetEncryptCategory();
encrypt_error MakeError(EncryptErrors code);

enum class RoutingErrors {
  timed_out = 1,
  not_in_range,
  not_connected
};

class routing_error : public maidsafe_error {
 public:
  routing_error(std::error_code ec, const std::string& what_arg) : maidsafe_error(ec, what_arg) {}
  routing_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit routing_error(std::error_code ec) : maidsafe_error(ec) {}
  routing_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  routing_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  routing_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(RoutingErrors code);
std::error_condition make_error_condition(RoutingErrors code);
const std::error_category& GetRoutingCategory();
routing_error MakeError(RoutingErrors code);

enum class NfsErrors {
  failed_to_get_data = 1,
  timed_out
};

class nfs_error : public maidsafe_error {
 public:
  nfs_error(std::error_code ec, const std::string& what_arg) : maidsafe_error(ec, what_arg) {}
  nfs_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit nfs_error(std::error_code ec) : maidsafe_error(ec) {}
  nfs_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  nfs_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  nfs_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(NfsErrors code);
std::error_condition make_error_condition(NfsErrors code);
const std::error_category& GetNfsCategory();
nfs_error MakeError(NfsErrors code);

enum class DriveErrors {
  no_drive_letter_available = 1,
  failed_to_mount,
  permission_denied
};

class drive_error : public maidsafe_error {
 public:
  drive_error(std::error_code ec, const std::string& what_arg) : maidsafe_error(ec, what_arg) {}
  drive_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit drive_error(std::error_code ec) : maidsafe_error(ec) {}
  drive_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  drive_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  drive_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(DriveErrors code);
std::error_condition make_error_condition(DriveErrors code);
const std::error_category& GetDriveCategory();
drive_error MakeError(DriveErrors code);

enum class VaultErrors {
  failed_to_join_network = 1,
  failed_to_handle_request,
  operation_not_supported,
  permission_denied,
  no_such_account,
  low_space,
  not_enough_space,
  unique_data_clash,
  data_available_not_given
};

class vault_error : public maidsafe_error {
 public:
  vault_error(std::error_code ec, const std::string& what_arg) : maidsafe_error(ec, what_arg) {}
  vault_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit vault_error(std::error_code ec) : maidsafe_error(ec) {}
  vault_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  vault_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  vault_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(VaultErrors code);
std::error_condition make_error_condition(VaultErrors code);
const std::error_category& GetVaultCategory();
vault_error MakeError(VaultErrors code);

enum class LifeStuffErrors {
  kPasswordFailure = 1
};

class lifestuff_error : public maidsafe_error {
 public:
  lifestuff_error(std::error_code ec, const std::string& what_arg) : maidsafe_error(ec, what_arg) {}
  lifestuff_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit lifestuff_error(std::error_code ec) : maidsafe_error(ec) {}
  lifestuff_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  lifestuff_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  lifestuff_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(LifeStuffErrors code);
std::error_condition make_error_condition(LifeStuffErrors code);
const std::error_category& GetLifeStuffCategory();
lifestuff_error MakeError(LifeStuffErrors code);

template <typename MaidsafeErrorCode>
inline void ThrowError(const MaidsafeErrorCode& code) {
  auto error(MakeError(code));
  if (error.code())
    boost::throw_exception(error);
}

}  // namespace maidsafe

namespace std {

template <>
struct is_error_code_enum<maidsafe::CommonErrors> : public true_type {};

template <>
struct is_error_code_enum<maidsafe::AsymmErrors> : public true_type {};

template <>
struct is_error_code_enum<maidsafe::PassportErrors> : public true_type {};

template <>
struct is_error_code_enum<maidsafe::EncryptErrors> : public true_type {};

template <>
struct is_error_code_enum<maidsafe::RoutingErrors> : public true_type {};

template <>
struct is_error_code_enum<maidsafe::NfsErrors> : public true_type {};

template <>
struct is_error_code_enum<maidsafe::DriveErrors> : public true_type {};

template <>
struct is_error_code_enum<maidsafe::VaultErrors> : public true_type {};

template <>
struct is_error_code_enum<maidsafe::LifeStuffErrors> : public true_type {};

}  // namespace std

#endif  // MAIDSAFE_COMMON_ERROR_H_
