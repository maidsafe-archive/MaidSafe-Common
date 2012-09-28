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

namespace maidsafe {

namespace detail {

class CommonCategory : public std::error_category {
 public:
  virtual const char* name() const MAIDSAFE_NOEXCEPT { return "Common"; }

  virtual std::string message(int error_value) const MAIDSAFE_NOEXCEPT {
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
      case CommonErrors::unknown:
      default:
        return "Unknown error in Common";
    }
  }
};

class AsymmCategory : public std::error_category {
 public:
  virtual const char* name() const MAIDSAFE_NOEXCEPT { return "Asymmetric Crypto"; }
  virtual std::string message(int error_value) const MAIDSAFE_NOEXCEPT {
    switch (static_cast<AsymmErrors>(error_value)) {
      case AsymmErrors::keys_generation_error:
        return "Error generating key pair";
      case AsymmErrors::keys_serialisation_error:
        return "Error serialising key pair";
      case AsymmErrors::keys_parse_error:
        return "Error parsing key pair";
      case AsymmErrors::invalid_private_key:
        return "Invalid private key";
      case AsymmErrors::data_empty:
        return "Input data is empty";
      case AsymmErrors::file_empty:
        return "Input file is empty";
      case AsymmErrors::invalid_signature:
        return "Invalid signature";
      case AsymmErrors::signature_empty:
        return "Signature is empty";
      case AsymmErrors::rsa_encryption_error:
        return "Error during RSA encryption";
      case AsymmErrors::rsa_decryption_error:
        return "Error during RSA decryption";
      case AsymmErrors::rsa_signing_error:
        return "Error during RSA signing";
      default:
        return "Unknown error in Asymm";
    }
  }
};

class LifeStuffCategory : public std::error_category {
 public:
  virtual const char* name() const MAIDSAFE_NOEXCEPT { return "LifeStuff"; }
  virtual std::string message(int error_value) const MAIDSAFE_NOEXCEPT {
    switch (static_cast<LifeStuffErrors>(error_value)) {
      case LifeStuffErrors::kAuthenticationError:
      default:
        return "Unknown error in LifeStuff";
    }
  }
};

class DefaultCategory : public std::error_category {
 public:
  virtual const char* name() const MAIDSAFE_NOEXCEPT { return "Default"; }
  virtual std::string message(int error_value) const MAIDSAFE_NOEXCEPT {
    switch (static_cast<ErrorConditions>(error_value)) {
      case ErrorConditions::filesystem_error:
      default:
        return "Unknown error";
    }
  }
};

}  // namespace detail


std::error_code make_error_code(CommonErrors code) {
  return std::error_code(static_cast<int>(code), GetCommonCategory());
}

std::error_code make_error_code(AsymmErrors code) {
  return std::error_code(static_cast<int>(code), GetAsymmCategory());
}

std::error_code make_error_code(LifeStuffErrors code) {
  return std::error_code(static_cast<int>(code), GetLifeStuffCategory());
}

std::error_condition make_error_condition(ErrorConditions condition) {
  return std::error_condition(static_cast<int>(condition), GetDefaultCategory());
}

const std::error_category& GetCommonCategory() {
  static detail::CommonCategory instance;
  return instance;
}

const std::error_category& GetAsymmCategory() {
  static detail::AsymmCategory instance;
  return instance;
}

const std::error_category& GetLifeStuffCategory() {
  static detail::LifeStuffCategory instance;
  return instance;
}

const std::error_category& GetDefaultCategory() {
  static detail::DefaultCategory instance;
  return instance;
}

}  // namespace maidsafe
