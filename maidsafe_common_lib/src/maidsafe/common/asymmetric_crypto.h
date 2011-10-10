/* Copyright (c) 2009 maidsafe.net limited
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

#ifndef MAIDSAFE_COMMON_ASYMM_H_
#define MAIDSAFE_COMMON_ASYMM_H_

#include <cstdint>
#include <string>
#include <algorithm>

#ifdef __MSVC__
#  pragma warning(push, 1)
#  pragma warning(disable: 4702)
#endif

#include "cryptopp/pubkey.h"

#ifdef __MSVC__
#  pragma warning(pop)
#  pragma warning(disable: 4505)
#endif

#include "maidsafe/common/version.h"

#if MAIDSAFE_COMMON_VERSION != 1003
#  error This API is not compatible with the installed library.\
    Please update the MaidSafe-Common library.
#endif


namespace maidsafe {

  typedef std::string Signature, PlainText, CipherText;
  
  
template<class Keys>
class AsymmetricCrypto {

 public:
  AsymmetricCrypto()  {}
  ~AsymmetricCrypto() {}
  typedef typename Keys::PrivateKey PrivateKey;
  typedef typename Keys::PublicKey PublicKey;
  
  int GenerateKeyPair(Keys *keypair) const;
  
  int Sign(const PlainText &data,
           const PrivateKey &priv_key,
           Signature *signature) const;
           
  int CheckSignature(const PlainText &plain_text,
                     const Signature &signature,
                     const PublicKey &pub_key) const;
                     
  int Encrypt(const PlainText &plain_text,
              const PublicKey &pub_key,
               CipherText *result ) const;
              
  int Decrypt(const CipherText &data,
              const PrivateKey &priv_key,
              PlainText *result
              ) const;

 private:
  AsymmetricCrypto &operator=(const AsymmetricCrypto&);
  AsymmetricCrypto(const AsymmetricCrypto&);
 
};






}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_ASYMM_H_
