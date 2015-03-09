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
#include <type_traits>
#include <vector>

#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"

#include "maidsafe/common/config.h"
#include "maidsafe/common/encode.h"
#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"

namespace maidsafe {

namespace detail {

// The BoundedString class holds a string-like object with lower and upper bounds checking of the
// string's size.  A default-constructed instance is generally unusable (most public functions
// throw) but can be assigned to safely.


// Helpers
#ifdef NDEBUG
#define INIT_DEBUG_STRING
#else
#define INIT_DEBUG_STRING , debug_string_(hex::Substr(string_))
#endif

template <typename T, class Enable = void>
struct Char;

template <typename T>
struct Char<T, typename std::enable_if<std::is_same<typename T::value_type, char>::value>::type> {
  using value_type = char;
};

template <typename T>
struct Char<T,
            typename std::enable_if<std::is_same<typename T::value_type, wchar_t>::value>::type> {
  using value_type = wchar_t;
};

template <typename T>
struct Char<T,
            typename std::enable_if<std::is_same<typename T::value_type, char16_t>::value>::type> {
  using value_type = char16_t;
};

template <typename T>
struct Char<T,
            typename std::enable_if<std::is_same<typename T::value_type, char32_t>::value>::type> {
  using value_type = char32_t;
};

template <typename T>
struct Char<
    T, typename std::enable_if<std::is_same<typename T::value_type, unsigned char>::value>::type> {
  using value_type = unsigned char;
};

// BoundedString
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++11-narrowing"
#endif
template <std::size_t min, std::size_t max = -1, typename String = std::vector<unsigned char>>
class BoundedString {
 public:
  using value_type = typename Char<String>::value_type;

#ifdef NDEBUG
  BoundedString()
      : string_(), valid_(!OutwithBounds()) {}  // Use OutwithBounds() to invoke static_asserts
#else
  BoundedString()
      : string_(),
        valid_(!OutwithBounds()),  // Use OutwithBounds() to invoke static_asserts
        debug_string_("Invalid string.") {}
#endif

  explicit BoundedString(String string)
      : string_(std::move(string)), valid_(true) INIT_DEBUG_STRING {
    if (OutwithBounds()) {
      LOG(kError) << "BoundedString::BoundedString(String) - invalid string size";
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::outside_of_bounds));
    }
  }

  template <typename T = String,
            typename std::enable_if<!std::is_same<T, std::string>::value>::type* = nullptr>
  explicit BoundedString(const std::string& string)
      : string_(string.begin(), string.end()), valid_(true) INIT_DEBUG_STRING {
    if (OutwithBounds()) {
      LOG(kError) << "BoundedString::BoundedString(const std::string&) - invalid string size";
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::outside_of_bounds));
    }
  }

  template <std::size_t other_min, std::size_t other_max, typename OtherStringType>
  explicit BoundedString(BoundedString<other_min, other_max, OtherStringType> other)
      : string_(other.string_.begin(), other.string_.end()), valid_(std::move(other.valid_)) {
    static_assert((min <= other_min) && (max >= other_max),
                  "Bounds of copied BoundedString must be within bounds of this BoundedString");
  }

  BoundedString(const BoundedString& other)
      : string_(other.string_), valid_(other.valid_) INIT_DEBUG_STRING {}

  BoundedString(BoundedString&& other) MAIDSAFE_NOEXCEPT
      : string_(std::move(other.string_)),
        valid_(std::move(other.valid_)) INIT_DEBUG_STRING {}

  BoundedString& operator=(const BoundedString&) = default;

  BoundedString& operator=(BoundedString&& other) MAIDSAFE_NOEXCEPT {
    string_ = std::move(other.string_);
    valid_ = std::move(other.valid_);
#ifndef NDEBUG
    debug_string_ = std::move(other.debug_string_);
#endif
    return *this;
  }

  template <std::size_t other_min, std::size_t other_max, typename OtherStringType>
  BoundedString& operator=(BoundedString<other_min, other_max, OtherStringType> other) {
    static_assert((min <= other_min) && (max >= other_max),
                  "Bounds of copied BoundedString must be within bounds of this BoundedString");
    // No need for check for self-assignment since these are different types
    swap(string_, other.string_);
    std::swap(valid_, other.valid_);
    return *this;
  }

  template <typename Archive>
  Archive& save(Archive& archive) const {
    return archive(string());
  }

  template <typename Archive>
  Archive& load(Archive& archive) {
    try {
      String temp_str_type;
      archive(temp_str_type);
      *this = BoundedString{std::move(temp_str_type)};
    } catch (const std::exception& e) {
      LOG(kWarning) << boost::diagnostic_information(e);
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
    }
    return archive;
  }

  const String& string() const {
    if (!valid_) {
      LOG(kError) << "BoundedString is uninitialised.";
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
    }
    return string_;
  }

  const value_type* data() const { return string().data(); }

  std::size_t size() const { return string().size(); }

  bool IsInitialised() const { return valid_; }

  template <std::size_t other_min, std::size_t other_max, typename OtherStringType>
  friend class BoundedString;

  template <std::size_t size, typename String>
  friend BoundedString<size, size, String> operator^(const BoundedString<size, size, String>&,
                                                     const BoundedString<size, size, String>&);

  bool OutwithBounds() const {
    static_assert(min <= max,
                  "Lower bound of BoundedString must be less than or equal to upper bound");
    return (string_.size() < min) || (string_.size() > max);
  }

  String string_;
  bool valid_;
#ifndef NDEBUG
  std::string debug_string_;
#endif
};
#ifdef __clang__
#pragma clang diagnostic pop
#endif



template <std::size_t min, std::size_t max, typename String>
inline bool operator==(const BoundedString<min, max, String>& lhs,
                       const BoundedString<min, max, String>& rhs) {
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

template <std::size_t min, std::size_t max, typename String>
inline bool operator!=(const BoundedString<min, max, String>& lhs,
                       const BoundedString<min, max, String>& rhs) {
  return !operator==(lhs, rhs);
}

template <std::size_t min, std::size_t max, typename String>
inline bool operator<(const BoundedString<min, max, String>& lhs,
                      const BoundedString<min, max, String>& rhs) {
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

template <std::size_t min, std::size_t max, typename String>
inline bool operator>(const BoundedString<min, max, String>& lhs,
                      const BoundedString<min, max, String>& rhs) {
  return operator<(rhs, lhs);
}

template <std::size_t min, std::size_t max, typename String>
inline bool operator<=(const BoundedString<min, max, String>& lhs,
                       const BoundedString<min, max, String>& rhs) {
  return !operator>(lhs, rhs);
}

template <std::size_t min, std::size_t max, typename String>
inline bool operator>=(const BoundedString<min, max, String>& lhs,
                       const BoundedString<min, max, String>& rhs) {
  return !operator<(lhs, rhs);
}

// This returns the bitwise XOR of the two fixed-size inputs.  Throws if either is uninitialised.
template <std::size_t size, typename String>
inline BoundedString<size, size, String> operator^(const BoundedString<size, size, String>& lhs,
                                                   const BoundedString<size, size, String>& rhs) {
  if (!lhs.valid_ || !rhs.valid_) {
    LOG(kError) << "BoundedString is uninitialised.";
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::uninitialised));
  }
  String result(size, 0);
  for (std::size_t i(0); i < size; ++i)
    result[i] = lhs.string_[i] ^ rhs.string_[i];
  return BoundedString<size, size, String>(std::move(result));
}

}  // namespace detail


// Overloads for hex- and base64-encoding a BoundedString.
namespace hex {

template <std::size_t min, std::size_t max = -1, typename String>
std::string Encode(const maidsafe::detail::BoundedString<min, max, String>& non_hex_input) {
  return Encode(non_hex_input.string());
}

}  // namespace hex

namespace base64 {

template <std::size_t min, std::size_t max = -1, typename String>
std::string Encode(const maidsafe::detail::BoundedString<min, max, String>& non_base64_input) {
  return Encode(non_base64_input.string());
}

}  // namespace base64

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_BOUNDED_STRING_H_
