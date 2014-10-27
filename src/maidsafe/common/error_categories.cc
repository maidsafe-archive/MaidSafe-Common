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

#include "maidsafe/common/error_categories.h"

#include "maidsafe/common/error.h"

namespace maidsafe {

namespace detail {

const char* CommonCategory::name() const MAIDSAFE_NOEXCEPT { return "MaidSafe Common"; }

std::string CommonCategory::message(int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<CommonErrors>(error_value)) {
    case CommonErrors::success:
      return "Success";
    case CommonErrors::pending_result:
      return "Result still pending";
    case CommonErrors::null_pointer:
      return "nullptr passed";
    case CommonErrors::invalid_node_id:
      return "Invalid NodeID";
    case CommonErrors::invalid_key_size:
      return "Invalid key size";
    case CommonErrors::invalid_string_size:
      return "Invalid string size";
    case CommonErrors::invalid_parameter:
      return "One or more invalid parameters were passed";
    case CommonErrors::invalid_conversion:
      return "Invalid conversion between types";
    case CommonErrors::file_too_large:
      return "File too large";
    case CommonErrors::uninitialised:
      return "Class is uninitialised.";
    case CommonErrors::already_initialised:
      return "Class has already been initialised.";
    case CommonErrors::hashing_error:
      return "Error during hashing";
    case CommonErrors::symmetric_encryption_error:
      return "Error during symmetric encryption";
    case CommonErrors::symmetric_decryption_error:
      return "Error during symmetric decryption";
    case CommonErrors::compression_error:
      return "Error during compression";
    case CommonErrors::uncompression_error:
      return "Error during uncompression";
    case CommonErrors::cannot_invoke_from_this_thread:
      return "This function cannot be invoked from this thread";
    case CommonErrors::cannot_exceed_limit:
      return "Cannot exceed specified limit";
    case CommonErrors::unable_to_handle_request:
      return "Unable to handle request";
    case CommonErrors::filesystem_io_error:
      return "Filesystem IO error";
    case CommonErrors::no_such_element:
      return "Element does not exist";
    case CommonErrors::serialisation_error:
      return "Error serialising to protocol buffer";
    case CommonErrors::parsing_error:
      return "Error parsing from protocol buffer";
    case CommonErrors::not_a_directory:
      return "Path is not a directory";
    case CommonErrors::db_busy:
      return "The database file is locked";
    case CommonErrors::db_not_presented:
      return "database not presented for SQL";
    case CommonErrors::db_error:
      return "SQL error";
    case CommonErrors::unknown:
    default:
      return "Unknown error in Common";
  }
}

std::error_condition CommonCategory::default_error_condition(int error_value) const
    MAIDSAFE_NOEXCEPT {
  switch (static_cast<CommonErrors>(error_value)) {
    case CommonErrors::null_pointer:
    case CommonErrors::invalid_node_id:
    case CommonErrors::invalid_key_size:
    case CommonErrors::invalid_string_size:
    case CommonErrors::invalid_parameter:
      return std::errc::invalid_argument;
    case CommonErrors::file_too_large:
      return std::errc::file_too_large;
    case CommonErrors::cannot_invoke_from_this_thread:
      return std::errc::operation_not_permitted;
    case CommonErrors::unable_to_handle_request:
      return std::errc::not_supported;
    case CommonErrors::filesystem_io_error:
      return std::errc::io_error;
    case CommonErrors::not_a_directory:
      return std::errc::not_a_directory;
    default:
      return std::error_condition(error_value, *this);
  }
}

const char* AsymmCategory::name() const MAIDSAFE_NOEXCEPT { return "MaidSafe Asymmetric Crypto"; }

std::string AsymmCategory::message(int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<AsymmErrors>(error_value)) {
    case AsymmErrors::keys_generation_error:
      return "Error generating key pair";
    case AsymmErrors::keys_serialisation_error:
      return "Error serialising key pair";
    case AsymmErrors::keys_parse_error:
      return "Error parsing key pair";
    case AsymmErrors::invalid_private_key:
      return "Invalid private key";
    case AsymmErrors::invalid_public_key:
      return "Invalid public key";
    case AsymmErrors::data_empty:
      return "Input data is empty";
    case AsymmErrors::invalid_file:
      return "Input file is missing or empty";
    case AsymmErrors::invalid_signature:
      return "Invalid signature";
    case AsymmErrors::signature_empty:
      return "Signature is empty";
    case AsymmErrors::encryption_error:
      return "Error during asymmetric encryption";
    case AsymmErrors::decryption_error:
      return "Error during asymmetric decryption";
    case AsymmErrors::signing_error:
      return "Error during asymmetric signing";
    default:
      return "Unknown error in Asymm";
  }
}

std::error_condition AsymmCategory::default_error_condition(int error_value) const
    MAIDSAFE_NOEXCEPT {
  switch (static_cast<AsymmErrors>(error_value)) {
    case AsymmErrors::data_empty:
    case AsymmErrors::invalid_file:
    case AsymmErrors::invalid_signature:
    case AsymmErrors::signature_empty:
    case AsymmErrors::signing_error:
      return std::errc::invalid_argument;
    default:
      return std::error_condition(error_value, *this);
  }
}

const char* PassportCategory::name() const MAIDSAFE_NOEXCEPT { return "MaidSafe Identity Ring"; }

std::string PassportCategory::message(int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<PassportErrors>(error_value)) {
    case PassportErrors::id_already_exists:
      return "ID already exists in the passport";
    default:
      return "Unknown error in Passport";
  }
}

std::error_condition PassportCategory::default_error_condition(int error_value) const
    MAIDSAFE_NOEXCEPT {
  //  switch (static_cast<PassportErrors>(error_value)) {
  //    default:
  return std::error_condition(error_value, *this);
  //  }
}

const char* EncryptCategory::name() const MAIDSAFE_NOEXCEPT { return "MaidSafe Encryption"; }

std::string EncryptCategory::message(int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<EncryptErrors>(error_value)) {
    case EncryptErrors::bad_sequence:
      return "Bad sequence";
    case EncryptErrors::no_data:
      return "No data";
    case EncryptErrors::invalid_encryption_version:
      return "Invalid version of encryption algorithm";
    case EncryptErrors::failed_to_write:
      return "Failed to write";
    case EncryptErrors::failed_to_prepare_for_write:
      return "Failed to prepare for write";
    case EncryptErrors::failed_to_get_chunk:
      return "Failed to get chunk";
    case EncryptErrors::failed_to_flush:
      return "Failed to flush";
    case EncryptErrors::failed_to_decrypt:
      return "Failed to decrypt";
    case EncryptErrors::failed_to_read:
      return "Failed to read";
    case EncryptErrors::encryptor_closed:
      return "Encryptor closed";
    default:
      return "Unknown error in Encrypt";
  }
}

std::error_condition EncryptCategory::default_error_condition(int error_value) const
    MAIDSAFE_NOEXCEPT {
  //  switch (static_cast<EncryptErrors>(error_value)) {
  //    default:
  return std::error_condition(error_value, *this);
  //  }
}

const char* RoutingCategory::name() const MAIDSAFE_NOEXCEPT { return "MaidSafe Routing"; }

std::string RoutingCategory::message(int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<RoutingErrors>(error_value)) {
    case RoutingErrors::timed_out:
      return "Timed out";
    case RoutingErrors::timer_cancelled:
      return "Timer cancelled";
    case RoutingErrors::not_in_range:
      return "Not in range";
    case RoutingErrors::not_connected:
      return "Not connected";
    default:
      return "Unknown error in Routing";
  }
}

std::error_condition RoutingCategory::default_error_condition(int error_value) const
    MAIDSAFE_NOEXCEPT {
  switch (static_cast<RoutingErrors>(error_value)) {
    case RoutingErrors::timed_out:
      return std::errc::timed_out;
    default:
      return std::error_condition(error_value, *this);
  }
}

const char* NfsCategory::name() const MAIDSAFE_NOEXCEPT { return "MaidSafe Network Filesystem"; }

std::string NfsCategory::message(int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<NfsErrors>(error_value)) {
    case NfsErrors::failed_to_get_data:
      return "Routing failed to return requested data";
    case NfsErrors::timed_out:
      return "Timed out";
    default:
      return "Unknown error in NFS";
  }
}

std::error_condition NfsCategory::default_error_condition(int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<NfsErrors>(error_value)) {
    case NfsErrors::timed_out:
      return std::errc::timed_out;
    default:
      return std::error_condition(error_value, *this);
  }
}

const char* DriveCategory::name() const MAIDSAFE_NOEXCEPT { return "MaidSafe Drive"; }

std::string DriveCategory::message(int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<DriveErrors>(error_value)) {
    case DriveErrors::no_drive_letter_available:
      return "There are no available drive letters left";
    case DriveErrors::failed_to_mount:
      return "Failed to mount the drive";
    case DriveErrors::permission_denied:
      return "Permission denied for given action";
    case DriveErrors::no_such_file:
      return "No such file";
    case DriveErrors::file_exists:
      return "File already exists";
    case DriveErrors::driver_not_installed:
      return "CbFs driver is not installed";
    default:
      return "Unknown error in Drive";
  }
}

std::error_condition DriveCategory::default_error_condition(int error_value) const
    MAIDSAFE_NOEXCEPT {
  switch (static_cast<DriveErrors>(error_value)) {
    case DriveErrors::permission_denied:
      return std::errc::permission_denied;
    case DriveErrors::file_exists:
      return std::errc::file_exists;
    default:
      return std::error_condition(error_value, *this);
  }
}

const char* VaultCategory::name() const MAIDSAFE_NOEXCEPT { return "MaidSafe Vault"; }

std::string VaultCategory::message(int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<VaultErrors>(error_value)) {
    case VaultErrors::failed_to_join_network:
      return "Failed to join network";
    case VaultErrors::failed_to_handle_request:
      return "Failed to handle request";
    case VaultErrors::operation_not_supported:
      return "Requested operation not supported";
    case VaultErrors::permission_denied:
      return "Permission denied for request";
    case VaultErrors::no_such_account:
      return "Account not known on this vault";
    case VaultErrors::low_space:
      return "Running low on space in account";
    case VaultErrors::not_enough_space:
      return "Not enough space in account";
    case VaultErrors::unique_data_clash:
      return "Attempt to overwrite existing unique data";
    case VaultErrors::data_available_not_given:
      return "Data is held by the network, but was not provided";
    case VaultErrors::account_already_exists:
      return "Attempt to create an account which already exists";
    case VaultErrors::data_already_exists:
      return "Attempt to put data which already exists";
    default:
      return "Unknown error in Vault";
  }
}

std::error_condition VaultCategory::default_error_condition(int error_value) const
    MAIDSAFE_NOEXCEPT {
  switch (static_cast<VaultErrors>(error_value)) {
    case VaultErrors::operation_not_supported:
      return std::errc::operation_not_supported;
    case VaultErrors::permission_denied:
      return std::errc::permission_denied;
    default:
      return std::error_condition(error_value, *this);
  }
}

const char* VaultManagerCategory::name() const MAIDSAFE_NOEXCEPT { return "MaidSafe VaultManager"; }

std::string VaultManagerCategory::message(int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<VaultManagerErrors>(error_value)) {
    case VaultManagerErrors::connection_not_found:
      return "IPC connection not found";
    case VaultManagerErrors::failed_to_connect:
      return "Failed to connect";
    case VaultManagerErrors::failed_to_listen:
      return "Failed to listen";
    case VaultManagerErrors::connection_aborted:
      return "Connection aborted";
    case VaultManagerErrors::ipc_message_too_large:
      return "IPC message too large";
    case VaultManagerErrors::timed_out:
      return "Timed out";
    case VaultManagerErrors::unvalidated_client:
      return "IPC message from unvalidated client refused";
    case VaultManagerErrors::vault_exited_with_error:
      return "Vault process exited with an error";
    case VaultManagerErrors::vault_terminated:
      return "Vault process required termination";
    default:
      return "Unknown error in VaultManager";
  }
}

std::error_condition VaultManagerCategory::default_error_condition(int error_value) const
    MAIDSAFE_NOEXCEPT {
  switch (static_cast<VaultManagerErrors>(error_value)) {
    case VaultManagerErrors::connection_not_found:
      return std::errc::not_connected;
    case VaultManagerErrors::failed_to_connect:
      return std::errc::connection_refused;
    case VaultManagerErrors::connection_aborted:
      return std::errc::connection_aborted;
    case VaultManagerErrors::ipc_message_too_large:
      return std::errc::message_size;
    case VaultManagerErrors::timed_out:
      return std::errc::timed_out;
    default:
      return std::error_condition(error_value, *this);
  }
}

const char* ApiCategory::name() const MAIDSAFE_NOEXCEPT { return "Client"; }

std::string ApiCategory::message(int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<ApiErrors>(error_value)) {
    case ApiErrors::kPasswordFailure:
      return "Failed to validate password";
    default:
      return "Unknown error in Client";
  }
}

std::error_condition ApiCategory::default_error_condition(int error_value) const MAIDSAFE_NOEXCEPT {
  //  switch (static_cast<ApiErrors>(error_value)) {
  //    default:
  return std::error_condition(error_value, *this);
  //  }
}

}  // namespace detail

}  // namespace maidsafe
