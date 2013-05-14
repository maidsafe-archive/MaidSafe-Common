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
template<size_t min, size_t max = -1, typename StringType = std::string>
class BoundedString {
 public:
  BoundedString()
      : string_(),
        // Use OutwithBounds() to invoke static_asserts
        valid_(!OutwithBounds()) {}

  explicit BoundedString(StringType string) : string_(std::move(string)), valid_(true) {
    if (OutwithBounds())
      ThrowError(CommonErrors::invalid_string_size);
  }

  friend void swap(BoundedString& first, BoundedString& second) MAIDSAFE_NOEXCEPT {
    using std::swap;
    swap(first.string_, second.string_);
    swap(first.valid_, second.valid_);
  }

  BoundedString(const BoundedString& other) : string_(other.string_), valid_(other.valid_) {}

  BoundedString(BoundedString&& other) MAIDSAFE_NOEXCEPT
      : string_(std::move(other.string_)),
        valid_(std::move(other.valid_)) {}

  BoundedString& operator=(BoundedString other) MAIDSAFE_NOEXCEPT {
    // No need for check for self-assignment since we're using the copy-and-swap idiom
    swap(*this, other);
    return *this;
  }

  BoundedString& operator+=(const BoundedString& other) {
    if (!valid_ || !other.valid_)
      ThrowError(CommonErrors::uninitialised);
    if (SizeOutOfBounds(string_.size()+other.string_.size()))
      ThrowError(CommonErrors::invalid_string_size);
    StringType temp(string_+other.string_);
    string_.swap(temp);
    return *this;
  }

  template<size_t other_min, size_t other_max, typename OtherStringType>
  explicit BoundedString(BoundedString<other_min, other_max, OtherStringType> other)
    : string_(other.string_.begin(), other.string_.end()), valid_(std::move(other.valid_)) {
    static_assert((min <= other_min) && (max >= other_max),
                  "Bounds of copied BoundedString must be within bounds of this BoundedString");
  }

  template<size_t other_min, size_t other_max, typename OtherStringType>
  BoundedString& operator=(BoundedString<other_min, other_max, OtherStringType> other) {
    static_assert((min <= other_min) && (max >= other_max),
                  "Bounds of copied BoundedString must be within bounds of this BoundedString");
    // No need for check for self-assignment since these are different types
    swap(string_, other.string_);
    std::swap(valid_, other.valid_);
    return *this;
  }

  template<size_t other_min, size_t other_max, typename OtherStringType>
  BoundedString& operator+=(const BoundedString<other_min, other_max, OtherStringType>& other) {
    if (!valid_ || !other.valid_)
      ThrowError(CommonErrors::uninitialised);
    if (SizeOutOfBounds(string_.size()+other.string_.size()))
      ThrowError(CommonErrors::invalid_string_size);
    StringType temp(string_+other.string_);
    string_.swap(temp);
    return *this;
  }

  const StringType& string() const {
    if (!valid_)
      ThrowError(CommonErrors::uninitialised);
    return string_;
  }

  bool IsInitialised() const { return valid_; }

  template<size_t other_min, size_t other_max, typename OtherStringType> friend class BoundedString;

 private:
  bool SizeOutOfBounds(std::string::size_type size) const {
    static_assert(min, "Lower bound of BoundedString must be non-zero");
    static_assert(min <= max,
                  "Lower bound of BoundedString must be less than or equal to upper bound");
    return (size < min) || (size > max);
  }

  bool OutwithBounds() const {
    return SizeOutOfBounds(string_.size());
  }
  StringType string_;
  bool valid_;
};
#ifdef __clang__
#  pragma clang diagnostic pop
#endif

template<size_t min, size_t max, typename StringType>
inline bool operator==(const BoundedString<min, max, StringType>& lhs,
                       const BoundedString<min, max, StringType>& rhs) {
  if (lhs.IsInitialised()) {
    if (rhs.IsInitialised()) {
      return lhs.string() == rhs.string();
    } else {
      return false;
    }
  } else {
    return !rhs.IsInitialised();
  }
}

template<size_t min, size_t max, typename StringType>
inline bool operator!=(const BoundedString<min, max, StringType>& lhs,
                       const BoundedString<min, max, StringType>& rhs) {
  return !operator==(lhs, rhs);
}

template<size_t min, size_t max, typename StringType>
inline bool operator<(const BoundedString<min, max, StringType>& lhs,
                      const BoundedString<min, max, StringType>& rhs) {
  if (lhs.IsInitialised()) {
    if (rhs.IsInitialised()) {
      return lhs.string() < rhs.string();
    } else {
      return false;
    }
  } else {
    return rhs.IsInitialised();
  }
}

template<size_t min, size_t max, typename StringType>
inline bool operator>(const BoundedString<min, max, StringType>& lhs,
                      const BoundedString<min, max, StringType>& rhs) {
  return operator<(rhs, lhs);
}

template<size_t min, size_t max, typename StringType>
inline bool operator<=(const BoundedString<min, max, StringType>& lhs,
                       const BoundedString<min, max, StringType>& rhs) {
  return !operator>(lhs, rhs);
}

template<size_t min, size_t max, typename StringType>
inline bool operator>=(const BoundedString<min, max, StringType>& lhs,
                       const BoundedString<min, max, StringType>& rhs) {
  return !operator<(lhs, rhs);
}

template<size_t lhs_min, size_t lhs_max, size_t rhs_min, size_t rhs_max, typename StringType>
inline BoundedString<lhs_min, lhs_max> operator+(BoundedString<lhs_min, lhs_max, StringType> lhs,
                                          const BoundedString<rhs_min, rhs_max, StringType>& rhs) {
  lhs += rhs;
  return lhs;
}

}  // namespace detail

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_BOUNDED_STRING_H_
