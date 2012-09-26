/*******************************************************************************
 *  Copyright 2012 maidsafe.net limited                                        *
 *                                                                             *
 *  The following source code is property of maidsafe.net limited and is not   *
 *  meant for external use.  The use of this code is governed by the licence   *
 *  file licence.txt found in the root of this directory and also on           *
 *  www.maidsafe.net.                                                          *
 *                                                                             *
 *  You are not free to copy, amend or otherwise use this source code without  *
 *  the explicit written permission of the board of directors of maidsafe.net. *
 ******************************************************************************/

#include "maidsafe/common/error.h"

namespace maidsafe {

std::error_code make_error_code(error_code code) {
  return std::error_code(static_cast<int>(code), error_category());
}

std::error_condition make_error_condition(error_condition condition) {
  return std::error_condition(static_cast<int>(condition), error_category());
}

const char* ErrorCategoryImpl::name() const MAIDSAFE_NOEXCEPT {
  return "maidsafe";
}

std::string ErrorCategoryImpl::message(int error_value) const {
  switch (static_cast<error_code>(error_value)) {
    case error_code::kBadStringLength:
      return "String must be length 64";
    case error_code::kInvalidNodeId:
      return "Invalid NodeID";
    default:
      return "Unknown error";
  }
}

std::error_condition ErrorCategoryImpl::default_error_condition(
    int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<error_code>(error_value)) {
    case error_code::kBadStringLength:
    case error_code::kInvalidNodeId:
      return error_condition::kCommon;
    default:
      return error_condition::kUnknownError;
  }
}

const std::error_category& error_category() {
  static ErrorCategoryImpl instance;
  return instance;
}

}  // namespace maidsafe
