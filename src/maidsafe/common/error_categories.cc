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
    case CommonErrors::cant_invoke_from_this_thread:
      return "This function cannot be invoked from this thread";
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
    case CommonErrors::cant_invoke_from_this_thread:
      return std::errc::operation_not_permitted;
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


const char* FobCategory::name() const MAIDSAFE_NOEXCEPT {
  return "Maidsafe Identity Ring";
}

std::string FobCategory::message(int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<FobErrors>(error_value)) {
    case FobErrors::fob_serialisation_error:
      return "Error serialising to protocol buffer representation";
    case FobErrors::fob_parsing_error:
      return "Error parsing from protocol buffer representation";
    default:
      return "Unknown error in Fob";
  }
}

std::error_condition FobCategory::default_error_condition(
    int error_value) const MAIDSAFE_NOEXCEPT {
//  switch (static_cast<FobErrors>(error_value)) {
//    default:
      return std::error_condition(error_value, *this);
//  }
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
