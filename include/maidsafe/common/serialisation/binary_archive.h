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

#ifndef MAIDSAFE_COMMON_SERIALISATION_BINARY_ARCHIVE_H_
#define MAIDSAFE_COMMON_SERIALISATION_BINARY_ARCHIVE_H_

#include <vector>

#include "cereal/cereal.hpp"
#include "boost/interprocess/streams/vectorstream.hpp"

#include "maidsafe/common/types.h"

namespace maidsafe {

using SerialisedData = std::vector<byte>;
using OutputVectorStream = boost::interprocess::basic_ovectorstream<SerialisedData>;
using InputVectorStream = boost::interprocess::basic_ivectorstream<SerialisedData>;

// These are largely copied from Cereal's own BinaryOutputArchive and BinaryInputArchive, so are not
// endian-safe.

class BinaryOutputArchive : public cereal::OutputArchive<BinaryOutputArchive> {
 public:
  explicit BinaryOutputArchive(OutputVectorStream& stream)
      : OutputArchive<BinaryOutputArchive>(this), itsStream(stream) {}

  void saveBinary(const void* data, std::size_t size) {
    auto const writtenSize = static_cast<std::size_t>(
        itsStream.rdbuf()->sputn(reinterpret_cast<const unsigned char*>(data), size));

    if (writtenSize != size)
      throw cereal::Exception("Failed to write " + std::to_string(size) +
                              " bytes to output stream! Wrote " + std::to_string(writtenSize));
  }

 private:
  OutputVectorStream& itsStream;
};

class BinaryInputArchive : public cereal::InputArchive<BinaryInputArchive> {
 public:
  explicit BinaryInputArchive(InputVectorStream& stream)
      : cereal::InputArchive<BinaryInputArchive>(this), itsStream(stream) {}

  void loadBinary(void* const data, std::size_t size) {
    auto const readSize = static_cast<std::size_t>(
        itsStream.rdbuf()->sgetn(reinterpret_cast<unsigned char*>(data), size));

    if (readSize != size)
      throw cereal::Exception("Failed to read " + std::to_string(size) +
                              " bytes from input stream! Read " + std::to_string(readSize));
  }

 private:
  InputVectorStream& itsStream;
};



// Saving for POD types to binary
template <class T>
inline typename std::enable_if<std::is_arithmetic<T>::value, void>::type CEREAL_SAVE_FUNCTION_NAME(
    BinaryOutputArchive& ar, T const& t) {
  ar.saveBinary(std::addressof(t), sizeof(t));
}

// Loading for POD types from binary
template <class T>
inline typename std::enable_if<std::is_arithmetic<T>::value, void>::type CEREAL_LOAD_FUNCTION_NAME(
    BinaryInputArchive& ar, T& t) {
  ar.loadBinary(std::addressof(t), sizeof(t));
}

// Serializing NVP types to binary
template <class Archive, class T>
inline CEREAL_ARCHIVE_RESTRICT(BinaryInputArchive, BinaryOutputArchive)
    CEREAL_SERIALIZE_FUNCTION_NAME(Archive& ar, cereal::NameValuePair<T>& t) {
  ar(t.value);
}

// Serializing SizeTags to binary
template <class Archive, class T>
inline CEREAL_ARCHIVE_RESTRICT(BinaryInputArchive, BinaryOutputArchive)
    CEREAL_SERIALIZE_FUNCTION_NAME(Archive& ar, cereal::SizeTag<T>& t) {
  ar(t.size);
}

// Saving binary data
template <class T>
inline void CEREAL_SAVE_FUNCTION_NAME(BinaryOutputArchive& ar, cereal::BinaryData<T> const& bd) {
  ar.saveBinary(bd.data, static_cast<std::size_t>(bd.size));
}

// Loading binary data
template <class T>
inline void CEREAL_LOAD_FUNCTION_NAME(BinaryInputArchive& ar, cereal::BinaryData<T>& bd) {
  ar.loadBinary(bd.data, static_cast<std::size_t>(bd.size));
}

}  // namespace maidsafe

// Register archives for polymorphic support
CEREAL_REGISTER_ARCHIVE(maidsafe::BinaryOutputArchive)
CEREAL_REGISTER_ARCHIVE(maidsafe::BinaryInputArchive)

// tie input and output archives together
CEREAL_SETUP_ARCHIVE_TRAITS(maidsafe::BinaryInputArchive, maidsafe::BinaryOutputArchive)

#endif  // MAIDSAFE_COMMON_SERIALISATION_BINARY_ARCHIVE_H_
