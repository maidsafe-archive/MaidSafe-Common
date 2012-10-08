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

#include <boost/throw_exception.hpp>

#include "maidsafe/common/error_categories.h"


namespace maidsafe {

std::error_code make_error_code(CommonErrors code) {
  return std::error_code(static_cast<int>(code), GetCommonCategory());
}

std::error_code make_error_code(AsymmErrors code) {
  return std::error_code(static_cast<int>(code), GetAsymmCategory());
}

std::error_code make_error_code(MaidsafeIdentityRingErrors code) {
  return std::error_code(static_cast<int>(code), GetMaidsafeIdentityRingCategory());
}

std::error_code make_error_code(LifeStuffErrors code) {
  return std::error_code(static_cast<int>(code), GetLifeStuffCategory());
}

std::error_condition make_error_condition(CommonErrors code) {
  return std::error_condition(static_cast<int>(code), GetCommonCategory());
}

std::error_condition make_error_condition(AsymmErrors code) {
  return std::error_condition(static_cast<int>(code), GetAsymmCategory());
}

std::error_condition make_error_condition(MaidsafeIdentityRingErrors code) {
  return std::error_condition(static_cast<int>(code), GetMaidsafeIdentityRingCategory());
}

std::error_condition make_error_condition(LifeStuffErrors code) {
  return std::error_condition(static_cast<int>(code), GetLifeStuffCategory());
}


void DoThrowError(const std::error_code& code) {
  std::system_error c(code);
  boost::throw_exception(c);
}

const std::error_category& GetCommonCategory() {
  static detail::CommonCategory instance;
  return instance;
}

const std::error_category& GetAsymmCategory() {
  static detail::AsymmCategory instance;
  return instance;
}

const std::error_category& GetMaidsafeIdentityRingCategory() {
  static detail::MaidsafeIdentityRingCategory instance;
  return instance;
}

const std::error_category& GetLifeStuffCategory() {
  static detail::LifeStuffCategory instance;
  return instance;
}

}  // namespace maidsafe
