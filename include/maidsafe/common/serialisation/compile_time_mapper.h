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
#include <memory>
#include <string>

#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"

#include "maidsafe/common/make_unique.h"
#include "maidsafe/common/types.h"

namespace maidsafe {

using SerialisableTypeTag = uint16_t;

template <SerialisableTypeTag Tag, typename Value, typename NextNode>
struct CompileTimeMapper {};
struct ERROR_given_tag_is_not_mapped_to_a_type;

template <typename...>
struct GetMap;

template <typename Value, typename... Types>
struct GetMap<Value, Types...> {
  using Map = CompileTimeMapper<Value::kSerialisableTypeTag, Value, typename GetMap<Types...>::Map>;
};

template <typename Value>
struct GetMap<Value> {
  using Map = CompileTimeMapper<Value::kSerialisableTypeTag, Value,
                                ERROR_given_tag_is_not_mapped_to_a_type>;
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



// ========== Serialisation/parsing helpers ========================================================
namespace detail {

template <typename StreamType>
struct Archive;

template <>
struct Archive<OutputVectorStream> {
  using type = BinaryOutputArchive;
};

template <>
struct Archive<InputVectorStream> {
  using type = BinaryInputArchive;
};

template <>
struct Archive<std::ostringstream> {
  using type = cereal::BinaryOutputArchive;
};

template <>
struct Archive<std::istringstream> {
  using type = cereal::BinaryInputArchive;
};

template <typename TypeToSerialise, typename StreamType>
std::unique_ptr<StreamType> Serialise(TypeToSerialise object_to_serialise) {
  auto binary_output_stream = maidsafe::make_unique<StreamType>();
  {
    typename Archive<StreamType>::type binary_output_archive(*binary_output_stream);
    binary_output_archive(TypeToSerialise::kSerialisableTypeTag,
                          std::forward<TypeToSerialise>(object_to_serialise));
  }
  return std::move(binary_output_stream);
}

template <typename StreamType>
SerialisableTypeTag TypeFromStream(StreamType& binary_input_stream) {
  auto tag = std::numeric_limits<SerialisableTypeTag>::max();
  {
    typename Archive<StreamType>::type binary_input_archive(binary_input_stream);
    binary_input_archive(tag);
  }
  return tag;
}

template <typename ParsedType, typename StreamType>
ParsedType Parse(StreamType& binary_input_stream) {
  ParsedType parsed_message;
  {
    typename Archive<StreamType>::type binary_input_archive(binary_input_stream);
    binary_input_archive(parsed_message);
  }
  return parsed_message;
}

}  // namespace detail



// ========== Serialisation/parsing using std::vector<byte> types ==================================
template <typename TypeToSerialise>
SerialisedData Serialise(TypeToSerialise object_to_serialise) {
  return detail::Serialise<TypeToSerialise, OutputVectorStream>(object_to_serialise)->vector();
}

inline SerialisableTypeTag TypeFromStream(InputVectorStream& binary_input_stream) {
  return detail::TypeFromStream(binary_input_stream);
}

template <typename ParsedType>
ParsedType Parse(InputVectorStream& binary_input_stream) {
  return detail::Parse<ParsedType, InputVectorStream>(binary_input_stream);
}



// ========== Serialisation/parsing using std::string types ========================================
template <typename TypeToSerialise>
std::string SerialiseToString(TypeToSerialise object_to_serialise) {
  return detail::Serialise<TypeToSerialise, std::ostringstream>(object_to_serialise)->str();
}

inline SerialisableTypeTag TypeFromStringStream(std::istringstream& binary_input_stream) {
  return detail::TypeFromStream(binary_input_stream);
}

template <typename ParsedType>
ParsedType ParseFromStringStream(std::istringstream& binary_input_stream) {
  return detail::Parse<ParsedType, std::istringstream>(binary_input_stream);
}

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_SERIALISATION_COMPILE_TIME_MAPPER_H_
