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

#ifndef MAIDSAFE_COMMON_ERROR_H_
#define MAIDSAFE_COMMON_ERROR_H_

#ifdef MAIDSAFE_WIN32
#  define MAIDSAFE_NOEXCEPT
#else
#  define MAIDSAFE_NOEXCEPT noexcept(true)
#endif

#include <system_error>
#include <string>

namespace maidsafe {

enum class error_code {
  kBadStringLength = 1,  // 0 would be no error
  kNoPrivateKey,
  kNoPublicKey,
  kDecryptFailed,
  kEncryptFailed,
  // network errors
  kNoBootstrapNodesFound,
  kNodeNotFound,
  kListenFailed,
  kAcceptFailed,
  kNoTransports,
  kTransportsFull,
  kNoAvailablePorts,
  kAddressInUse,
  kInvalidMessage,
  // drive errors
  kDriveNotMounted,
  kCannotMountDrive,
  kNoFilesystemDriver,
  // storage errors
  kStoreFailed,
  kGetFailed,
  kModifyFailed,
  kDeleteFailed
};

enum class error_condition {
  kCommon,
  kRudp,
  kRouting,
  kDrive,
  kUnknownError
};

std::error_code make_error_code(error_code code);
std::error_condition make_error_condition(error_condition condition);

class ErrorCategoryImpl : public std::error_category {
 public:
  virtual std::string message(int error_value) const;
  virtual const char* name() const MAIDSAFE_NOEXCEPT;
  virtual std::error_condition default_error_condition(int error_value) const MAIDSAFE_NOEXCEPT;
  // virtual bool equivalent(const std::error_code& code, int condition) const noexcept(true);
};

const std::error_category& error_category();

}  // namespace maidsafe


namespace std {

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Weffc++"
#endif
template <>
struct is_error_code_enum<maidsafe::error_code> : public true_type {};  //NOLINT (dirvine)

template <>
struct is_error_condition_enum<maidsafe::error_condition> : public true_type {};  //NOLINT (dirvine)
#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif

}  // namespace std

#endif  // MAIDSAFE_COMMON_ERROR_H_
