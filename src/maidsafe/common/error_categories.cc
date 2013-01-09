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

#include "maidsafe/common/error_categories.h"

#include "maidsafe/common/error.h"


namespace maidsafe {

namespace detail {

const char* CommonCategory::name() const MAIDSAFE_NOEXCEPT {
  return "Common";
}

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
      return "Invalid conversion between BoundedString types";
    case CommonErrors::file_too_large:
      return "File too large";
    case CommonErrors::uninitialised:
      return "Class is uninitialised.";
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
    case CommonErrors::cannot_exceed_max_disk_usage:
      return "Cannot exceed specified maximum disk usage";
    case CommonErrors::filesystem_io_error:
      return "Filesystem IO error";
    case CommonErrors::no_such_element:
      return "Element does not exist";
    case CommonErrors::unknown:
    default:
      return "Unknown error in Common";
  }
}

std::error_condition CommonCategory::default_error_condition(
    int error_value) const MAIDSAFE_NOEXCEPT {
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
    case CommonErrors::cannot_exceed_max_disk_usage:
      return std::errc::no_buffer_space;
    case CommonErrors::filesystem_io_error:
      return std::errc::io_error;
    default:
      return std::error_condition(error_value, *this);
  }
}


const char* AsymmCategory::name() const MAIDSAFE_NOEXCEPT {
  return "Asymmetric Crypto";
}

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

std::error_condition AsymmCategory::default_error_condition(
    int error_value) const MAIDSAFE_NOEXCEPT {
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


const char* PassportCategory::name() const MAIDSAFE_NOEXCEPT {
  return "Maidsafe Identity Ring";
}

std::string PassportCategory::message(int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<PassportErrors>(error_value)) {
    case PassportErrors::fob_serialisation_error:
      return "Error serialising to protocol buffer representation";
    case PassportErrors::fob_parsing_error:
      return "Error parsing fob from protocol buffer representation";
    case PassportErrors::mid_parsing_error:
      return "Error parsing MID or SMID from protocol buffer representation";
    case PassportErrors::tmid_parsing_error:
      return "Error parsing TMID from protocol buffer representation";
    case PassportErrors::no_confirmed_fob:
      return "No confirmed fob of requested type available";
    case PassportErrors::no_pending_fob:
      return "No pending fob of requested type available";
    case PassportErrors::passport_parsing_error:
      return "Error parsing passport";
    case PassportErrors::public_id_already_exists:
      return "Public ID already exists in the passport";
    case PassportErrors::no_such_public_id:
      return "No such public ID exists in the passport";
    default:
      return "Unknown error in Passport";
  }
}

std::error_condition PassportCategory::default_error_condition(
    int error_value) const MAIDSAFE_NOEXCEPT {
//  switch (static_cast<PassportErrors>(error_value)) {
//    default:
      return std::error_condition(error_value, *this);
//  }
}


const char* NfsCategory::name() const MAIDSAFE_NOEXCEPT {
  return "Maidsafe Network Filesystem";
}

std::string NfsCategory::message(int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<NfsErrors>(error_value)) {
    case NfsErrors::invalid_parameter:
      return "One or more invalid parameters were passed when constructing an NFS message";
    case NfsErrors::message_parsing_error:
      return "Error parsing NFS message from protocol buffer representation";
    case NfsErrors::maid_account_parsing_error:
      return "Error parsing MAID account from protocol buffer representation";
    case NfsErrors::pmid_registration_parsing_error:
      return "Error parsing PMID registration from protocol buffer representation";
    case NfsErrors::pmid_size_parsing_error:
      return "Error parsing PMID size details from protocol buffer representation";
    case NfsErrors::return_code_parsing_error:
      return "Error parsing NFS return code from protocol buffer representation";
    case NfsErrors::failed_to_get_data:
      return "Routing failed to return requested data";
    case NfsErrors::invalid_signature:
      return "Routing failed to return requested data";
    default:
      return "Unknown error in NFS";
  }
}

std::error_condition NfsCategory::default_error_condition(
    int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<NfsErrors>(error_value)) {
    case NfsErrors::invalid_parameter:
      return std::errc::invalid_argument;
    default:
      return std::error_condition(error_value, *this);
  }
}


const char* LifeStuffCategory::name() const MAIDSAFE_NOEXCEPT {
  return "LifeStuff";
}

std::string LifeStuffCategory::message(int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<LifeStuffErrors>(error_value)) {
    case LifeStuffErrors::kAuthenticationError:
    default:
      return "Unknown error in LifeStuff";
  }
}

std::error_condition LifeStuffCategory::default_error_condition(
    int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<LifeStuffErrors>(error_value)) {
    case LifeStuffErrors::kAuthenticationError:
    default:
      return std::error_condition(error_value, *this);
  }
}


}  // namespace detail

}  // namespace maidsafe
