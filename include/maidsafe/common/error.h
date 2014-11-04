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

#include <cstdint>
#include <string>
#include <system_error>

#include "boost/exception/all.hpp"

#include "maidsafe/common/config.h"
#include "maidsafe/common/tagged_value.h"

namespace maidsafe {

class maidsafe_error : public std::system_error {
 public:
  typedef TaggedValue<std::string, struct SerialisedErrorTag> serialised_type;
  maidsafe_error(std::error_code ec, const std::string& what_arg)
      : std::system_error(ec, what_arg), value_(0) {}
  maidsafe_error(std::error_code ec, const char* what_arg)
      : std::system_error(ec, what_arg), value_(0) {}
  explicit maidsafe_error(std::error_code ec) : std::system_error(ec), value_(0) {}
  maidsafe_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : std::system_error(ev, ecat, what_arg), value_(0) {}
  maidsafe_error(int ev, const std::error_category& ecat, const char* what_arg)
      : std::system_error(ev, ecat, what_arg), value_(0) {}
  maidsafe_error(int ev, const std::error_category& ecat)
      : std::system_error(ev, ecat), value_(0) {}

  template<typename Archive>
  Archive& serialize(Archive& ref_archive) {
    return ref_archive(value_);
  }

  std::int64_t value_;
};

int32_t ErrorToInt(maidsafe_error error);
maidsafe_error IntToError(int32_t);
maidsafe_error::serialised_type Serialise(maidsafe_error error);
maidsafe_error Parse(maidsafe_error::serialised_type serialised_error);

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
  already_initialised,
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
  not_a_directory,
  db_busy,
  db_not_presented,
  db_error,
  defaulted
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
  id_already_exists = 1
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
  no_data,
  invalid_encryption_version,
  failed_to_write,
  failed_to_prepare_for_write,
  failed_to_get_chunk,
  failed_to_flush,
  failed_to_decrypt,
  failed_to_read,
  encryptor_closed
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
  timer_cancelled,
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
  permission_denied,
  no_such_file,
  file_exists,
  driver_not_installed
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
  data_available_not_given,
  account_already_exists,
  data_already_exists
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

enum class VaultManagerErrors {
  connection_not_found = 1,
  failed_to_connect,
  failed_to_listen,
  connection_aborted,
  ipc_message_too_large,
  timed_out,
  unvalidated_client,
  vault_exited_with_error,
  vault_terminated
};

class vault_manager_error : public maidsafe_error {
 public:
  vault_manager_error(std::error_code ec, const std::string& what_arg)
      : maidsafe_error(ec, what_arg) {}
  vault_manager_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit vault_manager_error(std::error_code ec) : maidsafe_error(ec) {}
  vault_manager_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  vault_manager_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  vault_manager_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(VaultManagerErrors code);
std::error_condition make_error_condition(VaultManagerErrors code);
const std::error_category& GetVaultManagerCategory();
vault_manager_error MakeError(VaultManagerErrors code);

enum class ApiErrors {
  kPasswordFailure = 1
};

class api_error : public maidsafe_error {
 public:
  api_error(std::error_code ec, const std::string& what_arg) : maidsafe_error(ec, what_arg) {}
  api_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit api_error(std::error_code ec) : maidsafe_error(ec) {}
  api_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  api_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  api_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(ApiErrors code);
std::error_condition make_error_condition(ApiErrors code);
const std::error_category& GetApiCategory();
api_error MakeError(ApiErrors code);

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
struct is_error_code_enum<maidsafe::VaultManagerErrors> : public true_type {};

template <>
struct is_error_code_enum<maidsafe::ApiErrors> : public true_type {};

}  // namespace std

#endif  // MAIDSAFE_COMMON_ERROR_H_
