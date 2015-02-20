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

#include <forward_list>
#include <initializer_list>
#include <list>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "boost/range/adaptor/map.hpp"
#include "boost/range/iterator_range.hpp"
#include "boost/utility/string_ref.hpp"

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"

#include "maidsafe/common/hash.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/serialisation/serialisation.h"

namespace {

int serialize_call_count = 0;

enum FunctionDetails {
  kSerialize = 1,
  kSave = 2,
  kVoidReturn = 4,
  kArchiveRefReturn = 8,
  kWithoutVersion = 16,
  kWithVersion = 32
};

template <int details_in, bool use_function>
struct HasInternalFunction {
  static const bool use_serialize_for_hashing = use_function;
  int one, two, three;
  enum { expected_serialize_call_count = use_function };

  template <typename Archive, int details = details_in>
  typename std::enable_if<((details & kSerialize) == kSerialize) &&
                          ((details & kVoidReturn) == kVoidReturn) &&
                          ((details & kWithoutVersion) == kWithoutVersion)>::type
      serialize(Archive& archive) {
    ++serialize_call_count;
    archive(one, two, three);
  }

  template <typename Archive, int details = details_in>
  typename std::enable_if<((details & kSerialize) == kSerialize) &&
                          ((details & kVoidReturn) == kVoidReturn) &&
                          ((details & kWithVersion) == kWithVersion)>::type
      serialize(Archive& archive, const std::uint32_t version) {
    ++serialize_call_count;
    EXPECT_EQ(0u, version);
    archive(one, two, three);
  }

  template <typename Archive, int details = details_in>
  typename std::enable_if<((details & kSerialize) == kSerialize) &&
                              ((details & kArchiveRefReturn) == kArchiveRefReturn) &&
                              ((details & kWithoutVersion) == kWithoutVersion),
                          Archive>::type
      serialize(Archive& archive) {
    ++serialize_call_count;
    archive(one, two, three);
    return archive;
  }

  template <typename Archive, int details = details_in>
  typename std::enable_if<((details & kSerialize) == kSerialize) &&
                              ((details & kArchiveRefReturn) == kArchiveRefReturn) &&
                              ((details & kWithVersion) == kWithVersion),
                          Archive>::type
      serialize(Archive& archive, const std::uint32_t version) {
    ++serialize_call_count;
    EXPECT_EQ(0u, version);
    archive(one, two, three);
    return archive;
  }

  template <typename Archive, int details = details_in>
  typename std::enable_if<((details & kSave) == kSave) &&
                          ((details & kVoidReturn) == kVoidReturn) &&
                          ((details & kWithoutVersion) == kWithoutVersion)>::type
      save(Archive& archive) const {
    ++serialize_call_count;
    archive(one, two, three);
  }

  template <typename Archive, int details = details_in>
  typename std::enable_if<((details & kSave) == kSave) &&
                          ((details & kVoidReturn) == kVoidReturn) &&
                          ((details & kWithVersion) == kWithVersion)>::type
      save(Archive& archive, const std::uint32_t version) const {
    ++serialize_call_count;
    EXPECT_EQ(0u, version);
    archive(one, two, three);
  }

  template <typename Archive, int details = details_in>
  typename std::enable_if<((details & kSave) == kSave) &&
                              ((details & kArchiveRefReturn) == kArchiveRefReturn) &&
                              ((details & kWithoutVersion) == kWithoutVersion),
                          Archive>::type
      save(Archive& archive) const {
    ++serialize_call_count;
    archive(one, two, three);
    return archive;
  }

  template <typename Archive, int details = details_in>
  typename std::enable_if<((details & kSave) == kSave) &&
                              ((details & kArchiveRefReturn) == kArchiveRefReturn) &&
                              ((details & kWithVersion) == kWithVersion),
                          Archive>::type
      save(Archive& archive, const std::uint32_t version) const {
    ++serialize_call_count;
    EXPECT_EQ(0u, version);
    archive(one, two, three);
    return archive;
  }


  template <typename Archive, int details = details_in>
  typename std::enable_if<((details & kSave) == kSave) &&
                          ((details & kWithoutVersion) == kWithoutVersion)>::type
      load(Archive& archive) {
    archive(one, two, three);
  }

  template <typename Archive, int details = details_in>
  typename std::enable_if<((details & kSave) == kSave) &&
                          ((details & kWithVersion) == kWithVersion)>::type
      load(Archive& archive, const std::uint32_t version) {
    EXPECT_EQ(0u, version);
    archive(one, two, three);
  }

  template <typename HashAlgorithm, bool enable = !use_serialize_for_hashing>
  typename std::enable_if<enable>::type HashAppend(HashAlgorithm& hash) const {
    hash(one, two, three);
  }
};

using IgnoreInternalSerializeWithoutVersionVoidReturn =
    HasInternalFunction<kSerialize | kVoidReturn | kWithoutVersion, false>;
using IgnoreInternalSerializeWithVersionVoidReturn =
    HasInternalFunction<kSerialize | kVoidReturn | kWithVersion, false>;
using IgnoreInternalSerializeWithoutVersionArchiveRefReturn =
    HasInternalFunction<kSerialize | kArchiveRefReturn | kWithoutVersion, false>;
using IgnoreInternalSerializeWithVersionArchiveRefReturn =
    HasInternalFunction<kSerialize | kArchiveRefReturn | kWithVersion, false>;
using IgnoreInternalSaveWithoutVersionVoidReturn =
    HasInternalFunction<kSave | kVoidReturn | kWithoutVersion, false>;
using IgnoreInternalSaveWithVersionVoidReturn =
    HasInternalFunction<kSave | kVoidReturn | kWithVersion, false>;
using IgnoreInternalSaveWithoutVersionArchiveRefReturn =
    HasInternalFunction<kSave | kArchiveRefReturn | kWithoutVersion, false>;
using IgnoreInternalSaveWithVersionArchiveRefReturn =
    HasInternalFunction<kSave | kArchiveRefReturn | kWithVersion, false>;
using UseInternalSerializeWithoutVersionVoidReturn =
    HasInternalFunction<kSerialize | kVoidReturn | kWithoutVersion, true>;
using UseInternalSerializeWithVersionVoidReturn =
    HasInternalFunction<kSerialize | kVoidReturn | kWithVersion, true>;
using UseInternalSerializeWithoutVersionArchiveRefReturn =
    HasInternalFunction<kSerialize | kArchiveRefReturn | kWithoutVersion, true>;
using UseInternalSerializeWithVersionArchiveRefReturn =
    HasInternalFunction<kSerialize | kArchiveRefReturn | kWithVersion, true>;
using UseInternalSaveWithoutVersionVoidReturn =
    HasInternalFunction<kSave | kVoidReturn | kWithoutVersion, true>;
using UseInternalSaveWithVersionVoidReturn =
    HasInternalFunction<kSave | kVoidReturn | kWithVersion, true>;
using UseInternalSaveWithoutVersionArchiveRefReturn =
    HasInternalFunction<kSave | kArchiveRefReturn | kWithoutVersion, true>;
using UseInternalSaveWithVersionArchiveRefReturn =
    HasInternalFunction<kSave | kArchiveRefReturn | kWithVersion, true>;



template <int details, bool use_function>
struct HasExternalFunction {
  static const bool use_serialize_for_hashing = use_function;
  int one, two, three;
  enum { expected_serialize_call_count = use_function };

  template <typename Archive>
  Archive& CommonSerialise(Archive& archive,
                           std::uint32_t version = std::numeric_limits<std::uint32_t>::max()) {
    ++serialize_call_count;
    if (version != std::numeric_limits<std::uint32_t>::max())
      EXPECT_EQ(0u, version);
    archive(one, two, three);
    return archive;
  }

  template <typename Archive>
  Archive& CommonSave(Archive& archive,
                      std::uint32_t version = std::numeric_limits<std::uint32_t>::max()) const {
    ++serialize_call_count;
    if (version != std::numeric_limits<std::uint32_t>::max())
      EXPECT_EQ(0u, version);
    archive(one, two, three);
    return archive;
  }

  template <typename Archive>
  Archive& CommonLoad(Archive& archive,
                      std::uint32_t version = std::numeric_limits<std::uint32_t>::max()) {
    static_cast<void>(version);
    archive(one, two, three);
    return archive;
  }
};

using IgnoreExternalSerializeWithoutVersionVoidReturn =
    HasExternalFunction<kSerialize | kVoidReturn | kWithoutVersion, false>;
using IgnoreExternalSerializeWithVersionVoidReturn =
    HasExternalFunction<kSerialize | kVoidReturn | kWithVersion, false>;
using IgnoreExternalSerializeWithoutVersionArchiveRefReturn =
    HasExternalFunction<kSerialize | kArchiveRefReturn | kWithoutVersion, false>;
using IgnoreExternalSerializeWithVersionArchiveRefReturn =
    HasExternalFunction<kSerialize | kArchiveRefReturn | kWithVersion, false>;
using IgnoreExternalSaveWithoutVersionVoidReturn =
    HasExternalFunction<kSave | kVoidReturn | kWithoutVersion, false>;
using IgnoreExternalSaveWithVersionVoidReturn =
    HasExternalFunction<kSave | kVoidReturn | kWithVersion, false>;
using IgnoreExternalSaveWithoutVersionArchiveRefReturn =
    HasExternalFunction<kSave | kArchiveRefReturn | kWithoutVersion, false>;
using IgnoreExternalSaveWithVersionArchiveRefReturn =
    HasExternalFunction<kSave | kArchiveRefReturn | kWithVersion, false>;
using UseExternalSerializeWithoutVersionVoidReturn =
    HasExternalFunction<kSerialize | kVoidReturn | kWithoutVersion, true>;
using UseExternalSerializeWithVersionVoidReturn =
    HasExternalFunction<kSerialize | kVoidReturn | kWithVersion, true>;
using UseExternalSerializeWithoutVersionArchiveRefReturn =
    HasExternalFunction<kSerialize | kArchiveRefReturn | kWithoutVersion, true>;
using UseExternalSerializeWithVersionArchiveRefReturn =
    HasExternalFunction<kSerialize | kArchiveRefReturn | kWithVersion, true>;
using UseExternalSaveWithoutVersionVoidReturn =
    HasExternalFunction<kSave | kVoidReturn | kWithoutVersion, true>;
using UseExternalSaveWithVersionVoidReturn =
    HasExternalFunction<kSave | kVoidReturn | kWithVersion, true>;
using UseExternalSaveWithoutVersionArchiveRefReturn =
    HasExternalFunction<kSave | kArchiveRefReturn | kWithoutVersion, true>;
using UseExternalSaveWithVersionArchiveRefReturn =
    HasExternalFunction<kSave | kArchiveRefReturn | kWithVersion, true>;

template <typename Archive>
void serialize(Archive& archive, IgnoreExternalSerializeWithoutVersionVoidReturn& value) {
  value.CommonSerialise(archive);
}

template <typename Archive>
void serialize(Archive& archive, IgnoreExternalSerializeWithVersionVoidReturn& value,
               const std::uint32_t version) {
  value.CommonSerialise(archive, version);
}

template <typename Archive>
Archive& serialize(Archive& archive, IgnoreExternalSerializeWithoutVersionArchiveRefReturn& value) {
  return value.CommonSerialise(archive);
}

template <typename Archive>
Archive& serialize(Archive& archive, IgnoreExternalSerializeWithVersionArchiveRefReturn& value,
                   const std::uint32_t version) {
  return value.CommonSerialise(archive, version);
}

template <typename Archive>
void save(Archive& archive, const IgnoreExternalSaveWithoutVersionVoidReturn& value) {
  value.CommonSave(archive);
}

template <typename Archive>
void load(Archive& archive, IgnoreExternalSaveWithoutVersionVoidReturn& value) {
  value.CommonLoad(archive);
}

template <typename Archive>
void save(Archive& archive, const IgnoreExternalSaveWithVersionVoidReturn& value,
          const std::uint32_t version) {
  value.CommonSave(archive, version);
}

template <typename Archive>
void load(Archive& archive, IgnoreExternalSaveWithVersionVoidReturn& value,
          const std::uint32_t version) {
  value.CommonLoad(archive, version);
}

template <typename Archive>
Archive& save(Archive& archive, const IgnoreExternalSaveWithoutVersionArchiveRefReturn& value) {
  return value.CommonSave(archive);
}

template <typename Archive>
Archive& load(Archive& archive, IgnoreExternalSaveWithoutVersionArchiveRefReturn& value) {
  return value.CommonLoad(archive);
}

template <typename Archive>
Archive& save(Archive& archive, const IgnoreExternalSaveWithVersionArchiveRefReturn& value,
              const std::uint32_t version) {
  return value.CommonSave(archive, version);
}

template <typename Archive>
Archive& load(Archive& archive, IgnoreExternalSaveWithVersionArchiveRefReturn& value,
              const std::uint32_t version) {
  return value.CommonLoad(archive, version);
}

template <typename Archive>
void serialize(Archive& archive, UseExternalSerializeWithoutVersionVoidReturn& value) {
  value.CommonSerialise(archive);
}

template <typename Archive>
void serialize(Archive& archive, UseExternalSerializeWithVersionVoidReturn& value,
               const std::uint32_t version) {
  value.CommonSerialise(archive, version);
}

template <typename Archive>
Archive& serialize(Archive& archive, UseExternalSerializeWithoutVersionArchiveRefReturn& value) {
  return value.CommonSerialise(archive);
}

template <typename Archive>
Archive& serialize(Archive& archive, UseExternalSerializeWithVersionArchiveRefReturn& value,
                   const std::uint32_t version) {
  return value.CommonSerialise(archive, version);
}

template <typename Archive>
void save(Archive& archive, const UseExternalSaveWithoutVersionVoidReturn& value) {
  value.CommonSave(archive);
}

template <typename Archive>
void load(Archive& archive, UseExternalSaveWithoutVersionVoidReturn& value) {
  value.CommonLoad(archive);
}

template <typename Archive>
void save(Archive& archive, const UseExternalSaveWithVersionVoidReturn& value,
          const std::uint32_t version) {
  value.CommonSave(archive, version);
}

template <typename Archive>
void load(Archive& archive, UseExternalSaveWithVersionVoidReturn& value,
          const std::uint32_t version) {
  value.CommonLoad(archive, version);
}

template <typename Archive>
Archive& save(Archive& archive, const UseExternalSaveWithoutVersionArchiveRefReturn& value) {
  return value.CommonSave(archive);
}

template <typename Archive>
Archive& load(Archive& archive, UseExternalSaveWithoutVersionArchiveRefReturn& value) {
  return value.CommonLoad(archive);
}

template <typename Archive>
Archive& save(Archive& archive, const UseExternalSaveWithVersionArchiveRefReturn& value,
              const std::uint32_t version) {
  return value.CommonSave(archive, version);
}

template <typename Archive>
Archive& load(Archive& archive, UseExternalSaveWithVersionArchiveRefReturn& value,
              const std::uint32_t version) {
  return value.CommonLoad(archive, version);
}

template <typename HashAlgorithm>
void HashAppend(HashAlgorithm& hash, const IgnoreExternalSerializeWithoutVersionVoidReturn& value) {
  hash(value.one, value.two, value.three);
}

template <typename HashAlgorithm>
void HashAppend(HashAlgorithm& hash, const IgnoreExternalSerializeWithVersionVoidReturn& value) {
  hash(value.one, value.two, value.three);
}

template <typename HashAlgorithm>
void HashAppend(HashAlgorithm& hash,
                const IgnoreExternalSerializeWithoutVersionArchiveRefReturn& value) {
  hash(value.one, value.two, value.three);
}

template <typename HashAlgorithm>
void HashAppend(HashAlgorithm& hash,
                const IgnoreExternalSerializeWithVersionArchiveRefReturn& value) {
  hash(value.one, value.two, value.three);
}

template <typename HashAlgorithm>
void HashAppend(HashAlgorithm& hash, const IgnoreExternalSaveWithoutVersionVoidReturn& value) {
  hash(value.one, value.two, value.three);
}

template <typename HashAlgorithm>
void HashAppend(HashAlgorithm& hash, const IgnoreExternalSaveWithVersionVoidReturn& value) {
  hash(value.one, value.two, value.three);
}

template <typename HashAlgorithm>
void HashAppend(HashAlgorithm& hash,
                const IgnoreExternalSaveWithoutVersionArchiveRefReturn& value) {
  hash(value.one, value.two, value.three);
}

template <typename HashAlgorithm>
void HashAppend(HashAlgorithm& hash, const IgnoreExternalSaveWithVersionArchiveRefReturn& value) {
  hash(value.one, value.two, value.three);
}



struct MacroTestClass {
  int one, two, three;
  enum { expected_serialize_call_count = 1 };
};

template <typename Archive>
void serialize(Archive& archive, MacroTestClass& value, const std::uint32_t version) {
  ++serialize_call_count;
  EXPECT_EQ(10u, version);
  archive(value.one, value.two, value.three);
}



using CustomTypes = testing::Types<
    IgnoreInternalSerializeWithoutVersionVoidReturn, IgnoreInternalSerializeWithVersionVoidReturn,
    IgnoreInternalSerializeWithoutVersionArchiveRefReturn,
    IgnoreInternalSerializeWithVersionArchiveRefReturn, IgnoreInternalSaveWithoutVersionVoidReturn,
    IgnoreInternalSaveWithVersionVoidReturn, IgnoreInternalSaveWithoutVersionArchiveRefReturn,
    IgnoreInternalSaveWithVersionArchiveRefReturn, UseInternalSerializeWithoutVersionVoidReturn,
    UseInternalSerializeWithVersionVoidReturn, UseInternalSerializeWithoutVersionArchiveRefReturn,
    UseInternalSerializeWithVersionArchiveRefReturn, UseInternalSaveWithoutVersionVoidReturn,
    UseInternalSaveWithVersionVoidReturn, UseInternalSaveWithoutVersionArchiveRefReturn,
    UseInternalSaveWithVersionArchiveRefReturn, IgnoreExternalSerializeWithoutVersionVoidReturn,
    IgnoreExternalSerializeWithVersionVoidReturn,
    IgnoreExternalSerializeWithoutVersionArchiveRefReturn,
    IgnoreExternalSerializeWithVersionArchiveRefReturn, IgnoreExternalSaveWithoutVersionVoidReturn,
    IgnoreExternalSaveWithVersionVoidReturn, IgnoreExternalSaveWithoutVersionArchiveRefReturn,
    IgnoreExternalSaveWithVersionArchiveRefReturn, UseExternalSerializeWithoutVersionVoidReturn,
    UseExternalSerializeWithVersionVoidReturn, UseExternalSerializeWithoutVersionArchiveRefReturn,
    UseExternalSerializeWithVersionArchiveRefReturn, UseExternalSaveWithoutVersionVoidReturn,
    UseExternalSaveWithVersionVoidReturn, UseExternalSaveWithoutVersionArchiveRefReturn,
    UseExternalSaveWithVersionArchiveRefReturn, MacroTestClass>;

}  // namespace

MAIDSAFE_HASH_AND_CEREAL_CLASS_VERSION(MacroTestClass, 10)

namespace maidsafe {

template <>
struct UseSerializeForHashing<MacroTestClass> : std::true_type {};

namespace test {

TEST(HashTest, BEH_NumericRange) {
  const maidsafe::SeededHash<maidsafe::SipHash> hash{};
  const int data[] = {10, 20, 30};
  const std::uint64_t reference = hash(data);

  EXPECT_EQ(reference, hash(10, 20, 30, std::size_t(3)));
  EXPECT_EQ(reference, hash(std::array<int, 3>({{10, 20, 30}})));
  EXPECT_EQ(reference,
            hash(std::map<int, int>({{1, 10}, {2, 20}, {3, 30}}) | boost::adaptors::map_values));
  EXPECT_EQ(reference, hash(std::forward_list<int>({10, 20, 30})));
  EXPECT_EQ(reference, hash(std::list<int>({10, 20, 30})));
  EXPECT_EQ(reference, hash(std::initializer_list<int>({10, 20, 30})));
  EXPECT_EQ(reference, hash(std::set<int>({10, 20, 30})));
  EXPECT_EQ(reference, hash(std::vector<int>({10, 20, 30})));
}

TEST(HashTest, BEH_PairVerification) {
  {
    maidsafe::SipHash hash1{{{}}};
    maidsafe::SipHash hash2{{{}}};

    hash1(100, 1000);
    hash2(std::make_pair(100, 1000));

    EXPECT_EQ(hash1.Finalize(), hash2.Finalize());
  }
  {
    maidsafe::SipHash hash1{{{}}};
    maidsafe::SipHash hash2{{{}}};

    hash1(boost::string_ref("<-->"), 100.5);
    hash2(std::make_pair(boost::string_ref("<-->"), 100.5));

    EXPECT_EQ(hash1.Finalize(), hash2.Finalize());
  }
}

TEST(HashTest, BEH_PairRange) {
  const maidsafe::SeededHash<maidsafe::SipHash> hash{};
  const std::initializer_list<std::pair<int, int>> data = {{3, 10}, {50, 20}, {1000, 30}};
  const std::uint64_t reference = hash(data);

  EXPECT_EQ(reference, hash(std::forward_list<std::pair<int, int>>(data)));
  EXPECT_EQ(reference, hash(std::list<std::pair<int, int>>(data)));
  EXPECT_EQ(reference, hash(std::map<int, int>({{3, 10}, {50, 20}, {1000, 30}})));
  EXPECT_EQ(reference, hash(std::set<std::pair<int, int>>(data)));
  EXPECT_EQ(reference, hash(std::vector<std::pair<int, int>>(data)));

  EXPECT_EQ(reference,
            hash(std::forward_list<std::tuple<int, int>>(
                {std::make_tuple(3, 10), std::make_tuple(50, 20), std::make_tuple(1000, 30)})));
}

TEST(HashTest, BEH_TupleVerification) {
  {
    const maidsafe::SipHash hash1{{{}}};
    maidsafe::SipHash hash2{{{}}};
    hash2(std::tuple<>{});
    EXPECT_EQ(hash1.Finalize(), hash2.Finalize());
  }
  {
    maidsafe::SipHash hash1{{{}}};
    maidsafe::SipHash hash2{{{}}};

    hash1(100, 1000, 10000);
    hash2(std::make_tuple(100, 1000, 10000));

    EXPECT_EQ(hash1.Finalize(), hash2.Finalize());
  }
  {
    maidsafe::SipHash hash1{{{}}};
    maidsafe::SipHash hash2{{{}}};

    hash1(boost::string_ref("<-->"), 100.5, 90000);
    hash2(std::make_tuple(boost::string_ref("<-->"), 100.5, 90000));

    EXPECT_EQ(hash1.Finalize(), hash2.Finalize());
  }
}

TEST(HashTest, BEH_TupleRange) {
  const maidsafe::SeededHash<maidsafe::SipHash> hash{};
  const std::initializer_list<std::tuple<int, int, int>> data{
      std::make_tuple(3, 10, 1000), std::make_tuple(50, 20, 122), std::make_tuple(1000, 30, 33)};
  const std::uint64_t reference = hash(data);

  EXPECT_EQ(reference, hash(std::forward_list<std::tuple<int, int, int>>(data)));
  EXPECT_EQ(reference, hash(std::list<std::tuple<int, int, int>>(data)));
  EXPECT_EQ(reference, hash(std::vector<std::tuple<int, int, int>>(data)));
}

TEST(HashTest, BEH_FloatRange) {
  const maidsafe::SeededHash<maidsafe::SipHash> hash{};
  const std::initializer_list<std::pair<int, float>> data = {{3, 10.4f}, {50, -0.f}, {1000, 30.2f}};
  const std::uint64_t reference = hash(data);

  EXPECT_EQ(reference, hash(std::forward_list<std::pair<int, float>>(data)));
  EXPECT_EQ(reference, hash(std::list<std::pair<int, float>>(data)));
  EXPECT_EQ(reference, hash(std::map<int, float>({{3, 10.4f}, {50, -0.f}, {1000, 30.2f}})));
  EXPECT_EQ(reference, hash(std::set<std::pair<int, float>>(data)));
  EXPECT_EQ(reference, hash(std::vector<std::pair<int, float>>(data)));

  // Retry with positive 0
  EXPECT_EQ(reference,
            hash(std::vector<std::pair<int, float>>({{3, 10.4f}, {50, 0.f}, {1000, 30.2f}})));
}

TEST(HashTest, BEH_DoubleRange) {
  const maidsafe::SeededHash<maidsafe::SipHash> hash{};
  const std::initializer_list<std::pair<int, double>> data = {{3, 10.4}, {50, -0.f}, {1000, 30.2}};
  const std::uint64_t reference = hash(data);

  EXPECT_EQ(reference, hash(std::forward_list<std::pair<int, double>>(data)));
  EXPECT_EQ(reference, hash(std::list<std::pair<int, double>>(data)));
  EXPECT_EQ(reference, hash(std::map<int, double>({{3, 10.4}, {50, -0.f}, {1000, 30.2}})));
  EXPECT_EQ(reference, hash(std::set<std::pair<int, double>>(data)));
  EXPECT_EQ(reference, hash(std::vector<std::pair<int, double>>(data)));

  // Retry with positive 0
  EXPECT_EQ(reference,
            hash(std::vector<std::pair<int, double>>({{3, 10.4}, {50, 0.f}, {1000, 30.2}})));
}

TEST(HashTest, BEH_StringRange) {
  const maidsafe::SeededHash<maidsafe::SipHash> hash{};
  const std::initializer_list<std::string> data = {"string1", "string2", "string3"};
  const std::uint64_t reference = hash(data);

  EXPECT_EQ(reference,
            hash(std::map<std::string, int>({{"string1", 10}, {"string2", 20}, {"string3", 30}}) |
                 boost::adaptors::map_keys));
  EXPECT_EQ(reference, hash(std::forward_list<std::string>(data)));
  EXPECT_EQ(reference, hash(std::list<std::string>(data)));
  EXPECT_EQ(reference, hash(std::set<std::string>(data)));
  EXPECT_EQ(reference, hash(std::vector<std::string>(data)));

  std::vector<boost::string_ref> ref_data = {"string1-", "string2", "string3"};
  EXPECT_NE(reference, hash(ref_data));
  ref_data[0].remove_suffix(1);
  EXPECT_EQ(reference, hash(ref_data));
}

template <typename T>
class TypedHashTest : public testing::Test {};

TYPED_TEST_CASE(TypedHashTest, CustomTypes);

TYPED_TEST(TypedHashTest, BEH_PreferHashAppend) {
  const maidsafe::SeededHash<maidsafe::SipHash> hash{};
  serialize_call_count = 0;
  const std::uint64_t expected_result{hash(10, 20, 30)};
  TypeParam custom{10, 20, 30};
  EXPECT_EQ(expected_result, hash(custom));
  EXPECT_EQ(custom.expected_serialize_call_count, serialize_call_count);
  serialize_call_count = 0;
  EXPECT_EQ(expected_result, hash(std::move(custom)));
  EXPECT_EQ(custom.expected_serialize_call_count, serialize_call_count);
}

TYPED_TEST(TypedHashTest, BEH_CerealAndHashing) {
  const maidsafe::SeededHash<maidsafe::SipHash> hash{};
  TypeParam original = {-500, 5000, 50000};
  const std::uint64_t original_hash = hash(original);

  SerialisedData serialised{Serialise(original)};
  TypeParam copy{0, 0, 0};
  EXPECT_NE(original_hash, hash(copy));
  Parse(serialised, copy);
  EXPECT_EQ(original_hash, hash(copy));
}

TEST(HashTest, BEH_InHashMap) {
  std::unordered_map<std::pair<std::string, std::string>, int,
                     maidsafe::SeededHash<maidsafe::SipHash>> hash_table;

  hash_table[{"entry", "one"}] = -1;
  hash_table[{"entry", "two"}] = 2;
  hash_table[{"entri", "one"}] = 3;

  const auto value1 = hash_table[{"entry", "one"}];
  const auto value2 = hash_table[{"entry", "two"}];
  const auto value3 = hash_table[{"entri", "one"}];

  EXPECT_EQ(-1, value1);
  EXPECT_EQ(2, value2);
  EXPECT_EQ(3, value3);
}

}  // namespace test

}  // namespace maidsafe
