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


/*
 * There two flavours of serialization and de-serialization functions here. One that works on
 * strings and the other that works on streams. This is because the string ones create
 * stringstreams as local variables and stringstreams being locale aware have poor contruction
 * speed. Thus if the client code wants to provide streams himself so that he can cache and reuse
 * the streams, the functions which directly operate on streams can be used.
 */

#ifndef MAIDSAFE_COMMON_SERIALISATION_H_
#define MAIDSAFE_COMMON_SERIALISATION_H_

#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/boost_optional.hpp>

#include <cereal/archives/binary.hpp>
#include <string>

namespace maidsafe {

template<typename... TypesToSerialize>
inline std::ostream& ConvertToStream(std::ostream& ref_dest_stream,
                                     TypesToSerialize&&... ref_source_objs) {
  cereal::BinaryOutputArchive output_archive {ref_dest_stream};
  output_archive(std::forward<TypesToSerialize>(ref_source_objs)...);
  return ref_dest_stream;
}

template<typename... TypesToSerialize>
inline std::string ConvertToString(std::stringstream& ref_dest_stream,
                                   TypesToSerialize&&... ref_source_objs) {
  return static_cast<std::stringstream&>(
        ConvertToStream(ref_dest_stream,
                        std::forward<TypesToSerialize>(ref_source_objs)...)).str();
}

template<typename... TypesToSerialize>
inline std::string ConvertToString(TypesToSerialize&&... ref_source_objs) {
  std::stringstream str_stream;
  return ConvertToString(str_stream, std::forward<TypesToSerialize>(ref_source_objs)...);
}

template<typename... DeSerializeToTypes>
inline void ConvertFromStream(std::istream& ref_source_stream,
                              DeSerializeToTypes&... ref_dest_objs) {
  cereal::BinaryInputArchive input_archive {ref_source_stream};
  input_archive(ref_dest_objs...);
}

template<typename DeSerializeToType>
inline DeSerializeToType& ConvertFromStream(std::istream& ref_source_stream,
                                            DeSerializeToType& ref_dest_obj) {
  cereal::BinaryInputArchive input_archive {ref_source_stream};
  input_archive(ref_dest_obj);
  return ref_dest_obj;
}

template<typename DeSerializeToType>
inline DeSerializeToType ConvertFromStream(std::istream& ref_source_stream) {
  cereal::BinaryInputArchive input_archive {ref_source_stream};
  DeSerializeToType dest_obj;
  input_archive(dest_obj);
  return dest_obj;
}

template<typename... DeSerializeToTypes>
inline void ConvertFromString(const std::string& ref_source_string,
                              DeSerializeToTypes&... ref_dest_objs) {
  std::stringstream str_stream {ref_source_string};
  ConvertFromStream(str_stream, ref_dest_objs...);
}

template<typename DeSerializeToType>
inline DeSerializeToType& ConvertFromString(const std::string& ref_source_string,
                                            DeSerializeToType& ref_dest_obj) {
  std::stringstream str_stream {ref_source_string};
  return ConvertFromStream(str_stream, ref_dest_obj);
}

template<typename DeSerializeToType>
inline DeSerializeToType ConvertFromString(const std::string& ref_source_string) {
  std::stringstream str_stream {ref_source_string};
  return ConvertFromStream<DeSerializeToType>(str_stream);
}

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_SERIALISATION_H_
