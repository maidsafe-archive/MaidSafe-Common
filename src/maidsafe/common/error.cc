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
    case error_code::kNoPrivateKey:
      return "Invalid or missing private key";
    case error_code::kNoPublicKey:
      return "Invalid or missing public key";
    case error_code::kDecryptFailed:
      return "Decrypt failed";
    case error_code::kEncryptFailed:
      return "Encrypt failed";
    case error_code::kNoBootstrapNodesFound:
      return "Cannot connect to any supplied bootstrap nodes";
    case error_code::kNodeNotFound:
      return "Cannot find node";
    case error_code::kListenFailed:
      return "Listen for incoming connections failed";
    case error_code::kAcceptFailed:
      return "Accept connection failed";
    case error_code::kNoTransports:
      return "No transports available";
    case error_code::kTransportsFull:
      return "Transports full";
    case error_code::kNoAvailablePorts:
      return "No available ports";
    case error_code::kAddressInUse:
      return "Address already in use";
    case error_code::kInvalidMessage:
      return "Invalid message";
    case error_code::kDriveNotMounted:
      return "Drive not mounted";
    case error_code::kCannotMountDrive:
      return "Cannot mount drive";
    case error_code::kNoFilesystemDriver:
      return "Filesystem driver not installed";
    case error_code::kStoreFailed:
      return "Store data failed";
    case error_code::kGetFailed:
      return "Get data failed";
    case error_code::kModifyFailed:
      return "Modify data failed";
    case error_code::kDeleteFailed:
      return "Delete data failed";
    default:
      return "Unknown error";
  }
}

std::error_condition ErrorCategoryImpl::default_error_condition(
    int error_value) const MAIDSAFE_NOEXCEPT {
  switch (static_cast<error_code>(error_value)) {
    case error_code::kNoBootstrapNodesFound:
    case error_code::kNodeNotFound:
      return error_condition::kRouting;
    case error_code::kListenFailed:
    case error_code::kAcceptFailed:
    case error_code::kNoTransports:
    case error_code::kTransportsFull:
    case error_code::kNoAvailablePorts:
    case error_code::kAddressInUse:
    case error_code::kInvalidMessage:
      return error_condition::kRudp;
    case error_code::kDriveNotMounted:
    case error_code::kCannotMountDrive:
    case error_code::kNoFilesystemDriver:
      return error_condition::kDrive;
    default:
      return error_condition::kUnknownError;
  }
}

const std::error_category& error_category() {
  static ErrorCategoryImpl instance;
  return instance;
}

}  // namespace maidsafe
