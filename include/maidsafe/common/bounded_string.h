/*  Copyright 2012 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */

#ifndef MAIDSAFE_COMMON_BOUNDED_STRING_H_
#define MAIDSAFE_COMMON_BOUNDED_STRING_H_

#include <algorithm>
#include <string>

#include "maidsafe/common/config.h"
#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"

namespace maidsafe {

namespace detail {

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++11-narrowing"
#endif
template <size_t min, size_t max = -1, typename StringType = std::string>
class BoundedString {
 public:
  BoundedString()
      : string_(),
        // Use OutwithBounds() to invoke static_asserts
        valid_(!OutwithBounds()) {}

  explicit BoundedString(StringType string) : string_(std::move(string)), valid_(true) {
    if (OutwithBounds())
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_string_size));
  }

  friend void swap(BoundedString& first, BoundedString& second) MAIDSAFE_NOEXCEPT {
    using std::swap;
    swap(first.string_, second.string_);
    swap(first.valid_, second.valid_);
  }

  BoundedString(const BoundedString& other) : string_(other.string_), valid_(other.valid_) {}

  BoundedString(BoundedString&& other) MAIDSAFE_NOEXCEPT : string_(std::move(other.string_)),
                                                           valid_(std::move(other.valid_)) {}

  BoundedString& operator=(BoundedString other) MAIDSAFE_NOEXCEPT {
    // No need for check for self-assignment since we're using the copy-and-swap idiom
    swap(*this, other);
    return *this;
  }

  BoundedString& operator+=(const BoundedString& other) {
    if (!valid_ || !other.valid_) {
      LOG(kError) << "BoundedString one of class uninitialised in operator+=";
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
    }
    if (SizeOutOfBounds(string_.size() + other.string_.size())) {
      LOG(kError) << "BoundedString invalid_string_size in operator+=";
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_string_size));
    }
    StringType temp(string_ + other.string_);
    string_.swap(temp);
    return *this;
  }

  template <size_t other_min, size_t other_max, typename OtherStringType>
  explicit BoundedString(BoundedString<other_min, other_max, OtherStringType> other)
      : string_(other.string_.begin(), other.string_.end()), valid_(std::move(other.valid_)) {
    static_assert((min <= other_min) && (max >= other_max),
                  "Bounds of copied BoundedString must be within bounds of this BoundedString");
  }

  template <size_t other_min, size_t other_max, typename OtherStringType>
  BoundedString& operator=(BoundedString<other_min, other_max, OtherStringType> other) {
    static_assert((min <= other_min) && (max >= other_max),
                  "Bounds of copied BoundedString must be within bounds of this BoundedString");
    // No need for check for self-assignment since these are different types
    swap(string_, other.string_);
    std::swap(valid_, other.valid_);
    return *this;
  }

  template <size_t other_min, size_t other_max, typename OtherStringType>
  BoundedString& operator+=(const BoundedString<other_min, other_max, OtherStringType>& other) {
    if (!valid_ || !other.valid_) {
      LOG(kError) << "BoundedString one of class uninitialised in operator+=";
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
    }
    if (SizeOutOfBounds(string_.size() + other.string_.size())) {
      LOG(kError) << "BoundedString invalid_string_size in operator+=";
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_string_size));
    }
    StringType temp(string_ + other.string_);
    string_.swap(temp);
    return *this;
  }

  const StringType& string() const {
    if (!valid_) {
      LOG(kError) << "class uninitialised";
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
    }
    return string_;
  }

  bool IsInitialised() const { return valid_; }

  template <size_t other_min, size_t other_max, typename OtherStringType>
  friend class BoundedString;

 private:
  bool SizeOutOfBounds(std::string::size_type size) const {
    static_assert(min, "Lower bound of BoundedString must be non-zero");
    static_assert(min <= max,
                  "Lower bound of BoundedString must be less than or equal to upper bound");
    return (size < min) || (size > max);
  }

  bool OutwithBounds() const { return SizeOutOfBounds(string_.size()); }
  StringType string_;
  bool valid_;
};
#ifdef __clang__
#pragma clang diagnostic pop
#endif

template <size_t min, size_t max, typename StringType>
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

template <size_t min, size_t max, typename StringType>
inline bool operator!=(const BoundedString<min, max, StringType>& lhs,
                       const BoundedString<min, max, StringType>& rhs) {
  return !operator==(lhs, rhs);
}

template <size_t min, size_t max, typename StringType>
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

template <size_t min, size_t max, typename StringType>
inline bool operator>(const BoundedString<min, max, StringType>& lhs,
                      const BoundedString<min, max, StringType>& rhs) {
  return operator<(rhs, lhs);
}

template <size_t min, size_t max, typename StringType>
inline bool operator<=(const BoundedString<min, max, StringType>& lhs,
                       const BoundedString<min, max, StringType>& rhs) {
  return !operator>(lhs, rhs);
}

template <size_t min, size_t max, typename StringType>
inline bool operator>=(const BoundedString<min, max, StringType>& lhs,
                       const BoundedString<min, max, StringType>& rhs) {
  return !operator<(lhs, rhs);
}

template <size_t lhs_min, size_t lhs_max, size_t rhs_min, size_t rhs_max, typename StringType>
inline BoundedString<lhs_min, lhs_max> operator+(
    BoundedString<lhs_min, lhs_max, StringType> lhs,
    const BoundedString<rhs_min, rhs_max, StringType>& rhs) {
  lhs += rhs;
  return lhs;
}

}  // namespace detail

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_BOUNDED_STRING_H_
