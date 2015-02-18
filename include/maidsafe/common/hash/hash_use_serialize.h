/*  Copyright 2014 MaidSafe.net limited

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

#ifndef MAIDSAFE_COMMON_HASH_HASH_USE_SERIALIZE_H_
#define MAIDSAFE_COMMON_HASH_HASH_USE_SERIALIZE_H_

#include <cstdint>
#include <type_traits>

#define MAIDSAFE_HASH_AND_CEREAL_CLASS_VERSION(TYPE, VERSION_NUMBER) \
  CEREAL_CLASS_VERSION(TYPE, VERSION_NUMBER)                         \
  namespace maidsafe {                                               \
    template<> struct HashVersion<TYPE> :                            \
      std::integral_constant<std::uint32_t, VERSION_NUMBER> {};           \
  }

namespace maidsafe {

/*
   If you want to use the same "serialize" method that Cereal or
   boost::serialization uses for hashing, simply add
   const static bool use_serialize_for_hashing = true; in the declaration of
   your type. If that isn't possible, overload the trait:
   template<> struct UseSerializeForHashing<YourType> : std::true_type {};
*/
template<typename, typename Enable = void>
struct UseSerializeForHashing : std::false_type {};

template<typename Type>
struct UseSerializeForHashing<
    Type, typename std::enable_if<Type::use_serialize_for_hashing>::type> :
  std::true_type {};


/*
  If serialize with hashing is used, and versioning is used by the
  archiving tool, overload this trait to tell the hash algorithms what
  version to use while hashing. If using Cereal too, use the macro
  MAIDSAFE_HASH_AND_CEREAL_CLASS_VERSION(type, number)
*/
template<typename, typename Enable = void>
struct HashVersion : std::integral_constant<std::uint32_t, 0> {};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_HASH_HASH_USE_SERIALIZE_H_
