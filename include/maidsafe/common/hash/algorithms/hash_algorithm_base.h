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

#ifndef MAIDSAFE_COMMON_HASH_ALGORITHMS_HASH_ALGORITHM_BASE_H_
#define MAIDSAFE_COMMON_HASH_ALGORITHMS_HASH_ALGORITHM_BASE_H_

#include <cstdint>
#include <type_traits>
#include <utility>

#include "cereal/details/traits.hpp"

#include "maidsafe/common/hash/hash_use_serialize.h"

namespace maidsafe {

namespace detail {

template<typename Derived>
class HashAlgorithmBase {
  static_assert(!std::is_pointer<Derived>::value, "Bad used of CRTP");
  static_assert(!std::is_reference<Derived>::value, "Bad used of CRTP");

 public:
  template<typename Arg, typename... Args>
  void operator()(Arg&& param, Args&&... params) {
    Process(std::forward<Arg>(param), std::forward<Args>(params)...);
  }

  template<typename Arg>
  Derived& operator&(Arg&& param) {
    Process(std::forward<Arg>(param));
    return *self();
  }

 private:
  const Derived* self() const { return static_cast<const Derived*>(this); }
  Derived* self() { return static_cast<Derived*>(this); }

  template<typename Type>
  using Normalize =
    typename std::remove_reference<typename std::remove_cv<Type>::type>::type;

  template<typename Type>
  using IsVersionIntegral =
    std::integral_constant<
      bool,
      std::is_integral<typename HashVersion<Type>::value_type>::value>;

  //
  // Traits for selecting which function to call. VS 2013 doesn't
  // have SFINAE expression support, so get creative
  //
  template<typename> struct FunctionExists {};

  template<typename Function, typename... Args>
  struct FunctionExists<Function(Args...)> {
    template<typename TestFunction, typename... TestArgs>
    static auto Check(int)  //NOLINT  - Cpplint incorrectly thinks this is a cast
      -> decltype(std::declval<TestFunction>()(std::declval<TestArgs>()...), std::true_type());

    template<typename, typename...>
    static std::false_type Check(...);

    static const bool value =
        std::is_same<decltype(Check<Function, Args...>(0)), std::true_type>::value;
  };

  template<typename Function>
  using Has = std::integral_constant<bool, FunctionExists<Function>::value>;

  template<typename Function>
  using EnableIfHas = typename std::enable_if<FunctionExists<Function>::value>::type;

  struct HashAppendFunction {
    struct MemberFunction {
      template<typename Type>
      auto operator()(Derived& hash, Type&& type) const
          -> decltype(std::forward<Type>(type).HashAppend(hash)) {
        return std::forward<Type>(type).HashAppend(hash);
      }
    };

    struct NonMemberFunction {
      template<typename Type>
      auto operator()(Derived& hash, Type&& type) const
          -> decltype(HashAppend(hash, std::forward<Type>(type))) {
        return HashAppend(hash, std::forward<Type>(type));
      }
    };

    template<typename Type>
    EnableIfHas<MemberFunction(Derived&, Type)> operator()(
        Derived& hash, Type&& type) const {
      MemberFunction{}(hash, std::forward<Type>(type));
    }

    template<typename Type>
    EnableIfHas<NonMemberFunction(Derived&, Type)> operator()(
        Derived& hash, Type&& type) const {
      NonMemberFunction{}(hash, std::forward<Type>(type));
    }
  };

  struct SerializeFunction {
    // There's no need to incude Cereal save/serialize minimal functions since they relate to types
    // which are so basic they shouldn't be using serialisation to hash.
    template <typename T>
    using HasMemberSave =
        cereal::traits::has_member_save<typename std::remove_reference<T>::type, Derived>;
    template <typename T>
    using HasNonMemberSave =
        cereal::traits::has_non_member_save<typename std::remove_reference<T>::type, Derived>;
    template <typename T>
    using HasMemberSerialize =
        cereal::traits::has_member_serialize<typename std::remove_reference<T>::type, Derived>;
    template <typename T>
    using HasNonMemberSerialize =
        cereal::traits::has_non_member_serialize<typename std::remove_reference<T>::type, Derived>;
    template <typename T>
    using HasMemberVersionedSave =
        cereal::traits::has_member_versioned_save<typename std::remove_reference<T>::type, Derived>;
    template <typename T>
    using HasNonMemberVersionedSave =
        cereal::traits::has_non_member_versioned_save<typename std::remove_reference<T>::type,
                                                      Derived>;
    template <typename T>
    using HasMemberVersionedSerialize =
        cereal::traits::has_member_versioned_serialize<typename std::remove_reference<T>::type,
                                                       Derived>;
    template <typename T>
    using HasNonMemberVersionedSerialize =
        cereal::traits::has_non_member_versioned_serialize<typename std::remove_reference<T>::type,
                                                           Derived>;

   public:
    template <typename Type>
    typename std::enable_if<HasMemberSave<Type>::value>::type operator()(Derived& hash,
                                                                         Type&& type) const {
      std::forward<Type>(type).save(hash);
    }

    template <typename Type>
    typename std::enable_if<HasNonMemberSave<Type>::value>::type operator()(Derived& hash,
                                                                            Type&& type) const {
      save(hash, type);
    }

    template <typename Type>
    typename std::enable_if<HasMemberSerialize<Type>::value>::type operator()(Derived& hash,
                                                                              Type&& type) const {
      std::forward<Type>(type).serialize(hash);
    }

    template <typename Type>
    typename std::enable_if<HasNonMemberSerialize<Type>::value>::type operator()(
        Derived& hash, Type&& type) const {
      serialize(hash, type);
    }

    template <typename Type>
    typename std::enable_if<HasMemberVersionedSave<Type>::value>::type operator()(
        Derived& hash, Type&& type) const {
      static_assert(IsVersionIntegral<Normalize<Type>>::value,
                    "version must provide integral value");
      std::forward<Type>(type).save(hash, HashVersion<Normalize<Type>>{});
    }

    template <typename Type>
    typename std::enable_if<HasNonMemberVersionedSave<Type>::value>::type operator()(
        Derived& hash, Type&& type) const {
      static_assert(IsVersionIntegral<Normalize<Type>>::value,
                    "version must provide integral value");
      save(hash, type, HashVersion<Normalize<Type>>{});
    }

    template <typename Type>
    typename std::enable_if<HasMemberVersionedSerialize<Type>::value>::type operator()(
        Derived& hash, Type&& type) const {
      static_assert(IsVersionIntegral<Normalize<Type>>::value,
                    "version must provide integral value");
      std::forward<Type>(type).serialize(hash, HashVersion<Normalize<Type>>{});
    }

    template <typename Type>
    typename std::enable_if<HasNonMemberVersionedSerialize<Type>::value>::type operator()(
        Derived& hash, Type&& type) const {
      static_assert(IsVersionIntegral<Normalize<Type>>::value,
                    "version must provide integral value");
      serialize(hash, type, HashVersion<Normalize<Type>>{});
    }
  };

  // Process function which select which stand-alone function to call (or signal failure)
  template<typename Head, typename... Tail>
  void Process(Head&& param, Tail&&... params) {
    Process(std::forward<Head>(param));
    Process(std::forward<Tail>(params)...);
  }

  template<typename Arg>
  void Process(Arg&& param) {
    InvokeHashAppend(std::forward<Arg>(param), Has<HashAppendFunction(Derived&, Arg)>{});
  }

  void Process() {}

  template<typename Arg>
  void InvokeHashAppend(Arg&& param, std::true_type) {
    HashAppendFunction{}(*self(), std::forward<Arg>(param));
  }

  template<typename Arg>
  void InvokeHashAppend(Arg&& param, std::false_type) {
    static_assert(
        Has<SerializeFunction(Derived&, Arg)>::value,
        "This type does not have a HashAppend or serialize function");
    InvokeSerialize(std::forward<Arg>(param), Has<SerializeFunction(Derived&, Arg)>{});
  }

  template<typename Arg>
  void InvokeSerialize(Arg&& param, std::true_type) {
    static_assert(
        UseSerializeForHashing<typename std::remove_reference<Arg>::type>::value,
        "This type has a serialize/save method, but hash support has not been enabled");
    SerializeFunction{}(*self(), std::forward<Arg>(param));
  }

  template<typename Arg>
  void InvokeSerialize(Arg&&, std::false_type);
};

}  // namespace detail

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_HASH_ALGORITHMS_HASH_ALGORITHM_BASE_H_
