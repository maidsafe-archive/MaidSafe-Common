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
    use of the MaidSafe Software.                                                               */

/* To use this component there are several items to be created. An example of classes One Two Zero

enum class TypeTag: unsigned char {
  Zero,
  One,
  Two
};

//--------------------------------------------------------------------------------------
struct Zero;
struct One;
struct Two;

using MAP = GetMap<
  Pair<TypeTag::Zero, Zero>,
  Pair<TypeTag::One, One>,
  Pair<TypeTag::Two, Two>
>::MAP;

template<TypeTag Key>
using CustomType = typename Find<MAP, Key>::ResultCustomType;
A typical class would look like

struct Zero {
  template<typename Archive>
  void serialize(Archive& ref_archive) {
    ref_archive(i_data, c_data, sz_data);
  }

  std::ostream& Print(std::ostream& out_stream) const {
    out_stream << "[\n\t" << i_data
               << "\n\t" << c_data
               << "\n\t" << sz_data
               << "\n]\n\n";
    return out_stream;
  }

  int    i_data       {};
  char   c_data       {'Z'};
  std::string sz_data {"Zero"};

  const TypeTag e_TYPE_TAG {TypeTag::Zero};
};

*/

#ifndef MAIDSAFE_COMMON_SERIALISATION_H_
#define MAIDSAFE_COMMON_SERIALISATION_H_

#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/binary.hpp>

namespace maidsafe {

enum class TypeTag : unsigned char;

template <TypeTag Key, typename Value, typename NextNode>
struct CompileTimeMapper;
struct ERROR_given_tag_is_not_mapped_to_a_type;

template <TypeTag, typename>
struct KVPair;

template <typename...>
struct GetMap;

template <TypeTag Key, typename Value, typename... Types>
struct GetMap<KVPair<Key, Value>, Types...> {
  using MAP = CompileTimeMapper<Key, Value, typename GetMap<Types...>::MAP>;
};

template <TypeTag Key, typename Value>
struct GetMap<KVPair<Key, Value>> {
  using MAP = CompileTimeMapper<Key, Value, ERROR_given_tag_is_not_mapped_to_a_type>;
};

template <typename, TypeTag>
struct Find;

template <TypeTag Key, typename Value, typename NextNode, TypeTag KeyToFind>
struct Find<CompileTimeMapper<Key, Value, NextNode>, KeyToFind> {
  using ResultCustomType = typename Find<NextNode, KeyToFind>::ResultCustomType;
};

template <TypeTag KeyToFind, typename Value, typename NextNode>
struct Find<CompileTimeMapper<KeyToFind, Value, NextNode>, KeyToFind> {
  using ResultCustomType = Value;
};

template <typename TypeToSerialise>
std::string Serialise(TypeToSerialise&& obj_to_serialise) {
  std::stringstream string_stream;

  {
    cereal::BinaryOutputArchive output_bin_archive{string_stream};
    output_bin_archive(obj_to_serialise.e_TYPE_TAG,
                       std::forward<TypeToSerialise>(obj_to_serialise));
  }

  return string_stream.str();
}

inline TypeTag TypeFromStream(std::stringstream& ref_binary_stream) {
  unsigned char uc_tag;

  {
    cereal::BinaryInputArchive input_bin_archive{ref_binary_stream};
    input_bin_archive(uc_tag);
  }

  return static_cast<TypeTag>(uc_tag);
}

}  // namespace maidsafe

#endif  //  MAIDSAFE_COMMON_SERIALISATION_H_
