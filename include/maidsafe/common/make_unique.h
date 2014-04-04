/*  Copyright 2009 MaidSafe.net limited

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

// This is an implementation of make_unique as defined in http://isocpp.org/files/papers/N3656.txt
// and which can be replaced by std::make_unique once C++14 is available.

#ifndef MAIDSAFE_COMMON_MAKE_UNIQUE_H_
#define MAIDSAFE_COMMON_MAKE_UNIQUE_H_

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace maidsafe {

namespace detail {

template <typename T>
struct UniqueIf {
  typedef std::unique_ptr<T> SingleObject;
};

template <typename T>
struct UniqueIf<T[]> {
  typedef std::unique_ptr<T[]> UnknownBound;
};

template <typename T, size_t N>
struct UniqueIf<T[N]> {
  typedef void KnownBound;
};

}  // namespace detail

template <typename T, typename... Args>
typename detail::UniqueIf<T>::SingleObject make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
typename detail::UniqueIf<T>::UnknownBound make_unique(size_t n) {
  typedef typename std::remove_extent<T>::type U;
  return std::unique_ptr<T>(new U[n]());
}

template <typename T, typename... Args>
typename detail::UniqueIf<T>::KnownBound make_unique(Args&&...) = delete;

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_MAKE_UNIQUE_H_
