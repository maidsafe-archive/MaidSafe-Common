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

#ifndef MAIDSAFE_COMMON_SERIALISATION_COMPILE_TIME_MAPPER_H_
#define MAIDSAFE_COMMON_SERIALISATION_COMPILE_TIME_MAPPER_H_

#include <cstdint>
#include <limits>
#include <sstream>
#include <string>

#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"

namespace maidsafe {

using SerialisableTypeTag = uint16_t;

template <SerialisableTypeTag Tag, typename Value, typename NextNode>
struct CompileTimeMapper {};
struct ERROR_given_tag_is_not_mapped_to_a_type;

template <SerialisableTypeTag Tag, typename Value>
struct Serialisable;

template <typename...>
struct GetMap;

template <SerialisableTypeTag Tag, typename Value, typename... Types>
struct GetMap<Serialisable<Tag, Value>, Types...> {
  using Map = CompileTimeMapper<Tag, Value, typename GetMap<Types...>::Map>;
};

template <SerialisableTypeTag Tag, typename Value>
struct GetMap<Serialisable<Tag, Value>> {
  using Map = CompileTimeMapper<Tag, Value, ERROR_given_tag_is_not_mapped_to_a_type>;
};

template <typename, SerialisableTypeTag>
struct Find;

template <SerialisableTypeTag Tag, typename Value, typename NextNode, SerialisableTypeTag TagToFind>
struct Find<CompileTimeMapper<Tag, Value, NextNode>, TagToFind> {
  using ResultCustomType = typename Find<NextNode, TagToFind>::ResultCustomType;
};

template <SerialisableTypeTag TagToFind, typename Value, typename NextNode>
struct Find<CompileTimeMapper<TagToFind, Value, NextNode>, TagToFind> {
  using ResultCustomType = Value;
};

template <typename TypeToSerialise>
std::string Serialise(const TypeToSerialise& obj_to_serialise) {
  std::stringstream string_stream;
  {
    cereal::BinaryOutputArchive output_bin_archive{string_stream};
    output_bin_archive(obj_to_serialise.kSerialisableTypeTag,
                       std::forward<TypeToSerialise>(obj_to_serialise));
  }
  return string_stream.str();
}

SerialisableTypeTag TypeFromStream(std::stringstream& ref_binary_stream) {
  SerialisableTypeTag tag{std::numeric_limits<SerialisableTypeTag>::max()};
  {
    cereal::BinaryInputArchive input_bin_archive{ref_binary_stream};
    input_bin_archive(tag);
  }
  return tag;
}

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_SERIALISATION_COMPILE_TIME_MAPPER_H_
