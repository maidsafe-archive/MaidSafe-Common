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

#ifndef MAIDSAFE_COMMON_BOUNDED_STRING_H_
#define MAIDSAFE_COMMON_BOUNDED_STRING_H_

#include <algorithm>
#include <string>

#include "maidsafe/common/config.h"
#include "maidsafe/common/error.h"


namespace maidsafe {

namespace detail {

#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wc++11-narrowing"
#endif
template<size_t min, size_t max = -1>
class BoundedString {
 public:
  BoundedString()
      : string_(),
        // Use OutwithBounds() to invoke static_asserts
        valid_(!OutwithBounds()) {}

  explicit BoundedString(const std::string& string) : string_(string), valid_(false) {
    if (OutwithBounds())
      ThrowError(CommonErrors::invalid_string_size);
    valid_ = true;
  }

  friend void swap(BoundedString& first, BoundedString& second) MAIDSAFE_NOEXCEPT {
    std::swap(first.string_, second.string_);
    std::swap(first.valid_, second.valid_);
  }

  BoundedString(const BoundedString& other) : string_(other.string_), valid_(other.valid_) {}

  BoundedString(BoundedString&& other) MAIDSAFE_NOEXCEPT
      : string_(std::move(other.string_)),
        valid_(std::move(other.valid_)) {}

  BoundedString& operator=(BoundedString other) MAIDSAFE_NOEXCEPT {
    swap(*this, other);
    return *this;
  }

//  BoundedString& operator=(BoundedString&& other) {
//    string_ = std::move(other.string_);
//    return *this;
//  }

  template<size_t other_min, size_t other_max>
  explicit BoundedString(const BoundedString<other_min, other_max>& other)
      : string_(), valid_(false) {
    if (other.IsInitialised()) {
      string_ = other.string();
      if (OutwithBounds())
        ThrowError(CommonErrors::invalid_conversion);
      valid_ = true;
    }
  }

  template<size_t other_min, size_t other_max>
  BoundedString& operator=(BoundedString<other_min, other_max> other) {
    std::swap(string_, other.string_);
    std::swap(valid_, other.valid_);
    if (valid_) {
      if (OutwithBounds()) {
        // swap back to provide strong exception guarantee
        std::swap(string_, other.string_);
        std::swap(valid_, other.valid_);
          ThrowError(CommonErrors::invalid_conversion);
      }
    }
    return *this;
  }

  const std::string& string() const {
    if (!valid_)
      ThrowError(CommonErrors::invalid_string_size);
    return string_;
  }

  bool IsInitialised() const { return valid_; }

  template<size_t other_min, size_t other_max> friend class BoundedString;

 private:
  bool OutwithBounds() {
    static_assert(min, "Lower bound of BoundedString must be non-zero");
    static_assert(min <= max,
                  "Lower bound of BoundedString must be less than or equal to upper bound");
    return ((string_.size() < min) || string_.size() > max);
  }
  std::string string_;
  bool valid_;
};
#ifdef __clang__
#  pragma clang diagnostic pop
#endif

template<size_t min, size_t max>
inline bool operator==(const BoundedString<min, max>& lhs, const BoundedString<min, max>& rhs) {
  return lhs.string() == rhs.string();
}

template<size_t min, size_t max>
inline bool operator!=(const BoundedString<min, max>& lhs, const BoundedString<min, max>& rhs) {
  return !operator==(lhs, rhs);
}

template<size_t min, size_t max>
inline bool operator<(const BoundedString<min, max>& lhs, const BoundedString<min, max>& rhs) {
  return lhs.string() < rhs.string();
}

template<size_t min, size_t max>
inline bool operator>(const BoundedString<min, max>& lhs, const BoundedString<min, max>& rhs) {
  return operator<(rhs, lhs);
}

template<size_t min, size_t max>
inline bool operator<=(const BoundedString<min, max>& lhs, const BoundedString<min, max>& rhs) {
  return !operator>(lhs, rhs);
}

template<size_t min, size_t max>
inline bool operator>=(const BoundedString<min, max>& lhs, const BoundedString<min, max>& rhs) {
  return !operator<(lhs, rhs);
}

}  // namespace detail

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_BOUNDED_STRING_H_
