/*******************************************************************************
 *  Copyright 2011 maidsafe.net limited                                        *
 *                                                                             *
 *  The following source code is property of maidsafe.net limited and is not   *
 *  meant for external use.  The use of this code is governed by the licence   *
 *  file licence.txt found in the root of this directory and also on           *
 *  www.maidsafe.net.                                                          *
 *                                                                             *
 *  You are not free to copy, amend or otherwise use this source code without  *
 *  the explicit written permission of the board of directors of maidsafe.net. *
 *******************************************************************************
 */

#ifndef MAIDSAFE_COMMON_RETURN_CODES_H_
#define MAIDSAFE_COMMON_RETURN_CODES_H_

#include "maidsafe/common/version.h"

#if MAIDSAFE_COMMON_VERSION != 1004
# error This API is not compatible with the installed library.\
Please update the library.
#endif

namespace maidsafe {

enum CommonReturnCode {
  kSuccess = 0,
  kGeneralError = -100001,

  // RSA error codes
  kKeyGenerationError = -100100,
  kDataEmpty = -100101,
  kInvalidPublicKey = -100110,
  kInvalidPrivateKey = -100111,
  kRSAEncryptError = -101000,
  kRSADecryptError = -101001,
  kRSASigningError = -101010,
  kRSAInvalidSignature = -101011,
  kRSASignatureEmpty = -101100
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_RETURN_CODES_H_
